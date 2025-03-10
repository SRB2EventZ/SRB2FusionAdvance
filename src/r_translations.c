// SONIC ROBO BLAST 2
//-----------------------------------------------------------------------------
// Copyright (C) 1993-1996 by id Software, Inc.
// Copyright (C) 1998-2000 by DooM Legacy Team.
// Copyright (C) 1999-2018 by Sonic Team Junior.
//
// This program is free software distributed under the
// terms of the GNU General Public License, version 2.
// See the 'LICENSE' file for more details.
//-----------------------------------------------------------------------------
/// \file  r_translation.c
/// \brief Translation table handling

#include "doomdef.h"
#include "r_draw.h"
#include "r_local.h"

#define SKIN_RAMP_LENGTH 16
#define DEFAULT_STARTTRANSCOLOR 160
#define NUM_PALETTE_ENTRIES 256

const char *Color_Names[MAXSKINCOLORS] =
{
	"None",      // SKINCOLOR_NONE
	"White",     // SKINCOLOR_WHITE
	"Silver",    // SKINCOLOR_SILVER
	"Grey",      // SKINCOLOR_GREY
	"Black",     // SKINCOLOR_BLACK
	"Cyan",      // SKINCOLOR_CYAN
	"Teal",      // SKINCOLOR_TEAL
	"Steel_Blue",    // SKINCOLOR_STEELBLUE
	"Blue",      // SKINCOLOR_BLUE
	"Peacy",     // SKINCOLOR_PEACH
	"Tan",       // SKINCOLOR_TAN
	"Pink",      // SKINCOLOR_PINK
	"Lavender",  // SKINCOLOR_LAVENDER
	"Purple",    // SKINCOLOR_PURPLE
	"Orange",    // SKINCOLOR_ORANGE
	"Rosewood",  // SKINCOLOR_ROSEWOOD
	"Beige",     // SKINCOLOR_BEIGE
	"Brown",     // SKINCOLOR_BROWN
	"Red",       // SKINCOLOR_RED
	"Dark_Red",  // SKINCOLOR_DARKRED
	"Neon_Green",      // SKINCOLOR_NEONGREEN
	"Green",     // SKINCOLOR_GREEN
	"Zim",       // SKINCOLOR_ZIM
	"Olive",     // SKINCOLOR_OLIVE
	"Yellow",    // SKINCOLOR_YELLOW
	"Gold"       // SKINCOLOR_GOLD
};


const UINT8 Color_Opposite[MAXSKINCOLORS*2] =
{
	SKINCOLOR_NONE,8,   // SKINCOLOR_NONE
	SKINCOLOR_BLACK,10, // SKINCOLOR_WHITE
	SKINCOLOR_GREY,4,   // SKINCOLOR_SILVER
	SKINCOLOR_SILVER,12,// SKINCOLOR_GREY
	SKINCOLOR_WHITE,8,  // SKINCOLOR_BLACK
	SKINCOLOR_NONE,8,   // SKINCOLOR_CYAN
	SKINCOLOR_NONE,8,   // SKINCOLOR_TEAL
	SKINCOLOR_NONE,8,   // SKINCOLOR_STEELBLUE
	SKINCOLOR_ORANGE,9, // SKINCOLOR_BLUE
	SKINCOLOR_NONE,8,   // SKINCOLOR_PEACH
	SKINCOLOR_NONE,8,   // SKINCOLOR_TAN
	SKINCOLOR_NONE,8,   // SKINCOLOR_PINK
	SKINCOLOR_NONE,8,   // SKINCOLOR_LAVENDER
	SKINCOLOR_NONE,8,   // SKINCOLOR_PURPLE
	SKINCOLOR_BLUE,12,  // SKINCOLOR_ORANGE
	SKINCOLOR_NONE,8,   // SKINCOLOR_ROSEWOOD
	SKINCOLOR_NONE,8,   // SKINCOLOR_BEIGE
	SKINCOLOR_NONE,8,   // SKINCOLOR_BROWN
	SKINCOLOR_GREEN,5,  // SKINCOLOR_RED
	SKINCOLOR_NONE,8,   // SKINCOLOR_DARKRED
	SKINCOLOR_NONE,8,   // SKINCOLOR_NEONGREEN
	SKINCOLOR_RED,11,   // SKINCOLOR_GREEN
	SKINCOLOR_PURPLE,3, // SKINCOLOR_ZIM
	SKINCOLOR_NONE,8,   // SKINCOLOR_OLIVE
	SKINCOLOR_NONE,8,   // SKINCOLOR_YELLOW
	SKINCOLOR_NONE,8    // SKINCOLOR_GOLD
};

