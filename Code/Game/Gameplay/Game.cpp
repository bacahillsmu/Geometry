// Defines and Includes ---------------------------------------------------------------------------
#define WIN32_LEAN_AND_MEAN	
#include <winsock2.h>

// Commons ----------------------------------------------------------------------------------------
#include "Engine/Core/EngineCommon.hpp"
#include "Game/Framework/GameCommon.hpp"

// Engine Includes --------------------------------------------------------------------------------
#include "Engine/Core/WindowContext.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Core/NamedStrings.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Core/VertexLit.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Renderer/SpriteAnimationDefinition.hpp"
#include "Engine/Renderer/BitMapFont.hpp"
#include "Engine/Renderer/Prop.hpp"
#include "Engine/Renderer/Model.hpp"
#include "Engine/Renderer/TextureView.hpp"
#include "Engine/Renderer/CPUMesh.hpp"
#include "Engine/Renderer/Material.hpp"
#include "Engine/Math/Matrix44.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/UI/UIWidget.hpp"
#include "Engine/Core/Rgba.hpp"
#include "Engine/Input/InputSystem.hpp"

// Game Includes ----------------------------------------------------------------------------------
#include "Game/Framework/App.hpp"
#include "Game/Input/GameInput.hpp"
#include "Game/Gameplay/Map.hpp"

// Callbacks --------------------------------------------------------------------------------------
static bool QuitGame(EventArgs& args)
{
	UNUSED(args);

	g_theApp->HandleCloseApplication();

	return true;
}

// Constructor ------------------------------------------------------------------------------------
Game::Game() 
{
}

// Deconstructor ----------------------------------------------------------------------------------
Game::~Game()
{
	// Delete Pointers (template);
	DELETE_POINTER(m_gameMainCamera);
	DELETE_POINTER(m_uiCamera);
	DELETE_POINTER(m_map);
}

// -----------------------------------------------------------------------
// Flow;
// -----------------------------------------------------------------------
void Game::Init()
{
	// Ask the WindowContext for the Client's dimensions;
	IntVec2 clientMins = g_theWindowContext->GetClientMins();
	IntVec2 clientMaxs = g_theWindowContext->GetClientMaxs();

	// Save off the Client's dimensions, keeping in mind that Windows has 0,0 is top left, we want it bottom left;
	m_clientMins = Vec2((float)clientMins.x, (float)clientMaxs.y);
	m_clientMaxs = Vec2((float)clientMaxs.x, (float)clientMins.y);

	// Game Subscription Callbacks;
	g_theEventSystem->SubscriptionEventCallbackFunction("quit", QuitGame);

	m_map				= new Map();
	m_gameMainCamera	= new Camera();
	m_uiCamera			= new Camera();	

	m_gameMainCamera->SetOrthographicProjection(Vec2::ZERO, Vec2(Map::WIDTH, Map::HEIGHT));
	m_uiCamera->SetOrthographicProjection(Vec2::ZERO, Vec2(Map::WIDTH, Map::HEIGHT));


	/*
	m_masterUIWidget = new UIWidget(
		AABB2::MakeFromMinsMaxs(m_clientMins, m_clientMaxs),	// Bounding Box, entire screen;
		Vec4(1.0f, 1.0f, 0.0f, 0.0f),							// Virtual Size, entire bounding box;
		Vec4(0.5f, 0.5f, 0.0f, 0.0f)							// Virtual Position, center of bounding box;
	);
	*/
	
}

// -----------------------------------------------------------------------
void Game::Startup()
{
	// Mouse Settings;
	g_theWindowContext->ShowMouse();							// During WindowContext's Init, we hid the mouse;
	g_theWindowContext->SetMouseMode(MOUSE_MODE_ABSOLUTE);		// Mouse position is where the mouse is;
	// g_theWindowContext->SetMouseMode(MOUSE_MODE_RELATIVE);	// Mouse position is locked to center;

	m_clearColor = Rgba::WHITE;

	StartLoadingAssets();

	MakeConvexPolys2D();

	m_mainRayStart = Vec2(0.0f);
	m_mainRayEnd = Vec2(Map::WIDTH / 2.0f, Map::HEIGHT / 2.0f);
	m_mainRayLine = Line(m_mainRayStart, m_mainRayEnd, 0.25f);

	m_map = new Map();
	m_map->Startup();
}

