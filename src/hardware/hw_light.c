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
//-----------------------------------------------------------------------------
/// \file
/// \brief Corona/Dynamic/Static lighting add on by Hurdler
///	!!! Under construction !!!

#include "../doomdef.h"

#ifdef HWRENDER
#include "hw_light.h"
#include "hw_drv.h"
#include "../i_video.h"
#include "../z_zone.h"
#include "../m_random.h"
#include "../m_bbox.h"
#include "../w_wad.h"
#include "../r_state.h"
#include "../r_main.h"
#include "../p_local.h"

//=============================================================================
//                                                                      DEFINES
//=============================================================================

#define DL_SQRRADIUS(x) dynlights->p_lspr[(x)]->dynamic_sqrradius
#define DL_RADIUS(x) dynlights->p_lspr[(x)]->dynamic_radius
#define LIGHT_POS(i) dynlights->position[(i)]

#define DL_HIGH_QUALITY
//#define STATICLIGHT  //Hurdler: TODO!
//#define LIGHTMAPFLAGS  (PF_Masked|PF_Clip|PF_NoAlphaTest)  // debug see overdraw
#define LIGHTMAPFLAGS (PF_Modulated|PF_Additive|PF_Clip)

#ifdef ALAM_LIGHTING
static dynlights_t view_dynlights[2]; // 2 players in splitscreen mode
static dynlights_t *dynlights = &view_dynlights[0];
#endif

#define UNDEFINED_SPR   0x0 // actually just for testing
#define CORONA_SPR      0x1 // a light source which only emit a corona
#define DYNLIGHT_SPR    0x2 // a light source which is only used for dynamic lighting
#define LIGHT_SPR       (DYNLIGHT_SPR|CORONA_SPR)
#define ROCKET_SPR      (DYNLIGHT_SPR|CORONA_SPR|0x10)
//#define MONSTER_SPR     4
//#define AMMO_SPR        8
//#define BONUS_SPR      16

//Hurdler: now we can change those values via FS :)
light_t lspr[NUMLIGHTS] =
{
	// type       offset x,   y  coronas color, c_size,light color,l_radius, sqr radius computed at init
	// UNDEFINED: 0
	{ UNDEFINED_SPR,  0.0f,   0.0f, 0x00000000,  24.0f, 0x00000000,   0.0f, 0.0f},
	// weapons
	// RINGSPARK_L
	{ LIGHT_SPR,      0.0f,   0.0f, 0x0000e0ff,  16.0f, 0x0000e0ff,  32.0f, 0.0f}, // Tails 09-08-2002
	// SUPERSONIC_L
	{ DYNLIGHT_SPR,   0.0f,   0.0f, 0xff00e0ff,  32.0f, 0xff00e0ff, 128.0f, 0.0f}, // Tails 09-08-2002
	// SUPERSPARK_L
	{ LIGHT_SPR,      0.0f,   0.0f, 0xe0ffffff,   8.0f, 0xe0ffffff,  64.0f, 0.0f},
	// INVINCIBLE_L
	{ DYNLIGHT_SPR,   0.0f,   0.0f, 0x10ffaaaa,  16.0f, 0x10ffaaaa, 128.0f, 0.0f},
	// GREENSHIELD_L
	{ DYNLIGHT_SPR,   0.0f,   0.0f, 0x602b7337,  32.0f, 0x602b7337, 128.0f, 0.0f},
	// BLUESHIELD_L
	{ DYNLIGHT_SPR,   0.0f,   0.0f, 0x60cb0000,  32.0f, 0x60cb0000, 128.0f, 0.0f},

	// tall lights
	// YELLOWSHIELD_L
	{ DYNLIGHT_SPR,   0.0f,   0.0f, 0x601f7baf,  32.0f, 0x601f7baf, 128.0f, 0.0f},

	// REDSHIELD_L
	{ DYNLIGHT_SPR,   0.0f,   0.0f, 0x600000cb,  32.0f, 0x600000cb, 128.0f, 0.0f},

	// BLACKSHIELD_L // Black light? lol
	{ DYNLIGHT_SPR,   0.0f,   0.0f, 0x60010101,  32.0f, 0x60ff00ff, 128.0f, 0.0f},

	// WHITESHIELD_L
	{ DYNLIGHT_SPR,   0.0f,   0.0f, 0x60ffffff,  32.0f, 0x60ffffff, 128.0f, 0.0f},

	// SMALLREDBALL_L
	{ DYNLIGHT_SPR,   0.0f,   0.0f, 0x606060f0,   0.0f, 0x302070ff,  32.0f, 0.0f},

	// small lights
	// RINGLIGHT_L
	{ DYNLIGHT_SPR,   0.0f,   0.0f, 0x60b0f0f0,   0.0f, 0x30b0f0f0, 100.0f, 0.0f},
	// GREENSMALL_L
	{    LIGHT_SPR,   0.0f,  14.0f, 0x6070ff70,  60.0f, 0x4070ff70, 100.0f, 0.0f},
	// REDSMALL_L
	{    LIGHT_SPR,   0.0f,  14.0f, 0x705070ff,  60.0f, 0x405070ff, 100.0f, 0.0f},

	// type       offset x,   y  coronas color, c_size,light color,l_radius, sqr radius computed at init
	// GREENSHINE_L
	{    LIGHT_SPR,   0.0f,   0.0f, 0xff00ff00,  64.0f, 0xff00ff00, 256.0f, 0.0f},
	// ORANGESHINE_L
	{    LIGHT_SPR,   0.0f,   0.0f, 0xff0080ff,  64.0f, 0xff0080ff, 256.0f, 0.0f},
	// PINKSHINE_L
	{    LIGHT_SPR,   0.0f,   0.0f, 0xffe080ff,  64.0f, 0xffe080ff, 256.0f, 0.0f},
	// BLUESHINE_L
	{    LIGHT_SPR,   0.0f,   0.0f, 0xffff0000,  64.0f, 0xffff0000, 256.0f, 0.0f},
	// REDSHINE_L
	{    LIGHT_SPR,   0.0f,   0.0f, 0xff0000ff,  64.0f, 0xff0000ff, 256.0f, 0.0f},
	// LBLUESHINE_L
	{    LIGHT_SPR,   0.0f,   0.0f, 0xffff8080,  64.0f, 0xffff8080, 256.0f, 0.0f},
	// GREYSHINE_L
	{    LIGHT_SPR,   0.0f,   0.0f, 0xffe0e0e0,  64.0f, 0xffe0e0e0, 256.0f, 0.0f},

	// monsters
	// REDBALL_L
	{ DYNLIGHT_SPR,   0.0f,   0.0f, 0x606060ff,   0.0f, 0x606060ff, 100.0f, 0.0f},
	// GREENBALL_L
	{ DYNLIGHT_SPR,   0.0f,   0.0f, 0x6060ff60, 120.0f, 0x6060ff60, 100.0f, 0.0f},
	// BLUEBALL_L
	{ DYNLIGHT_SPR,   0.0f,   0.0f, 0x60ff6060, 120.0f, 0x60ff6060, 100.0f, 0.0f},

	// NIGHTSLIGHT_L
	{    LIGHT_SPR,   0.0f,   6.0f, 0x60ffffff,  16.0f, 0x30ffffff,  32.0f, 0.0f},

	// JETLIGHT_L
	{ DYNLIGHT_SPR,   0.0f,   6.0f, 0x60ffaaaa,  16.0f, 0x30ffaaaa,  64.0f, 0.0f},

	// GOOPLIGHT_L
	{ DYNLIGHT_SPR,   0.0f,   6.0f, 0x60ff00ff,  16.0f, 0x30ff00ff,  32.0f, 0.0f},

	// STREETLIGHT_L
	{ LIGHT_SPR,      0.0f,   0.0f, 0xe0ffffff,  64.0f, 0xe0ffffff, 384.0f, 0.0f},
};

