
#include<gameCore.hpp>
#include"utils.hpp"
#include<fstream>
#include<iostream>
#include"utils.hpp"
#include<glm/trigonometric.hpp>

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

MapData GameCore::getMapData() const 
{
	return MapData{ m_gameCamera.fov, m_gameCamera.maxRenderDist, m_entityTransform, m_gameMap };
}

bool GameCore::check_out_of_map_bounds(const glm::vec2& pos) const 
{
	return (pos[0] < 0 || pos[1] < 0 || pos[0] >= m_gameMap.x || pos[1] >= m_gameMap.y);
}

void GameCore::start_internal_time()
{
	m_lastTime = std::chrono::high_resolution_clock::now();
}

void GameCore::chech_position_in_map(const glm::vec2& rayPosInMap, EntityType& hitMarker) const
{
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
}

std::vector<RayInfo> GameCore::view_by_ray_casting() const
{
	std::vector<RayInfo> rayInfoVec;
	rayInfoVec.reserve(m_gameCamera.pixelWidth);
	
	glm::mat2x2 rotationMat = vecMath::rotation_mat2x2(glm::degrees( m_gameCamera.fov ));
	glm::vec2 playerForwDir{ glm::cos(m_entityTransform.forewardAngle), glm::sin(m_entityTransform.forewardAngle) };
	glm::vec2 rayIncrement = (playerForwDir * rotationMat ) * m_gameCamera.rayPrecision;
	glm::vec2 startPos = m_entityTransform.coords;

	for (int i = 0; i < m_gameCamera.pixelWidth; ++i) 
	{
		glm::vec2 currentRay{ 0,0 };
		EntityType hitMarker = EntityType::Empty;

		while (hitMarker == EntityType::Empty && (currentRay.x * currentRay.x + currentRay.y * currentRay.y) < m_gameCamera.maxRenderDist)
		{
			glm::vec2 rayPosInMap = startPos + currentRay;
			chech_position_in_map(rayPosInMap, hitMarker);
			currentRay += rayIncrement;
		}
		//std::cout << i << " " << glm::length(currentRay) << " " << rayIncrement[0] << " " << rayIncrement[1] << " " << m_entityTransform.forewardAngle << std::endl;
		rayInfoVec.push_back({hitMarker, currentRay});

		rayIncrement = rayIncrement * vecMath::rotation_mat2x2(-m_gameCamera.fov/m_gameMap.x);
		
	}
	return rayInfoVec;
}

void GameCore::update_entities()
{
	//calculate delta time and update internal clock
	auto currentTime = std::chrono::high_resolution_clock::now();
	int deltaTime = (currentTime - m_lastTime).count();
	m_lastTime = currentTime;
	float correctionFactor = 0.000000001f;

	//check_collision
	glm::vec2 moveAttempt = m_entityTransform.coords +
		(glm::vec2(glm::cos(m_entityTransform.forewardAngle), 
			glm::sin(m_entityTransform.forewardAngle))
			* (m_pInputCache.foreward * deltaTime * correctionFactor));

	moveAttempt = moveAttempt +
		(glm::vec2(-glm::sin(m_entityTransform.forewardAngle),
			glm::cos(m_entityTransform.forewardAngle))
			* (m_pInputCache.lateral * deltaTime * correctionFactor));

	EntityType hitMarker = EntityType::Empty;
	chech_position_in_map(moveAttempt, hitMarker);

	//update entitys
	if (hitMarker == EntityType::Empty) //check for any unblocking tiles 
	{
		m_entityTransform.coords = moveAttempt;
	}

	m_entityTransform.forewardAngle += m_pInputCache.rotate * deltaTime * correctionFactor;

	m_pInputCache.foreward = 0;
	m_pInputCache.lateral = 0;
	m_pInputCache.rotate = 0;
}


void GameCore::PlayerControler::rotate(float angle) const
{
	gameCore.m_pInputCache.rotate += angle;
}
void GameCore::PlayerControler::move_foreward(float amount) const
{
	gameCore.m_pInputCache.foreward += amount;
}

void GameCore::PlayerControler::move_strafe(float amount) const
{
	gameCore.m_pInputCache.lateral += amount;
}

bool GameCore::load_map(const std::string& filePath) 
{
	return fill_map_form_file(&m_gameMap, m_entityTransform, filePath);
}