// -----------------------------------------------------------------------
void Game::BeginFrame()
{
	m_mainRayLine = Line(m_mainRayStart, m_mainRayEnd, 0.25f);
}

// -----------------------------------------------------------------------
void Game::Update(float deltaSeconds)
{
	// Async Loading;
	if(m_stillLoading)
	{
		if(!DoneLoading())
		{
			ContinueLoading();
		}
		else
		{
			EndLoadingThreads();
			m_stillLoading = false;
		}
	}

	m_worldMousePosition.x = RangeMap(g_theInputSystem->GetMousePosition().x, 0.0f, (float)g_theWindowContext->GetClientDimensions().x, 0.0f, Map::WIDTH);
	m_worldMousePosition.y = RangeMap(g_theInputSystem->GetMousePosition().y, 0.0f, (float)g_theWindowContext->GetClientDimensions().y, 0.0f, Map::HEIGHT);

	CheckForInput();

	MakeInvisibleRaycasts();

	MakeMainRaycast();

	m_map->Update(deltaSeconds);
}

// -----------------------------------------------------------------------
void Game::Render()
{
	// Game Camera;
	m_colorTargetView = g_theRenderer->GetFrameColorTarget();
	m_gameMainCamera->SetColorTargetView(m_colorTargetView);
	g_theRenderer->BeginCamera(m_gameMainCamera);
	g_theRenderer->ClearColorTargets(m_clearColor);

	m_map->Render();

	// Rendering our invisible raycasts;
	for (int raycastIndex = 0; raycastIndex < m_numberOfInvisibleRaycasts; ++raycastIndex)
	{
		std::vector<Vertex_PCU> raycastVerts;
		AddVertsForLine2D(raycastVerts, m_invisibleRays[raycastIndex], Rgba::GRAY);

		g_theRenderer->BindShader("Data/Shaders/default_unlit_devconsole.shader");
		g_theRenderer->BindTextureViewWithSampler(0, nullptr);
		g_theRenderer->DrawVertexArray((int)raycastVerts.size(), &raycastVerts[0]);
	}

	// Rendering each convex poly;
	for(int convexPolyIndex = 0; convexPolyIndex < m_convexPoly2Ds.size(); ++convexPolyIndex)
	{
		m_convexPoly2Ds[convexPolyIndex].Render();
	}

	// Render each convex poly's edges;
	for (int i = 0; i < m_polyEdges.size(); ++i)
	{
		std::vector<Vertex_PCU> edgeVerts;
		AddVertsForLine2D(edgeVerts, m_polyEdges[i], Rgba::GRAY);

		g_theRenderer->BindShader("Data/Shaders/default_unlit_devconsole.shader");
		g_theRenderer->BindTextureViewWithSampler(0, nullptr);
		g_theRenderer->DrawVertexArray((int)edgeVerts.size(), &edgeVerts[0]);
	}

	// Rendering the impact points of our invisible rays;
	if(m_invisibleRaysIntersections.size() > 0)
	{
		std::vector<Vertex_PCU> intersectVerts;
		for (int raycastIntersectionIndex = 0; raycastIntersectionIndex < m_invisibleRaysIntersections.size(); ++raycastIntersectionIndex)
		{
			AddVertsForDisc2D(intersectVerts, m_invisibleRaysIntersections[raycastIntersectionIndex], 0.4f, Rgba::RED);
		}

		g_theRenderer->BindShader("Data/Shaders/default_unlit_devconsole.shader");
		g_theRenderer->BindTextureViewWithSampler(0, nullptr);
		g_theRenderer->DrawVertexArray((int)intersectVerts.size(), &intersectVerts[0]);
	}

	// Rendering our invisible raycasts;
	std::vector<Vertex_PCU> mainRaycastVerts;
	AddVertsForLine2D(mainRaycastVerts, m_mainRayLine, Rgba::GRAY);

	g_theRenderer->BindShader("Data/Shaders/default_unlit_devconsole.shader");
	g_theRenderer->BindTextureViewWithSampler(0, nullptr);
	g_theRenderer->DrawVertexArray((int)mainRaycastVerts.size(), &mainRaycastVerts[0]);

	// Rendering the impact points of our main ray;
	if (m_mainRaysIntersections.size() > 0)
	{
		std::vector<Vertex_PCU> intersectVerts;
		for (int raycastIntersectionIndex = 0; raycastIntersectionIndex < m_mainRaysIntersections.size(); ++raycastIntersectionIndex)
		{
			AddVertsForDisc2D(intersectVerts, m_mainRaysIntersections[raycastIntersectionIndex], 0.4f, Rgba::RED);
		}

		g_theRenderer->BindShader("Data/Shaders/default_unlit_devconsole.shader");
		g_theRenderer->BindTextureViewWithSampler(0, nullptr);
		g_theRenderer->DrawVertexArray((int)intersectVerts.size(), &intersectVerts[0]);
	}
	

	g_theRenderer->EndCamera();

	// UI Camera;
	m_colorTargetView = g_theRenderer->GetFrameColorTarget();
	m_uiCamera->SetColorTargetView(m_colorTargetView);
	g_theRenderer->BeginCamera(m_uiCamera);

	g_theRenderer->EndCamera();
}