light_t *t_lspr[NUMSPRITES] =
{
	&lspr[NOLIGHT],     // SPR_NULL
	&lspr[NOLIGHT],     // SPR_UNKN

	&lspr[NOLIGHT],     // SPR_THOK
	&lspr[SUPERSONIC_L],// SPR_PLAY

	// Enemies
	&lspr[NOLIGHT],     // SPR_POSS
	&lspr[NOLIGHT],     // SPR_SPOS
	&lspr[NOLIGHT],     // SPR_FISH
	&lspr[NOLIGHT],     // SPR_BUZZ Graue 03-10-2004
	&lspr[NOLIGHT],     // SPR_RBUZ Graue 03-10-2004
	&lspr[NOLIGHT],     // SPR_JETB
	&lspr[NOLIGHT],     // SPR_JETW
	&lspr[NOLIGHT],     // SPR_JETG
	&lspr[NOLIGHT],     // SPR_CCOM
	&lspr[NOLIGHT],     // SPR_DETN
	&lspr[NOLIGHT],     // SPR_SKIM
	&lspr[NOLIGHT],     // SPR_TRET
	&lspr[NOLIGHT],     // SPR_TURR
	&lspr[NOLIGHT],     // SPR_SHRP
	&lspr[NOLIGHT],     // SPR_JJAW
	&lspr[NOLIGHT],     // SPR_SNLR
	&lspr[NOLIGHT],     // SPR_VLTR
	&lspr[NOLIGHT],     // SPR_PNTY
	&lspr[NOLIGHT],     // SPR_ARCH
	&lspr[NOLIGHT],     // SPR_CBFS
	&lspr[NOLIGHT],     // SPR_SPSH
	&lspr[NOLIGHT],     // SPR_ESHI
	&lspr[NOLIGHT],     // SPR_GSNP
	&lspr[NOLIGHT],     // SPR_MNUS
	&lspr[NOLIGHT],     // SPR_SSHL
	&lspr[NOLIGHT],     // SPR_UNID
	&lspr[NOLIGHT],     // SPR_BBUZ

	// Generic Boos Items
	&lspr[JETLIGHT_L],     // SPR_JETF // Boss jet fumes

	// Boss 1, (Greenflower)
	&lspr[NOLIGHT],     // SPR_EGGM

	// Boss 2, (Techno Hill)
	&lspr[NOLIGHT],     // SPR_EGGN
	&lspr[NOLIGHT],     // SPR_TNKA
	&lspr[NOLIGHT],     // SPR_TNKB
	&lspr[NOLIGHT],     // SPR_SPNK
	&lspr[NOLIGHT],     // SPR_GOOP

	// Boss 3 (Deep Sea)
	&lspr[NOLIGHT],     // SPR_EGGO
	&lspr[NOLIGHT],     // SPR_PRPL
	&lspr[NOLIGHT],     // SPR_FAKE

	// Boss 4 (Castle Eggman)
	&lspr[NOLIGHT],     // SPR_EGGP
	&lspr[REDBALL_L],   // SPR_EFIR

	// Boss 5 (Arid Canyon)
	&lspr[NOLIGHT],     // SPR_EGGQ

	// Boss 6 (Red Volcano)
	&lspr[NOLIGHT],     // SPR_EEGR

	// Boss 7 (Dark City)
	&lspr[NOLIGHT],     // SPR_BRAK
	&lspr[NOLIGHT],     // SPR_BGOO
	&lspr[NOLIGHT],     // SPR_BMSL

	// Boss 8 (Egg Rock)
	&lspr[NOLIGHT],     // SPR_EGGT

	&lspr[NOLIGHT], //SPR_RCKT
	&lspr[NOLIGHT], //SPR_ELEC
	&lspr[NOLIGHT], //SPR_TARG
	&lspr[NOLIGHT], //SPR_NPLM
	&lspr[NOLIGHT], //SPR_MNPL

	// Metal Sonic
	&lspr[NOLIGHT],     // SPR_METL
	&lspr[NOLIGHT],     // SPR_MSCF
	&lspr[NOLIGHT],     // SPR_MSCB

	// Collectible Items
	&lspr[NOLIGHT],     // SPR_RING
	&lspr[NOLIGHT],     // SPR_TRNG
	&lspr[NOLIGHT],     // SPR_EMMY
	&lspr[BLUEBALL_L],     // SPR_TOKE
	&lspr[REDBALL_L],     // SPR_RFLG
	&lspr[BLUEBALL_L],     // SPR_BFLG
	&lspr[NOLIGHT],     // SPR_NWNG
	&lspr[NOLIGHT],     // SPR_EMBM
	&lspr[NOLIGHT],     // SPR_CEMG
	&lspr[NOLIGHT],     // SPR_EMER

	// Interactive Objects
	&lspr[NOLIGHT],     // SPR_FANS
	&lspr[NOLIGHT],     // SPR_BUBL
	&lspr[NOLIGHT],     // SPR_SIGN
	&lspr[NOLIGHT],     // SPR_STEM
	&lspr[NOLIGHT],     // SPR_SPIK
	&lspr[NOLIGHT],     // SPR_SFLM
	&lspr[NOLIGHT],     // SPR_USPK
	&lspr[NOLIGHT],     // SPR_STPT
	&lspr[NOLIGHT],     // SPR_BMNE

	// Monitor Boxes
	&lspr[NOLIGHT],     // SPR_SRBX
	&lspr[NOLIGHT],     // SPR_RRBX
	&lspr[NOLIGHT],     // SPR_BRBX
	&lspr[NOLIGHT],     // SPR_SHTV
	&lspr[NOLIGHT],     // SPR_PINV
	&lspr[NOLIGHT],     // SPR_YLTV
	&lspr[NOLIGHT],     // SPR_BLTV
	&lspr[NOLIGHT],     // SPR_BKTV
	&lspr[NOLIGHT],     // SPR_WHTV
	&lspr[NOLIGHT],     // SPR_GRTV
	&lspr[NOLIGHT],     // SPR_ELTV
	&lspr[NOLIGHT],     // SPR_EGGB
	&lspr[NOLIGHT],     // SPR_MIXU
	&lspr[NOLIGHT],     // SPR_RECY
	&lspr[NOLIGHT],     // SPR_QUES
	&lspr[NOLIGHT],     // SPR_GBTV
	&lspr[NOLIGHT],     // SPR_PRUP
	&lspr[NOLIGHT],     // SPR_PTTV

	// Monitor Miscellany
	&lspr[NOLIGHT],     // SPR_MTEX

	// Projectiles
	&lspr[NOLIGHT],     // SPR_MISL
	&lspr[NOLIGHT],     // SPR_TORP
	&lspr[NOLIGHT],     // SPR_ENRG
	&lspr[NOLIGHT],     // SPR_MINE
	&lspr[NOLIGHT],     // SPR_JBUL
	&lspr[SMALLREDBALL_L], // SPR_TRLS
	&lspr[NOLIGHT],     // SPR_CBLL
	&lspr[NOLIGHT],     // SPR_AROW
	&lspr[NOLIGHT],     // SPR_CFIR

	// Greenflower Scenery
	&lspr[NOLIGHT],     // SPR_FWR1
	&lspr[NOLIGHT],     // SPR_FWR2
	&lspr[NOLIGHT],     // SPR_FWR3
	&lspr[NOLIGHT],     // SPR_FWR4
	&lspr[NOLIGHT],     // SPR_BUS1
	&lspr[NOLIGHT],     // SPR_BUS2

	// Techno Hill Scenery
	&lspr[NOLIGHT],     // SPR_THZP
	&lspr[REDBALL_L],     // SPR_ALRM

	// Deep Sea Scenery
	&lspr[NOLIGHT],     // SPR_GARG
	&lspr[NOLIGHT],     // SPR_SEWE
	&lspr[NOLIGHT],     // SPR_DRIP
	&lspr[NOLIGHT],     // SPR_CRL1
	&lspr[NOLIGHT],     // SPR_CRL2
	&lspr[NOLIGHT],     // SPR_CRL3
	&lspr[NOLIGHT],     // SPR_BCRY

	// Castle Eggman Scenery
	&lspr[NOLIGHT],     // SPR_CHAN
	&lspr[REDBALL_L],   // SPR_FLAM
	&lspr[NOLIGHT],     // SPR_ESTA
	&lspr[NOLIGHT],     // SPR_SMCH
	&lspr[NOLIGHT],     // SPR_BMCH
	&lspr[NOLIGHT],     // SPR_SMCE
	&lspr[NOLIGHT],     // SPR_BMCE

	// Arid Canyon Scenery
	&lspr[NOLIGHT],     // SPR_BTBL
	&lspr[NOLIGHT],     // SPR_STBL
	&lspr[NOLIGHT],     // SPR_CACT

	// Red Volcano Scenery
	&lspr[REDBALL_L],   // SPR_FLME
	&lspr[REDBALL_L],   // SPR_DFLM

	// Dark City Scenery

	// Egg Rock Scenery

	// Christmas Scenery
	&lspr[NOLIGHT],     // SPR_XMS1
	&lspr[NOLIGHT],     // SPR_XMS2
	&lspr[NOLIGHT],     // SPR_XMS3

	// Botanic Serenity Scenery
	&lspr[NOLIGHT],     // SPR_BSZ1
	&lspr[NOLIGHT],     // SPR_BSZ2
	&lspr[NOLIGHT],     // SPR_BSZ3
	&lspr[NOLIGHT],     // SPR_BSZ4
	&lspr[NOLIGHT],     // SPR_BSZ5
	&lspr[NOLIGHT],     // SPR_BSZ6
	&lspr[NOLIGHT],     // SPR_BSZ7
	&lspr[NOLIGHT],     // SPR_BSZ8

	// Stalagmites
	&lspr[NOLIGHT],     // SPR_STLG

	// Disco Ball
	&lspr[NOLIGHT],     // SPR_DBAL

	// ATZ Red Crystal
	&lspr[NOLIGHT],     // SPR_RCRY

	// Powerup Indicators
	&lspr[REDSHIELD_L],     // SPR_ARMA
	&lspr[NOLIGHT],     // SPR_ARMF
	&lspr[NOLIGHT],     // SPR_ARMB
	&lspr[WHITESHIELD_L],     // SPR_WIND
	&lspr[YELLOWSHIELD_L],     // SPR_MAGN
	&lspr[GREENSHIELD_L],     // SPR_ELEM
	&lspr[BLUESHIELD_L],     // SPR_FORC
	&lspr[GREENSHIELD_L],     // SPR_PITY
	&lspr[INVINCIBLE_L],     // SPR_IVSP
	&lspr[SUPERSPARK_L],     // SPR_SSPK

	&lspr[NOLIGHT],     // SPR_GOAL

	// Freed Animals
	&lspr[NOLIGHT],     // SPR_BIRD
	&lspr[NOLIGHT],     // SPR_BUNY
	&lspr[NOLIGHT],     // SPR_MOUS
	&lspr[NOLIGHT],     // SPR_CHIC
	&lspr[NOLIGHT],     // SPR_COWZ
	&lspr[NOLIGHT],     // SPR_RBRD

	// Springs
	&lspr[NOLIGHT],     // SPR_SPRY
	&lspr[NOLIGHT],     // SPR_SPRR
	&lspr[NOLIGHT],     // SPR_SPRB Graue
	&lspr[NOLIGHT],     // SPR_YSPR
	&lspr[NOLIGHT],     // SPR_RSPR

	// Environmentals Effects
	&lspr[NOLIGHT],     // SPR_RAIN
	&lspr[NOLIGHT],     // SPR_SNO1
	&lspr[NOLIGHT],     // SPR_SPLH
	&lspr[NOLIGHT],     // SPR_SPLA
	&lspr[NOLIGHT],     // SPR_SMOK
	&lspr[NOLIGHT],     // SPR_BUBP
	&lspr[NOLIGHT],     // SPR_BUBO
	&lspr[NOLIGHT],     // SPR_BUBN
	&lspr[NOLIGHT],     // SPR_BUBM
	&lspr[NOLIGHT],     // SPR_POPP
	&lspr[SUPERSPARK_L], // SPR_TFOG
	&lspr[NIGHTSLIGHT_L],     // SPR_SEED // Sonic CD flower seed
	&lspr[NOLIGHT],     // SPR_PRTL

	// Game Indicators
	&lspr[NOLIGHT],     // SPR_SCOR
	&lspr[NOLIGHT],     // SPR_DRWN
	&lspr[NOLIGHT],     // SPR_TTAG
	&lspr[NOLIGHT],     // SPR_GFLG

	// Ring Weapons
	&lspr[RINGLIGHT_L],     // SPR_RRNG
	&lspr[RINGLIGHT_L],     // SPR_RNGB
	&lspr[RINGLIGHT_L],     // SPR_RNGR
	&lspr[RINGLIGHT_L],     // SPR_RNGI
	&lspr[RINGLIGHT_L],     // SPR_RNGA
	&lspr[RINGLIGHT_L],     // SPR_RNGE
	&lspr[RINGLIGHT_L],     // SPR_RNGS
	&lspr[RINGLIGHT_L],     // SPR_RNGG

	&lspr[RINGLIGHT_L],     // SPR_PIKB
	&lspr[RINGLIGHT_L],     // SPR_PIKR
	&lspr[RINGLIGHT_L],     // SPR_PIKA
	&lspr[RINGLIGHT_L],     // SPR_PIKE
	&lspr[RINGLIGHT_L],     // SPR_PIKS
	&lspr[RINGLIGHT_L],     // SPR_PIKG

	&lspr[RINGLIGHT_L],     // SPR_TAUT
	&lspr[RINGLIGHT_L],     // SPR_TGRE
	&lspr[RINGLIGHT_L],     // SPR_TSCR

	// Mario-specific stuff
	&lspr[NOLIGHT],     // SPR_COIN
	&lspr[NOLIGHT],     // SPR_CPRK
	&lspr[NOLIGHT],     // SPR_GOOM
	&lspr[NOLIGHT],     // SPR_BGOM
	&lspr[REDBALL_L],     // SPR_FFWR
	&lspr[SMALLREDBALL_L],     // SPR_FBLL
	&lspr[NOLIGHT],     // SPR_SHLL
	&lspr[REDBALL_L],     // SPR_PUMA
	&lspr[NOLIGHT],     // SPR_HAMM
	&lspr[NOLIGHT],     // SPR_KOOP
	&lspr[REDBALL_L],     // SPR_BFLM
	&lspr[NOLIGHT],     // SPR_MAXE
	&lspr[NOLIGHT],     // SPR_MUS1
	&lspr[NOLIGHT],     // SPR_MUS2
	&lspr[NOLIGHT],     // SPR_TOAD

	// NiGHTS Stuff
	&lspr[SUPERSONIC_L],     // SPR_NDRN // NiGHTS drone
	&lspr[SUPERSONIC_L],     // SPR_SUPE // NiGHTS character flying
	&lspr[SUPERSONIC_L],     // SPR_SUPZ // NiGHTS hurt
	&lspr[SUPERSONIC_L],     // SPR_NDRL // NiGHTS character drilling
	&lspr[NOLIGHT],     // SPR_NSPK
	&lspr[NOLIGHT],     // SPR_NBMP
	&lspr[NOLIGHT],     // SPR_HOOP
	&lspr[NOLIGHT],     // SPR_HSCR
	&lspr[NOLIGHT],     // SPR_NPRU
	&lspr[NOLIGHT],     // SPR_CAPS
	&lspr[SUPERSONIC_L], // SPR_SUPT

	// Debris
	&lspr[RINGSPARK_L],  // SPR_SPRK
	&lspr[NOLIGHT],      // SPR_BOM1
	&lspr[SUPERSPARK_L], // SPR_BOM2
	&lspr[SUPERSPARK_L], // SPR_BOM3
	&lspr[NOLIGHT],      // SPR_BOM4

	// Crumbly rocks
	&lspr[NOLIGHT],     // SPR_ROIA
	&lspr[NOLIGHT],     // SPR_ROIB
	&lspr[NOLIGHT],     // SPR_ROIC
	&lspr[NOLIGHT],     // SPR_ROID
	&lspr[NOLIGHT],     // SPR_ROIE
	&lspr[NOLIGHT],     // SPR_ROIF
	&lspr[NOLIGHT],     // SPR_ROIG
	&lspr[NOLIGHT],     // SPR_ROIH
	&lspr[NOLIGHT],     // SPR_ROII
	&lspr[NOLIGHT],     // SPR_ROIJ
	&lspr[NOLIGHT],     // SPR_ROIK
	&lspr[NOLIGHT],     // SPR_ROIL
	&lspr[NOLIGHT],     // SPR_ROIM
	&lspr[NOLIGHT],     // SPR_ROIN
	&lspr[NOLIGHT],     // SPR_ROIO
	&lspr[NOLIGHT],     // SPR_ROIP

	// Blue Spheres
	&lspr[NOLIGHT],     // SPR_BBAL

	// Gravity Well Objects
	&lspr[NOLIGHT],     // SPR_GWLG
	&lspr[NOLIGHT],     // SPR_GWLR

	// SRB1 Sprites
	&lspr[NOLIGHT],     // SPR_SRBA
	&lspr[NOLIGHT],     // SPR_SRBB
	&lspr[NOLIGHT],     // SPR_SRBC
	&lspr[NOLIGHT],     // SPR_SRBD
	&lspr[NOLIGHT],     // SPR_SRBE
	&lspr[NOLIGHT],     // SPR_SRBF
	&lspr[NOLIGHT],     // SPR_SRBG
	&lspr[NOLIGHT],     // SPR_SRBH
	&lspr[NOLIGHT],     // SPR_SRBI
	&lspr[NOLIGHT],     // SPR_SRBJ
	&lspr[NOLIGHT],     // SPR_SRBK
	&lspr[NOLIGHT],     // SPR_SRBL
	&lspr[NOLIGHT],     // SPR_SRBM
	&lspr[NOLIGHT],     // SPR_SRBN
	&lspr[NOLIGHT],     // SPR_SRBO

	// Free slots
	&lspr[NOLIGHT],
	&lspr[NOLIGHT],
	&lspr[NOLIGHT],
	&lspr[NOLIGHT],
	&lspr[NOLIGHT],
	&lspr[NOLIGHT],
	&lspr[NOLIGHT],
	&lspr[NOLIGHT],
	&lspr[NOLIGHT],
	&lspr[NOLIGHT],
	&lspr[NOLIGHT],
	&lspr[NOLIGHT],
	&lspr[NOLIGHT],
	&lspr[NOLIGHT],
	&lspr[NOLIGHT],
	&lspr[NOLIGHT],
	&lspr[NOLIGHT],
	&lspr[NOLIGHT],
	&lspr[NOLIGHT],
	&lspr[NOLIGHT],
	&lspr[NOLIGHT],
	&lspr[NOLIGHT],
	&lspr[NOLIGHT],
	&lspr[NOLIGHT],
	&lspr[NOLIGHT],
	&lspr[NOLIGHT],
	&lspr[NOLIGHT],
	&lspr[NOLIGHT],
	&lspr[NOLIGHT],
	&lspr[NOLIGHT],
	&lspr[NOLIGHT],
	&lspr[NOLIGHT],
	&lspr[NOLIGHT],
	&lspr[NOLIGHT],
	&lspr[NOLIGHT],
	&lspr[NOLIGHT],
	&lspr[NOLIGHT],
	&lspr[NOLIGHT],
	&lspr[NOLIGHT],
	&lspr[NOLIGHT],
	&lspr[NOLIGHT],
	&lspr[NOLIGHT],
	&lspr[NOLIGHT],
	&lspr[NOLIGHT],
	&lspr[NOLIGHT],
	&lspr[NOLIGHT],
	&lspr[NOLIGHT],
	&lspr[NOLIGHT],
	&lspr[NOLIGHT],
	&lspr[NOLIGHT],
	&lspr[NOLIGHT],
	&lspr[NOLIGHT],
	&lspr[NOLIGHT],
	&lspr[NOLIGHT],
	&lspr[NOLIGHT],
	&lspr[NOLIGHT],
	&lspr[NOLIGHT],
	&lspr[NOLIGHT],
	&lspr[NOLIGHT],
	&lspr[NOLIGHT],
	&lspr[NOLIGHT],
	&lspr[NOLIGHT],
	&lspr[NOLIGHT],
	&lspr[NOLIGHT],
	&lspr[NOLIGHT],
	&lspr[NOLIGHT],
	&lspr[NOLIGHT],
	&lspr[NOLIGHT],
	&lspr[NOLIGHT],
	&lspr[NOLIGHT],
	&lspr[NOLIGHT],
	&lspr[NOLIGHT],
	&lspr[NOLIGHT],
	&lspr[NOLIGHT],
	&lspr[NOLIGHT],
	&lspr[NOLIGHT],
	&lspr[NOLIGHT],
	&lspr[NOLIGHT],
	&lspr[NOLIGHT],
	&lspr[NOLIGHT],
	&lspr[NOLIGHT],
	&lspr[NOLIGHT],
	&lspr[NOLIGHT],
	&lspr[NOLIGHT],
	&lspr[NOLIGHT],
	&lspr[NOLIGHT],
	&lspr[NOLIGHT],
	&lspr[NOLIGHT],
	&lspr[NOLIGHT],
	&lspr[NOLIGHT],
	&lspr[NOLIGHT],
	&lspr[NOLIGHT],
	&lspr[NOLIGHT],
	&lspr[NOLIGHT],
	&lspr[NOLIGHT],
	&lspr[NOLIGHT],
	&lspr[NOLIGHT],
	&lspr[NOLIGHT],
	&lspr[NOLIGHT],
	&lspr[NOLIGHT],
	&lspr[NOLIGHT],
	&lspr[NOLIGHT],
	&lspr[NOLIGHT],
	&lspr[NOLIGHT],
	&lspr[NOLIGHT],
	&lspr[NOLIGHT],
	&lspr[NOLIGHT],
	&lspr[NOLIGHT],
	&lspr[NOLIGHT],
	&lspr[NOLIGHT],
	&lspr[NOLIGHT],
	&lspr[NOLIGHT],
	&lspr[NOLIGHT],
	&lspr[NOLIGHT],
	&lspr[NOLIGHT],
	&lspr[NOLIGHT],
	&lspr[NOLIGHT],
	&lspr[NOLIGHT],
	&lspr[NOLIGHT],
	&lspr[NOLIGHT],
	&lspr[NOLIGHT],
	&lspr[NOLIGHT],
	&lspr[NOLIGHT],
	&lspr[NOLIGHT],
	&lspr[NOLIGHT],
	&lspr[NOLIGHT],
};

