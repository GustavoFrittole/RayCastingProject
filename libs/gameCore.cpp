
#include"gameCore.hpp"
#include<fstream>
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
//TODO: check order of initializer List
GameCore::GameCore(GameCamera gc, GameMap& gameMap, EntityTransform& entityTransform) : m_gameCamera(gc), m_entityTransform(entityTransform),
										m_processorCount(get_thread_number()), m_rayInfoArr(gc.pixelWidth)
{
	m_gameMap.x = gameMap.x;
	m_gameMap.y = gameMap.y;
	m_gameMap.generated = gameMap.generated;
	m_gameMap.cells.swap(gameMap.cells);
	if (m_gameMap.generated)
	{
		if (m_gameMap.cells.get() == nullptr)
			m_gameMap.cells = std::make_unique<std::string>();
		m_mapGenerator = std::make_unique<MapGenerator>((int)m_entityTransform.coords.x, (int)m_entityTransform.coords.y, m_gameMap.x, m_gameMap.y, *(m_gameMap.cells));
	}
		
}

MapData GameCore::getMapData() const 
{
	return MapData{ m_gameCamera.fov, m_gameCamera.maxRenderDist, m_entityTransform, m_gameMap};
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
		switch (m_gameMap.cells->at((int)rayPosInMap.x + (int)rayPosInMap.y * m_gameMap.x))
		{
		case 'w':
			hitMarker = EntityType::Wall;
			break;
		case 'b':
			hitMarker = EntityType::Baudry;
			break;
		default:
			break;
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

	auto cast_in_interval = [this, &startPos](int start, int end, math::Vect2 rayIncrement) mutable 
	{
		for (int i = start; i < end; ++i)
		{
			math::Vect2 currentRay{ 0,0 };
			EntityType hitMarker = EntityType::NoHit;

			while (hitMarker == EntityType::NoHit && (currentRay.x * currentRay.x + currentRay.y * currentRay.y) < m_gameCamera.maxRenderDist * m_gameCamera.maxRenderDist)
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

void GameCore::update_entities()
{
	//calculate delta time and update internal clock
	auto currentTime = std::chrono::high_resolution_clock::now();
	int deltaTime = (currentTime - m_lastTime).count();
	m_lastTime = currentTime;
	float correctionFactor = 0.000000001f;

	//decompose movment in x and y(world) axis and check collions separatly

	//check_collision X axis
	math::Vect2 moveAttempt = m_entityTransform.coords +
		(math::Vect2(std::cos(m_entityTransform.forewardAngle) * m_pInputCache.foreward 
					-std::sin(m_entityTransform.forewardAngle) * m_pInputCache.lateral, 0)
					* (deltaTime * correctionFactor));

	EntityType hitMarker = EntityType::NoHit;
	chech_position_in_map(moveAttempt, hitMarker);

	//update entities
	if (hitMarker == EntityType::NoHit) //check for any unblocking tiles 
	{
		m_entityTransform.coords = moveAttempt;
	}


	//check_collision Y axis
	moveAttempt = m_entityTransform.coords +
		(math::Vect2(0, std::sin(m_entityTransform.forewardAngle) * m_pInputCache.foreward +
					std::cos(m_entityTransform.forewardAngle) * m_pInputCache.lateral)
					* ( deltaTime * correctionFactor));



	hitMarker = EntityType::NoHit;
	chech_position_in_map(moveAttempt, hitMarker);

	//update entities
	if (hitMarker == EntityType::NoHit) //check for any unblocking tiles 
	{
		m_entityTransform.coords = moveAttempt;
	}

	//rotate
	m_entityTransform.forewardAngle += m_pInputCache.rotate * deltaTime * correctionFactor;

	//reset cached values
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