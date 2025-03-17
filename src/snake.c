// SONIC ROBO BLAST 2
//-----------------------------------------------------------------------------
// Copyright (C) 2018-2023 by Louis-Antoine de Moulins de Rochefort.
//
// This program is free software distributed under the
// terms of the GNU General Public License, version 2.
// See the 'LICENSE' file for more details.
//-----------------------------------------------------------------------------
/// \file  snake.c
/// \brief Snake minigame for the download screen.

#include "snake.h"
#include "g_input.h"
#include "m_random.h"
#include "s_sound.h"
#include "screen.h"
#include "v_video.h"
#include "w_wad.h"
#include "z_zone.h"


consvar_t cv_snake = {"snake", "Off", CV_SAVE, CV_OnOff, NULL, 0, NULL, NULL, 0, 0, NULL};

// Added some extra backgrounds :p - chromaticpipe
const char *snake_backgrounds[] = {

    // Greenflower Zone
    "GFZFLR01",
    "GFZFLR09",
    "GFZFLR02",
    "GFZTIL01",

    // Techno Hill Zone
    "THZFLR30",
    "THZFLR23",
    "THZFLR01", 
    "THZFLR27",

    // Deep Sea Zone
    "DSZRCKF1",
    "DSZFLR11",
    "DSZFLR13",
    "DSZFLR19",

    // Castle Eggman Zone
    "CEZFLR01",
    "CEZFLR06",
    "CEZFLR09",
    "DMAFLAT1",

    // Arid Canyon Zone
    "ACZFLR01",
    "ACZFLR24",
    "ACZFLR10",
    "ACZFLR26",

    // Red Volcano Zone
	"MMFLRB4",
	"RVZWALF1",
	"RVZWALF4",
	"RVZWALF5",

    // Egg Rock Zone
    "ERZRCKF1",
    "ERZRCKF7",
    "ERZBLUF4",
    "ERZREDF3",
};

snake_t *snake = NULL;

void Snake_InitVars(void)
{
	CV_RegisterVar(&cv_snake);
}

void Snake_Initialise(void)
{
	if (!snake)
		snake = malloc(sizeof(snake_t));


	snake->paused = false;
	snake->pausepressed = false;
	snake->time = 0;
	snake->nextupdate = SNAKE_SPEED;
	snake->gameover = false;

	snake->snakedir = 0;
	snake->snakeprevdir = snake->snakedir;

	snake->snakelength = 1;
	snake->snakex[0] = rand() % SNAKE_NUM_BLOCKS_X;
	snake->snakey[0] = rand() % SNAKE_NUM_BLOCKS_Y;
	snake->snakecolor[0] = rand() % 256;

	snake->applex = rand() % SNAKE_NUM_BLOCKS_X;
	snake->appley = rand() % SNAKE_NUM_BLOCKS_Y;
	snake->applecolor = rand() % 256;
	snake->background = M_RandomKey(sizeof(snake_backgrounds) / sizeof(*snake_backgrounds));
}