#ifdef ALAM_LIGHTING

//=============================================================================
//                                                                       PROTOS
//=============================================================================

static void HWR_SetLight(void);

// --------------------------------------------------------------------------
// calcul la projection d'un point sur une droite (determin� par deux
// points) et ensuite calcul la distance (au carr� de ce point au point
// project�sur cette droite
// --------------------------------------------------------------------------
static float HWR_DistP2D(FOutVector *p1, FOutVector *p2, FVector *p3, FVector *inter)
{
	if (p1->z == p2->z)
	{
		inter->x = p3->x;
		inter->z = p1->z;
	}
	else if (p1->x == p2->x)
	{
		inter->x = p1->x;
		inter->z = p3->z;
	}
	else
	{
		register float local, pente;
		// Wat een mooie formula! Hurdler's math;-)
		pente = (p1->z-p2->z) / (p1->x-p2->x);
		local = p1->z - p1->x*pente;
		inter->x = (p3->z - local + p3->x/pente) * (pente/(pente*pente+1));
		inter->z = inter->x*pente + local;
	}

	return (p3->x-inter->x)*(p3->x-inter->x) + (p3->z-inter->z)*(p3->z-inter->z);
}

// check if sphere (radius r) centred in p3 touch the bounding box defined by p1, p2
static boolean SphereTouchBBox3D(FOutVector *p1, FOutVector *p2, FVector *p3, float r)
{
	float minx = p1->x,maxx = p2->x,miny = p2->y,maxy = p1->y,minz = p2->z,maxz = p1->z;

	if (minx > maxx)
	{
		minx = maxx;
		maxx = p1->x;
	}
	if (miny > maxy)
	{
		miny = maxy;
		maxy = p2->y;
	}
	if (minz > maxz)
	{
		minz = maxz;
		maxz = p2->z;
	}

	if (minx-r > p3->x) return false;
	if (maxx+r < p3->x) return false;
	if (miny-r > p3->y) return false;
	if (maxy+r < p3->y) return false;
	if (minz-r > p3->z) return false;
	if (maxz+r < p3->z) return false;
	return true;
}