// -----------------------------------------------------------------------
void Game::EndFrame()
{
	
}

// -----------------------------------------------------------------------
// Cameras;
// -----------------------------------------------------------------------

// -----------------------------------------------------------------------
// Assets;
// -----------------------------------------------------------------------
void Game::StartLoadingAssets()
{
	EnqueueWorkForTexturesAndGPUMeshes();

	uint coreCount = std::thread::hardware_concurrency();
	for (uint i = 0; i < coreCount; ++i)
	{
		//std::thread loadThread( CallImageAndMeshLoadThread ); 
		//m_threads.push_back( loadThread );
		m_threads.emplace_back(CallImageAndMeshLoadThread);
	}
}

// -----------------------------------------------------------------------
void Game::EnqueueWorkForTexturesAndGPUMeshes()
{
	StartLoadingTexture("Data/Sprites/Jobs/Knight/Knight1M-S.png");

	m_stillLoading = true;
}

// -----------------------------------------------------------------------
void Game::ImageAndMeshLoadThread()
{
	bool doneLoading = false;

	ImageLoading imageLoading;
	CPUMeshLoading cpuLoading;

	while (!doneLoading)
	{
		bool gotImageWork = imageLoadingFromDiscQueue.Dequeue(&imageLoading);
		if (gotImageWork)
		{
			imageLoading.image = new Image(imageLoading.imageName.c_str());
			imageCreatingTextureQueue.Enqueue(imageLoading);
		}

		bool gotCPUWork = cpuLoadingFromDiscQueue.Dequeue(&cpuLoading);
		if (gotCPUWork)
		{
			cpuLoading.cpuMesh = new CPUMesh();
			cpuLoading.cpuMesh->SetLayout<Vertex_Lit>();
			CreateMeshFromFile(cpuLoading.meshName.c_str(), cpuLoading.cpuMesh);
			cpuCreatingGPUMeshQueue.Enqueue(cpuLoading);
		}

		if (!gotImageWork && !gotCPUWork)
		{
			doneLoading = true;
		}

		Sleep(0);
	}
}

// -----------------------------------------------------------------------
void Game::StartLoadingTexture(std::string nameOfTexture)
{
	ImageLoading loading = ImageLoading(nameOfTexture);

	m_objectLoading++;
	imageLoadingFromDiscQueue.Enqueue(loading);
}

// -----------------------------------------------------------------------
void Game::ContinueLoading()
{
	ImageLoading imageLoading;
	CPUMeshLoading cpuLoading;

	bool gotImageWork = imageCreatingTextureQueue.Dequeue(&imageLoading);
	if (gotImageWork)
	{
		g_theRenderer->CreateTextureViewFromImage(imageLoading.image, imageLoading.imageName);
		m_objectLoading--;
		delete imageLoading.image;
		imageLoading.image = nullptr;
	}

	bool gotGPUWork = cpuCreatingGPUMeshQueue.Dequeue(&cpuLoading);
	if (gotGPUWork)
	{
		g_theRenderer->CreateAndRegisterGPUMesh(cpuLoading.cpuMesh, cpuLoading.meshName);
		m_objectLoading--;
		delete cpuLoading.cpuMesh;
		cpuLoading.cpuMesh = nullptr;
	}
}

