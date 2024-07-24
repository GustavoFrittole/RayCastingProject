#include"mapGenerator.hpp"
#include <random>

void generateMap(int startX, int startY, int width, int height, std::string& tiles) 
{
	std::random_device rd; 
	std::mt19937 gen(rd());
	std::uniform_int_distribution<> distr(25, 63);
	distr(gen);
	tiles.assign(width * height, 'w');
}