// Hurdler: The old code was removed by me because I don't think it will be used one day.
//          (It's still available on the CVS for educational purpose: Revision 1.8)

// --------------------------------------------------------------------------
// calcul du dynamic lighting sur les murs
// lVerts contient les coords du mur sans le mlook (up/down)
// --------------------------------------------------------------------------
void HWR_WallLighting(FOutVector *wlVerts)
{
	int             i, j;

	// dynlights->nb == 0 if cv_grdynamiclighting.value is not set
	for (j = 0; j < dynlights->nb; j++)
	{
		FVector         inter;
		FSurfaceInfo    Surf;
		float           dist_p2d, d[4], s;

		// check bounding box first
		if (SphereTouchBBox3D(&wlVerts[2], &wlVerts[0], &LIGHT_POS(j), DL_RADIUS(j))==false)
			continue;
		d[0] = wlVerts[2].x - wlVerts[0].x;
		d[1] = wlVerts[2].z - wlVerts[0].z;
		d[2] = LIGHT_POS(j).x - wlVerts[0].x;
		d[3] = LIGHT_POS(j).z - wlVerts[0].z;
		// backface cull
		if (d[2]*d[1] - d[3]*d[0] < 0)
			continue;
		// check exact distance
		dist_p2d = HWR_DistP2D(&wlVerts[2], &wlVerts[0],  &LIGHT_POS(j), &inter);
		if (dist_p2d >= DL_SQRRADIUS(j))
			continue;

		d[0] = (float)sqrt((wlVerts[0].x-inter.x)*(wlVerts[0].x-inter.x)
			+ (wlVerts[0].z-inter.z)*(wlVerts[0].z-inter.z));
		d[1] = (float)sqrt((wlVerts[2].x-inter.x)*(wlVerts[2].x-inter.x)
			+ (wlVerts[2].z-inter.z)*(wlVerts[2].z-inter.z));
		//dAB = sqrtf((wlVerts[0].x-wlVerts[2].x)*(wlVerts[0].x-wlVerts[2].x)+(wlVerts[0].z-wlVerts[2].z)*(wlVerts[0].z-wlVerts[2].z));
		//if ((d[0] < dAB) && (d[1] < dAB)) // test if the intersection is on the wall
		//{
		//    d[0] = -d[0]; // if yes, the left distcance must be negative for texcoord
		//}
		// test if the intersection is on the wall
		if ((wlVerts[0].x < inter.x && wlVerts[2].x > inter.x) ||
		     (wlVerts[0].x > inter.x && wlVerts[2].x < inter.x) ||
		     (wlVerts[0].z < inter.z && wlVerts[2].z > inter.z) ||
		     (wlVerts[0].z > inter.z && wlVerts[2].z < inter.z))
		{
			d[0] = -d[0]; // if yes, the left distcance must be negative for texcoord
		}
		d[2] = d[1]; d[3] = d[0];
#ifdef DL_HIGH_QUALITY
		s = 0.5f / DL_RADIUS(j);
#else
		s = 0.5f / sqrtf(DL_SQRRADIUS(j)-dist_p2d);
#endif
		for (i = 0; i < 4; i++)
		{
			wlVerts[i].s = (float)(0.5f + d[i]*s);
			wlVerts[i].t = (float)(0.5f + (wlVerts[i].y-LIGHT_POS(j).y)*s*1.2f);
		}

		HWR_SetLight();

		Surf.PolyColor.rgba = LONG(dynlights->p_lspr[j]->dynamic_color);
#ifdef DL_HIGH_QUALITY
		Surf.PolyColor.s.alpha = (UINT8)((1-dist_p2d/DL_SQRRADIUS(j))*Surf.PolyColor.s.alpha);
#endif
#ifdef CORONASRESTORED
        if (P_MobjWasRemoved(dynlights->mo[j]))
			return;
#endif
		if (!dynlights->mo[j]->state)
			return;
		// next state is null so fade out with alpha
		if (dynlights->mo[j]->state->nextstate == S_NULL)
			Surf.PolyColor.s.alpha = (UINT8)(((float)dynlights->mo[j]->tics/(float)dynlights->mo[j]->state->tics)*Surf.PolyColor.s.alpha);

		HWD.pfnDrawPolygon (&Surf, wlVerts, 4, LIGHTMAPFLAGS);

	} // end for (j = 0; j < dynlights->nb; j++)
}