// -----------------------------------------------------------------------
bool Game::DoneLoading()
{
	return m_objectLoading == 0;
}

// -----------------------------------------------------------------------
void Game::EndLoadingThreads()
{
	for (std::thread& t : m_threads)
	{
		t.join();
	}

	m_threads.clear();
}

// -----------------------------------------------------------------------
void Game::MakeConvexPolys2D()
{
	m_convexPoly2Ds.clear();
	m_polyEdges.clear();
	m_convexPoly2Ds.reserve(m_numberOfConvexPolys);
	for (int i = 0; i < m_numberOfConvexPolys; ++i)
	{
		ConvexPoly2D convexPoly;

		float minRadius = 4.0f;
		float maxRadius = 8.0f;
		float minAngleMovement = 10.0f;
		float maxAngleMovement = 120.0f;
		int numVerts = 0;

		float radius = g_theRandomNumberGenerator->GetRandomFloatInRange(minRadius, maxRadius);

		float angle = 0.0f;
		while (angle <= 360.0f)
		{
			float angleMovement = g_theRandomNumberGenerator->GetRandomFloatInRange(minAngleMovement, maxAngleMovement);
			angle += angleMovement;

			if (angle > 360.0f)
			{
				break;
			}

			numVerts++;

			Vec2 vert = Vec2::MakeFromPolarDegrees(angle, radius);
			convexPoly.m_verts.push_back(vert);
		}

		convexPoly.m_numVerts = numVerts;

		Vec2 minBoundsWithBuffer = Vec2::ZERO + Vec2(radius, radius);
		Vec2 maxBoundsWithBuffer = Vec2(Map::WIDTH, Map::HEIGHT) - Vec2(radius, radius);

		convexPoly.m_center = g_theRandomNumberGenerator->GetRandomVec2InRange(minBoundsWithBuffer, maxBoundsWithBuffer);

		m_convexPoly2Ds.push_back(convexPoly);
	}
}

// -----------------------------------------------------------------------
void Game::MakeInvisibleRaycasts()
{
	if(!m_refreshInvisbleRaycasts)
	{
		return;
	}

	m_refreshInvisbleRaycasts = false;
	m_invisibleRays.clear();
	m_invisibleRays.reserve(m_numberOfInvisibleRaycasts);
	m_invisibleRaysIntersections.clear();
	for (int i = 0; i < m_numberOfInvisibleRaycasts; ++i)
	{
		Vec2 start = g_theRandomNumberGenerator->GetRandomVec2InRange(Vec2(0.0f, 0.0f), Vec2(Map::WIDTH, Map::HEIGHT));
		Vec2 end = g_theRandomNumberGenerator->GetRandomVec2InRange(Vec2(0.0f, 0.0f), Vec2(Map::WIDTH, Map::HEIGHT));

		Line raycastLine = Line(start, end, 0.25f);
		m_invisibleRays.push_back(raycastLine);

		for (int convexPolyIndex = 0; convexPolyIndex < m_convexPoly2Ds.size(); ++convexPolyIndex)
		{
			for (int vertIndex = 0; vertIndex < m_convexPoly2Ds[convexPolyIndex].m_numVerts; vertIndex++)
			{
				int nextIndex = vertIndex + 1;
				if (nextIndex == m_convexPoly2Ds[convexPolyIndex].m_numVerts)
				{
					nextIndex = 0;
				}

				Vec2 polyCenter = m_convexPoly2Ds[convexPolyIndex].m_center;
				Vec2 polyVert1 = polyCenter + m_convexPoly2Ds[convexPolyIndex].m_verts[vertIndex];
				Vec2 polyVert2 = polyCenter + m_convexPoly2Ds[convexPolyIndex].m_verts[nextIndex];

				Line convexPolySide = Line(polyVert1, polyVert2, 0.25f);
				m_polyEdges.push_back(convexPolySide);

				Vec2 possibleIntersection = Vec2(0.0f, 0.0f);
				bool intersect = DoLinesOverlap(raycastLine, convexPolySide, &possibleIntersection);
				if (intersect)
				{
					m_invisibleRaysIntersections.push_back(possibleIntersection);
				}
			}
		}
	}
}

