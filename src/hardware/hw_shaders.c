// SONIC ROBO BLAST 2
//-----------------------------------------------------------------------------
// Copyright (C) 2021 by Sonic Team Junior.
//
// This program is free software distributed under the
// terms of the GNU General Public License, version 2.
// See the 'LICENSE' file for more details.
//-----------------------------------------------------------------------------
/// \file hw_shaders.h
/// \brief Handles the shaders used by the game.

#ifdef HWRENDER

#include "hw_glob.h"
#include "hw_drv.h"
#include "../z_zone.h"
#include "hw_shaders.h"

// ================
//  Shader sources
// ================

static struct {
	const char *vertex;
	const char *fragment;
} const gl_shadersources[] = {
	// Floor shader
	{GLSL_DEFAULT_VERTEX_SHADER, GLSL_SOFTWARE_FRAGMENT_SHADER_FLOORS},

	// Wall shader
	{GLSL_DEFAULT_VERTEX_SHADER, GLSL_SOFTWARE_FRAGMENT_SHADER_WALLS},

	// Sprite shader
	{GLSL_DEFAULT_VERTEX_SHADER, GLSL_SOFTWARE_FRAGMENT_SHADER_WALLS},

	// Model shader
	{GLSL_MODEL_VERTEX_SHADER, GLSL_SOFTWARE_MODEL_FRAGMENT_SHADER},

	// Water shader
	{GLSL_DEFAULT_VERTEX_SHADER, GLSL_WATER_FRAGMENT_SHADER},

	// Fog shader
	{GLSL_DEFAULT_VERTEX_SHADER, GLSL_FOG_FRAGMENT_SHADER},

	// Sky shader
	{GLSL_DEFAULT_VERTEX_SHADER, GLSL_SKY_FRAGMENT_SHADER},

	// Palette postprocess shader
	{GLSL_DEFAULT_VERTEX_SHADER, GLSL_PALETTE_POSTPROCESS_SHADER},

	{NULL, NULL},
};

typedef struct
{
	int base_shader; // index of base shader_t
	int custom_shader; // index of custom shader_t
} shadertarget_t;

typedef struct
{
	char *vertex;
	char *fragment;
	boolean compiled;
} shader_t; // these are in an array and accessed by indices

// the array has NUMSHADERTARGETS entries for base shaders and for custom shaders
// the array could be expanded in the future to fit "dynamic" custom shaders that
// aren't fixed to shader targets
static shader_t gl_shaders[NUMSHADERTARGETS*2];

static shadertarget_t gl_shadertargets[NUMSHADERTARGETS];

#define MODEL_LIGHTING_DEFINE "#define SRB2_MODEL_LIGHTING"
#define PALETTE_RENDERING_DEFINE "#define SRB2_PALETTE_RENDERING"

// Initialize shader variables and the backend's shader system. Load the base shaders.
// Returns false if shaders cannot be used.
boolean HWR_InitShaders(void)
{
	int i;

	if (!HWD.pfnInitShaders())
		return false;

	for (i = 0; i < NUMSHADERTARGETS; i++)
	{
		// set up string pointers for base shaders
		gl_shaders[i].vertex = Z_StrDup(gl_shadersources[i].vertex);
		gl_shaders[i].fragment = Z_StrDup(gl_shadersources[i].fragment);
		// set shader target indices to correct values
		gl_shadertargets[i].base_shader = i;
		gl_shadertargets[i].custom_shader = -1;
	}

	HWR_CompileShaders();

	return true;
}

// helper function: strstr but returns an int with the substring position
// returns INT32_MAX if not found
static INT32 strstr_int(const char *str1, const char *str2)
{
	char *location = strstr(str1, str2);
	if (location)
		return location - str1;
	else
		return INT32_MAX;
}

