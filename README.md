# Silly ray casting based game
Based on SFML
## Usage
The map is are either loaded from `map.txt` or generated, depending on the ralative flag in the same file (see example in assets).
The map is generated as a maze using a [randomized DFS](https://en.wikipedia.org/wiki/Maze_generation_algorithm#Randomized_depth-first_search) algorithm,
and the process of creation is displayed at game start.
## Controls
- WASD to move,
- mouse right and left or arrows `<` `>` to turn left and right,
- `ESC` to pause and view map, 
- `tab` to view full screen map without pausing,
- `e` to calculate shortest path (will be displayed in full screen map, as of now not implemented for custom maps).
## 
Pre game maze generation:

![mazegen](https://github.com/user-attachments/assets/3bf60432-273b-4bbc-b7fd-d76891879657)

In game map display while game paused:

![map](https://github.com/user-attachments/assets/be7005ea-6335-411e-824a-6ce9ebf63a94)

In game minimap:

![minimap](https://github.com/user-attachments/assets/c6700558-e933-44c5-8c10-7b3721ebbfc1)

Goal reached display, outer pink walls:

![goal](https://github.com/user-attachments/assets/1487f270-17d1-4839-a431-0cd6be634573)

Display calculated path to goal:

![path](https://github.com/user-attachments/assets/a99333a5-a15f-419f-9b8c-f10a07a279ad)

## known issues
 - The currently adopted ray casting tecnique is far inferior compared to algorithms like DDA and others.
 - ~~The rendering process would gratly benefit form the employment of parallel computing (at least cpu multithreading).~~
 - There's a glitch in wall shading that becomes evident when the rander distance radius is lower than 6 units (this is due to the imprecision of the ray casting, wich is to be replaced).
 - ~~Many static elements that are calculated per frame (textures and masks mainly) could be easly generated only once at start time.~~
 - ~~Current implementation of multi threading is inefficient due to recreation of threads~~

