#ifndef GAMECORE_HPP
#define GAMECORE_HPP

#include<cstdint>
#include<memory>
#include<glm/vec2.hpp>
#include<string>

struct EntityTransform 
{
	glm::vec2 coords{};
	float forewardAngle = 0;
};

struct GameCamera 
{
	uint16_t screenXY[2]{};
	float fov = 0.f;
	float maxRenderDist = 0.f;
	float rayPrecision = 0.1f;
};

struct GameMap
{
	uint16_t x = 0, y = 0;
	std::string cells;
};

struct RayInfo
{
	char entityHit;
	glm::vec2 hitPos;
};

bool fill_map_form_file(GameMap*, EntityTransform& , const std::string& );

class GameCore 
{
public:
	GameCore(GameCamera gc)
		: m_gameCamera(gc) {}

	bool load_map(const std::string&);

	std::unique_ptr<RayInfo[]>  view_by_ray_casting();

private:
	EntityTransform m_entityTransform;
	GameCamera m_gameCamera;
	GameMap m_gameMap;
};

#endif