// Creates a preprocessed copy of the shader according to the current graphics settings
// Returns a pointer to the results on success and NULL on failure.
// Remember memory management of the returned string.
static char *HWR_PreprocessShader(char *original)
{
	const char *line_ending = "\n";
	int line_ending_len;
	char *read_pos = original;
	int insertion_pos = 0;
	int original_len = strlen(original);
	int distance_to_end = original_len;
	int new_len;
	char *new_shader;
	char *write_pos;

	if (strstr(original, "\r\n"))
	{
		line_ending = "\r\n";
		// check that all line endings are same,
		// otherwise the parsing code won't function correctly
		while ((read_pos = strchr(read_pos, '\n')))
		{
			read_pos--;
			if (*read_pos != '\r')
			{
				CONS_Alert(CONS_ERROR, "HWR_PreprocessShader: Shader contains mixed line ending types. Please use either only LF (Unix) or only CRLF (Windows) line endings.\n");
				return NULL;
			}
			read_pos += 2;
		}
		read_pos = original;
	}

	line_ending_len = strlen(line_ending);

	// We need to find a place to put the #define commands.
	// To stay within GLSL specs, they must be *after* the #version define,
	// if there is any. So we need to look for that. And also let's not
	// get fooled if there is a #version inside a comment!
	// Time for some string parsing :D

#define STARTSWITH(str, with_what) !strncmp(str, with_what, sizeof(with_what)-1)
#define ADVANCE(amount) read_pos += amount; distance_to_end -= amount;
	while (true)
	{
		// we're at the start of a line or at the end of a block comment.
		// first get any possible whitespace out of the way
		int whitespace_len = strspn(read_pos, " \t");
		if (whitespace_len == distance_to_end)
			break; // we got to the end
		ADVANCE(whitespace_len)

		if (STARTSWITH(read_pos, "#version"))
		{
			// getting closer
			INT32 newline_pos = strstr_int(read_pos, line_ending);
			INT32 line_comment_pos = strstr_int(read_pos, "//");
			INT32 block_comment_pos = strstr_int(read_pos, "/*");
			if (newline_pos == INT32_MAX && line_comment_pos == INT32_MAX &&
				block_comment_pos == INT32_MAX)
			{
				// #version is at the end of the file. Probably not a valid shader.
				CONS_Alert(CONS_ERROR, "HWR_PreprocessShader: Shader unexpectedly ends after #version.\n");
				return NULL;
			}
			else
			{
				// insert at the earliest occurence of newline or comment after #version
				insertion_pos = min(line_comment_pos, block_comment_pos);
				insertion_pos = min(newline_pos, insertion_pos);
				insertion_pos += read_pos - original;
				break;
			}
		}
		else
		{
			// go to next newline or end of next block comment if it starts before the newline
			// and is not inside a line comment
			INT32 newline_pos = strstr_int(read_pos, line_ending);
			INT32 line_comment_pos;
			INT32 block_comment_pos;
			// optimization: temporarily put a null at the line ending, so strstr does not needlessly
			// look past it since we're only interested in the current line
			if (newline_pos != INT32_MAX)
				read_pos[newline_pos] = '\0';
			line_comment_pos = strstr_int(read_pos, "//");
			block_comment_pos = strstr_int(read_pos, "/*");
			// restore the line ending, remove the null we just put there
			if (newline_pos != INT32_MAX)
				read_pos[newline_pos] = line_ending[0];
			if (line_comment_pos < block_comment_pos)
			{
				// line comment found, skip rest of the line
				if (newline_pos != INT32_MAX)
				{
					ADVANCE(newline_pos + line_ending_len)
				}
				else
				{
					// we got to the end
					break;
				}
			}
			else if (block_comment_pos < line_comment_pos)
			{
				// block comment found, skip past it
				INT32 block_comment_end;
				ADVANCE(block_comment_pos + 2)
				block_comment_end = strstr_int(read_pos, "*/");
				if (block_comment_end == INT32_MAX)
				{
					// could also leave insertion_pos at 0 and let the GLSL compiler
					// output an error message for this broken comment
					CONS_Alert(CONS_ERROR, "HWR_PreprocessShader: Encountered unclosed block comment in shader.\n");
					return NULL;
				}
				ADVANCE(block_comment_end + 2)
			}
			else if (newline_pos == INT32_MAX)
			{
				// we got to the end
				break;
			}
			else
			{
				// nothing special on this line, move to the next one
				ADVANCE(newline_pos + line_ending_len)
			}
		}
	}
#undef STARTSWITH
#undef ADVANCE

	// Calculate length of modified shader.
	new_len = original_len;
	if (cv_grmodellighting.value)
		new_len += sizeof(MODEL_LIGHTING_DEFINE) - 1 + 2 * line_ending_len;
	if (cv_grpaletterendering.value)
		new_len += sizeof(PALETTE_RENDERING_DEFINE) - 1 + 2 * line_ending_len;

	// Allocate memory for modified shader.
	new_shader = Z_Malloc(new_len + 1, PU_STATIC, NULL);

	read_pos = original;
	write_pos = new_shader;

	// Copy the part before our additions.
	M_Memcpy(write_pos, original, insertion_pos);
	read_pos += insertion_pos;
	write_pos += insertion_pos;

#define WRITE_DEFINE(define) \
	{ \
		strcpy(write_pos, line_ending); \
		write_pos += line_ending_len; \
		strcpy(write_pos, define); \
		write_pos += sizeof(define) - 1; \
		strcpy(write_pos, line_ending); \
		write_pos += line_ending_len; \
	}

	// Write the additions.
	if (cv_grmodellighting.value)
		WRITE_DEFINE(MODEL_LIGHTING_DEFINE)
	if (cv_grpaletterendering.value)
		WRITE_DEFINE(PALETTE_RENDERING_DEFINE)

#undef WRITE_DEFINE

	// Copy the part after our additions.
	M_Memcpy(write_pos, read_pos, original_len - insertion_pos);

	// Terminate the new string.
	new_shader[new_len] = '\0';

	return new_shader;
}

