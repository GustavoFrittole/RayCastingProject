#ifndef GAMECORE_HPP
#define GAMECORE_HPP

#include "mapGenerator.hpp"
#include "gameDataStructures.hpp"

#define DEFAULT_MAP_PATH "map.txt"

struct PlayerInputCache
{
	float foreward = 0;
	float lateral = 0;
	float rotatation = 0;
};

class GameCore
{
public:
	GameCore() = delete;
	GameCore(GameCameraVars& gc, GameMap&, EntityTransform&);

	void update_entities();
	void view_by_ray_casting(bool cameraPlane);
	void start_internal_time();

	const GameCameraVecs& get_camera_vecs() const { return m_cameraVecs; }
	const RayInfoArr& get_ray_info_arr()  const { return m_rayInfoArr; }

	std::vector<Entity>& get_entities() { return m_entities; } 
	void add_entity(int, const EntityTransform&);
	void add_entity(const Entity&);

	bool generate_map_step();
	bool generate_map();

	class GameController : public game::IGameController
	{
	public:
		GameController(GameCore& gc) : gameCore(gc) {};
		void rotate(float) const override;
		void move_foreward(float) const override;
		void move_strafe(float) const override;
	private:
		GameCore& gameCore;
	};

	game::IGameController& get_playerController();

private:
	GameCameraVars& m_gameCamera;
	GameMap& m_gameMap;
	EntityTransform& m_playerTransform;
	GameCameraVecs m_cameraVecs{ {1,0}, {0,1} };

	PlayerInputCache m_pInputCache{};
	std::unique_ptr<GameController> m_playerController;

	std::chrono::time_point<std::chrono::high_resolution_clock> m_lastTime;
	int m_processorCount = 1;

	RayInfoArr m_rayInfoArr;
	std::unique_ptr<MapGenerator> m_mapGenerator;
	std::vector<Entity> m_entities;

	void view_walls(bool);
	void view_billboards(bool);
	
	void chech_position_in_map(int, int, HitType&) const;
	void chech_position_in_map(const math::Vect2&, HitType&) const;

	bool check_out_of_map_bounds(const math::Vect2 &) const;
	bool check_out_of_map_bounds(int, int) const;

	bool move_entity_space(EntityTransform&, float, float, float);
	bool move_entity_with_collisions_entity_space(EntityTransform&, float, float, float);
	//bool move_entity_with_collisions_world_space(EntityTransform&, const math::Vect2&, float);
};

#endif