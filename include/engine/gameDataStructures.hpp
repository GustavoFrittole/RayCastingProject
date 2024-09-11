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
		math::Vect2 coordinates{};
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

	struct GameCameraPlane
	{
		math::Vect2 forewardDirection;
		math::Vect2 plane;
	};

	struct GameCameraView
	{
		const EntityTransform& transform{};
		const GameCameraVars& vars{};
		const GameCameraPlane& vecs{};
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
		int width = 0, height = 0;
		bool generated = false;
		std::unique_ptr<std::string> cells;
	};

	enum class HitType : char
	{
		Wall = 'w',
		Baudry = 'b',
		Nothing = ' ',
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

	enum class SpriteAlignment
	{
		TopWindow,
		Ceiling,
		Center,
		Floor,
		BottomWindow
	};

	enum class TextVerticalAlignment
	{
		TopWindow,
		Center,
		BottomWindow
	};

	enum class TextHorizontalAlignment
	{
		Left,
		Center,
		Right,
	};

	struct Billboard
	{
		Billboard(int id) :
			id(id)
		{}

		//associated texture id
		int id = -1;

		//position is screen space, modified by core
		float positionOnScreen = 0.f;

		//distance from camera, modified by core
		float distance = 0.f;

		//relative size, 1 equals to cell size (based on distance)
		float size = 1.f;

		//where the spriteis drawn verically
		SpriteAlignment alignment = SpriteAlignment::Center;
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
		math::Vect2 movementSpeed{};
		float rotationSpeed = 0.f;
		math::Vect2 movementAcceleration{};
		float rotationAcceleraion = 0.f;
	};

	struct IEntity
	{
		/// @brief 
		/// @param id : id of a sprite to be drawn at this entity location. If id=-1 nothing will be drawn.  
		/// @param et : the starting transform of this entity
		IEntity(int id, const EntityTransform& et) :
			m_billboard(id),
			m_transform(et)
		{}
		/// @brief set both sprite and collision sizes
		/// @param size 
		void set_size(float size) 
		{ 
			m_collisionSize = size; 
			m_billboard.size = size; 
		}

		void apply_force(const math::Vect2& force)
		{
			if (m_physical.mass != 0)
				m_physical.movementAcceleration += force / m_physical.mass;
		}

		/// @brief called once at creation, after environment is ready.
		virtual void on_create() = 0;

		/// @brief called once every game cycle.
		virtual void on_update() = 0;

		/// @brief called once every game cycle, after all on_update() calls.
		virtual void on_late_update() = 0;

		/// @brief called every cycle in which two entities distance is lower then their combined collision size.
		/// @param  the type of the other entity that took part in the collision
		virtual void on_hit(EntityType) = 0;

		void set_destroyed(bool destroyed) { m_destroyed = destroyed; }
		bool get_destroyed() { return m_destroyed; }

		void set_interactible(bool interactible) { m_interactible = interactible; }
		bool get_interactible() { return m_interactible; }

		void set_active(bool active) { m_active = active; }
		bool get_active() { return m_active; }

		void set_visible(bool visible) { m_visible = visible; }
		bool get_visible() { return m_visible; }

		Billboard m_billboard;
		EntityTransform m_transform{};
		PhysicalVars m_physical{};
		float m_collisionSize = 1.f;
		EntityType m_type = EntityType::prop;

	protected:
		//flags entities that are to be removed
		bool m_destroyed = false;
		//deactivate distance based interactions (on_hit)
		bool m_interactible = false;
		//deactivete on_update script
		bool m_active = true;
		//automatically changed based on distance from cam
		bool m_visible = false;
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
		std::string fontFilePath;
		std::string wallTexFilePath;
		std::string boundryTexFilePath;
		std::string floorTexFilePath;
		std::string ceilingTexFilePath;
		std::string skyTexFilePath;
	};
}

#endif