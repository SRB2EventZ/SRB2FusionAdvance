// SONIC ROBO BLAST 2
//-----------------------------------------------------------------------------
// Copyright (C) 2018-2023 by Louis-Antoine de Moulins de Rochefort.
//
// This program is free software distributed under the
// terms of the GNU General Public License, version 2.
// See the 'LICENSE' file for more details.
//-----------------------------------------------------------------------------
/// \file  snake.h
/// \brief Snake minigame for the download screen.

#ifndef __SNAKE__
#define __SNAKE__

#include "doomtype.h"

#define SNAKE_SPEED 5

#define SNAKE_NUM_BLOCKS_X 20
#define SNAKE_NUM_BLOCKS_Y 10
#define SNAKE_BLOCK_SIZE 12
#define SNAKE_BORDER_SIZE 12

#define SNAKE_MAP_WIDTH  (SNAKE_NUM_BLOCKS_X * SNAKE_BLOCK_SIZE)
#define SNAKE_MAP_HEIGHT (SNAKE_NUM_BLOCKS_Y * SNAKE_BLOCK_SIZE)

#define SNAKE_LEFT_X ((BASEVIDWIDTH - SNAKE_MAP_WIDTH) / 2 - SNAKE_BORDER_SIZE)
#define SNAKE_RIGHT_X (SNAKE_LEFT_X + SNAKE_MAP_WIDTH + SNAKE_BORDER_SIZE * 2 - 1)
#define SNAKE_BOTTOM_Y (BASEVIDHEIGHT - 48)
#define SNAKE_TOP_Y (SNAKE_BOTTOM_Y - SNAKE_MAP_HEIGHT - SNAKE_BORDER_SIZE * 2 + 1)

typedef struct snake_s
{
	boolean paused;
	boolean pausepressed;
	tic_t time;
	tic_t nextupdate;
	boolean gameover;
    UINT32 background;

	UINT8 snakedir;
	UINT8 snakeprevdir;

	UINT16 snakelength;
	UINT8 snakex[SNAKE_NUM_BLOCKS_X * SNAKE_NUM_BLOCKS_Y];
	UINT8 snakey[SNAKE_NUM_BLOCKS_X * SNAKE_NUM_BLOCKS_Y];
	UINT8 snakecolor[SNAKE_NUM_BLOCKS_X * SNAKE_NUM_BLOCKS_Y];

	UINT8 applex;
	UINT8 appley;
	UINT8 applecolor;
} snake_t;

extern snake_t *snake;

void Snake_Initialise(void);
void Snake_Handle(void);
void Snake_Draw(void);
void Snake_InitVars(void);

extern consvar_t cv_snake;

#endif