// preprocess and compile shader at gl_shaders[index]
static void HWR_CompileShader(int index)
{
	char *vertex_source = gl_shaders[index].vertex;
	char *fragment_source = gl_shaders[index].fragment;

	if (vertex_source)
	{
		char *preprocessed = HWR_PreprocessShader(vertex_source);
		if (!preprocessed) return;
		HWD.pfnLoadShader(index, preprocessed, HWD_SHADERSTAGE_VERTEX);
	}
	if (fragment_source)
	{
		char *preprocessed = HWR_PreprocessShader(fragment_source);
		if (!preprocessed) return;
		HWD.pfnLoadShader(index, preprocessed, HWD_SHADERSTAGE_FRAGMENT);
	}

	gl_shaders[index].compiled = HWD.pfnCompileShader(index);
}

// compile or recompile shaders
void HWR_CompileShaders(void)
{
	int i;

	for (i = 0; i < NUMSHADERTARGETS; i++)
	{
		int custom_index = gl_shadertargets[i].custom_shader;
		HWR_CompileShader(i);
		if (!gl_shaders[i].compiled)
			CONS_Alert(CONS_ERROR, "HWR_CompileShaders: Compilation failed for base %s shader!\n", shaderxlat[i].type);
		if (custom_index != -1)
		{
			HWR_CompileShader(custom_index);
			if (!gl_shaders[custom_index].compiled)
				CONS_Alert(CONS_ERROR, "HWR_CompileShaders: Recompilation failed for the custom %s shader! See the console messages above for more information.\n", shaderxlat[i].type);
		}
	}
}

int HWR_GetShaderFromTarget(int shader_target)
{
	int custom_shader = gl_shadertargets[shader_target].custom_shader;
	// use custom shader if following are true
	// - custom shader exists
	// - custom shader has been compiled successfully
	// - custom shaders are enabled
	// - custom shaders are allowed by the server
	if (custom_shader != -1 && gl_shaders[custom_shader].compiled &&
		cv_grshaders.value == 1 && cv_grallowshaders.value)
		return custom_shader;
	else
		return gl_shadertargets[shader_target].base_shader;
}

