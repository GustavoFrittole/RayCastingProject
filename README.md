[![Build-tests](https://github.com/GustavoFrittole/RayCastingProject/actions/workflows/test-builds.yml/badge.svg)](https://github.com/GustavoFrittole/RayCastingProject/actions/workflows/test-builds.yml)

# Ray casting based maze explorer
Personal project exploring the concept of ray casting.
## Build instructions
The project relies on SFML (for user input and window handling). If it can't be found locally by cmake, it will be downloaded from github and compiled.
### Windows
```
git clone https://github.com/GustavoFrittole/RayCastingProject.git
cd RayCastingProject
```
```
cmake -DSFML_BUILD_NETWORK=false -DSFML_BUILD_AUDIO=false -S . -B ./build
```
```
cmake --build ./build
```
```
./build/bin/demo.exe
```
### Linux 
Same as above, but when building on linux having the SFML dependency installed in advance might be more practical. For Debian is:
```
sudo apt-get install libsfml-dev
```
Regarding SFML dependencies check this project's workflow file [build-ubuntu](https://github.com/GustavoFrittole/RayCastingProject/blob/652de14edd2ba82c59bac9e2bb2f2771dd5f1e0c/.github/workflows/test-builds.yml) or the [sfml guide](https://www.sfml-dev.org/tutorials/2.6/start-cmake.php).


## Features
Pseudo 3d environment generated via ray casting in which the user can explore a maze (generation displayed at launch) or a custom map. 
- in-game map and mini map;
- calculation and display of shortest path to goal in generated mazes;
- real time switch of distance measuring: euclidean distance (curvilinear perspective) or projection to camera plain (linear perspective);
- customizable parameters (in `config.json`);
- custom textures for walls, floor, ceiling and sky;
- custom billboard sprite rendering in both perspective modes.
- scriptable entities
Note: distance based shading of horizontal planes (ceiling/floor) and sky are only available in linear perspective mode.

## Usage
Various variables are imported from the json file `config.json` that can be found in the asset folder, usage explained below. Scriptable game entities (includes player, static sprites and active entities) can be created and added as shown in `demo.cpp`.
The maze is generated using a [randomized DFS](https://en.wikipedia.org/wiki/Maze_generation_algorithm#Randomized_depth-first_search) algorithm, and the process of creation is displayed at game start.

### `assets/config.json` variables
- gameCamera: field of view in degrees, rays max length, ray precision;
- gameMap: width, height, option to generate a maze with given dimensions, file form where to load a rectangular map of given (or less) dimensions;
- screenStats: scale of the minimap (3 means that the center of the m.m. will be at 1/3 of window height, right align), multiplier of the wall height;
- controls: mouse speed and movement speed;
- textures: file paths for all textures;
- sprites: a list of pairs of an id (int) and a path(string) from where the sprite will be loaded.

Note: ~~a sprite size multiplier,~~ an option for using flat colors instead of textures ~~and the possibility of using the same texture for multiple sprites~~ are, as of now, missing features. Ray precision has no effect on the DDA algorithm that is currently in use.

## Controls
- Built in:
   - `ESC` to pause and view map, 
   - `tab` to view full screen map without pausing,
   - `e` to calculate shortest path (will be displayed in full screen map, as of now not implemented for custom maps),
   - `space` to toggle camera plane on and off,
   - `R` to toggle sky on and off (only linear perspective).
- Available for scripting:
   - WASD to move,
   - move mouse left and right or `<` `>` to turn left and right,
   - mouse left click or `q` is labled as "leftTrigger"
   - mouse right click or `e` is labled as "rightTrigger"

Note: Controls available for scripting are only exposed by the game handler; a possible implementation is shown in the demo project. The game starts paused. Press `ESC` to gain control.

### 0.1.0

![path](https://github.com/user-attachments/assets/f1382797-0022-4488-bfb5-c3c704b4340b)
:-------------------------:

### 0.1.1
Wall, floor/ceiling and sky textures | Linear - Curvilinear perspective switch   
:-------------------------:|:-------------------------:
![sky](https://github.com/user-attachments/assets/aebb4eef-8195-496f-8c16-8616085422c4)  |  ![switch](https://github.com/user-attachments/assets/159c4ac2-e2bf-49a2-89ed-49960b84b41b)

### new
Billboard sprites
:-------------------------:
![sprites](https://github.com/user-attachments/assets/cf18bbb5-7e49-47b8-9718-380f52d8962a)

## Basic class diagram
old | post refactor
:-------------------------:|:-------------------------:
![Caster drawio](https://github.com/user-attachments/assets/6165682c-98fd-404e-9333-6a98c0315d25) | to add
*All not specified relations are 1:1*

## Known issues
- mouse speed lowers as the frame rate increases (whilst arrows rotation behaves correctly);
- ~~drawing sprites has a big impact on frame rate (multi threading not yet implemented in sprite drawing);~~

## Resources
 - Textures:
    - skybox: edited Adobe Stock Standard License photo thumbnail;
    - other textures: edited DALL·E 3 generated images;
 - Linear perspective:
    - [Lodev article](https://lodev.org/cgtutor/raycasting.html).
