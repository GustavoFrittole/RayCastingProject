[![Build-tests](https://github.com/GustavoFrittole/RayCastingProject/actions/workflows/test-builds.yml/badge.svg)](https://github.com/GustavoFrittole/RayCastingProject/actions/workflows/test-builds.yml)

# Ray casting based maze explorer
Personal project exploring the concept of ray casting.
## Build instructions
The project relies on SFML (for user input and window handling). If it can't be found locally by cmake, it will be downloaded form github and compiled.
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
./build/bin/main.exe
```
### Linux 
Same as above, but when building on linux having the SFML dependency installed in advance might be more practical. For Debian is:
```
sudo apt-get install libsfml-dev
```
Regarding SFML dependencies check this project's worklflow file [build-ubuntu](https://github.com/GustavoFrittole/RayCastingProject/blob/652de14edd2ba82c59bac9e2bb2f2771dd5f1e0c/.github/workflows/test-builds.yml) or the [sfml guide](https://www.sfml-dev.org/tutorials/2.6/start-cmake.php).


## Features
Pseudo 3d environment generated via ray casting in wich the user can explore a maze (generation displayed at launch) or a custom map. Custom textures, in-game map and mini map, calculation and display of shortest path to goal in generated mazes, customizable parameters (in `config.json`), real time switch from euclidean distance measuring to projection to camera plain (linear perspective), billboard sprites.
Note: distance based shading of horizontal planes (ceiling/floor) is only available in linear perspective mode.

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

## Usage
Various variables are imported from the json file `config.json` that can be found in the asset folder. Amongst other options, the user can choose to load their own map by turning "gameMap" -> "generated" to false and specifying a file path, can modify the map (that must retain rectangular shape and dimensions according to what stated in the config file) and change textures. If the map is generated, there's no need to provide a map file.
The map is generated as a maze using a [randomized DFS](https://en.wikipedia.org/wiki/Maze_generation_algorithm#Randomized_depth-first_search) algorithm,
and the process of creation is displayed at game start.
## Controls
- WASD to move,
- mouse left and right or `<` `>` to turn left and right,
- `ESC` to pause and view map, 
- `tab` to view full screen map without pausing,
- `e` to calculate shortest path (will be displayed in full screen map, as of now not implemented for custom maps),
- `space` to toggle camera plane on and off,
- `R` to toggle sky on and off (only linear perspective).

Note: the game starts paused. Press `ESC` to gain control


## Basic class diagram
![Caster drawio](https://github.com/user-attachments/assets/6165682c-98fd-404e-9333-6a98c0315d25)
*All not specified relations are 1:1*

## Known issues
- mouse speed lowers as the frame rate increases (whilst arrows rotation behaves correctly);
- drawing sprites has a big impact on frame rate (multi threading not yet implemented in sprite drawing);
### Structural issues
- GameGraphics is bloated, this makes reusing code harder;
- Treating Core and Graphics as separate modules instead of layers would help decoupling.

## Resources
 - Textures:
    - skybox: edited Adobe Stock Standard License photo thumbnail;
    - other textures: edited DALL·E 3 generated images;
 - Linear perspective:
    - [Lodev article](https://lodev.org/cgtutor/raycasting.html).
