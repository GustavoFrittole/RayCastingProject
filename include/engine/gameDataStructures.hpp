#ifndef GAMEDATASTRUCTURES_HPP
#define GAMEDATASTRUCTURES_HPP

#include "utils.hpp"
#include <memory>

#define WINDOW_HEIGHT 720

namespace windowVars
{
	constexpr int g_windowHeight = WINDOW_HEIGHT;
	constexpr int g_windowWidth = g_windowHeight * 16 / 9;
}

namespace rcm
{
	struct EntityTransform
	{
		math::Vect2 coords{};
		float forewardAngle = 0.f;
	};

	struct GameCameraVars
	{
		int pixelWidth = 0;
		int pixelHeight = 0;
		float fov = 90.f;
		float maxRenderDist = 10.f;
		float rayPrecision = 0.1f;
	};

	struct GameCameraVecs
	{
		math::Vect2 forewardDirection;
		math::Vect2 plane;
	};

	struct GameCameraView
	{
		const EntityTransform& transform{};
		const GameCameraVars& vars{};
		const GameCameraVecs& vecs{};
	};

	struct ControlsSensitivity
	{
		float mouseSens = 1.f;
		float movementSpeed = 1.f;
	};

	struct GraphicsVars
	{
		int frameRate = 0;
		int minimapScale = 6;
		float halfWallHeight = 0.5f;
		float maxSightDepth = 10.f;
	};

	struct GameMap
	{
		int x = 0, y = 0;
		bool generated = false;
		std::unique_ptr<std::string> cells;
	};

	enum class HitType : char
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

	struct RayInfo
	{
		HitType entityHit = HitType::NoHit;
		math::Vect2 hitPos = { 0, 0 };
		float length = 0;
		CellSide lastSideChecked = CellSide::Unknown;
	};

	class RayInfoArr
	{
	public:
		RayInfoArr(int size) : arrSize(size) { m_rayArr = new RayInfo[size]{}; }
		~RayInfoArr() { delete[] m_rayArr; }
		RayInfo& at(int);
		const RayInfo& const_at(int) const;
		RayInfoArr& operator=(const RayInfoArr&) = delete;
		RayInfoArr(const RayInfoArr&) = delete;

		const int arrSize;
	private:
		RayInfo* m_rayArr;
	};

	struct Billboard
	{
		Billboard(int id) :
			id(id)
		{}
		int id = -1;
		float positionOnScreen = 0.f;
		float distance = 0.f;
		float size = 1.f;
	};

	enum class EntityType
	{
		player,
		projectile,
		enemy,
		prop,
		wall
	};

	struct PhysicalVars
	{
		//unaffected by external forces and architectural boundaries
		bool isGhosted = false;

		float movFrictionCoef = 0.f;
		float rotFrictionCoef = 0.f;
		float mass = 0.f;
		math::Vect2 speed{};
		float rotationSpeed = 0.f;
		math::Vect2 acceleration{};
		float rotationAcceleraion = 0.f;
	};

	struct IEntity
	{
		IEntity(int id, const EntityTransform& et) :
			m_billboard(id),
			m_transform(et)
		{}
		void set_size(float size) { m_collisionSize = size; m_billboard.size = size; }
		void apply_force(const math::Vect2& force)
		{
			if (m_physical.mass != 0)
				m_physical.acceleration += force / m_physical.mass;
		}
		virtual void on_create() = 0;
		virtual void on_update() = 0;
		virtual void on_hit(EntityType) = 0;

		Billboard m_billboard;
		EntityTransform m_transform{};
		PhysicalVars m_physical{};
		float m_collisionSize = 1.f;
		EntityType m_type = EntityType::prop;

		//flags entities that are to be removed
		bool destroyed = false;
		//deactivate distance based interactions (on_hit)
		bool interactible = false;
		//deactivete on_update script
		bool active = true;
		//automatically changed based on distance from cam
		bool visible = false;
	};

	struct GameStateVars
	{
		bool hadFocus = false;
		bool isPaused = false;
		bool isFindPathRequested = false;
		bool isTabbed = false;
		bool isLinearPersp = true;
		bool drawSky = false;
		bool drawTextUi = false;
	};

	struct InputCache
	{
		float foreward = 0;
		float lateral = 0;
		float rotatation = 0;
		bool leftTrigger = false;
		bool rightTrigger = false;
	};

	struct GameAssets
	{
		std::string wallTexFilePath;
		std::string boundryTexFilePath;
		std::string floorTexFilePath;
		std::string ceilingTexFilePath;
		std::string skyTexFilePath;
	};
}

#endif