UINT8 colortranslations[MAXTRANSLATIONS][16] = {
	{  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0}, // SKINCOLOR_NONE
        {  0,   0,   1,   1,   2,   2,   3,   3,   4,   4,   5,   5,   6,   6,   7,   7}, // SKINCOLOR_WHITE
        {  3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15,  16,  17,  18}, // SKINCOLOR_SILVER
        {  8,   9,  10,  11,  12,  13,  14,  15,  16,  17,  18,  19,  20,  21,  22,  23}, // SKINCOLOR_GREY
        { 24,  24,  25,  25,  26,  26,  27,  27,  28,  28,  29,  29,  30,  30,  31,  31}, // SKINCOLOR_BLACK
        {208, 208, 209, 210, 211, 211, 212, 213, 214, 214, 215, 216, 217, 217, 218, 219}, // SKINCOLOR_CYAN
        {247, 247, 247, 247, 220, 220, 220, 221, 221, 221, 222, 222, 222, 223, 223, 223}, // SKINCOLOR_TEAL
        {200, 200, 201, 201, 202, 202, 203, 203, 204, 204, 205, 205, 206, 206, 207, 207}, // SKINCOLOR_STEELBLUE
        {226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239, 240, 241}, // SKINCOLOR_BLUE
        { 64,  65,  66,  67,  68,  69,  70,  71,  72,  73,  74,  75,  76,  77,  78,  79}, // SKINCOLOR_PEACH
        { 72,  73,  74,  75,  76,  77,  78,  79,  48,  49,  50,  51,  52,  53,  54,  55}, // SKINCOLOR_TAN
        {144, 144, 145, 145, 146, 146, 147, 147,  148, 148,  149,  149, 150, 150,  151,  151}, // SKINCOLOR_PINK
        {248, 248, 249, 249, 250, 250, 251, 251,  252,  252, 253,  253, 254, 254,  255,  255}, // SKINCOLOR_LAVENDER
        {192, 192, 193, 193, 194, 194, 195, 195, 196, 196,  197, 197,  198, 198, 199, 199}, // SKINCOLOR_PURPLE
        { 82,   83,  84,  85,  86,  87,  88,  89,  90,  91,   92,  93,   94,  95,  152,  155}, // SKINCOLOR_ORANGE
        { 90,   92,  93,  94,  95,  95, 152, 153, 154, 154,  155, 156, 157,  158,  159,  141}, // SKINCOLOR_ROSEWOOD
        { 32,	33,  34,  35,  36,  37,	 38,  39,  40,  41,   42,  43,  44,   45,   46,   47}, // SKINCOLOR_BEIGE
        { 48,	49,  50,  51,  52,  53,	 54,  55,  56,  57,   58,  59,	60,   61,   62,   63}, // SKINCOLOR_BROWN
        {125, 126, 127, 128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140}, // SKINCOLOR_RED
        {133, 133, 134, 134, 135, 135, 136, 136, 137, 137, 138, 138, 139, 139, 140, 140}, // SKINCOLOR_DARKRED
        {160, 184, 184, 184, 185, 185, 186, 186, 186, 187, 187, 188, 188, 188, 189, 189}, // SKINCOLOR_NEONGREEN
        {160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175}, // SKINCOLOR_GREEN
        {176,  176,  177,  177,  178,  178, 179, 179,  180,  180,  181,  181, 182,  182,  183,  183}, // SKINCOLOR_ZIM
        {105,  105,  105,  106,  106,  107, 107, 108,  108,  108,  109,  109, 110,  110,  111,  111}, // SKINCOLOR_OLIVE
        {103,  103,  104,  104,  105,  105, 106, 106,  107,  107,  108,  108, 109,  109,  110,  110}, // SKINCOLOR_YELLOW
        {112, 112, 113, 113, 114, 114, 115, 115, 116, 116, 117, 117, 118,  118,  119,  119}, // SKINCOLOR_GOLD


        // SKINCOLOR_SUPER1 - SKINCOLOR_SUPER5 //
        {120, 120, 120, 120, 120, 120, 120, 120, 120, 120,  96,   97,   98,   99, 100,  101}, // 1
        { 96,  97,  98,  99,  100, 112, 101, 101, 102, 102, 103,  103,  104,  104, 113, 114}, // 2
        { 98,  99, 100, 112,  101, 101, 102, 102, 103, 103, 104,  104,  113,  114, 115, 116}, // 3
        {112, 101, 101, 102,  102, 103, 103, 104, 104, 113, 114,  115,  116,  117, 118, 119}, // 4
        {112, 101, 102, 103,  103, 103, 104, 104, 113, 114, 115,  116,  117,  118, 119, 157}, // 5

        // SKINCOLOR_TSUPER1 - SKINCOLOR_TSUPER5 //
        {120, 120, 120, 120,  120, 120, 120, 120, 120, 120, 80,  81,  82,  83,  84,  85}, // 1
        {120, 120, 120, 120,  80,   80,  81,  81,  82,  82, 83,  83,  84,  84,  85,  85}, // 2
        {120, 120,  80,  80,  81,   81,  82,  82,  83,  83, 84,  84,  85,  85,  86,  86}, // 3
        {120,  80,   81,  82,  83,   84,  85,  86,  87, 115, 115,  116,  117,  117, 118, 119}, // 4
        { 80,  81,  82,  83,  84,  85, 86, 87, 115, 115, 116,  116,  117,  118, 118, 119}, // 5

        // SKINCOLOR_KSUPER1 - SKINCOLOR_KSUPER5 //
        {120, 120, 120, 120, 120, 120, 120, 120, 120, 120, 121, 123, 125, 127, 129, 132}, // 1
        {120, 120, 120, 120, 120, 120, 120, 120, 121, 122, 124, 125, 127, 128, 130, 132}, // 2
        {120, 120, 120, 120, 120, 120, 121, 122, 123, 124, 125, 127, 128, 129, 130, 132}, // 3
        {120, 120, 120, 120, 121, 122, 123, 124, 125, 126, 127, 128, 129, 130, 131, 132}, // 4
        {120, 120, 121, 121, 122, 123, 124, 125, 126, 126, 127, 128, 129, 130, 131, 132}, // 5
};