// --------------------------------------------------------------------------
// calcul du dynamic lighting sur le sol
// clVerts contient les coords du sol avec le mlook (up/down)
// --------------------------------------------------------------------------
void HWR_PlaneLighting(FOutVector *clVerts, int nrClipVerts)
{
	int     i, j;
	FOutVector p1,p2;

	p1.z = FIXED_TO_FLOAT(hwbbox[BOXTOP   ]);
	p1.x = FIXED_TO_FLOAT(hwbbox[BOXLEFT  ]);
	p2.z = FIXED_TO_FLOAT(hwbbox[BOXBOTTOM]);
	p2.x = FIXED_TO_FLOAT(hwbbox[BOXRIGHT ]);
	p2.y = clVerts[0].y;
	p1.y = clVerts[0].y;

	for (j = 0; j < dynlights->nb; j++)
	{
		FSurfaceInfo    Surf;
		float           dist_p2d, s;

		// BP: The kickass Optimization: check if light touch bounding box
		if (SphereTouchBBox3D(&p1, &p2, &dynlights->position[j], DL_RADIUS(j))==false)
			continue;
		// backface cull
		//Hurdler: doesn't work with new TANDL code
		if ((clVerts[0].y > atransform.z)       // true mean it is a ceiling false is a floor
			 ^ (LIGHT_POS(j).y < clVerts[0].y)) // true mean light is down plane false light is up plane
			 continue;
		dist_p2d = (clVerts[0].y-LIGHT_POS(j).y);
		dist_p2d *= dist_p2d;
		// done in SphereTouchBBox3D
		//if (dist_p2d >= DL_SQRRADIUS(j))
		//    continue;

#ifdef DL_HIGH_QUALITY
		s = 0.5f / DL_RADIUS(j);
#else
		s = 0.5f / sqrtf(DL_SQRRADIUS(j)-dist_p2d);
#endif
		for (i = 0; i < nrClipVerts; i++)
		{
			clVerts[i].s = 0.5f + (clVerts[i].x-LIGHT_POS(j).x)*s;
			clVerts[i].t = 0.5f + (clVerts[i].z-LIGHT_POS(j).z)*s*1.2f;
		}

		HWR_SetLight();

		Surf.PolyColor.rgba = LONG(dynlights->p_lspr[j]->dynamic_color);
#ifdef DL_HIGH_QUALITY
		Surf.PolyColor.s.alpha = (unsigned char)((1 - dist_p2d/DL_SQRRADIUS(j))*Surf.PolyColor.s.alpha);
#endif
		if (!dynlights->mo[j]->state)
			return;
		// next state is null so fade out with alpha
		if ((dynlights->mo[j]->state->nextstate == S_NULL))
			Surf.PolyColor.s.alpha = (unsigned char)(((float)dynlights->mo[j]->tics/(float)dynlights->mo[j]->state->tics)*Surf.PolyColor.s.alpha);

		HWD.pfnDrawPolygon (&Surf, clVerts, nrClipVerts, LIGHTMAPFLAGS);

	} // end for (j = 0; j < dynlights->nb; j++)
}


