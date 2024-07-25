# Silly ray casting engine
Bare bones and badly optimized ray casting based game.
## Usage
The map is are either loaded from `map.txt` oe generated, depending on the ralative flag in the same file (see example in assets).
The map is generated as a maze using a [randomized DFS](https://en.wikipedia.org/wiki/Maze_generation_algorithm#Randomized_depth-first_search) algorithm,
and the process of creation is displayed at game start.
## Controls
WASD for direction, mouse right and left for turning, ESC to pause and view map.
## 
Pre game maze generation:

![mazegen](https://github.com/user-attachments/assets/3bf60432-273b-4bbc-b7fd-d76891879657)

In game map display while game paused:

![map](https://github.com/user-attachments/assets/be7005ea-6335-411e-824a-6ce9ebf63a94)

In game minimap:

![minimap](https://github.com/user-attachments/assets/c6700558-e933-44c5-8c10-7b3721ebbfc1)

## known issues
 - The currently adopted ray casting tecnique is far inferior compared to algorithms like DDA and others.
 - The rendering process would gratly benefit form the employment of parallel computing (at least cpu multithreading).
 - Currently outer walls are not properly displayed.
 - Theres a glitch in wall shading that becomes evident when the rander distance radius is lower than 6 units (this is due to the imprecision of the ray casting, wich is to be replaced).

