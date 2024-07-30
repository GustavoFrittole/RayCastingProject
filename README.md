
# Ray casting based maze explorer
Personal project exploring the concept of ray casting
## How to build
```
git clone https://github.com/GustavoFrittole/RayCastingProject.git
cd RayCastingProject
```
```
cmake -G"MinGW Makefiles" -DCMAKE_BUILD_TYPE=release -DSFML_BUILD_NETWORK=false -DSFML_BUILD_AUDIO=false -S . -B build/release
```
```
cmake --build ./build/release
```
Cmake will copy the asset folder in the user specified binary folder (./build/release). To run the code directly, move the copied asset folder in the same dir of the executable.
```
./build/rel/bin/app.exe
```
The project relies on SFML (for controls, handling and drawing on windows) and will attempt to fetch it unless the package can be retrieved. When building on linux having this dependency installed in advance might be more practical.
## Usage
Various variables will be imported from the json file `config.json` that can be found in the asset folder. Amongst other options, the user can choose to load their own map by turning "gameMap" -> "generated" to false and specifying a file path. If the map is generated, there's no need to provide a map file.
The map is generated as a maze using a [randomized DFS](https://en.wikipedia.org/wiki/Maze_generation_algorithm#Randomized_depth-first_search) algorithm,
and the process of creation is displayed at game start.
## Controls
- WASD to move,
- mouse right and left or arrows `<` `>` to turn left and right,
- `ESC` to pause and view map, 
- `tab` to view full screen map without pausing,
- `e` to calculate shortest path (will be displayed in full screen map, as of now not implemented for custom maps).
## Functionalities
Pre game map generation, full screen map and mini map, graphical diplay of shortest path, customizable parameters (FOV 360° in image 5).

![path](https://github.com/user-attachments/assets/f1382797-0022-4488-bfb5-c3c704b4340b)
## Basic class diagram
![Caster drawio](https://github.com/user-attachments/assets/6165682c-98fd-404e-9333-6a98c0315d25)

*All not specified relations are 1:1*

## known issues
 - The currently adopted ray casting technique is far inferior compared to algorithms like DDA and others.
 -  - There's a glitch in wall shading that becomes evident when the render distance radius is lower than 6 units (this is due to the imprecision of the ray casting, which is to be replaced).
 - GameGraphics is bloated.
 - ~~The rendering process would greatly benefit form the employment of parallel computing (at least cpu multithreading).~~
 - ~~Many static elements that are calculated per frame (textures and masks mainly) could be easily generated only once at start time.~~
 - ~~Current implementation of multi threading is inefficient due to recreation of threads~~