void R_GenerateTranslationColormap(UINT8 *dest_colormap, INT32 skinnum, UINT8 color)
{
	INT32 i;
	INT32 starttranscolor;

	// Handle a couple of simple special cases
	if (skinnum == TC_BOSS || skinnum == TC_ALLWHITE || skinnum == TC_METALSONIC || color == SKINCOLOR_NONE)
	{
		for (i = 0; i < NUM_PALETTE_ENTRIES; i++)
		{
			if (skinnum == TC_ALLWHITE) dest_colormap[i] = 0;
			else dest_colormap[i] = (UINT8)i;
		}

		// White!
		if (skinnum == TC_BOSS)
			dest_colormap[31] = 0;
		else if (skinnum == TC_METALSONIC)
			dest_colormap[239] = 0;

		return;
	}

	starttranscolor = (skinnum != TC_DEFAULT) ? skins[skinnum].starttranscolor : DEFAULT_STARTTRANSCOLOR;

	// Fill in the entries of the palette that are fixed
	for (i = 0; i < starttranscolor; i++)
		dest_colormap[i] = (UINT8)i;

	for (i = (UINT8)(starttranscolor + 16); i < NUM_PALETTE_ENTRIES; i++)
		dest_colormap[i] = (UINT8)i;

	// Build the translated ramp
	for (i = 0; i < SKIN_RAMP_LENGTH; i++)
	{
		// Sryder 2017-10-26: What was here before was most definitely not particularly readable, check above for new color translation table
		dest_colormap[starttranscolor + i] = colortranslations[color][i];
	}

}

UINT8 R_GetColorByName(const char *name)
{
	UINT8 color = (UINT8)atoi(name);
	if (color > 0 && color < MAXSKINCOLORS)
		return color;
	for (color = 1; color < MAXSKINCOLORS; color++)
		if (!stricmp(Color_Names[color], name))
			return color;
	return 0;
}

