#ifndef GAMEDATASTRUCTURES_HPP
#define GAMEDATASTRUCTURES_HPP

#include "utils.hpp"
#include <memory>

#define WINDOW_HEIGHT 720

namespace windowVars
{
	constexpr int g_screenHeight = WINDOW_HEIGHT;
	constexpr int g_screenWidth = g_screenHeight * 16 / 9;
}

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

struct ControlsVars
{
	float mouseSens = 1.f;
	float movementSpeed = 1.f;
};

struct GraphicsVars
{
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

struct RayInfo
{
	EntityType entityHit = EntityType::NoHit;
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

struct Sprite
{
	std::string texturePath;
	EntityTransform transform;
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

struct GameStateVars
{
	bool hadFocus = false;
	bool isPaused = false;
	bool isFindPathRequested = false;
	bool isTabbed = false;
	bool isLinearPersp = true;
	bool drawSky = false;
};

namespace game
{
	class IGameController
	{
	public:
		virtual void rotate(float) const = 0;
		virtual void move_foreward(float) const = 0;
		virtual void move_strafe(float) const = 0;
	};
}

struct GameAssets
{
	std::string wallTexFilePath;
	std::string boundryTexFilePath;
	std::string floorTexFilePath;
	std::string ceilingTexFilePath;
	std::string skyTexFilePath;
};

#endif