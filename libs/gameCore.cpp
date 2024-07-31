
#include"gameCore.hpp"
#include<fstream>
#include<thread>
#include<stdexcept>
#include<cmath>
#include <iostream>

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
										m_playerController(*this),  m_processorCount(get_thread_number()), m_rayInfoArr(gc.pixelWidth)
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
	return check_out_of_map_bounds((int)pos.x, (int)pos.y);
}
bool GameCore::check_out_of_map_bounds(int posX, int posY) const
{
	return (posX < 0 || posY < 0 || posX >= m_gameMap.x || posY >= m_gameMap.y);
}

void GameCore::start_internal_time()
{
	m_lastTime = std::chrono::high_resolution_clock::now();
}

void GameCore::chech_position_in_map(const math::Vect2& rayPosInMap, EntityType& hitMarker) const
{
	chech_position_in_map((int)rayPosInMap.x, (int)rayPosInMap.y, hitMarker);
}
void GameCore::chech_position_in_map(int rayPosInMapX, int rayPosInMapY, EntityType& hitMarker) const
{
	if (!check_out_of_map_bounds(rayPosInMapX, rayPosInMapY))
	{
		switch (m_gameMap.cells->at(rayPosInMapX + rayPosInMapY * m_gameMap.x))
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


//Thread version that recretes the threads every frame (worse than no threads)
//void GameCore::view_by_ray_casting()
//{	
//	math::Mat2x2 rotationMat = math::rotation_mat2x2( m_gameCamera.fov /2);
//	math::Vect2 playerForwDir{ std::cos(m_entityTransform.forewardAngle), std::sin(m_entityTransform.forewardAngle) };
//	math::Vect2 rayIncrement = (playerForwDir * rotationMat ) * m_gameCamera.rayPrecision;
//	math::Vect2 startPos = m_entityTransform.coords;
//
//	auto cast_in_interval = [this, &startPos](int start, int end, math::Vect2 rayIncrement) mutable 
//	{
//		for (int i = start; i < end; ++i)
//		{
//			math::Vect2 currentRay{ 0,0 };
//			EntityType hitMarker = EntityType::NoHit;
//
//			while (hitMarker == EntityType::NoHit && (currentRay.x * currentRay.x + currentRay.y * currentRay.y) < m_gameCamera.maxRenderDist * m_gameCamera.maxRenderDist)
//			{
//				math::Vect2 rayPosInMap = startPos + currentRay;
//				chech_position_in_map(rayPosInMap, hitMarker);
//				currentRay += rayIncrement;
//			}
//			m_rayInfoArr.at(i) = { hitMarker, currentRay, currentRay.lenght()};
//			rayIncrement = rayIncrement * math::rotation_mat2x2(-m_gameCamera.fov / m_gameCamera.pixelWidth);
//		}
//	};
//
//	int sectionsSize = m_gameCamera.pixelWidth / (m_processorCount*1);
//	std::vector<std::thread> threadVec;
//	int currentSection;
//	for (currentSection = 0; currentSection < m_gameCamera.pixelWidth - sectionsSize; currentSection += sectionsSize)
//	{
//		threadVec.push_back(std::thread(cast_in_interval, currentSection, (currentSection + sectionsSize), rayIncrement * math::rotation_mat2x2(-(m_gameCamera.fov / m_gameCamera.pixelWidth) * currentSection )));
//	}
//	if (currentSection != m_gameCamera.pixelWidth)
//	{
//		int lastSectionSize = (m_gameCamera.pixelWidth - (currentSection));
//		cast_in_interval( m_gameCamera.pixelWidth - lastSectionSize, m_gameCamera.pixelWidth, rayIncrement * math::rotation_mat2x2(-(m_gameCamera.fov / m_gameCamera.pixelWidth) * currentSection));
//	}
//		
//	for (auto& t : threadVec) 
//	{
//		t.join();
//	}
//}

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

void GameCore::PlayerController::rotate(float angle) const
{
	gameCore.m_pInputCache.rotate += angle;
}
void GameCore::PlayerController::move_foreward(float amount) const
{
	gameCore.m_pInputCache.foreward += amount;
}
void GameCore::PlayerController::move_strafe(float amount) const
{
	gameCore.m_pInputCache.lateral += amount;
}

//void GameCore::view_by_ray_casting()
//{
//	math::Mat2x2 rotationMat = math::rotation_mat2x2(m_gameCamera.fov/2);
//	math::Vect2 playerForwDir{ std::cos(m_entityTransform.forewardAngle), std::sin(m_entityTransform.forewardAngle) };
//	math::Vect2 rayIncrement = (playerForwDir * rotationMat) * m_gameCamera.rayPrecision;
//	math::Vect2 startPos = m_entityTransform.coords;
//
//	for (int i = 0; i < m_gameCamera.pixelWidth; ++i)
//	{
//		math::Vect2 currentRay{ 0,0 };
//		EntityType hitMarker = EntityType::Nothing;
//
//		while (hitMarker == EntityType::Nothing && (currentRay.x * currentRay.x + currentRay.y * currentRay.y) < m_gameCamera.maxRenderDist * m_gameCamera.maxRenderDist)
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

void GameCore::view_by_ray_casting()
{
	math::Mat2x2 halfFOVRotationMat = math::rotation_mat2x2(m_gameCamera.fov / 2);
	math::Vect2 playerForwDir{ std::cos(m_entityTransform.forewardAngle), std::sin(m_entityTransform.forewardAngle) };
	math::Vect2 currentRayDir = playerForwDir * halfFOVRotationMat;
	math::Vect2 startPos = m_entityTransform.coords;
	math::Mat2x2 rayRotationIncrementMat = math::rotation_mat2x2(-m_gameCamera.fov / m_gameCamera.pixelWidth);

	for (int i = 0; i < m_gameCamera.pixelWidth; ++i)
	{
		//DDA

		math::Vect2 startingPos = m_entityTransform.coords;
		EntityType hitMarker = EntityType::Nothing;

		//increment in ray length that projects in an unitaty movement (unitaryStep) in X and Y direction
		float lengthIncrementX = std::sqrtf(1 + std::powf(currentRayDir.y / currentRayDir.x, 2));
		float lengthIncrementY = std::sqrtf(1 + std::powf(currentRayDir.x / currentRayDir.y, 2));

		//same as currentRay but rounded to int 
		int currentPosInMap[2] = { startingPos.x, startingPos.y };

		//length of the ray at progressive intersections with cell on the x and y axis
		float rayLenghtAtIntersectX;
		float rayLenghtAtIntersectY;

		int unitaryStepX;
		int unitaryStepY;

		float currentRayLength = 0;
		//initialize steps so they match the ray direction
		//also take care of the first shorter not unitary step
		if (currentRayDir.x < 0)
		{
			unitaryStepX = -1;
			rayLenghtAtIntersectX = (startingPos.x - currentPosInMap[0]) * lengthIncrementX;
		}
		else
		{
			unitaryStepX = 1;
			rayLenghtAtIntersectX = (float(currentPosInMap[0] + 1) - startingPos.x) * lengthIncrementX;
		}
		if (currentRayDir.y < 0)
		{
			unitaryStepY = -1;
			rayLenghtAtIntersectY = (startingPos.y - currentPosInMap[1]) * lengthIncrementY;
		}
		else
		{
			unitaryStepY = 1;
			rayLenghtAtIntersectY = (float(currentPosInMap[1] +1 ) - startingPos.y ) * lengthIncrementY;
		}

		//The ray is incremented in order to reach the next cell intersection
		//switching axis when one side becomes shorter then the other
		while (hitMarker == EntityType::Nothing && currentRayLength < m_gameCamera.maxRenderDist)
		{
			if (rayLenghtAtIntersectX < rayLenghtAtIntersectY)
			{
				currentPosInMap[0] += unitaryStepX;
				currentRayLength = rayLenghtAtIntersectX;
				rayLenghtAtIntersectX += lengthIncrementX;
			}
			else
			{
				currentPosInMap[1] += unitaryStepY;
				currentRayLength = rayLenghtAtIntersectY;
				rayLenghtAtIntersectY += lengthIncrementY;
			}
			chech_position_in_map(currentPosInMap[0], currentPosInMap[1], hitMarker);
			
		}

		m_rayInfoArr.at(i) = { hitMarker, currentRayDir * currentRayLength, currentRayLength };
		//std::cout << currentRayLength << " " << currentRayDir.y << std::endl;
		//rotate ray for next iteration
		currentRayDir *= math::rotation_mat2x2(-m_gameCamera.fov / m_gameCamera.pixelWidth);
	}
}
/*
{
	for (int x = 0; x < w; x++)
	{
		//calculate ray position and direction
		//double cameraX = 2 * x / (double)w - 1; //x-coordinate in camera space
		//double rayDirX = dirX + planeX * cameraX;
		//double rayDirY = dirY + planeY * cameraX;

		//which box of the map we're in
		int mapX = int(posX);
		int mapY = int(posY);

		//length of ray from current position to next x or y-side
		double sideDistX;
		double sideDistY;

		//length of ray from one x or y-side to next x or y-side
		double deltaDistX = (rayDirX == 0) ? 1e30 : std::abs(1 / rayDirX);
		double deltaDistY = (rayDirY == 0) ? 1e30 : std::abs(1 / rayDirY);
		double perpWallDist;

		//what direction to step in x or y-direction (either +1 or -1)
		int stepX;
		int stepY;

		int hit = 0; //was there a wall hit?
		int side; //was a NS or a EW wall hit?

		//calculate step and initial sideDist
		if (rayDirX < 0)
		{
			stepX = -1;
			sideDistX = (posX - mapX) * deltaDistX;
		}
		else
		{
			stepX = 1;
			sideDistX = (mapX + 1.0 - posX) * deltaDistX;
		}
		if (rayDirY < 0)
		{
			stepY = -1;
			sideDistY = (posY - mapY) * deltaDistY;
		}
		else
		{
			stepY = 1;
			sideDistY = (mapY + 1.0 - posY) * deltaDistY;
		}
		//perform DDA
		while (hit == 0)
		{
			//jump to next map square, either in x-direction, or in y-direction
			if (sideDistX < sideDistY)
			{
				sideDistX += deltaDistX;
				mapX += stepX;
				side = 0;
			}
			else
			{
				sideDistY += deltaDistY;
				mapY += stepY;
				side = 1;
			}
			//Check if ray has hit a wall
			if (worldMap[mapX][mapY] > 0) hit = 1;
		}

		//Calculate distance of perpendicular ray (Euclidean distance would give fisheye effect!)
		if (side == 0) perpWallDist = (sideDistX - deltaDistX);
		else          perpWallDist = (sideDistY - deltaDistY);

}*/