// -----------------------------------------------------------------------
void Game::MakeMainRaycast()
{
	m_mainRaysIntersections.clear();
	Vec2 start = m_mainRayStart;
	Vec2 end = m_mainRayEnd;

	Line raycastLine = Line(start, end, 0.25f);

	for (int convexPolyIndex = 0; convexPolyIndex < m_convexPoly2Ds.size(); ++convexPolyIndex)
	{
		for (int vertIndex = 0; vertIndex < m_convexPoly2Ds[convexPolyIndex].m_numVerts; vertIndex++)
		{
			int nextIndex = vertIndex + 1;
			if (nextIndex == m_convexPoly2Ds[convexPolyIndex].m_numVerts)
			{
				nextIndex = 0;
			}

			Vec2 polyCenter = m_convexPoly2Ds[convexPolyIndex].m_center;
			Vec2 polyVert1 = polyCenter + m_convexPoly2Ds[convexPolyIndex].m_verts[vertIndex];
			Vec2 polyVert2 = polyCenter + m_convexPoly2Ds[convexPolyIndex].m_verts[nextIndex];

			Line convexPolySide = Line(polyVert1, polyVert2, 0.25f);
			m_polyEdges.push_back(convexPolySide);

			Vec2 possibleIntersection = Vec2(0.0f, 0.0f);
			bool intersect = DoLinesOverlap(raycastLine, convexPolySide, &possibleIntersection);
			if (intersect)
			{
				m_mainRaysIntersections.push_back(possibleIntersection);
			}
		}
	}
}

// -----------------------------------------------------------------------
void Game::CheckForInput()
{
	if (g_theGameInput->IsF8Pressed())
	{
		MakeConvexPolys2D();
	}
	if (g_theGameInput->IsMKeyPressed())
	{
		if (m_numberOfConvexPolys != m_maxNumberOfConvexPolys)
		{
			m_numberOfConvexPolys = m_numberOfConvexPolys * 2;
			m_numberOfConvexPolys = Clamp(m_numberOfConvexPolys, m_minNumberOfConvexPolys, m_maxNumberOfConvexPolys);
			MakeConvexPolys2D();
		}
	}
	if (g_theGameInput->IsNKeyPressed())
	{
		if (m_numberOfConvexPolys != m_minNumberOfConvexPolys)
		{
			m_numberOfConvexPolys = m_numberOfConvexPolys / 2;
			m_numberOfConvexPolys = Clamp(m_numberOfConvexPolys, m_minNumberOfConvexPolys, m_maxNumberOfConvexPolys);
			MakeConvexPolys2D();
		}
	}
	if(g_theGameInput->IsLKeyPressed())
	{
		if (m_numberOfInvisibleRaycasts != m_maxNumberOfInvisibleRaycasts)
		{
			m_numberOfInvisibleRaycasts = m_numberOfInvisibleRaycasts * 2;
			m_numberOfInvisibleRaycasts = Clamp(m_numberOfInvisibleRaycasts, m_minNumberOfInvisibleRaycasts, m_maxNumberOfInvisibleRaycasts);
			m_refreshInvisbleRaycasts = true;
			MakeInvisibleRaycasts();
		}
	}
	if (g_theGameInput->IsKKeyPressed())
	{
		if (m_numberOfInvisibleRaycasts != m_minNumberOfInvisibleRaycasts)
		{
			m_numberOfInvisibleRaycasts = m_numberOfInvisibleRaycasts / 2;
			m_numberOfInvisibleRaycasts = Clamp(m_numberOfInvisibleRaycasts, m_minNumberOfInvisibleRaycasts, m_maxNumberOfInvisibleRaycasts);
			m_refreshInvisbleRaycasts = true;
			MakeInvisibleRaycasts();
		}
	}
	if(g_theGameInput->IsSKeyPressed())
	{
		m_mainRayStart = m_worldMousePosition;
	}
	if (g_theGameInput->IsEKeyPressed())
	{
		m_mainRayEnd = m_worldMousePosition;
	}
}

// -----------------------------------------------------------------------
// static;
// -----------------------------------------------------------------------
void CallImageAndMeshLoadThread()
{
	g_theApp->m_theGame->ImageAndMeshLoadThread();
}