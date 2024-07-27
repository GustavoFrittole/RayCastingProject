#ifndef GAMECORE_HPP
#define GAMECORE_HPP

#include<cstdint>
#include<vector>
#include<chrono>
#include"utils.hpp"
#include"mapGenerator.hpp"

#define DEFAULT_MAP_PATH "map.txt"

enum class EntityType : char
{
	Wall = 'w',
	Baudry = 'b',
	Nothing = 'n',
	Goal = 'g',
	Oob = 'o',
	NoHit = '\0'
};

struct EntityTransform 
{
	math::Vect2 coords{};
	float forewardAngle = 0;
};

struct GameCamera 
{
	int pixelWidth = 0;
	int pixelHeight = 0;
	float fov = 90.f;
	float maxRenderDist = 10.f;
	float rayPrecision = 0.1f;
};

struct GameMap
{
	uint16_t x = 0, y = 0;
	std::string cells;
};

struct MapData
{
	const float& fov;
	const float& maxRenderDist;
	const EntityTransform& playerTransform;
	const GameMap& gameMap;
	const bool& generated;
};

struct RayInfo
{
	EntityType entityHit = EntityType::NoHit;
	math::Vect2 hitPos = {0, 0};
};

class RayInfoArr 
{
public:
	RayInfoArr(int size) : arrSize(size) { m_rayArr = new RayInfo[size]{}; }
	~RayInfoArr() { delete [] m_rayArr; }
	RayInfo& at(int);
	const RayInfo& const_at(int) const;
	RayInfoArr& operator=(const RayInfoArr&) = delete;
	RayInfoArr(const RayInfoArr&) = delete;

	const int arrSize;
private:
	RayInfo* m_rayArr;
};
struct PlayerInputCache
{
	float foreward = 0;
	float lateral = 0;
	float rotate = 0;
};

class GameCore
{
public:
	GameCore() = delete;
	GameCore(GameCamera gc, const std::string&);

	bool load_map_form_file();
	void update_entities();
	void view_by_ray_casting();
	void start_internal_time();
	MapData getMapData() const;
	const RayInfoArr& getRayInfoArr() { return m_rayInfoArr; };
	bool generate_map_step() { return ((m_mapGenerator.get() != nullptr) && m_mapGenerator->generate_map_step()); };
	bool generate_map() { return ((m_mapGenerator.get() != nullptr) && m_mapGenerator->generate_map()); }

	//get_minimap_info();

	//should be singelton
	class PlayerControler 
	{
	public:
		PlayerControler(GameCore& gc) : gameCore(gc){}
		void rotate(float) const;
		void move_foreward(float) const;
		void move_strafe(float) const;
	private:
		GameCore& gameCore;
	};

private:
	EntityTransform m_entityTransform{};
	GameCamera m_gameCamera{};
	GameMap m_gameMap{};
	PlayerInputCache m_pInputCache{};
	std::chrono::time_point<std::chrono::high_resolution_clock> m_lastTime;
	int m_processorCount = 1;
	RayInfoArr m_rayInfoArr;
	std::string m_gameMapFilePath = DEFAULT_MAP_PATH;
	std::unique_ptr<MapGenerator> m_mapGenerator;
	bool m_map_is_generated = false;
	
	inline void GameCore::chech_position_in_map(const math::Vect2&, EntityType&) const;
	bool check_out_of_map_bounds(const math::Vect2 &) const;
};

#endif