static lumpnum_t coronalumpnum = LUMPERROR;
#ifndef NEWCORONAS
// --------------------------------------------------------------------------
// coronas lighting
// --------------------------------------------------------------------------
void HWR_DoCoronasLighting(FOutVector *outVerts, gr_vissprite_t *spr)
{
	light_t   *p_lspr;

	if (coronalumpnum == LUMPERROR)
		return;

	//CONS_Debug(DBG_RENDER, "sprite (type): %d (%s)\n", spr->type, sprnames[spr->type]);
	p_lspr = t_lspr[spr->mobj->sprite];
	if ((spr->mobj->state>=&states[S_EXPLODE1] && spr->mobj->state<=&states[S_EXPLODE3])
	 || (spr->mobj->state>=&states[S_FATSHOTX1] && spr->mobj->state<=&states[S_FATSHOTX3]))
	{
		p_lspr = &lspr[ROCKETEXP_L];
	}

	if (cv_grcoronas.value && (p_lspr->type & CORONA_SPR))
	{ // it's an object which emits light
		FOutVector      light[4];
		FSurfaceInfo    Surf;
		float           cx = 0.0f, cy = 0.0f, cz = 0.0f; // gravity center
		float           size;
		UINT8           i;

		switch (p_lspr->type)
		{
			case LIGHT_SPR:
				size  = p_lspr->corona_radius  * ((outVerts[0].z+120.0f)/950.0f); // d'ou vienne ces constante ?
				break;
			case ROCKET_SPR:
				p_lspr->corona_color = (((M_RandomByte()>>1)&0xff)<<24)|0x0040ff;
				// don't need a break
			case CORONA_SPR:
				size  = p_lspr->corona_radius  * ((outVerts[0].z+60.0f)/100.0f); // d'ou vienne ces constante ?
				break;
			default:
				I_Error("HWR_DoCoronasLighting: unknow light type %d",p_lspr->type);
				return;
		}
		if (size > p_lspr->corona_radius)
			size = p_lspr->corona_radius;
		size *= FIXED_TO_FLOAT(cv_grcoronasize.value<<1);

		// compute position doing average
		for (i = 0; i < 4; i++)
		{
			cx += outVerts[i].x;
			cy += outVerts[i].y;
			cz += outVerts[i].z;
		}
		cx /= 4.0f;  cy /= 4.0f;  cz /= 4.0f;

		// more realistique corona !
		if (cz >= 255*8+250)
			return;
		Surf.PolyColor.rgba = p_lspr->corona_color;
		if (cz > 250.0f)
			Surf.PolyColor.s.alpha = 0xff-((int)cz-250)/8;
		else
			Surf.PolyColor.s.alpha = 0xff;

		// do not be hide by sprite of the light itself !
		cz = cz - 2.0f;

		// Bp; je comprend pas, ou est la rotation haut/bas ?
		//     tu ajoute un offset a y mais si la tu la reguarde de haut
		//     sa devrais pas marcher ... comprend pas :(
		//     (...) bon je croit que j'ai comprit il est tout pourit le code ?
		//           car comme l'offset est minime sa ce voit pas !
		light[0].x = cx-size;  light[0].z = cz;
		light[0].y = cy-size*1.33f+p_lspr->light_yoffset;
		light[0].s = 0.0f;   light[0].t = 0.0f;

		light[1].x = cx+size;  light[1].z = cz;
		light[1].y = cy-size*1.33f+p_lspr->light_yoffset;
		light[1].s = 1.0f;   light[1].t = 0.0f;

		light[2].x = cx+size;  light[2].z = cz;
		light[2].y = cy+size*1.33f+p_lspr->light_yoffset;
		light[2].s = 1.0f;   light[2].t = 1.0f;

		light[3].x = cx-size;  light[3].z = cz;
		light[3].y = cy+size*1.33f+p_lspr->light_yoffset;
		light[3].s = 0.0f;   light[3].t = 1.0f;

		HWR_GetPic(coronalumpnum);  /// \todo use different coronas

		HWD.pfnDrawPolygon (&Surf, light, 4, PF_Modulated | PF_Additive | PF_Clip | PF_Corona | PF_NoDepthTest);
	}
}
#endif

