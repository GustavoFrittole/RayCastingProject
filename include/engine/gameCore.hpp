#ifndef GAMECORE_HPP
#define GAMECORE_HPP

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

enum class CellSide 
{
	Vert,
	Hori,
	Unknown
};

struct GameStateData
{
	const float& fov;
	const float& maxRenderDist;
	const EntityTransform& playerTransform;
	const GameMap& gameMap;
	const math::Vect2& m_cameraDir;
	const math::Vect2& m_cameraPlane;
};

struct RayInfo
{
	EntityType entityHit = EntityType::NoHit;
	math::Vect2 hitPos = {0, 0};
	float length = 0;
	CellSide lastSideChecked = CellSide::Unknown;
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

struct Billboard
{
	Billboard(int id, const EntityTransform& et) :
		id(id),
		entityTransform(et)
	{}
	int id = -1;
	EntityTransform entityTransform;
	float distance = 0.f;
	float positionOnScreen = 0.f;
	bool active = true;
	bool visible = false;
};

class GameCore
{
public:
	GameCore() = delete;
	GameCore(GameCamera gc, GameMap&, EntityTransform&);

	void update_entities();
	void view_by_ray_casting(bool cameraPlane);
	void start_internal_time();
	GameStateData get_map_data() const;
	const RayInfoArr& get_ray_info_arr()  const { return m_rayInfoArr; }
	const std::vector<Billboard>& get_billboards_info_arr() const { return m_billboards; } 
	bool generate_map_step() { return ((m_mapGenerator.get() != nullptr) && m_mapGenerator->generate_map_step()); }
	bool generate_map() { return ((m_mapGenerator.get() != nullptr) && m_mapGenerator->generate_map()); }
	void add_billboard_sprite( int, const EntityTransform& );

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
	math::Vect2 m_cameraPlane{1,0};
	math::Vect2 m_cameraDir{0,1};
	EntityTransform m_playerTransform{};
	GameMap m_gameMap{};
	PlayerInputCache m_pInputCache{};
	PlayerController m_playerController;
	std::chrono::time_point<std::chrono::high_resolution_clock> m_lastTime;
	int m_processorCount = 1;
	RayInfoArr m_rayInfoArr;
	std::unique_ptr<MapGenerator> m_mapGenerator;
	std::vector<Billboard> m_billboards;

	void view_walls(bool);
	void view_billboards(bool);
	
	void chech_position_in_map(int, int, EntityType&) const;
	void chech_position_in_map(const math::Vect2&, EntityType&) const;

	bool check_out_of_map_bounds(const math::Vect2 &) const;
	bool check_out_of_map_bounds(int, int) const;
};

#endif