#ifndef GAMECORE_HPP
#define GAMECORE_HPP

#include<vector>
#include"mapGenerator.hpp"
#include"dataManager.hpp"

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

struct MapData
{
	const float& fov;
	const float& maxRenderDist;
	const EntityTransform& playerTransform;
	const GameMap& gameMap;
};

struct RayInfo
{
	EntityType entityHit = EntityType::NoHit;
	math::Vect2 hitPos = {0, 0};
	float dist = 0;
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
	GameCore(GameCamera gc, GameMap&, EntityTransform&);

	void update_entities();
	void view_by_ray_casting();
	void start_internal_time();
	MapData getMapData() const;
	const RayInfoArr& getRayInfoArr() { return m_rayInfoArr; };
	bool generate_map_step() { return ((m_mapGenerator.get() != nullptr) && m_mapGenerator->generate_map_step()); }
	bool generate_map() { return ((m_mapGenerator.get() != nullptr) && m_mapGenerator->generate_map()); }

	//should be singelton
	class PlayerController 
	{
	public:
		PlayerController(GameCore& gc) : gameCore(gc){}
		void rotate(float) const;
		void move_foreward(float) const;
		void move_strafe(float) const;
	private:
		GameCore& gameCore;
	};

	PlayerController& get_playerController() { return m_playerController; }

private:
	GameCamera m_gameCamera{};
	EntityTransform m_entityTransform{};
	GameMap m_gameMap{};
	PlayerInputCache m_pInputCache{};
	PlayerController m_playerController;
	std::chrono::time_point<std::chrono::high_resolution_clock> m_lastTime;
	int m_processorCount = 1;
	RayInfoArr m_rayInfoArr;
	std::unique_ptr<MapGenerator> m_mapGenerator;
	
	inline void GameCore::chech_position_in_map(const math::Vect2&, EntityType&) const;
	inline void GameCore::chech_position_in_map(int, int, EntityType&) const;
	bool check_out_of_map_bounds(const math::Vect2 &) const;
	bool check_out_of_map_bounds(int, int) const;
};

#endif