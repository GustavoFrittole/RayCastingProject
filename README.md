
# Ray casting based maze explorer
Personal project exploring the concept of ray casting.
## Building
The project relies on SFML (for user input and window handling). If it  can't be found locally by cmake, it will be downloaded form github and compiled.
When building on linux having this dependency installed in advance might be more practical.
### Windows
```
git clone https://github.com/GustavoFrittole/RayCastingProject.git
cd RayCastingProject
```
```
cmake -G"<Choose generator>" -DSFML_BUILD_NETWORK=false -DSFML_BUILD_AUDIO=false -S .
```
```
cmake --build ./build
```
```
./build/bin/main.exe
```
### Linux 
Same as above but it is advised to install SFML in advance. For Debian is:
```
sudo apt-get install libsfml-dev
```
Regarding SFML dependencies check this project's worklflow file [build-ubuntu](https://github.com/GustavoFrittole/RayCastingProject/blob/652de14edd2ba82c59bac9e2bb2f2771dd5f1e0c/.github/workflows/test-builds.yml) or the [sfml guide](https://www.sfml-dev.org/tutorials/2.6/start-cmake.php).

## Features
Pre-game map generation, custom map, in-game full screen map and mini map, calculation and display of shortest path to goal in generated mazes, customizable parameters (in `config.json`), real time switch from euclidean distance measuring to projection to camera plain (linear perspective).
Note: distance based shading of horizontal planes (sky/floor) is only available in linear perspective mode.

### 0.1.1 : Linear - Euclidean real time switch
![switch](https://github.com/user-attachments/assets/235d9133-62e0-4de6-b666-10f7ce739400)

### 0.1.0
![path](https://github.com/user-attachments/assets/f1382797-0022-4488-bfb5-c3c704b4340b)

## Usage
Various variables are imported from the json file `config.json` that can be found in the asset folder. Amongst other options, the user can choose to load their own map by turning "gameMap" -> "generated" to false and specifying a file path, can modify the map (that must retain rectangular shape and dimensions according to what stated in the config file) and change textures. If the map is generated, there's no need to provide a map file.
The map is generated as a maze using a [randomized DFS](https://en.wikipedia.org/wiki/Maze_generation_algorithm#Randomized_depth-first_search) algorithm,
and the process of creation is displayed at game start.
## Controls
- WASD to move,
- mouse right and left or arrows `<` `>` to turn left and right,
- `ESC` to pause and view map, 
- `tab` to view full screen map without pausing,
- `e` to calculate shortest path (will be displayed in full screen map, as of now not implemented for custom maps).
- `space` to toggle camera plane on and off.

Note: the game starts paused. Press `ESC` to gain control


## Basic class diagram
![Caster drawio](https://github.com/user-attachments/assets/6165682c-98fd-404e-9333-6a98c0315d25)
*All not specified relations are 1:1*

## known issues
- mouse speed lowers as the frame rate increases (whilst arrows rotation behaves correctly);
- ~~rendering thread pool is deadlocked if width%12 = 0;~~
- ~~json parser doest return error when options are missing, making the bug difficult to read;~~
- ~~texture are not inverted in accordance with the orientation of the faces of the cube.~~



