[![Build-tests](https://github.com/GustavoFrittole/RayCastingProject/actions/workflows/test-builds.yml/badge.svg)](https://github.com/GustavoFrittole/RayCastingProject/actions/workflows/test-builds.yml)

# Ray casting based maze explorer
Personal project exploring the concept of ray casting.
## Build instructions
The project relies on the SFML library (for user input and window handling). If it can't be found locally, cmake will automatically download it from github and it will be compiled with the rest of the project.
### Windows
```
git clone https://github.com/GustavoFrittole/RayCastingProject.git
```
```
cd RayCastingProject
```
```
cmake -S . -B ./build
```
```
cmake --build ./build
```
```
./build/bin/demo.exe
```
### Linux 
When building on linux having the SFML dependency installed in advance might be more practical. For Debian is:
```
sudo apt-get install libsfml-dev
```
Regarding SFML dependencies, this project's workflow file [build-ubuntu](https://github.com/GustavoFrittole/RayCastingProject/blob/652de14edd2ba82c59bac9e2bb2f2771dd5f1e0c/.github/workflows/test-builds.yml) can serve as example. More info on the [sfml guide](https://www.sfml-dev.org/tutorials/2.6/start-cmake.php).


## Features
Pseudo 3d environment generated via ray casting that allows to explore a maze (generation displayed at launch) or a custom map. More features in order of implementation:
- in-game map and mini map;
- calculation and display of shortest path to goal in generated mazes;
- switch between euclidean distance (curvilinear perspective) and projection to camera plain (linear perspective);
- customizable parameters (in `config.json`);
- custom textures for walls, floor, ceiling and sky;
- custom billboard sprite rendering in both perspective modes.
- scriptable entities
Note: distance based shading of horizontal planes (ceiling/floor) and sky are only available in linear perspective mode.

## Usage
Various variables are imported from the json file `assets/config.json`, usage explained in the next section. The project is roughly configured as a entity–component–system, so game logic \ game play related code is implemented throgh scriptable entities. Entities can interact with the game handler api and between each other. Entities and be easily defined as shown and described in `demo.cpp` and `demoEntities.hpp`.
The map (at the moment there is only one map and only one "scene") can be custom made and loaded from a file, or the user can choose to generate a maze (through the config file). The maze is generated using a [randomized DFS](https://en.wikipedia.org/wiki/Maze_generation_algorithm#Randomized_depth-first_search) algorithm, and the process of creation is displayed at game start.

### `assets/config.json` variables, each section contains:
- gameCamera: 
   - field of view in degrees, 
   - rays max length, 
   - rays precision;
- gameMap: 
   - map width, 
   - map height, 
   - option to generate a maze with given dimensions, 
   - file form where to load a rectangular map of given (or less) dimensions;
- screenStats: 
   - scale factor for the minimap (3 means that the center of the minimap will be at 1/3 of window height, right alignment),
   - scale factor for the wall height (basically the vertical fov);
- controls: 
   - mouse speed,
   - movement speed;
- assets:
   - font used for the simple text ui;
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
   - WASD affects cached values for frontal and lateral movement,
   - move movement or `<` `>` to turn left and right,
   - mouse left click or `q` is labeled as "leftTrigger" action,
   - mouse right click or `e` is labeled as "rightTrigger" action.

Note: Controls available for scripting are only exposed by the game handler; a possible implementation is shown in the demo project. The game starts paused. Press `ESC` to gain control.
___
### 0.1.0
![path](https://github.com/user-attachments/assets/f1382797-0022-4488-bfb5-c3c704b4340b)
:-------------------------:

0.1.1 | 0.2.0 (preview)
:-------------------------:|:-------------------------:
![switch](https://github.com/user-attachments/assets/159c4ac2-e2bf-49a2-89ed-49960b84b41b) | ![scriptable](https://github.com/user-attachments/assets/1e74a11c-d919-4587-b194-eebb06e8c960)

## Basic class diagram
old | post refactor (0.2.0)
:-------------------------:|:-------------------------:
![Caster drawio](https://github.com/user-attachments/assets/9e436ad2-37ac-475a-bd19-af297e51c40f) | ![Caster2 drawio](https://github.com/user-attachments/assets/6cc88bfc-6c43-4cdf-b52e-64a4bc934561)


*All not specified relations are 1:1*

## Known issues
- mouse speed lowers as the frame rate increases (whilst arrows rotation behaves correctly);
- ~~drawing sprites has a big impact on frame rate (multi threading not yet implemented in sprite drawing);~~

## Resources
 - Textures:
    - textures: edited DALL·E 3 and FLUX.1 schnell generated images;
    - sprite textures: original
 - Linear perspective:
    - [Lodev article](https://lodev.org/cgtutor/raycasting.html).
