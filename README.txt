/*****************************\
*        Save The Rock!       *
*         (README)            *
*        Version 1.0          *
* Author: Alexander Dzhoganov *
\*****************************/


  _______________________
// 0. Source code and compiling the game \\

(1) Source

Game's source can be found in the src/ folder of the game's distribution. I recommend reading src/CODE_WALKTHROUGH.txt before you start going trough any actual code.

(2) How to compile

Your distribution of the game comes with a precompiled Windows binary.
If you do not plan on modifying the game's source code you do not have to recompile and should skip to section 1.

You can compile the game using any Windows C++ compiler. A Linux port is on the way, but is not yet complete.
A project file for DevC++ is provided with the distribution in the folder src/devcpp.
If you're not using DevC++ you should add src/includes/ to your compiler includes search path
and compile against the following libraries:
GLFW (http://glfw.sourceforge.net)
Audiere (http://audiere.sourceforge.net)
OpenGL and GLU

  ______________________
// 1. Starting the game \\

Game premise: Your best friend the Rock has been captured again, play through the game's levels to rescue him from certain doom.

A precompiled Windows executable - game.exe can be found in the bin/ folder of the distribution.

  __________________________________
// 2. Requirements and instructions \\

Minimum system requirements:
- Windows 2000
- Intel Pentium 4 1.6Ghz, Intel Atom Z or equivalent
- 512MB RAM
- OpenGL 1.3 capable video card

Recommended system requirements:
- Windows 7
- Intel Pentium Dual Core 1.6Ghz or equivalent
- 1024MB RAM
- OpenGL 1.5 capable video card

Run the tutorial found in the game's main menu to obtain instructions on game controls and mechanics.

  ____________________
// 3. Troubleshooting \\

(1) The game crashes immediately/doesn't display anything after starting the executable.

This could be a problem either with your OpenGL implementation or the game's resource folder - media/.
If for some reason your media/ folder became corrupted, obtain the original one from the game's distribution.

Some OpenGL implementations fail to initialize (for no apparent reason) on Windows XP, because of bad compatibility mode settings.
Remove all compatibility mode settings from the game.exe or, if the game still crashes, set compatibility mode to Windows 2000.

Last, it could be a multiple displays problem.
Some video cards do not support hardware acceleration over multiple displays
and Save The Rock! is certainly not designed for this, you should check your settings.

(2) I see a black screen after starting the game/ All I see is white rectangles.

This is most likely a problem with the OpenGL extensions the game requires. For some reason your drivers are reporting
extensions your video card doesn't actually have. Reinstalling your video drivers may solve the problem.

(3) I have severe framerate issues.

ASUS Eee (Intel Atom) users and users with older PC's could encounter severe framerate drops during objects-heavy parts of a level,
when there are a lot of objects really close to each other.
This could be countered by lowering resolution and disabling antialiasing in the game's settings menu.
Truth is, the game is quite CPU heavy in certain parts and there is no trivial fix for that.
If you're still interested in running the game on a slow CPU you should open src/includes/engine/constants.h and locate the line

...
#define PHYS_ITERATIONS 3
...

change it to

..
#define PHYS_ITERATIONS 2
..

Then recompile the game by following the instructions in section 0.

This change should lower CPU requirements by quite a bit, but it could introduce severe bugs as the physics engine needs at least
three iterations per frame to work as intended.

If you're really enthusiastic you could change the values of MIN_PHYS_ITERATIONS to 2 and MAX_PHYS_ITERATIONS to a number from 3 to 5,
which would turn on the engine's internal benchmark, which will automatically adjust iterations based on current performance.
This however makes the physics simulation somewhat unpredictable (as values change rapidly to account for framerate), which is undesirable in a platformer and is off by default.