#ifdef NEWCORONAS
// use the lightlist of the frame to draw the coronas at the top of everythink
void HWR_DrawCoronas(void)
{
	int       j;

	if (!cv_grcoronas.value || dynlights->nb <= 0 || coronalumpnum == LUMPERROR)
		return;

	HWR_GetPic(coronalumpnum);  /// \todo use different coronas
	for (j = 0;j < dynlights->nb;j++)
	{
		FOutVector      light[4];
		FSurfaceInfo    Surf;
		float           cx = LIGHT_POS(j).x;
		float           cy = LIGHT_POS(j).y;
		float           cz = LIGHT_POS(j).z; // gravity center
		float           size;
		light_t         *p_lspr = dynlights->p_lspr[j];

		// it's an object which emits light
		if (!(p_lspr->type & CORONA_SPR))
			continue;

		transform(&cx,&cy,&cz);

		// more realistique corona !
		if (cz >= 255*8+250)
			continue;
		Surf.PolyColor.rgba = p_lspr->corona_color;
		if (cz > 250.0f)
			Surf.PolyColor.s.alpha = (UINT8)(0xff-(UINT8)(((int)cz-250)/8));
		else
			Surf.PolyColor.s.alpha = 0xff;

		switch (p_lspr->type)
		{
			case LIGHT_SPR:
				size  = p_lspr->corona_radius  * ((cz+120.0f)/950.0f); // d'ou vienne ces constante ?
				break;
			case ROCKET_SPR:
				Surf.PolyColor.s.alpha = (UINT8)((M_RandomByte()>>1)&0xff);
				// don't need a break
			case CORONA_SPR:
				size  = p_lspr->corona_radius  * ((cz+60.0f)/100.0f); // d'ou vienne ces constante ?
				break;
			default:
				I_Error("HWR_DoCoronasLighting: unknow light type %d",p_lspr->type);
				continue;
		}
		if (size > p_lspr->corona_radius)
			size = p_lspr->corona_radius;
		size = (float)(FIXED_TO_FLOAT(cv_grcoronasize.value<<1)*size);

		// put light little forward the sprite so there is no
		// z-buffer problem (coplanar polygons)
		// BP: use PF_Decal do not help :(
		cz = cz - 5.0f;

		light[0].x = cx-size;  light[0].z = cz;
		light[0].y = cy-size*1.33f;
		light[0].s = 0.0f;   light[0].t = 0.0f;

		light[1].x = cx+size;  light[1].z = cz;
		light[1].y = cy-size*1.33f;
		light[1].s = 1.0f;   light[1].t = 0.0f;

		light[2].x = cx+size;  light[2].z = cz;
		light[2].y = cy+size*1.33f;
		light[2].s = 1.0f;   light[2].t = 1.0f;

		light[3].x = cx-size;  light[3].z = cz;
		light[3].y = cy+size*1.33f;
		light[3].s = 0.0f;   light[3].t = 1.0f;

		HWD.pfnDrawPolygon (&Surf, light, 4, PF_Modulated | PF_Additive | PF_Clip | PF_NoDepthTest | PF_Corona);
	}
}
#endif

// --------------------------------------------------------------------------
// Remove all the dynamic lights at eatch frame
// --------------------------------------------------------------------------
void HWR_ResetLights(void)
{
	memset(dynlights,0,sizeof(dynlights_t));
}

// --------------------------------------------------------------------------
// Change view, thus change lights (splitscreen)
// --------------------------------------------------------------------------
void HWR_SetLights(int viewnumber)
{
	dynlights = &view_dynlights[viewnumber];
}

// --------------------------------------------------------------------------
// Add a light for dynamic lighting
// The light position is already transformed execpt for mlook
// --------------------------------------------------------------------------
void HWR_DL_AddLight(gr_vissprite_t *spr, GLPatch_t *patch)
{
	light_t   *p_lspr;


	// uncapped/interpolation
	interpmobjstate_t interp = {0};

	if (R_UsingFrameInterpolation())
	{
		R_InterpolateMobjState(spr->mobj, rendertimefrac, &interp);
	}
	else
	{
		R_InterpolateMobjState(spr->mobj, FRACUNIT, &interp);
	}

	//Hurdler: moved here because it's better;-)
	(void)patch;
	if (!cv_grdynamiclighting.value)
		return;

	if (!spr->mobj)
#ifdef PARANOIA
		I_Error("vissprite without mobj !!!");
#else
		return;
#endif

	// check if sprite contain dynamic light
	p_lspr = t_lspr[spr->mobj->sprite];
	if ((p_lspr->type&DYNLIGHT_SPR)
	  && ((p_lspr->type != LIGHT_SPR) || cv_grstaticlighting.value)
	  && (dynlights->nb < DL_MAX_LIGHT)

	  && spr->mobj->state)
	{
		LIGHT_POS(dynlights->nb).x = FIXED_TO_FLOAT(interp.x);
		LIGHT_POS(dynlights->nb).y = FIXED_TO_FLOAT(interp.z)+FIXED_TO_FLOAT(spr->mobj->height>>1)+p_lspr->light_yoffset;
		LIGHT_POS(dynlights->nb).z = FIXED_TO_FLOAT(interp.y);

		P_SetTarget(&dynlights->mo[dynlights->nb], spr->mobj);

		dynlights->p_lspr[dynlights->nb] = p_lspr;

		dynlights->nb++;
	}
}

static GLPatch_t lightmappatch;

void HWR_InitLight(void)
{
	size_t i;

	// precalculate sqr radius
	for (i = 0;i < NUMLIGHTS;i++)
		lspr[i].dynamic_sqrradius = lspr[i].dynamic_radius*lspr[i].dynamic_radius;

	lightmappatch.mipmap->downloaded = false;
	coronalumpnum = W_CheckNumForName("CORONA");
}

// -----------------+
// HWR_SetLight     : Download a disc shaped alpha map for rendering fake lights
// -----------------+
static void HWR_SetLight(void)
{
	int    i, j;

	if (!lightmappatch.mipmap->downloaded && !lightmappatch.mipmap->grInfo.data)
	{

		UINT16 *Data = Z_Malloc(129*128*sizeof (UINT16), PU_HWRCACHE, &lightmappatch.mipmap->grInfo.data);

		for (i = 0; i < 128; i++)
		{
			for (j = 0; j < 128; j++)
			{
				int pos = ((i-64)*(i-64))+((j-64)*(j-64));
				if (pos <= 63*63)
					Data[i*128+j] = (UINT16)(((UINT8)(255-(4*(float)sqrt((float)pos)))) << 8 | 0xff);
				else
					Data[i*128+j] = 0;
			}
		}
		lightmappatch.mipmap->grInfo.format = GR_TEXFMT_ALPHA_INTENSITY_88;

		lightmappatch.width = 128;
		lightmappatch.height = 128;
		lightmappatch.mipmap->width = 128;
		lightmappatch.mipmap->height = 128;
		lightmappatch.mipmap->grInfo.smallLodLog2 = GR_LOD_LOG2_128;
		lightmappatch.mipmap->grInfo.largeLodLog2 = GR_LOD_LOG2_128;
		lightmappatch.mipmap->grInfo.aspectRatioLog2 = GR_ASPECT_LOG2_1x1;
		lightmappatch.mipmap->flags = 0; //TF_WRAPXY; // DEBUG: view the overdraw !
	}
	HWD.pfnSetTexture(lightmappatch.mipmap);

	// The system-memory data can be purged now.
	Z_ChangeTag(lightmappatch.mipmap->grInfo.data, PU_HWRCACHE_UNLOCKED);
}

//**********************************************************
// Hurdler: new code for faster static lighting and and T&L
//**********************************************************

#ifdef STATICLIGHT
// is this really necessary?
static sector_t *lgr_backsector;
static seg_t *lgr_curline;
#endif

