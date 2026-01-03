# DoomIIRPG-RE

![image](https://github.com/Erick194/DoomIIRPG-RE/assets/41172072/6249f7bd-18e6-4838-b1ec-8654d18cc1b4)<br>
https://www.doomworld.com/forum/topic/135602

Doom II RPG Reverse Engineering By [GEC]<br />
Created by Erick Vásquez García.

Current version 0.1

You need CMake to make the project.<br />
What you need for the project is:
  * SDL2
  * Zlib
  * OpenAL
  * Devkitpro
  * Switch-dev

Default key configuration:

Move Forward: D-Pad Up, L-Stick Up<br />
Move Backward: D-Pad Down, L-Stick Down<br />
Move Left: D-Pad Left, L-Stick Left<br />
Move Right: D-Pad Right, L-Stick Right<br />
Turn Left: ZL<br />
Turn Right: ZR<br />
Atk/Talk/Use: A<br />
Next Weapon: R<br />
Prev Weapon: L<br />
Pass Turn: B<br />
Automap: Minus<br />
Menu Open/Back: Plus<br />
Menu Items/Info: Y<br />
Menu Drinks: X<br />

## Save and config data

All user data is stored in `/switch/doom2rpg/DoomRPG.app` - these files are compatible with the PC release.

## Building instructions

This port uses SDL2 and SDL2_Mixer

1. Install [Devkitpro] and Switch-dev package, also make sure `DEVKITPRO` env variable is set
1. Install SDL2 and SDL2_Mixer by Pacman
1. git clone https://github.com/efimandreev0/DoomRPGII-RE-NSLite.git && cd DoomRPGII-RE-NSLite
1. mkdir build && cd build
1. cmake .. -DCMAKE_TOOLCHAIN_FILE=$DEVKITPRO/cmake/Switch.cmake
1. make
