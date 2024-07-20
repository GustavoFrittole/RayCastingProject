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

struct  MapData 
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


struct PlayerInputCache
{
	float foreward = 0;
	float lateral = 0;
	float rotate = 0;
};

bool fill_map_form_file(GameMap*, EntityTransform& , const std::string& );

class GameCore 
{
public:
	GameCore(GameCamera gc)
		: m_gameCamera(gc) {}

	bool load_map(const std::string&);
	void update_entities();
	std::vector<RayInfo>  view_by_ray_casting() const;
	void start_internal_time();

	//get_minimap_info();
	//idea: gameCore should contain different transforms for different entitys in a dedicated strucure
	//		atm implementing new entities is premature
	//should be singelton
	class PlayerControler 
	{
	public:
		PlayerControler(GameCore* gc) : gameCore(gc){}
		void rotate(float) const;
		void move_foreward(float) const;
		void move_strafe(float) const;
	private:
		GameCore* gameCore;
	};

private:
	EntityTransform m_entityTransform{};
	GameCamera m_gameCamera{};
	GameMap m_gameMap{};
	PlayerInputCache m_pInputCache{};
	std::chrono::time_point<std::chrono::high_resolution_clock> m_lastTime;
	
	inline void GameCore::chech_position_in_map(const glm::vec2&, EntityType&) const;
	bool check_out_of_map_bounds(const glm::vec2 &) const;
};

#endif