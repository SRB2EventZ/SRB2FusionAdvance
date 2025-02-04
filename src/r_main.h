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
/// \file  r_main.h
/// \brief Rendering variables, consvars, defines

#ifndef __R_MAIN__
#define __R_MAIN__

#include "d_player.h"
#include "r_data.h"

//
// POV related.
//
extern fixed_t viewcos, viewsin;
extern INT32 viewheight;
extern INT32 centerx, centery;

extern fixed_t centerxfrac, centeryfrac;
extern fixed_t projection, projectiony;

extern size_t validcount, linecount, loopcount, framecount; 

// The fraction of a tic being drawn (for interpolation between two tics)
extern fixed_t rendertimefrac;
// Evaluated delta tics for this frame (how many tics since the last frame)
extern fixed_t renderdeltatics;



//
// Lighting LUT.
// Used for z-depth cuing per column/row,
//  and other lighting effects (sector ambient, flash).
//

// Lighting constants.
// Now with 32 levels.
#define LIGHTLEVELS 32
#define LIGHTSEGSHIFT 3
#include "r_fps.h" // Uncapped framerate -- Fury
#include "i_system.h"
#define MAXLIGHTSCALE 48
#define LIGHTSCALESHIFT 12
#define MAXLIGHTZ 128
#define LIGHTZSHIFT 20

extern lighttable_t *scalelight[LIGHTLEVELS][MAXLIGHTSCALE];
extern lighttable_t *scalelightfixed[MAXLIGHTSCALE];
extern lighttable_t *zlight[LIGHTLEVELS][MAXLIGHTZ];

// Number of diminishing brightness levels.
// There a 0-31, i.e. 32 LUT in the COLORMAP lump.
#define NUMCOLORMAPS 32

// Utility functions.
//
// R_PointOnSide
// Traverse BSP (sub) tree,
// check point against partition plane.
// Returns side 0 (front) or 1 (back).
//
// killough 5/2/98: reformatted
//
FUNCINLINE static ATTRINLINE INT32 R_PointOnSide(fixed_t x, fixed_t y, const node_t *restrict node)
{
	if (!node->dx)
		return x <= node->x ? node->dy > 0 : node->dy < 0;
	if (!node->dy)
		return y <= node->y ? node->dx < 0 : node->dx > 0;
	x -= node->x;
	y -= node->y;
	// Try to quickly decide by looking at sign bits.
	// also use a mask to avoid branch prediction
	INT32 mask = (node->dy ^ node->dx ^ x ^ y) >> 31;
	return (mask & ((node->dy ^ x) < 0)) |  // (left is negative)
	(~mask & (FixedMul(y, node->dx>>FRACBITS) >= FixedMul(node->dy>>FRACBITS, x)));
}

FUNCINLINE static ATTRINLINE INT32 R_PointOnSideFast(fixed_t x, fixed_t y, const node_t *node)
{
	// use cross product to determine side quickly
	return (INT64)(y - node->y) * node->dx - (INT64)(x - node->x) * node->dy > 0;
}

INT32 R_PointOnSegSide(fixed_t x, fixed_t y, seg_t *restrict line);
angle_t R_PointToAngle(fixed_t x, fixed_t y);
angle_t R_PointToAngle2(fixed_t px2, fixed_t py2, fixed_t px1, fixed_t py1);
angle_t R_PointToAngleEx(INT32 x2, INT32 y2, INT32 x1, INT32 y1);
fixed_t R_PointToDist(fixed_t x, fixed_t y);
fixed_t R_PointToDist2(fixed_t px2, fixed_t py2, fixed_t px1, fixed_t py1);

fixed_t R_ScaleFromGlobalAngle(angle_t visangle);
subsector_t *R_IsPointInSubsector(fixed_t x, fixed_t y);

boolean R_DoCulling(line_t *cullheight, line_t *viewcullheight, fixed_t vz, fixed_t bottomh, fixed_t toph);

//
// R_PointInSubsector
//
FUNCINLINE static ATTRINLINE subsector_t *R_PointInSubsector(fixed_t x, fixed_t y)
{
	size_t nodenum = numnodes-1;
	while (!(nodenum & NF_SUBSECTOR))
		nodenum = nodes[nodenum].children[R_PointOnSide(x, y, nodes+nodenum)];
	return &subsectors[nodenum & ~NF_SUBSECTOR];
}

//
// REFRESH - the actual rendering functions.
//

extern consvar_t cv_showhud, cv_translucenthud;
extern consvar_t cv_homremoval;
extern consvar_t cv_chasecam, cv_chasecam2;
extern consvar_t cv_flipcam, cv_flipcam2;
extern consvar_t cv_shadow, cv_shadowoffs, cv_shadowposition;
extern consvar_t cv_translucency;
extern consvar_t cv_precipdensity, cv_drawdist, cv_drawdist_nights, cv_drawdist_precip;
extern consvar_t cv_skybox;
extern consvar_t cv_tailspickup; 

// Uncapped Framerate
extern tic_t prev_tics; 
extern consvar_t cv_frameinterpolation;



// Called by startup code.
void R_Init(void);

// just sets setsizeneeded true
extern boolean setsizeneeded;
void R_SetViewSize(void);

// do it (sometimes explicitly called)
void R_ExecuteSetViewSize(void);

void R_SkyboxFrame(player_t *player);

void R_SetupFrame(player_t *player, boolean skybox);
// Called by G_Drawer.
void R_RenderPlayerView(player_t *player);

// add commands related to engine, at game startup
void R_RegisterEngineStuff(void);
#endif
