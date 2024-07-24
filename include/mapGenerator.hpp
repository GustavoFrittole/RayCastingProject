#ifndef MAPGENERATOR_HPP
#define MAPGENERATOR_HPP
#include<string>
class MapGenerator
{
public:
	void generate_map(int startX, int startY, int width, int height, std::string& tiles);
};

#endif