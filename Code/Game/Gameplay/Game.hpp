#pragma once
#include "Engine/Async/AsyncQueue.hpp"
#include "Engine/Core/Rgba.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Renderer/Shader.hpp"
#include "Engine/Math/AABB2.hpp"

#include <chrono>
#include <queue>
#include <thread>
#include <vector>

class OrbitCamera;
class BitMapFont;
class GPUMesh;
class CPUMesh;
class Map;
class TextureView;
class Prop;
struct IntVec2;
struct Camera;
class ConwaysGameOfLife;
class UIWidget;


class Game
{
	
public:

	// Constructor/Deconstructor
	Game();
	~Game();

	// Flow;
	void Init();
	void Startup();
	void BeginFrame();
	void Update(float deltaSeconds);
	void Render();
	void EndFrame();

	// Cameras;
	void CreateCameras();
	void UpdateGameCamera( float deltaSeconds );
	void UpdateFocalPointPosition( float deltaSeconds );
	void UpdateGameCameraPosition();

	// Tiles;
	//void CreateTiles();
	//void ConwaysGameOfLife();


public:
	
	// Game Bounds;
	Vec2 m_clientMins					= Vec2(0.0f, 0.0f);
	Vec2 m_clientMaxs					= Vec2(0.0f, 0.0f);
	Vec2 m_worldMins					= Vec2(0.0f, 0.0f);
	Vec2 m_worldMaxs					= Vec2(0.0f, 0.0f);
	
	// Cameras;
	Camera* m_gameMainCamera			= nullptr;
	Camera* m_uiCamera					= nullptr;
	Vec2 m_focalPoint					= Vec2( 0.0f, 0.0f );

	// Color Target View;
	ColorTargetView* m_colorTargetView	= nullptr;
	Rgba m_clearColor					= Rgba(0.5f, 0.5f, 0.5f, 1.0f);

	// Conways Game of Life;
	ConwaysGameOfLife* m_conwaysGameOfLife = nullptr;

	// UI;
	UIWidget* m_masterUIWidget = nullptr;

};
