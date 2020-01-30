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
class UIWidget;
class Match;

struct ImageLoading
{
	ImageLoading(){}
	ImageLoading(std::string name_):imageName(name_){}

	std::string imageName;
	Image* image;
};

struct CPUMeshLoading
{
	CPUMeshLoading() {}
	CPUMeshLoading(std::string name_):meshName(name_){}

	std::string meshName;
	CPUMesh* cpuMesh;
};

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

	// Async Loading and Assets;
	void StartLoadingAssets();
	void EnqueueWorkForTexturesAndGPUMeshes();
	void ImageAndMeshLoadThread();
	void StartLoadingTexture(std::string nameOfTexture);
	void ContinueLoading();
	bool DoneLoading();
	void EndLoadingThreads();


public:
	
	// Game Bounds;
	Vec2 m_clientMins					= Vec2(0.0f, 0.0f);
	Vec2 m_clientMaxs					= Vec2(0.0f, 0.0f);
	
	// Cameras;
	Camera* m_gameMainCamera			= nullptr;
	Camera* m_uiCamera					= nullptr;
	Vec2 m_focalPoint					= Vec2( 0.0f, 0.0f );

	// Color Target View;
	ColorTargetView* m_colorTargetView	= nullptr;
	Rgba m_clearColor					= Rgba(0.5f, 0.5f, 0.5f, 1.0f);

	// Async Loading and Assets;
	bool m_stillLoading = false;
	int m_objectLoading = 0;
	std::vector<std::thread> m_threads;
	AsyncQueue<ImageLoading> imageLoadingFromDiscQueue;
	AsyncQueue<ImageLoading> imageCreatingTextureQueue;
	AsyncQueue<CPUMeshLoading> cpuLoadingFromDiscQueue;
	AsyncQueue<CPUMeshLoading> cpuCreatingGPUMeshQueue;

	// Match;
	Match* m_match = nullptr;

};

void CallImageAndMeshLoadThread();