// p1 et p2 c'est le deux bou du seg en float
static inline void HWR_BuildWallLightmaps(FVector *p1, FVector *p2, int lighnum, seg_t *line)
{
	lightmap_t *lp;

	// (...) calcul presit de la projection et de la distance

//	if (dist_p2d >= DL_SQRRADIUS(lightnum))
//		return;

	// (...) attention faire le backfase cull histoir de faire mieux que Q3 !

	(void)lighnum;
	(void)p1;
	(void)p2;
	lp = malloc(sizeof (*lp));
	lp->next = line->lightmaps;
	line->lightmaps = lp;

	// (...) encore des b�calcul bien lourd et on stock tout sa dans la lightmap
}

#ifdef STATICLIGHT
static void HWR_AddLightMapForLine(int lightnum, seg_t *line)
{
	/*
	int                 x1;
	int                 x2;
	angle_t             angle1;
	angle_t             angle2;
	angle_t             span;
	angle_t             tspan;
	*/
	FVector             p1,p2;

	lgr_curline = line;
	lgr_backsector = line->backsector;

	// Reject empty lines used for triggers and special events.
	// Identical floor and ceiling on both sides,
	//  identical light levels on both sides,
	//  and no middle texture.
/*
	if ( lgr_backsector->ceilingpic == gr_frontsector->ceilingpic
	  && lgr_backsector->floorpic == gr_frontsector->floorpic
	  && lgr_backsector->lightlevel == gr_frontsector->lightlevel
	  && lgr_curline->sidedef->midtexture == 0)
	{
		return;
	}
*/

	p1.y = FIXED_TO_FLOAT(lgr_curline->v1->y);
	p1.x = FIXED_TO_FLOAT(lgr_curline->v1->x);
	p2.y = FIXED_TO_FLOAT(lgr_curline->v2->y);
	p2.x = FIXED_TO_FLOAT(lgr_curline->v2->x);

	// check bbox of the seg
//	if (CircleTouchBBox(&p1, &p2, &LIGHT_POS(lightnum), DL_RADIUS(lightnum))==false)
//		return;

	HWR_BuildWallLightmaps(&p1, &p2, lightnum, line);
}

/// \todo see what HWR_AddLine does
static void HWR_CheckSubsector(size_t num, fixed_t *bbox)
{
	int         count;
	seg_t       *line;
	subsector_t *sub;
	FVector     p1,p2;
	int         lightnum;

	p1.y = FIXED_TO_FLOAT(bbox[BOXTOP   ]);
	p1.x = FIXED_TO_FLOAT(bbox[BOXLEFT  ]);
	p2.y = FIXED_TO_FLOAT(bbox[BOXBOTTOM]);
	p2.x = FIXED_TO_FLOAT(bbox[BOXRIGHT ]);


	if (num < numsubsectors)
	{
		sub = &subsectors[num];         // subsector
		for (lightnum = 0; lightnum < dynlights->nb; lightnum++)
		{
//			if (CircleTouchBBox(&p1, &p2, &LIGHT_POS(lightnum), DL_RADIUS(lightnum))==false)
//				continue;

			count = sub->numlines;          // how many linedefs
			line = &segs[sub->firstline];   // first line seg
			while (count--)
			{
				HWR_AddLightMapForLine (lightnum, line);       // compute lightmap
				line++;
			}
		}
	}
}


// --------------------------------------------------------------------------
// Hurdler: this adds lights by mobj.
// --------------------------------------------------------------------------
static void HWR_AddMobjLights(mobj_t *thing)
{
	if (t_lspr[thing->sprite]->type & CORONA_SPR)
	{
		LIGHT_POS(dynlights->nb).x = FIXED_TO_FLOAT(thing->x);
		LIGHT_POS(dynlights->nb).y = FIXED_TO_FLOAT(thing->z) + t_lspr[thing->sprite]->light_yoffset;
		LIGHT_POS(dynlights->nb).z = FIXED_TO_FLOAT(thing->y);

		dynlights->p_lspr[dynlights->nb] = t_lspr[thing->sprite];

		dynlights->nb++;
		if (dynlights->nb > DL_MAX_LIGHT)
			dynlights->nb = DL_MAX_LIGHT;
	}
}

//Hurdler: The goal of this function is to walk through all the bsp starting
//         on the top.
//         We need to do that to know all the lights in the map and all the walls
static void HWR_ComputeLightMapsInBSPNode(int bspnum, fixed_t *bbox)
{
	if (bspnum & NF_SUBSECTOR) // Found a subsector?
	{
		if (bspnum == -1)
			HWR_CheckSubsector(0, bbox);  // probably unecessary: see boris' comment in hw_bsp
		else
			HWR_CheckSubsector(bspnum&(~NF_SUBSECTOR), bbox);
		return;
	}
	HWR_ComputeLightMapsInBSPNode(nodes[bspnum].children[0], nodes[bspnum].bbox[0]);
	HWR_ComputeLightMapsInBSPNode(nodes[bspnum].children[1], nodes[bspnum].bbox[1]);
}

static void HWR_SearchLightsInMobjs(void)
{
	thinker_t *         th;
	//mobj_t *            mobj;

	// search in the list of thinkers
	for (th = thinkercap.next; th != &thinkercap; th = th->next)
	{
		// a mobj ?
		if (th->function.acp1 == (actionf_p1)P_MobjThinker)
			HWR_AddMobjLights((mobj_t *)th);
	}
}
#endif

//
// HWR_CreateStaticLightmaps()
//
void HWR_CreateStaticLightmaps(int bspnum)
{
#ifdef STATICLIGHT
	CONS_Debug(DBG_RENDER, "HWR_CreateStaticLightmaps\n");

	dynlights->nb = 0;

	// First: Searching for lights
	// BP: if i was you, I will make it in create mobj since mobj can be create
	//     at runtime now with fragle scipt
	HWR_SearchLightsInMobjs();
	CONS_Debug(DBG_RENDER, "%d lights found\n", dynlights->nb);

	// Second: Build all lightmap for walls covered by lights
	validcount++; // to be sure
	HWR_ComputeLightMapsInBSPNode(bspnum, NULL);

	dynlights->nb = 0;
#else
	(void)bspnum;
#endif
}

/**
 \todo

  - Les coronas ne sont pas g�er avec le nouveau systeme, seul le dynamic lighting l'est
  - calculer l'offset des coronas au chargement du level et non faire la moyenne
	au moment de l'afficher
	 BP: euh non en fait il faux encoder la position de la light dans le sprite
		 car c'est pas focement au mileux de plus il peut en y avoir plusieur (chandelier)
  - changer la comparaison pour l'affichage des coronas (+ un epsilon)
	BP: non non j'ai trouver mieux :) : lord du AddSprite tu rajoute aussi la coronas
		dans la sprite list ! avec un z de epsilon (attention au ZCLIP_PLANE) et donc on
		l'affiche en dernier histoir qu'il puisse etre cacher par d'autre sprite :)
		Bon fait metre pas mal de code special dans hwr_project sprite mais sa vaux le
		coup
  - gerer dynamic et static : retenir le nombre de lightstatic et clearer toute les
		light > lightstatic (les dynamique) et les lightmap correspondant dans les segs
		puit refaire une passe avec le code si dessus mais rien que pour les dynamiques
		(tres petite modification)
  - finalement virer le hack splitscreen, il n'est plus necessaire !
*/
#endif
#endif // HWRENDER
