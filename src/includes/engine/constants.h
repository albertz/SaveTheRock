/*  Save The Rock v1.0
    Copyright (C) 2010 Alexander Dzhoganov
    See game.cpp for full copyright notice. */


#ifndef __CONSTANTS_H
#define __CONSTANTS_H

/*
 Copyright 2010 Alexander Dzhoganov
*/

#define GL_TEXTURE_RECTANGLE_EXT 0x84F5
#define GL_ARRAY_BUFFER 0x8892
#define GL_STREAM_DRAW 0x88E0
#define GL_DYNAMIC_DRAW 0x88E8
#define GL_STATIC_DRAW 0x88E4

// ENGINE CONSTANTS //

#define MAX_TEXT_BUFFER 1024

// FILESYSTEM CONSTANTS
#define FILESYSTEM_ROOT "..\\media\\"
#define FILESYSTEM_LEVELS_DIR "levels\\"
#define FILESYSTEM_SOUNDS_DIR "sounds\\"
#define FILESYSTEM_MUSIC_DIR "music\\"

// RENDERER CONSTANTS
#ifdef _WIN32
#define FONT_PATH "..\\media\\font.tga"
#else
#define FONT_PATH "../media/font.tga"
#endif
#define FONT_SIZE 64
#define FONT_TEXTURE_SIZE 1024.f
#define VERTEX_ARRAY_SIZE 64000
#define GLOW_ITERATIONS 5
#define MAX_TEXTURES 128
#define MAX_TEXTURE_NAME 32
#define MAX_EFFECTS_N 32
#define MAX_VIDEO_MODES 128
#define MAX_DISPLAYED_VIDEO_MODES 14

// GUI constants
#define CURSOR_SIZE 32
#define FADEIN_MULTIPLIER 4
#define FADEOUT_MULTIPLIER 4
#define MENU_FONT 0
#define MENU_COLOR color(0.4f, 0.4f, 0.4f, 1.f)
#define MENU_TEXT_COLOR color(1.f, 1.f, 1.f, 1.f)
#define MENU_TEXT_COLOR_HOVER color(0.f, 0.f, 0.f, 1.f)
#define MENU_TEXT_COLOR_SELECTED color(0.7f, 0.7f, 0.7f, 1.f)
#define LEVEL_TEXT_COLOR color(0.f, 0.f, 0.f, 0.f)
#define INPUTBOX_COLOR color(0.3f, 0.3f, 0.3f, 1.f)
#define INPUTBOX_TEXT_COLOR color(1.f, 1.f, 1.f, 1.f)
#define INPUTBOX_CHAR_LIMIT 32
#define INFOBOX_DEFAULT_TIMEOUT 4.f
#define INFOBOX_NO_TIMEOUT -1.f
#define MAX_CUTSCENE_FRAMES 32

// LEVEL CONSTANTS
#define LEVEL_SIZE_X 210
#define LEVEL_SIZE_Y 210
#define MAX_CHECKPOINTS 64
#define MAX_NAMED_POSITIONS 256
#define NAMED_POSITION_LEN 256
// PPT - pixels per tile
#define PPT_X 32
#define PPT_Y 32
#define BG_COLOR 0.25f, 0.25f, 0.25f, 1.f
#define TILE_FACE_COLOR color(0.4f, 0.4f, 0.4f, 1.f)
#define TILE_COLOR 0.5f, 0.5f, 0.5f, 1.f
#define CULLING_EPSILON 1.5f
#define NP_SEARCH_BOX_X 16.f
#define NP_SEARCH_BOX_Y 16.f

// SPRITE CONSTANTS
#define COLOR_KEY_R 255
#define COLOR_KEY_G 0
#define COLOR_KEY_B 255

// COLLISION AND PHYSICS CONSTANTS
#define GRID_SIZE 100
#define MAX_PHYS_OBJECTS 512
#define PHYSICS_MAX_VELOCITY (26.f*26.f)
#define GRAVITY_Y -1500.f
#define P_INF 9999999.f
#define N_INF -9999999.f
#define COLLISION_EPSILON 1.f
#define PHYSICS_TIME_STEP 0.01f
#define PHYS_ITERATIONS 3
#define DEFAULT_PHYS_ITERATIONS PHYS_ITERATIONS
#define MIN_PHYS_ITERATIONS PHYS_ITERATIONS
#define MAX_PHYS_ITERATIONS PHYS_ITERATIONS
#define MIN_VELOCITY 0.001f
#define MIN_VELOCITY_SQUARED MIN_VELOCITY*MIN_VELOCITY
#define RAGDOLL_FRICTION_COEFF 0.009f
#define HERO_FRICTION_COEFF 0.06f
#define MAX_PROJECTILE_LIFETIME 5.f
#define PHYSICS_SLEEP_EPSILON 250.f

// INPUT HANDLING
#define KEYMAP_SIZE 255
#define MAX_CAMERA_MOVE_SPEED 2000.f

// SOUND
#define MAX_SOUNDS_N 64
#define MAX_SOUND_NAME 32
#define MUSIC_TRACK_SWITCH_TIME 120.f

// GAME CONSTANTS

#define MAX_VIEW_RANGE 800.f
#define MAX_VIEW_RANGE_SQ MAX_VIEW_RANGE*MAX_VIEW_RANGE

// Some helper functions
#define deg2rad(x) x/57.2957795
#define sign(x) ( (x) < 0 ? (-1) : (1) )
#define GL_OFFSET(x) ((char *)NULL + (x))

// debugging helpers
#define coutA cout << "A" << endl;
#define coutB cout << "B" << endl;
#define coutC cout << "C" << endl;
#define coutD cout << "D" << endl;
#define cout(x) cout << "x" << endl

const int BITS_IN_INT = sizeof(int)*8;
extern int w_width;
extern int w_height;

#define N_PI 3.141592653589793f
#define N_E 2.718281828f

/* STRING MESSAGES */
#define GNU_COPYRIGHT_NOTICE "Save The Rock!\n\
Copyright (C) 2010 Alexander Dzhoganov\n\n\
This program is free software: you can redistribute it and/or modify\n\
it under the terms of the GNU General Public License as published by\n\
the Free Software Foundation, either version 3 of the License, or\n\
(at your option) any later version.\n\n\
This program is distributed in the hope that it will be useful\n\
but WITHOUT ANY WARRANTY; without even the implied warranty of\n\
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n\
GNU General Public License for more details.\n\
You should have received a copy of the GNU General Public License\n\
along with this program.  If not, see <www.gnu.org/licenses/>."

#define LEVELEDITOR_WELCOME_MSG "Use the right mouse button to cancel actions.\nUse the arrow keys to move the camera.\nto bring up the editing menu.\nClick anywhere with the right mouse button\nThis is the level editor.\nWelcome!"

#endif
