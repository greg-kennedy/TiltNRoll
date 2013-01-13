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

/* game.h */
#ifndef GAME_H_
#define GAME_H_

#include "common.h"

void init_game();
void destroy_game();
void draw_game();
bool handle_game();

int simulate_game(void *);

#endif
