/*
  Tilt-n-Roll, copyright 2007 Greg Kennedy.  All rights reserved.
  
    Tilt-n-Roll is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    Tilt-n-Roll is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Tilt-n-Roll; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA  
*/

/* common.h */

// Threading used to be supported by enabling this define, but
// DON'T DO IT NOW!  I've done a lot of work without adding appropriate
// mutex safety stuff so it is likely to crash your game.
//#define THREADED

#ifndef COMMON_H
#define COMMON_H

#ifdef UNIMOTION
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_mixer.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <SDL_main.h>
#include "unimotion.h"
#else
#include <SDL/SDL.h>
#include <SDL/SDL_opengl.h>
#include <SDL/SDL_mixer.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_ttf.h>
#endif

#include <math.h>
#include <time.h>
//#include <zlib.h>
#include <dirent.h>

#define NUM_SFX 19

#ifndef max
#define max(a,b) (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b) (((a) < (b)) ? (a) : (b))
#endif

#ifndef USETEX
#define USETEX(a) if (a != last_used_tex) { glBindTexture(GL_TEXTURE_2D, a); last_used_tex = a; }
#endif

struct s_level
{
       char name[30];
       int num;
       unsigned char tile[64][64];
       unsigned char timelimit;
       s_level *next;
       s_level *prev;
};

struct s_levelset
{
       char name[50];
       char author[50];
       char version[10];
       int tileset;
       s_level *top;
};

SDL_Surface *loadImage(const char * filename);

#endif