void Snake_Handle(void)
{
	UINT8 x, y;
	UINT16 i;

	// Handle retry
	if (snake->gameover && (PLAYER1INPUTDOWN(gc_jump) || gamekeydown[KEY_ENTER]))
	{
		Snake_Initialise();
		snake->pausepressed = true; // Avoid accidental pause on respawn
	}

	// Handle pause
	if (PLAYER1INPUTDOWN(gc_pause) || gamekeydown[KEY_ENTER])
	{
		if (!snake->pausepressed)
			snake->paused = !snake->paused;
		snake->pausepressed = true;
	}
	else
		snake->pausepressed = false;

	if (snake->paused)
		return;

	snake->time++;

	// Update direction
	if (gamekeydown[KEY_LEFTARROW])
	{
		if (snake->snakeprevdir != 2)
			snake->snakedir = 1;
	}
	else if (gamekeydown[KEY_RIGHTARROW])
	{
		if (snake->snakeprevdir != 1)
			snake->snakedir = 2;
	}
	else if (gamekeydown[KEY_UPARROW])
	{
		if (snake->snakeprevdir != 4)
			snake->snakedir = 3;
	}
	else if (gamekeydown[KEY_DOWNARROW])
	{
		if (snake->snakeprevdir != 3)
			snake->snakedir = 4;
	}

	snake->nextupdate--;
	if (snake->nextupdate)
		return;
	snake->nextupdate = SNAKE_SPEED;

	snake->snakeprevdir = snake->snakedir;

	if (snake->gameover)
		return;

	// Find new position
	x = snake->snakex[0];
	y = snake->snakey[0];
	switch (snake->snakedir)
	{
		case 1:
			if (x > 0)
				x--;
			else
				snake->gameover = true;
			break;
		case 2:
			if (x < SNAKE_NUM_BLOCKS_X - 1)
				x++;
			else
				snake->gameover = true;
			break;
		case 3:
			if (y > 0)
				y--;
			else
				snake->gameover = true;
			break;
		case 4:
			if (y < SNAKE_NUM_BLOCKS_Y - 1)
				y++;
			else
				snake->gameover = true;
			break;
	}
	if (snake->gameover)
		return;


	// Check collision with snake
	for (i = 1; i < snake->snakelength - 1; i++)
		if (x == snake->snakex[i] && y == snake->snakey[i])
		{
			snake->gameover = true;
			S_StartSound(NULL, sfx_lose);
			return;
		}

	// Check collision with apple
	if (x == snake->applex && y == snake->appley)
	{
		snake->snakelength++;
		snake->snakex[snake->snakelength - 1] = snake->snakex[snake->snakelength - 2];
		snake->snakey[snake->snakelength - 1] = snake->snakey[snake->snakelength - 2];
		snake->snakecolor[snake->snakelength - 1] = snake->applecolor;

		snake->applex = rand() % SNAKE_NUM_BLOCKS_X;
		snake->appley = rand() % SNAKE_NUM_BLOCKS_Y;
		snake->applecolor = rand() % 256;

		S_StartSound(NULL, sfx_s3k6b);
	}

	// Move
	for (i = snake->snakelength - 1; i > 0; i--)
	{
		snake->snakex[i] = snake->snakex[i - 1];
		snake->snakey[i] = snake->snakey[i - 1];
	}
	snake->snakex[0] = x;
	snake->snakey[0] = y;
}


void Snake_Draw(void)
{
	UINT16 i;

	// Background	
	V_DrawFlatFill(
		SNAKE_LEFT_X + SNAKE_BORDER_SIZE,
		SNAKE_TOP_Y  + SNAKE_BORDER_SIZE,
		SNAKE_MAP_WIDTH,
		SNAKE_MAP_HEIGHT,
		W_GetNumForName(snake_backgrounds[snake->background])
	);


	// Borders
	V_DrawFill(SNAKE_LEFT_X, SNAKE_TOP_Y, SNAKE_BORDER_SIZE + SNAKE_MAP_WIDTH, SNAKE_BORDER_SIZE, 242); // Top
	V_DrawFill(SNAKE_LEFT_X + SNAKE_BORDER_SIZE + SNAKE_MAP_WIDTH, SNAKE_TOP_Y, SNAKE_BORDER_SIZE, SNAKE_BORDER_SIZE + SNAKE_MAP_HEIGHT, 242); // Right
	V_DrawFill(SNAKE_LEFT_X + SNAKE_BORDER_SIZE, SNAKE_TOP_Y + SNAKE_BORDER_SIZE + SNAKE_MAP_HEIGHT, SNAKE_BORDER_SIZE + SNAKE_MAP_WIDTH, SNAKE_BORDER_SIZE, 242); // Bottom
	V_DrawFill(SNAKE_LEFT_X, SNAKE_TOP_Y + SNAKE_BORDER_SIZE, SNAKE_BORDER_SIZE, SNAKE_BORDER_SIZE + SNAKE_MAP_HEIGHT, 242); // Left

	// Apple
	V_DrawFill(
		SNAKE_LEFT_X + SNAKE_BORDER_SIZE + snake->applex * SNAKE_BLOCK_SIZE,
		SNAKE_TOP_Y  + SNAKE_BORDER_SIZE + snake->appley * SNAKE_BLOCK_SIZE,
		SNAKE_BLOCK_SIZE,
		SNAKE_BLOCK_SIZE,
		snake->applecolor
	);

	// Snake
	if (!snake->gameover || snake->time % 8 < 8 / 2) // Blink if game over
		for (i = 0; i < snake->snakelength; i++)
		{
			V_DrawFill(
				SNAKE_LEFT_X + SNAKE_BORDER_SIZE + snake->snakex[i] * SNAKE_BLOCK_SIZE,
				SNAKE_TOP_Y  + SNAKE_BORDER_SIZE + snake->snakey[i] * SNAKE_BLOCK_SIZE,
				SNAKE_BLOCK_SIZE,
				SNAKE_BLOCK_SIZE,
				snake->snakecolor[i]
			);
		}

	// Length
	V_DrawString(SNAKE_RIGHT_X + 4, SNAKE_TOP_Y, V_MONOSPACE, va("%u", snake->snakelength));
}

