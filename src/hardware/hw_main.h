// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// Copyright (C) 1998-2000 by DooM Legacy Team.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
//-----------------------------------------------------------------------------
/// \file
/// \brief 3D render mode functions

#ifndef __HWR_MAIN_H__
#define __HWR_MAIN_H__

#include "hw_data.h"
#include "hw_defs.h"

#include "../am_map.h"
#include "../d_player.h"
#include "../r_defs.h"
#include "../m_perfstats.h"

// Startup & Shutdown the hardware mode renderer
void HWR_Startup(void);
void HWR_Shutdown(void);

void HWR_drawAMline(const fline_t *fl, INT32 color);
void HWR_FadeScreenMenuBack(UINT32 color, INT32 height);
void HWR_DrawConsoleBack(UINT32 color, INT32 height);
void HWR_RenderSkyboxView(INT32 viewnumber, player_t *player);
void HWR_RenderPlayerView(INT32 viewnumber, player_t *player);
void HWR_ClearSkyDome(void);
void HWR_BuildSkyDome(void);
void HWR_DrawViewBorder(INT32 clearlines);
void HWR_DrawFlatFill(INT32 x, INT32 y, INT32 w, INT32 h, lumpnum_t flatlumpnum);
void HWR_InitTextureMapping(void);
void HWR_SetViewSize(void);
void HWR_DrawPatch(GLPatch_t *gpatch, INT32 x, INT32 y, INT32 option);
void HWR_DrawFixedPatch(GLPatch_t *gpatch, fixed_t x, fixed_t y, fixed_t scale, INT32 option, const UINT8 *colormap);
void HWR_DrawCroppedPatch(GLPatch_t *gpatch, fixed_t x, fixed_t y, fixed_t scale, INT32 option, fixed_t sx, fixed_t sy, fixed_t w, fixed_t h);
void HWR_DrawCroppedPatch(GLPatch_t *gpatch, fixed_t x, fixed_t y, INT32 option, fixed_t scale, fixed_t sx, fixed_t sy, fixed_t w, fixed_t h);
void HWR_MakePatch (const patch_t *patch, GLPatch_t *grPatch, GLMipmap_t *grMipmap, boolean makebitmap);
void HWR_CreatePlanePolygons(INT32 bspnum);
void HWR_CreateStaticLightmaps(INT32 bspnum);
void HWR_PrepLevelCache(size_t pnumtextures);
void HWR_DrawFill(INT32 x, INT32 y, INT32 w, INT32 h, INT32 color);
void HWR_DrawConsoleFill(INT32 x, INT32 y, INT32 w, INT32 h, UINT32 color, INT32 options);	// Lat: separate flags from color since color needs to be an uint to work right.
void HWR_DrawPic(INT32 x,INT32 y,lumpnum_t lumpnum);

UINT8 *HWR_GetScreenshot(void);
boolean HWR_Screenshot(const char *pathname);

void HWR_AddCommands(void);
void HWR_CorrectSWTricks(void);
void transform(float *cx, float *cy, float *cz);
FBITFIELD HWR_TranstableToAlpha(INT32 transtablenum, FSurfaceInfo *pSurf);
INT32 HWR_GetTextureUsed(void);
void HWR_DoPostProcessor(player_t *player);
void HWR_StartScreenWipe(void);
void HWR_EndScreenWipe(void);
void HWR_DrawIntermissionBG(void);
void HWR_DoWipe(UINT8 wipenum, UINT8 scrnnum);
void HWR_MakeScreenFinalTexture(void);
void HWR_DrawScreenFinalTexture(int width, int height);

// This stuff is put here so MD2's can use them
boolean HWR_UseShader(void);
void HWR_Lighting(FSurfaceInfo *Surface, INT32 light_level, extracolormap_t *colormap);
UINT8 HWR_FogBlockAlpha(INT32 light, extracolormap_t *colormap); // Let's see if this can work


extern CV_PossibleValue_t granisotropicmode_cons_t[];

#ifdef ALAM_LIGHTING
extern consvar_t cv_grdynamiclighting;
extern consvar_t cv_grstaticlighting;
extern consvar_t cv_grcoronas;
extern consvar_t cv_grcoronasize;
#endif
extern consvar_t cv_grshaders, cv_grallowshaders;
extern consvar_t cv_grmd2;
extern consvar_t cv_grmodelinterpolation;
extern consvar_t cv_grgammared;
extern consvar_t cv_grgammagreen;
extern consvar_t cv_grgammablue;
extern consvar_t cv_grfiltermode;
extern consvar_t cv_granisotropicmode;
extern consvar_t cv_grcorrecttricks;
extern consvar_t cv_grfovchange;
extern consvar_t cv_grsolvetjoin;
extern consvar_t cv_grshearing;
extern consvar_t cv_grspritebillboarding;
extern consvar_t cv_grmodelrollangle;
extern consvar_t cv_grmodellighting;
extern consvar_t cv_grskydome;
extern consvar_t cv_glloadingscreen;
extern consvar_t cv_grfakecontrast;
extern consvar_t cv_grslopecontrast;
extern consvar_t cv_grbatching;
extern consvar_t cv_grsecbright;

extern float gr_viewwidth, gr_viewheight, gr_baseviewwindowy;

extern float gr_viewwindowx, gr_basewindowcentery;

// BP: big hack for a test in lighting ref : 1249753487AB
extern fixed_t *hwbbox;
extern FTransform atransform;

// Render stats
extern ps_metric_t ps_hw_nodesorttime;
extern ps_metric_t ps_hw_nodedrawtime;
extern ps_metric_t ps_hw_spritesorttime;
extern ps_metric_t ps_hw_spritedrawtime;

// Render stats for batching
extern ps_metric_t ps_hw_numpolys;
extern ps_metric_t ps_hw_numverts;
extern ps_metric_t ps_hw_numcalls;
extern ps_metric_t ps_hw_numshaders;
extern ps_metric_t ps_hw_numtextures;
extern ps_metric_t ps_hw_numpolyflags;
extern ps_metric_t ps_hw_numcolors;
extern ps_metric_t ps_hw_batchsorttime;
extern ps_metric_t ps_hw_batchdrawtime;

//bye bye floorinfo

extern boolean gr_shadersavailable;

#endif
