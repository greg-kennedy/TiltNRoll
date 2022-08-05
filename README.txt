TILT-N-ROLL
GREG KENNEDY
VERSION 1.2
Released August 4, 2022
===

What's New

Version 1.2 corrects several bugs with the previous release:
* Ball rotation now matches its travel direction
* Friction calculation on floors is now fixed
* Bulldozers did not correctly respawn after death

and adds some other gameplay changes:
* Tweaks to power-ups and better indicators
* Some Tough levels have time limits adjusted
* Reduced file size of sounds and textures

===

Introduction

Tilt-n-Roll is a 3d OpenGL remake of the classic ball-rolling game.  You
control the angle of the level using the mouse, keyboard, or orientation of
your laptop.  By turning the world you can guide your marble over various
obstacles to the exit.

As part of Intel's 2007 Game Demo contest, this game is competitive in the
Best Game on the Go category:
* Best Game on the Go: Tilt-n-Roll offers support for the HDAPS system in
  laptops with tilt sensors.  This allows users to play the game using the
  computer itself as a controller - something which is obviously not possible
  with a big heavy desktop PC : )

Additional features of the game:
* The built-in level editor can be used to create and distribute levels with
  friends.
* The game is built on portable libraries and official releases run
  in Windows, Mac and Linux
* Source code released under the GNU GPL!

===

Controls

In-game:

If you have a Mac laptop or an IBM ThinkPad, you can tilt your laptop to tilt
the world.  This may require some special attention to computer settings: look
for an HDAPS option on the Windows control panel to enable support for the
sensors and to calibrate their sensitivity.
Additionally Mac users can 'pop' the laptop up and down to activate powerups.
Mac users should refer to README-osx.txt to get Unimotion support working.

For everyone else:
Arrow keys, mouse position - Tilt world
Escape - exit game
Any other key or mouse button - Use powerup

Editor:
Left click on menu - activate menu item
Scroll wheel over tiles list, F2 or F3 - scroll through tile list
Scroll wheel over map, F2 or F3 - zoom the map
Arrow keys - pan around the map
Left click - place tile
Right click - erase tile
PgUp, PgDown - set height of placed tiles
Home, End - adjust level time limit +/- 5 seconds

See the file tile-descriptions.txt for information on what each tile does.

===

Limitations of this release:

* Two tiles did not make the cut.
* An additional Jungle tileset is not present.
* The ability to edit levelset name, author and version is not available.
* Some strange lockups are possible when starting the game - cause unknown.
  I believe it was due to my use of threading, so that has been removed.
  If the game does appear to lock up, hit ESC repeatedly to try exiting.
  Restart the computer and try again.

===

Contact:

You can reach me at kennedy.greg@gmail.com or view my webpage at
https://greg-kennedy.com

===

License:

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

Check the file COPYING for a copy of the GPL.

Tilt-n-Roll contains source code from the Unimotion project, which is released
under the LGPL.  See
http://members.optusnet.com.au/lbramsay/programs/unimotion.html
for more information.

Tilt-n-Roll ships with a modified version of the SDL library.  The patch to add
Thinkpad tilt sensor support comes from Mark A. Smith with IBM research, and
is included in the ZIP as sdl-event.c.patch.  See
http://www.almaden.ibm.com/cs/people/marksmith/sdl.html
for more information.

SDL library binaries are included in the distribution.  This license must be
attached to any redistribution of SDL libraries:
Please include this notice with the runtime environment:

This library is distributed under the terms of the GNU LGPL license:
http://www.gnu.org/copyleft/lesser.html

The source is available from the libraries page at the SDL website:
http://www.libsdl.org/
