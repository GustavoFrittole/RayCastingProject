
#include<gameCore.hpp>
#include"utils.hpp"
#include<fstream>
#include<iostream>
#include<thread>
#include<stdexcept>
#include<cmath>

RayInfo& RayInfoArr::at(int index)
{
	if (index < 0 || index >= arrSize)
		throw std::invalid_argument("Index is out of range.");
	else
		return m_rayArr[index];
}
const RayInfo& RayInfoArr::const_at(int index) const 
{
	if (index < 0 || index >= arrSize)
		throw std::invalid_argument("Index is out of range.");
	else
		return m_rayArr[index];
}

GameCore::GameCore(GameCamera gc) : m_gameCamera(gc), m_processorCount(std::thread::hardware_concurrency()), m_rayInfoArr(gc.pixelWidth)
{
	if (m_processorCount < 1)
		m_processorCount = 1;
}

MapData GameCore::getMapData() const 
{
	return MapData{ m_gameCamera.fov, m_gameCamera.maxRenderDist, m_entityTransform, m_gameMap };
}

bool GameCore::check_out_of_map_bounds(const math::Vect2& pos) const 
{
	return (pos.x < 0 || pos.y < 0 || pos.x >= m_gameMap.x || pos.y >= m_gameMap.y);
}

void GameCore::start_internal_time()
{
	m_lastTime = std::chrono::high_resolution_clock::now();
}

void GameCore::chech_position_in_map(const math::Vect2& rayPosInMap, EntityType& hitMarker) const
{
	if (!check_out_of_map_bounds(rayPosInMap))
	{
		switch (m_gameMap.cells[(int)rayPosInMap.x + (int)rayPosInMap.y * m_gameMap.x])
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

void GameCore::view_by_ray_casting()
{	
	math::Mat2x2 rotationMat = math::rotation_mat2x2( m_gameCamera.fov /2);
	math::Vect2 playerForwDir{ std::cos(m_entityTransform.forewardAngle), std::sin(m_entityTransform.forewardAngle) };
	math::Vect2 rayIncrement = (playerForwDir * rotationMat ) * m_gameCamera.rayPrecision;
	math::Vect2 startPos = m_entityTransform.coords;

	auto cast_in_interval = [this, &startPos ](int start, int end, math::Vect2 rayIncrement) mutable 
	{
		for (int i = start; i < end; ++i)
		{
			math::Vect2 currentRay{ 0,0 };
			//std::cout << "teat" << i << " " << std::endl;
			EntityType hitMarker = EntityType::Empty;

			while (hitMarker == EntityType::Empty && (currentRay.x * currentRay.x + currentRay.y * currentRay.y) < m_gameCamera.maxRenderDist * m_gameCamera.maxRenderDist)
			{
				math::Vect2 rayPosInMap = startPos + currentRay;
				chech_position_in_map(rayPosInMap, hitMarker);
				currentRay += rayIncrement;
			}
			m_rayInfoArr.at(i) = { hitMarker, currentRay };
			rayIncrement = rayIncrement * math::rotation_mat2x2(-m_gameCamera.fov / m_gameCamera.pixelWidth);
		}
	};

	int sectionsSize = m_gameCamera.pixelWidth / (m_processorCount*1);
	std::vector<std::thread> threadVec;
	int currentSection;
	for (currentSection = 0; currentSection < m_gameCamera.pixelWidth - sectionsSize; currentSection += sectionsSize)
	{
		threadVec.push_back(std::thread(cast_in_interval, currentSection, (currentSection + sectionsSize), rayIncrement * math::rotation_mat2x2(-(m_gameCamera.fov / m_gameCamera.pixelWidth) * currentSection )));
	}
	//std::cout << "teat" << currentSection << " " << std::endl;
	if (currentSection != m_gameCamera.pixelWidth)
	{
		int lastSectionSize = (m_gameCamera.pixelWidth - (currentSection));
		cast_in_interval( m_gameCamera.pixelWidth - lastSectionSize, m_gameCamera.pixelWidth, rayIncrement * math::rotation_mat2x2(-(m_gameCamera.fov / m_gameCamera.pixelWidth) * currentSection));
	}
		
	for (auto& t : threadVec) 
	{
		t.join();
	}
}

//void GameCore::view_by_ray_casting_nothreads()
//{
//	math::Mat2x2 rotationMat = math::rotation_mat2x2(m_gameCamera.fov/2);
//	math::Vect2 playerForwDir{ std::cos(m_entityTransform.forewardAngle), std::sin(m_entityTransform.forewardAngle) };
//	math::Vect2 rayIncrement = (playerForwDir * rotationMat) * m_gameCamera.rayPrecision;
//	math::Vect2 startPos = m_entityTransform.coords;
//
//	for (int i = 0; i < m_gameCamera.pixelWidth; ++i)
//	{
//		math::Vect2 currentRay{ 0,0 };
//		//std::cout << "teat" << i << " " << std::endl;
//		EntityType hitMarker = EntityType::Empty;
//
//		while (hitMarker == EntityType::Empty && (currentRay.x * currentRay.x + currentRay.y * currentRay.y) < m_gameCamera.maxRenderDist * m_gameCamera.maxRenderDist)
//		{
//			math::Vect2 rayPosInMap = startPos + currentRay;
//			chech_position_in_map(rayPosInMap, hitMarker);
//			currentRay += rayIncrement;
//		}
//		
//		m_rayInfoArr.at(i) = { hitMarker, currentRay };
//		rayIncrement = rayIncrement * math::rotation_mat2x2( - m_gameCamera.fov / m_gameCamera.pixelWidth);
//	}
//}

void GameCore::update_entities()
{
	//calculate delta time and update internal clock
	auto currentTime = std::chrono::high_resolution_clock::now();
	int deltaTime = (currentTime - m_lastTime).count();
	m_lastTime = currentTime;
	float correctionFactor = 0.000000001f;

	//check_collision
	math::Vect2 moveAttempt = m_entityTransform.coords +
		(math::Vect2(std::cos(m_entityTransform.forewardAngle), 
			std::sin(m_entityTransform.forewardAngle))
			* (m_pInputCache.foreward * deltaTime * correctionFactor));

	moveAttempt = moveAttempt +
		(math::Vect2(-std::sin(m_entityTransform.forewardAngle),
			::cos(m_entityTransform.forewardAngle))
			* (m_pInputCache.lateral * deltaTime * correctionFactor));

	EntityType hitMarker = EntityType::Empty;
	chech_position_in_map(moveAttempt, hitMarker);

	//update entities
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

bool fill_map_form_file(GameMap* map, EntityTransform& et, const std::string& filePath)
{
	std::ifstream file(filePath);
	if (file.is_open())
	{
		file >> map->x >> map->y >> et.coords.x >> et.coords.y >> et.forewardAngle;
		et.forewardAngle = (et.forewardAngle * 3.14159265358979323846) / 180;
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

bool fill_map_form_generator(GameMap* map, EntityTransform& et, MapGenerator& mg)
{
		return false;
}