#ifndef GAMECORE_HPP
#define GAMECORE_HPP

#include "mapGenerator.hpp"
#include "gameDataStructures.hpp"

#define DEFAULT_MAP_PATH "map.txt"

class GameCore
{
public:
	GameCore() = delete;
	GameCore(rcm::GameCameraVars& gc, rcm::GameMap&, rcm::EntityTransform&);

	void update_entities();
	void remove_destroyed_entities();
	void view_by_ray_casting(bool cameraPlane);
	void start_internal_time();

	const rcm::GameCameraPlane& get_camera_vecs() const { return m_cameraVecs; }
	const rcm::RayInfoArr& get_ray_info_arr()  const { return m_rayInfoArr; }

	std::vector<std::unique_ptr<rcm::IEntity>>& get_entities() { return m_entities; }
	void add_entity(rcm::IEntity *);


	/// @brief Change the HitType if a physical map structure is hit, otherwise leave it as is
	/// @param rayPosInMapX
	/// @param rayPosInMapY
	/// @param hitMarker : marker to change if structure is struck
	void chech_position_in_map(int, int, rcm::HitType&) const;

	/// @brief Change the HitType if a physical map structure is hit, otherwise leave it as is
	/// @param rayPosInMap map position to check
	/// @param hitMarker marker to change if structure is struck
	void chech_position_in_map(const math::Vect2&, rcm::HitType&) const;

	bool generate_map_step();
	bool generate_map();

private:
	rcm::GameCameraVars& m_gameCamera;
	rcm::GameMap& m_gameMap;
	rcm::EntityTransform& m_playerTransform;
	rcm::GameCameraPlane m_cameraVecs{ {1,0}, {0,1} };

	std::chrono::time_point<std::chrono::high_resolution_clock> m_lastTime;
	int m_processorCount = 1;

	rcm::RayInfoArr m_rayInfoArr;
	std::unique_ptr<MapGenerator> m_mapGenerator;
	std::vector<std::unique_ptr<rcm::IEntity>> m_entities;

	void view_walls(bool);
	void view_billboards(bool);

	bool check_out_of_map_bounds(const math::Vect2 &) const;
	bool check_out_of_map_bounds(int, int) const;

	bool move_entity_space(rcm::EntityTransform&, float, float, float);
	bool move_entity_with_collisions_entity_space(rcm::EntityTransform&, float, float, float, float);
	//bool move_entity_with_collisions_world_space(EntityTransform&, const math::Vect2&, float);
};

#endif