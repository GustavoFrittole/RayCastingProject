
# Ray casting based maze explorer
Personal project exploring the concept of ray casting
## How to build
```
git clone https://github.com/GustavoFrittole/RayCastingProject.git
cd RayCastingProject
```
```
cmake -G"<Choose generator>" -DSFML_BUILD_NETWORK=false -DSFML_BUILD_AUDIO=false -S .
```
```
cmake --build ./build/release
```
Cmake will copy the asset folder in the user specified binary folder (./build/release). To run the code directly, move the copied asset folder in the same dir of the executable.
```
./build/release/bin/main.exe
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
- `space` to toggle camera plane on and off.

Note: the game starts paused. Press `ESC` to gain control

## Functionalities
Pre-game map generation, full screen map and mini map, graphical display of shortest path, customizable parameters (FOV 360° in image 5).

![path](https://github.com/user-attachments/assets/f1382797-0022-4488-bfb5-c3c704b4340b)
## Basic class diagram
![Caster drawio](https://github.com/user-attachments/assets/6165682c-98fd-404e-9333-6a98c0315d25)

*All not specified relations are 1:1*

## known issues
- mouse speed lowers as the frame rate increases (whilst arrows rotation behaves correctly);
- ~~rendering thread pool is deadlocked if width%12 = 0;~~
- ~~json parser doest return error when options are missing, making the bug difficult to read;~~
- ~~texture are not inverted in accordance with the orientation of the faces of the cube.~~



