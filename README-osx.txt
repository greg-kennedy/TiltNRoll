OSX notes:

Dependencies
===
Mac users will need to obtain the following libraries to play this game:
* SDL
* SDL_mixer
* SDL_image
* SDL_ttf
* SDL_image

Check http://www.libsdl.org/ for sources.

Tilt sensors
===
Here are the steps required to play Tilt-n-Roll with the tilt sensors on the mac:

1)  Calibrate your tilt sensor.
    Using the terminal, start the program 'calibrate' from ./unimotion/bin/calibrate
    Then follow these steps:
      * Turn your laptop 90 degrees to the right
      * Press Enter
      * type xscale <enter>
      * Type in the first number after the r: line above and press Enter.

      * Turn your laptop 90 degrees back
      * Press Enter
      * type yscale <enter>
      * Type in the second number after the r: line above and press Enter.

      * type zscale <enter>
      * Enter a number such that the third number after r: divided by your entry is
         less than 1 (e.g. if r: 3 5 250 is displayed, try entering 300 since 250/300 < 1)

    Try it out by pressing Enter a few times while turning your laptop.  Numbers after the
      f: line should be in the range -1.0 to 1.0 corresponding to 90 degree extremes.

    Press CTRL+D when done.

2)  Start the game.  Run tnr-osx-threaded from the ./game/ directory.  You should be able
    to use the tilt to control the angle while the game is playing.

3)  If you notice the level seems to tilt the opposite direction, re-calibrate but enter
    negative values instead (e.g. if the left-right is reversed and you had previously
    used 150, enter -150 instead)

4)  To adjust sensitivity, you can play with the xscale/yscale/zscale values.  Higher
    numbers are less sensitive.