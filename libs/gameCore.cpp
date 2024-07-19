#include<gameCore.hpp>
#include"utils.hpp"
#include<fstream>
#include <glm/ext/matrix_transform.hpp> 

bool fill_map_form_file(GameMap* map, EntityTransform& et, const std::string& filePath)

{
	std::ifstream file(filePath);
	if (file.is_open())
	{
		file >> map->x >> map->y >> et.coords[0] >> et.coords[1] >> et.forewardAngle;
		std::string line;
		while (std::getline(file, line))
		{
			map->cells += line;
		}
		return true;
	}
	else
		return false;
}

std::unique_ptr<RayInfo[]> GameCore::view_by_ray_casting() 
{
	std::unique_ptr<RayInfo[]> rayInfoVec = std::make_unique<RayInfo[]>(m_gameCamera.screenXY[1]);
	/*
	glm::mat2x2 rotationMat{ 1, 0, 1, 0 };
	glm::rotate(rotationMat, 0.4f, { 1,0 });
	glm::vec2 rayIncrement{m_gameCamera.rayPrecision,m_gameCamera.rayPrecision}
	*///TODO : WRITE ROTATION MATRIX 2D

	return rayInfoVec;
}

bool GameCore::load_map(const std::string& filePath) 
{
	return fill_map_form_file(&m_gameMap, m_entityTransform, filePath);
}