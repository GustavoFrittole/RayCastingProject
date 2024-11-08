#include "gameCore.hpp"
#include <fstream>
#include <thread>
#include <stdexcept>
#include <cmath>
#include <cassert>

#define TIME_CORRECTION 1e-9f

using namespace rcm;

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

GameCore::GameCore(GameCameraVars& gameCameraVars, GameMap& gameMap, EntityTransform& transform) : 
	m_gameCamera(gameCameraVars),
	m_gameMap(gameMap),
	m_playerTransform(transform),
	m_processorCount(utils::get_thread_number()),
	m_rayInfoArr(gameCameraVars.pixelWidth)
{

	if (m_gameMap.generated)
	{
		if (m_gameMap.cells.get() == nullptr)
			m_gameMap.cells = std::make_unique<std::string>();

		m_mapGenerator = std::make_unique<MapGenerator>((int)m_playerTransform.coordinates.x, (int)m_playerTransform.coordinates.y, m_gameMap.width, m_gameMap.height, *(m_gameMap.cells));
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
	return (posX < 0 || posY < 0 || posX >= m_gameMap.width || posY >= m_gameMap.height);
}

void GameCore::start_internal_time()
{
	m_lastTime = std::chrono::high_resolution_clock::now();
}


void GameCore::chech_position_in_map(const math::Vect2& rayPosInMap, HitType& hitMarker) const
{
	chech_position_in_map((int)rayPosInMap.x, (int)rayPosInMap.y, hitMarker);
}

void GameCore::chech_position_in_map(int rayPosInMapX, int rayPosInMapY, HitType& hitMarker) const
{
	if (!check_out_of_map_bounds(rayPosInMapX, rayPosInMapY))
	{
		switch (m_gameMap.cells->at(rayPosInMapX + rayPosInMapY * m_gameMap.width))
		{
		case 'w':
			hitMarker = HitType::Wall;
			break;
		case 'b':
			hitMarker = HitType::Baudry;
			break;
		default:
			break;
		}
	}
	else
	{
		hitMarker = HitType::Oob;
	}
}

void GameCore::add_entity(IEntity* entity)
{
	m_entities.emplace_back(entity);
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
	float timeFactor = TIME_CORRECTION * deltaTime;

	//-----entities movement------
	
	for (std::unique_ptr<IEntity>& entity : m_entities)
	{
		float frictionModule(entity->m_physical.movementFrictionCoef * entity->m_physical.mass);
		//apply friction module in opposite direction of movement (rolling friction)
		math::Vect2 friction = (entity->m_physical.movementSpeed) * (- frictionModule);

		if (!entity->m_physical.isGhosted)
		{
			//apply acceleration
			entity->m_physical.movementSpeed += ((friction + entity->m_physical.movementAcceleration) * timeFactor);
			entity->m_physical.rotationSpeed += ((entity->m_physical.rotationFrictionCoef * (- entity->m_physical.mass) + entity->m_physical.rotationAcceleraion) * timeFactor);

			//apply speed
			move_entity_with_collisions_entity_space(entity->m_transform,
				entity->m_physical.movementSpeed.x * timeFactor,
				entity->m_physical.movementSpeed.y * timeFactor,
				entity->m_physical.rotationSpeed * timeFactor,
				entity->m_collisionSize);
		}
		else
		{
			//only apply speed
			if (!move_entity_space(entity->m_transform,
				entity->m_physical.movementSpeed.x * timeFactor,
				entity->m_physical.movementSpeed.y * timeFactor,
				entity->m_physical.rotationSpeed * timeFactor))
			{
				//entities oob or inside walls get destroyed
				entity->set_destroyed(true);
			}
		}
	}

	//update camera direction
	m_cameraVecs.forewardDirection = {
		std::cos(m_playerTransform.forewardAngle),
		std::sin(m_playerTransform.forewardAngle)
	};
	m_cameraVecs.plane = math::Vect2(
		m_cameraVecs.forewardDirection.y,
		-m_cameraVecs.forewardDirection.x
	) * std::tan(m_gameCamera.fov / 2) * 2;
}

void GameCore::remove_destroyed_entities()
{
	for (std::vector<std::unique_ptr<IEntity>>::iterator it = m_entities.begin(); it != m_entities.end();)
	{
		if ((*it)->get_destroyed())
			it = m_entities.erase(it);
		else
			++it;
	}
}

void GameCore::view_by_ray_casting(bool useCameraPlane)
{
	view_walls(useCameraPlane);
	view_billboards(useCameraPlane);
}

void GameCore::view_walls(bool useCameraPlane)
{
	math::Vect2 startPos = m_playerTransform.coordinates;
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

		math::Vect2 startingPos = m_playerTransform.coordinates;
		HitType hitMarker = HitType::Nothing;

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
		while (hitMarker == HitType::Nothing)
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
	for (std::unique_ptr<IEntity>& entity : m_entities)
	{
		if(entity->get_active())
		{
			math::Vect2 rayToCamera = entity->m_transform.coordinates - m_playerTransform.coordinates;
			float euclideanRayLength = rayToCamera.Length();

			//relative to camera foreward view
			float entityRelativeAngle = m_playerTransform.forewardAngle - math::vec_to_rad(rayToCamera);
			if (entityRelativeAngle >= PI)
				entityRelativeAngle -= 2 * PI;
			else if (entityRelativeAngle <= -PI)
				entityRelativeAngle += 2 * PI;

			entity->m_billboard.cameraAngle = entity->m_transform.forewardAngle - math::vec_to_rad(rayToCamera);
			if (entity->m_billboard.cameraAngle >= PI)
				entity->m_billboard.cameraAngle -= 2 * PI;
			else if (entity->m_billboard.cameraAngle <= -PI)
				entity->m_billboard.cameraAngle += 2 * PI;

			if (!useCameraPlane)
			{
				entity->m_billboard.distance = euclideanRayLength;

				//mapping to screen pos
				entity->m_billboard.positionOnScreen = (m_gameCamera.fov / 2 + (entityRelativeAngle)) / (m_gameCamera.fov) * m_gameCamera.pixelWidth;

				entity->m_visible = entity->m_billboard.distance < m_gameCamera.maxRenderDist;
			}
			else
			{
				entity->m_billboard.distance = euclideanRayLength * std::cos(entityRelativeAngle);

				//mapping to screen pos
				float projectionOnPlane = euclideanRayLength * std::sin(entityRelativeAngle);
				float planeLength = std::tan(m_gameCamera.fov / 2) * 2;

				//since rays are obtained by adding the plane position to a vertical vector: (planePos, 1) * rayLength
				//the position on plane can be derived from the vector's x component normalized
				float positionOnPlane = projectionOnPlane / entity->m_billboard.distance;
				entity->m_billboard.positionOnScreen = (positionOnPlane / planeLength + 0.5f) * m_gameCamera.pixelWidth;

				entity->m_visible = entity->m_billboard.distance < m_gameCamera.maxRenderDist;
			}
		}
	}
}

bool GameCore::move_entity_with_collisions_entity_space(EntityTransform& transform, float front, float latereal, float rotation, float collisionSize)
{
	bool hasMoved = false;

	//rotate
	transform.forewardAngle += rotation;

	if (transform.forewardAngle >= PI)
		transform.forewardAngle -= 2 * PI;
	else if (transform.forewardAngle <= -PI)
		transform.forewardAngle += 2 * PI;

	if(front != 0.f || latereal != 0.f)
	{
		if (front != 0.f && latereal != 0.f)
		{
			//movement values are assumed to have equal absulute value when active
			assert(std::abs(front) == std::abs(latereal));
			front = front / SQRT2;
			latereal = latereal / SQRT2;
		}

		//decompose movment in x and y (world) axis and check collions separatly
		//--------- check_collision X axis ------------
		math::Vect2 moveAttempt = (math::Vect2(std::cos(transform.forewardAngle) * front
			- std::sin(transform.forewardAngle) * latereal, 0));

		math::Vect2 extension{};

		auto check_if_empty_space = [this](const math::Vect2& pos) mutable ->bool
			{
				HitType hitMarker = HitType::NoHit;

				chech_position_in_map(pos, hitMarker);

				return hitMarker == HitType::NoHit;
			};

		if (moveAttempt.x != 0.f)
		{
			//make sure collision size is smaller than wall size
			if (collisionSize < 1.f)
			{
				if (moveAttempt.x < 0)
					extension.x = -collisionSize;
				else
					extension.x = collisionSize;
			}

			if (check_if_empty_space(transform.coordinates + (moveAttempt + extension)) &&
				check_if_empty_space(transform.coordinates + (moveAttempt + math::Vect2(extension.x, collisionSize))) &&
				check_if_empty_space(transform.coordinates + (moveAttempt + math::Vect2(extension.x, -collisionSize))))
			{
				hasMoved = true;
			}
			else
			{
				moveAttempt.x = 0.f;
			}
		}

		//--------- check_collision Y axis ------------
		moveAttempt += (math::Vect2(0, std::sin(transform.forewardAngle) * front +
			std::cos(transform.forewardAngle) * latereal));

		if(moveAttempt.y != 0.f)
		{
			if (collisionSize < 1.f)
			{
				if (moveAttempt.y < 0)
					extension.y = -collisionSize;
				else
					extension.y = collisionSize;
			}

			if (check_if_empty_space(transform.coordinates + (moveAttempt + extension)) &&
				check_if_empty_space(transform.coordinates + (moveAttempt + math::Vect2(-collisionSize, extension.y))) &&
				check_if_empty_space(transform.coordinates + (moveAttempt + math::Vect2(collisionSize, extension.y))))
			{
				hasMoved = true;
			}
			else
			{
				moveAttempt.y = 0.f;
			}
		}	
		transform.coordinates += moveAttempt;
	}

	return hasMoved;
}

bool GameCore::move_entity_space(EntityTransform& transform, float front, float latereal, float rotation)
{
	//decompose movment in x and y(world) axis and check collions separatly

//check_collision X axis
	math::Vect2 moveAttempt = transform.coordinates +
		(math::Vect2(	std::cos(transform.forewardAngle) * front
						- std::sin(transform.forewardAngle) * latereal, 
						std::sin(transform.forewardAngle) * front +
						std::cos(transform.forewardAngle) * latereal));

	if (check_out_of_map_bounds(moveAttempt))
		return false;

	transform.coordinates = moveAttempt;

	//rotate
	transform.forewardAngle += rotation;

	if (transform.forewardAngle >= PI)
		transform.forewardAngle -= 2 * PI;
	else if (transform.forewardAngle <= -PI)
		transform.forewardAngle += 2 * PI;

	return true;
}

//Simple but inefficient form of ray casting 
//  
//void GameCore::view_by_ray_casting()
//{
//	math::Mat2x2 rotationMat = math::rotation_mat2x2(m_gameCamera.fov/2);
//	math::Vect2 playerForwDir{ std::cos(m_playerTransform.forewardAngle), std::sin(m_playerTransform.forewardAngle) };
//	math::Vect2 rayIncrement = (playerForwDir * rotationMat) * m_gameCamera.rayPrecision;
//	math::Vect2 startPos = m_playerTransform.coordinates;
//
//	for (int i = 0; i < m_gameCamera.pixelWidth; ++i)
//	{
//		math::Vect2 currentRay{ 0,0 };
//		HitType hitMarker = HitType::Nothing;
//
//		while (hitMarker == HitType::Nothing && (currentRay.x * currentRay.x + currentRay.y * currentRay.y) < m_gameCamera.maxRenderDist * m_gameCamera.maxRenderDist)
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