static inline UINT16 HWR_FindShaderDefs(UINT16 wadnum)
{
	UINT16 i;
	lumpinfo_t *lump_p;

	lump_p = wadfiles[wadnum]->lumpinfo;
	for (i = 0; i < wadfiles[wadnum]->numlumps; i++, lump_p++)
		if (memcmp(lump_p->name, "SHADERS", 7) == 0)
			return i;

	return INT16_MAX;
}

customshaderxlat_t shaderxlat[] =
{
	{"Flat", SHADER_FLOOR},
	{"WallTexture", SHADER_WALL},
	{"Sprite", SHADER_SPRITE},
	{"Model", SHADER_MODEL},
	{"WaterRipple", SHADER_WATER},
	{"Fog", SHADER_FOG},
	{"Sky", SHADER_SKY},
	{"PalettePostprocess", SHADER_PALETTE_POSTPROCESS},
	{NULL, 0},
};

void HWR_LoadAllCustomShaders(void)
{
	INT32 i;

	// read every custom shader
	for (i = 0; i < numwadfiles; i++)
		HWR_LoadCustomShadersFromFile(i, (wadfiles[i]->type == RET_PK3));
}

void HWR_LoadCustomShadersFromFile(UINT16 wadnum, boolean PK3)
{
	UINT16 lump;
	char *shaderdef, *line;
	char *stoken;
	char *value;
	size_t size;
	int linenum = 1;
	int shadertype = 0;
	int i;
	boolean modified_shaders[NUMSHADERTARGETS] = {0};

	if (!gr_shadersavailable)
		return;

	lump = HWR_FindShaderDefs(wadnum);
	if (lump == INT16_MAX)
		return;

	shaderdef = W_CacheLumpNumPwad(wadnum, lump, PU_CACHE);
	size = W_LumpLengthPwad(wadnum, lump);

	line = Z_Malloc(size+1, PU_STATIC, NULL);
	M_Memcpy(line, shaderdef, size);
	line[size] = '\0';

	stoken = strtok(line, "\r\n ");
	while (stoken)
	{
		if ((stoken[0] == '/' && stoken[1] == '/')
			|| (stoken[0] == '#'))// skip comments
		{
			stoken = strtok(NULL, "\r\n");
			goto skip_field;
		}

		if (!stricmp(stoken, "GLSL"))
		{
			value = strtok(NULL, "\r\n ");
			if (!value)
			{
				CONS_Alert(CONS_WARNING, "HWR_LoadCustomShadersFromFile: Missing shader type (file %s, line %d)\n", wadfiles[wadnum]->filename, linenum);
				stoken = strtok(NULL, "\r\n"); // skip end of line
				goto skip_lump;
			}

			if (!stricmp(value, "VERTEX"))
				shadertype = 1;
			else if (!stricmp(value, "FRAGMENT"))
				shadertype = 2;

skip_lump:
			stoken = strtok(NULL, "\r\n ");
			linenum++;
		}
		else
		{
			value = strtok(NULL, "\r\n= ");
			if (!value)
			{
				CONS_Alert(CONS_WARNING, "HWR_LoadCustomShadersFromFile: Missing shader target (file %s, line %d)\n", wadfiles[wadnum]->filename, linenum);
				stoken = strtok(NULL, "\r\n"); // skip end of line
				goto skip_field;
			}

			if (!shadertype)
			{
				CONS_Alert(CONS_ERROR, "HWR_LoadCustomShadersFromFile: Missing shader type (file %s, line %d)\n", wadfiles[wadnum]->filename, linenum);
				Z_Free(line);
				return;
			}

			for (i = 0; shaderxlat[i].type; i++)
			{
				if (!stricmp(shaderxlat[i].type, stoken))
				{
					size_t shader_string_length;
					char *shader_source;
					char *shader_lumpname;
					UINT16 shader_lumpnum;
					int shader_index; // index in gl_shaders

					if (PK3)
					{
						shader_lumpname = Z_Malloc(strlen(value) + 12, PU_STATIC, NULL);
						strcpy(shader_lumpname, "Shaders/sh_");
						strcat(shader_lumpname, value);
						shader_lumpnum = W_CheckNumForFullNamePK3(shader_lumpname, wadnum, 0);
					}
					else
					{
						shader_lumpname = Z_Malloc(strlen(value) + 4, PU_STATIC, NULL);
						strcpy(shader_lumpname, "SH_");
						strcat(shader_lumpname, value);
						shader_lumpnum = W_CheckNumForNamePwad(shader_lumpname, wadnum, 0);
					}

					if (shader_lumpnum == INT16_MAX)
					{
						CONS_Alert(CONS_ERROR, "HWR_LoadCustomShadersFromFile: Missing shader source %s (file %s, line %d)\n", shader_lumpname, wadfiles[wadnum]->filename, linenum);
						Z_Free(shader_lumpname);
						continue;
					}

					shader_string_length = W_LumpLengthPwad(wadnum, shader_lumpnum) + 1;
					shader_source = Z_Malloc(shader_string_length, PU_STATIC, NULL);
					W_ReadLumpPwad(wadnum, shader_lumpnum, shader_source);
					shader_source[shader_string_length-1] = '\0';

					shader_index = shaderxlat[i].id + NUMSHADERTARGETS;
					if (!modified_shaders[shaderxlat[i].id])
					{
						// this will clear any old custom shaders from previously loaded files
						// Z_Free checks if the pointer is NULL!
						Z_Free(gl_shaders[shader_index].vertex);
						gl_shaders[shader_index].vertex = NULL;
						Z_Free(gl_shaders[shader_index].fragment);
						gl_shaders[shader_index].fragment = NULL;
					}
					modified_shaders[shaderxlat[i].id] = true;

					if (shadertype == 1)
					{
						if (gl_shaders[shader_index].vertex)
						{
							CONS_Alert(CONS_WARNING, "HWR_LoadCustomShadersFromFile: %s is overwriting another %s vertex shader from the same addon! (file %s, line %d)\n", shader_lumpname, shaderxlat[i].type, wadfiles[wadnum]->filename, linenum);
							Z_Free(gl_shaders[shader_index].vertex);
						}
						gl_shaders[shader_index].vertex = shader_source;
					}
					else
					{
						if (gl_shaders[shader_index].fragment)
						{
							CONS_Alert(CONS_WARNING, "HWR_LoadCustomShadersFromFile: %s is overwriting another %s fragment shader from the same addon! (file %s, line %d)\n", shader_lumpname, shaderxlat[i].type, wadfiles[wadnum]->filename, linenum);
							Z_Free(gl_shaders[shader_index].fragment);
						}
						gl_shaders[shader_index].fragment = shader_source;
					}

					Z_Free(shader_lumpname);
				}
			}

skip_field:
			stoken = strtok(NULL, "\r\n= ");
			linenum++;
		}
	}

	for (i = 0; i < NUMSHADERTARGETS; i++)
	{
		if (modified_shaders[i])
		{
			int shader_index = i + NUMSHADERTARGETS; // index to gl_shaders
			gl_shadertargets[i].custom_shader = shader_index;
			HWR_CompileShader(shader_index);
			if (!gl_shaders[shader_index].compiled)
				CONS_Alert(CONS_ERROR, "HWR_LoadCustomShadersFromFile: A compilation error occured for the %s shader in file %s. See the console messages above for more information.\n", shaderxlat[i].type, wadfiles[wadnum]->filename);
		}
	}

	Z_Free(line);
	return;
}

const char *HWR_GetShaderName(INT32 shader)
{
	INT32 i;

	for (i = 0; shaderxlat[i].type; i++)
	{
		if (shaderxlat[i].id == shader)
			return shaderxlat[i].type;
	}

	return "Unknown";
}

#endif // HWRENDER
