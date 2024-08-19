#include "gameCore.hpp"
#include <fstream>
#include <thread>
#include <stdexcept>
#include <cmath>

#define TIME_CORRECTION 0.000000001f

//----------------------RayInfo----------------------

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

//----------------------GameCore----------------------

GameCore::GameCore(GameCameraVars& gameCameraVars, GameMap& gameMap, EntityTransform& entityTransform) : 
	m_gameCamera(gameCameraVars),
	m_gameMap(gameMap),
	m_playerTransform(entityTransform),
	m_processorCount(get_thread_number()),
	m_rayInfoArr(gameCameraVars.pixelWidth)
{

	if (m_gameMap.generated)
	{
		if (m_gameMap.cells.get() == nullptr)
			m_gameMap.cells = std::make_unique<std::string>();

		m_mapGenerator = std::make_unique<MapGenerator>((int)m_playerTransform.coords.x, (int)m_playerTransform.coords.y, m_gameMap.x, m_gameMap.y, *(m_gameMap.cells));
	}
	//camera plane vars
	m_cameraVecs.forewardDirection = { std::cos(m_playerTransform.forewardAngle), std::sin(m_playerTransform.forewardAngle) };
	m_cameraVecs.plane = math::Vect2( m_cameraVecs.forewardDirection.y , -m_cameraVecs.forewardDirection.x ) * std::tan( m_gameCamera.fov/2 ) * 2;
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

void GameCore::add_billboard_sprite(int id, const EntityTransform& entityTransform)
{
	m_billboards.emplace_back(id, entityTransform);
}

bool GameCore::generate_map_step() 
{ 
	return ((m_mapGenerator.get() != nullptr) && m_mapGenerator->generate_map_step()); 
}
bool GameCore::generate_map() 
{ 
	return ((m_mapGenerator.get() != nullptr) && m_mapGenerator->generate_map()); 
}

void GameCore::update_entities()
{
	//calculate delta time and update internal clock
	auto currentTime = std::chrono::high_resolution_clock::now();
	int deltaTime = (currentTime - m_lastTime).count();
	m_lastTime = currentTime;
	float correctionFactor = TIME_CORRECTION * deltaTime;

	//decompose movment in x and y(world) axis and check collions separatly

	//check_collision X axis
	math::Vect2 moveAttempt = m_playerTransform.coords +
		(math::Vect2(std::cos(m_playerTransform.forewardAngle) * m_pInputCache.foreward 
					-std::sin(m_playerTransform.forewardAngle) * m_pInputCache.lateral, 0)
					* (correctionFactor));

	EntityType hitMarker = EntityType::NoHit;
	chech_position_in_map(moveAttempt, hitMarker);

	//update entities
	if (hitMarker == EntityType::NoHit) //check for any unblocking tiles 
	{
		m_playerTransform.coords = moveAttempt;
	}


	//check_collision Y axis
	moveAttempt = m_playerTransform.coords +
		(math::Vect2(0, std::sin(m_playerTransform.forewardAngle) * m_pInputCache.foreward +
					std::cos(m_playerTransform.forewardAngle) * m_pInputCache.lateral)
					* (correctionFactor));



	hitMarker = EntityType::NoHit;
	chech_position_in_map(moveAttempt, hitMarker);

	//update entities
	if (hitMarker == EntityType::NoHit) //check for any unblocking tiles 
	{
		m_playerTransform.coords = moveAttempt;
	}

	//rotate
	m_playerTransform.forewardAngle += m_pInputCache.rotate * correctionFactor;
	m_cameraVecs.forewardDirection = { std::cos(m_playerTransform.forewardAngle), std::sin(m_playerTransform.forewardAngle) };
	m_cameraVecs.plane = math::Vect2(m_cameraVecs.forewardDirection.y, -m_cameraVecs.forewardDirection.x) * std::tan(m_gameCamera.fov / 2) * 2;

	if (m_playerTransform.forewardAngle >= PI)
		m_playerTransform.forewardAngle -= 2*PI;
	else if(m_playerTransform.forewardAngle <= -PI)
		m_playerTransform.forewardAngle += 2*PI;

	//reset cached values
	m_pInputCache.foreward = 0;
	m_pInputCache.lateral = 0;
	m_pInputCache.rotate = 0;
}

void GameCore::GameController::rotate(float angle) const
{
	gameCore.m_pInputCache.rotate += angle;
}
void GameCore::GameController::move_foreward(float amount) const
{
	gameCore.m_pInputCache.foreward += amount;
}
void GameCore::GameController::move_strafe(float amount) const
{
	gameCore.m_pInputCache.lateral += amount;
}

void GameCore::view_by_ray_casting(bool useCameraPlane)
{
	view_walls(useCameraPlane);
	view_billboards(useCameraPlane);
}

void GameCore::view_walls(bool useCameraPlane)
{
	math::Vect2 startPos = m_playerTransform.coords;
	math::Vect2 currentRayDir;
	math::Vect2 rayRotationIncrement;
	math::Mat2x2 rayRotationIncrementMat;

	if (useCameraPlane)
	{
		currentRayDir = m_cameraVecs.forewardDirection - m_cameraVecs.plane / 2;
		rayRotationIncrement = m_cameraVecs.plane / (m_gameCamera.pixelWidth);
	}
	else
	{
		math::Mat2x2 halfFOVRotationMat = math::rotation_mat2x2(m_gameCamera.fov / 2);
		math::Vect2 playerForwDir{ std::cos(m_playerTransform.forewardAngle), std::sin(m_playerTransform.forewardAngle) };
		currentRayDir = playerForwDir * halfFOVRotationMat;
		rayRotationIncrementMat = math::rotation_mat2x2(-m_gameCamera.fov / m_gameCamera.pixelWidth);
	}

	for (int i = 0; i < m_gameCamera.pixelWidth; ++i)
	{
		//DDA

		math::Vect2 startingPos = m_playerTransform.coords;
		EntityType hitMarker = EntityType::Nothing;

		float lengthIncrementX;
		float lengthIncrementY;

		if (useCameraPlane)
		{
			//increment in ray length that projects in an unitaty movement (unitaryStep) in X and Y direction
			lengthIncrementX = (1 / std::abs(currentRayDir.x));
			lengthIncrementY = (1 / std::abs(currentRayDir.y));
		}
		else
		{
			lengthIncrementX = std::sqrt(1 + std::pow(currentRayDir.y / currentRayDir.x, 2));
			lengthIncrementY = std::sqrt(1 + std::pow(currentRayDir.x / currentRayDir.y, 2));
		}
    
		//same as currentRay but rounded to int 
		int currentPosInMap[2] = { (int)startingPos.x, (int)startingPos.y };

		//length of the ray at progressive intersections with cell on the x and y axis
		float rayLengthAtIntersectX;
		float rayLengthAtIntersectY;

		int unitaryStepX;
		int unitaryStepY;

		float rayLength = 0;

		//initialize steps so they match the ray direction
		//also take care of the first shorter not unitary step
		if (currentRayDir.x < 0)
		{
			unitaryStepX = -1;
			rayLengthAtIntersectX = (startingPos.x - currentPosInMap[0]) * lengthIncrementX;
		}
		else
		{
			unitaryStepX = 1;
			rayLengthAtIntersectX = (float(currentPosInMap[0]) + (1 - startingPos.x)) * lengthIncrementX;
		}
		if (currentRayDir.y < 0)
		{
			unitaryStepY = -1;
			rayLengthAtIntersectY = (startingPos.y - currentPosInMap[1]) * lengthIncrementY;
		}
		else
		{
			unitaryStepY = 1;
			rayLengthAtIntersectY = (float(currentPosInMap[1]) + (1 - startingPos.y)) * lengthIncrementY;
		}

		//keeps track of what was the last cell side checked
		CellSide lastSideChecked = CellSide::Unknown;

		//The ray is incremented in order to reach the next cell intersection
		//switching axis when one side becomes shorter then the other
		while (hitMarker == EntityType::Nothing)
		{
			if (rayLengthAtIntersectX < rayLengthAtIntersectY)
			{
				if (rayLengthAtIntersectX > m_gameCamera.maxRenderDist)
					break;
				currentPosInMap[0] += unitaryStepX;
				lastSideChecked = CellSide::Vert;
				rayLengthAtIntersectX += lengthIncrementX;
			}
			else
			{
				if (rayLengthAtIntersectY > m_gameCamera.maxRenderDist)
					break;
				currentPosInMap[1] += unitaryStepY;
				lastSideChecked = CellSide::Hori;
				rayLengthAtIntersectY += lengthIncrementY;
			}
			chech_position_in_map(currentPosInMap[0], currentPosInMap[1], hitMarker);
			
		}

		//Roll back the increment that reached inside a solid cell.
		if (lastSideChecked == CellSide::Vert)
			rayLength = rayLengthAtIntersectX - lengthIncrementX;
		else 
			rayLength = rayLengthAtIntersectY - lengthIncrementY;

		m_rayInfoArr.at(i) = { hitMarker, currentRayDir * rayLength, rayLength, lastSideChecked };

		if (useCameraPlane)
		{
			//rotate ray for next iteration
			currentRayDir += rayRotationIncrement;
		}
		else
		{
			currentRayDir *= math::rotation_mat2x2(-m_gameCamera.fov / m_gameCamera.pixelWidth);
		}
	}
}

void GameCore::view_billboards(bool useCameraPlane)
{
	for (Billboard& b : m_billboards)
	{
		if(b.active)
		{
			math::Vect2 rayToCamera = b.entityTransform.coords - m_playerTransform.coords;
			float euclideanRayLength = rayToCamera.Length();

			//relative to camera foreward view
			float relativeAngle = m_playerTransform.forewardAngle - math::vec_to_rad(rayToCamera);
			if (relativeAngle >= PI)
				relativeAngle -= 2 * PI;
			else if (relativeAngle <= -PI)
				relativeAngle += 2 * PI;

			if (!useCameraPlane)
			{
				b.distance = euclideanRayLength;

				//mapping to screen pos
				b.positionOnScreen = (m_gameCamera.fov / 2 + (relativeAngle)) / (m_gameCamera.fov) * m_gameCamera.pixelWidth;

				b.visible = (b.distance < m_gameCamera.maxRenderDist);
			}
			else
			{
				b.distance = euclideanRayLength * std::cos(relativeAngle);

				//mapping to screen pos
				float projectionOnPlane = euclideanRayLength * std::sin(relativeAngle);
				float planeLength = std::tan(m_gameCamera.fov / 2) * 2;
				//since rays are obrained by adding the plane position to a vertical vector: (planePos, 1) * rayLength
				//the position on plane can be derived from the vector's x component normalized
				float positionOnPlane = projectionOnPlane / b.distance;
				b.positionOnScreen = (positionOnPlane / planeLength + 0.5f) * m_gameCamera.pixelWidth;

				b.visible = (b.distance < m_gameCamera.maxRenderDist);
			}
		}
	}
}

game::IGameController& GameCore::get_playerController() 
{
	if (m_playerController.get() == nullptr)
		m_playerController = std::make_unique<GameController>(*this);

	return *(m_playerController);
}

//Simple but inefficient form of ray casting 
//  
//void GameCore::view_by_ray_casting()
//{
//	math::Mat2x2 rotationMat = math::rotation_mat2x2(m_gameCamera.fov/2);
//	math::Vect2 playerForwDir{ std::cos(m_playerTransform.forewardAngle), std::sin(m_playerTransform.forewardAngle) };
//	math::Vect2 rayIncrement = (playerForwDir * rotationMat) * m_gameCamera.rayPrecision;
//	math::Vect2 startPos = m_playerTransform.coords;
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
