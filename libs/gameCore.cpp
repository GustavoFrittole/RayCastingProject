#include<gameCore.hpp>
#include"utils.hpp"
#include<fstream>
#include<iostream>

bool fill_map_form_file(GameMap* map, EntityTransform& et, const std::string& filePath)

{
	std::ifstream file(filePath);
	if (file.is_open())
	{
		file >> map->x >> map->y >> et.coords[0] >> et.coords[1] >> et.forewardAngle;
		et.forewardAngle = glm::radians(et.forewardAngle);
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

bool GameCore::check_out_of_map_bounds(const glm::vec2& pos) 
{
	return (pos[0] < 0 || pos[1] < 0 || pos[0] >= m_gameMap.x || pos[1] >= m_gameMap.y);
}

std::vector<RayInfo> GameCore::view_by_ray_casting()
{
	std::vector<RayInfo> rayInfoVec;
	rayInfoVec.reserve(m_gameCamera.screenXY[0]);
	
	glm::mat2x2 rotationMat = vecMath::rotation_mat2x2(m_gameCamera.fov * 0.5f);
	glm::vec2 playerForwDir{ glm::cos(m_entityTransform.forewardAngle), glm::sin(-m_entityTransform.forewardAngle) };
	glm::vec2 rayIncrement = (playerForwDir * rotationMat) * m_gameCamera.rayPrecision;
	glm::vec2 startPos = m_entityTransform.coords;

	for (int i = 0; i < m_gameCamera.screenXY[0]; ++i) 
	{
		glm::vec2 currentRay{ 0,0 };
		EntityType hitMarker = EntityType::Empty;

		while (hitMarker == EntityType::Empty && (currentRay.x * currentRay.x + currentRay.y * currentRay.y) < m_gameCamera.maxRenderDist)
		{
			glm::vec2 rayPosInMap = startPos + currentRay;
			if (!check_out_of_map_bounds(rayPosInMap)) 
			{
				switch (m_gameMap.cells[glm::floor(rayPosInMap[0]) + glm::floor(rayPosInMap[1]) * m_gameMap.x]) 
				{
					case 'w':
						hitMarker = EntityType::Wall;
						break;
					case ' ': default: 
						break; //todo: add map features
				}
			}
			else
			{
				hitMarker = EntityType::Oob;
			}
			currentRay += rayIncrement;
		}
		//std::cout << i << " " << glm::length(currentRay) << " " << rayIncrement[0] << " " << rayIncrement[1] << " " << m_entityTransform.forewardAngle << std::endl;
		rayInfoVec.push_back({hitMarker, currentRay+startPos});

		rayIncrement = rayIncrement * vecMath::rotation_mat2x2(-m_gameCamera.fov/m_gameMap.x);
		
	}
	m_entityTransform.forewardAngle += 0.02f;
	return rayInfoVec;
}

bool GameCore::load_map(const std::string& filePath) 
{
	return fill_map_form_file(&m_gameMap, m_entityTransform, filePath);
}