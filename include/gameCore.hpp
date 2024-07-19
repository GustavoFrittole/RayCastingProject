#ifndef GAMECORE_HPP
#define GAMECORE_HPP

#include<cstdint>
#include<memory>
#include<string>
#include"utils.hpp"
#include<vector>

enum class EntityType : char {
	Wall = 'w',
	Oob = 'o',
	Empty = '\0'
};

struct  MinimapData 
{
	
};

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
	EntityType entityHit;
	glm::vec2 hitPos;
};

bool fill_map_form_file(GameMap*, EntityTransform& , const std::string& );

class GameCore 
{
public:
	GameCore(GameCamera gc)
		: m_gameCamera(gc) {}

	bool load_map(const std::string&);

	std::vector<RayInfo>  view_by_ray_casting();

	//get_minimap_info();

private:
	EntityTransform m_entityTransform;
	GameCamera m_gameCamera;
	GameMap m_gameMap;

	bool check_out_of_map_bounds(const glm::vec2 &);
};

#endif