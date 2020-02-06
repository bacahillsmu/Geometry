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
#include "Engine/Math/Ray.hpp"


// Game Includes ----------------------------------------------------------------------------------
#include "Game/Framework/App.hpp"
#include "Game/Input/GameInput.hpp"
#include "Game/Gameplay/Map.hpp"
#include "Game/SpatialHashing/SpatialHashingDisc.hpp"

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
	DELETE_POINTER(m_spatialHashingDisc);
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
	m_spatialHashingDisc = new SpatialHashingDisc();

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
	// g_theWindowContext->ShowMouse();							// During WindowContext's Init, we hid the mouse;
	g_theWindowContext->SetMouseMode(MOUSE_MODE_ABSOLUTE);		// Mouse position is where the mouse is;
	// g_theWindowContext->SetMouseMode(MOUSE_MODE_RELATIVE);	// Mouse position is locked to center;

	m_clearColor = Rgba::WHITE;

	StartLoadingAssets();

	MakeConvexPolys2D();

	m_mainRayStart = Vec2(0.0f, 0.0f);
	m_mainRayEnd = m_mainRayStart + Vec2(10.0f, 10.0f);

	m_map = new Map();
	m_map->Startup();
}

// -----------------------------------------------------------------------
void Game::BeginFrame()
{
	m_frameCounter++;
	m_timeAtStartOfFrame = GetCurrentTimeSeconds();
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

	m_spatialHashingDisc->Update();

	m_worldMousePosition.x = RangeMap(g_theInputSystem->GetMousePosition().x, 0.0f, (float)g_theWindowContext->GetClientDimensions().x, 0.0f, Map::WIDTH);
	m_worldMousePosition.y = RangeMap(g_theInputSystem->GetMousePosition().y, 0.0f, (float)g_theWindowContext->GetClientDimensions().y, 0.0f, Map::HEIGHT);

	m_mouseColor = Rgba::BLUE;
	CheckForInput();

	if(m_spatialHashing)
	{
		MakeInvisibleRaycastsWithSpatialHashing();
	}
	else
	{
		MakeInvisibleRaycasts();
	}

	if(m_spatialHashing)
	{
		MakeMainRaycastWithSpatialHashing();
	}
	else
	{
		MakeMainRaycast();
	}
	

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
	
	g_theRenderer->BindShader("Data/Shaders/default_unlit_devconsole.shader");
	g_theRenderer->BindTextureViewWithSampler(0, nullptr);

	CheckAndDrawAnyDebugVerts();

	for(int convexPolyIndex = 0; convexPolyIndex < m_convexPoly2Ds.size(); ++convexPolyIndex)
	{
		m_convexPoly2Ds[convexPolyIndex].Render();
	}

	if(m_shouldRenderCheckPolys)
	{
		for (ConvexPoly2D* poly : m_convexPoly2DsToCheck)
		{
			poly->SpecialRender();
		}
	}
	

	

	DrawDebugInformation();

	m_map->Render();

	DrawMainRaycast();

	

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
	m_timeAtEndOfFrame = GetCurrentTimeSeconds();
	m_frameTime = (m_timeAtEndOfFrame - m_timeAtStartOfFrame) * 1000.0;

	m_totalFrameTime += m_frameTime;
	m_averageFrameTime = m_totalFrameTime / (float)m_frameCounter;
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
		Vec2 minBoundsWithBuffer = Vec2::ZERO + Vec2(radius, radius);
		Vec2 maxBoundsWithBuffer = Vec2(Map::WIDTH, Map::HEIGHT) - Vec2(radius, radius);
		Vec2 center = g_theRandomNumberGenerator->GetRandomVec2InRange(minBoundsWithBuffer, maxBoundsWithBuffer);

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

			Vec2 vert = Vec2::MakeFromPolarDegrees(angle, radius) + center;
			convexPoly.m_verts.push_back(vert);
		}

		convexPoly.m_numVerts = numVerts;
		
		convexPoly.m_radius = radius;

		m_convexPoly2Ds.push_back(convexPoly);
	}

	RefreshPolyEdges();

	CreateOrRefreshSpatialHashingDiscs();

	
}

// -----------------------------------------------------------------------
void Game::MakeInvisibleRaycasts()
{
	m_raycastTimerStart = GetCurrentTimeSeconds();

	m_invisibleRays.clear();
	m_invisibleRays.reserve(m_numberOfInvisibleRaycasts);
	m_invisibleRaysIntersections.clear();
	for (int i = 0; i < m_numberOfInvisibleRaycasts; ++i)
	{
		Vec2 start = g_theRandomNumberGenerator->GetRandomVec2InRange(Vec2(0.0f, 0.0f), Vec2(Map::WIDTH, Map::HEIGHT));
		Vec2 end = g_theRandomNumberGenerator->GetRandomVec2InRange(Vec2(0.0f, 0.0f), Vec2(Map::WIDTH, Map::HEIGHT));

		Line raycastLine = Line(start, end, 0.25f);
		m_invisibleRays.push_back(raycastLine);

		for(const Line& polyEdge : m_polyEdges)
		{
			Vec2 possibleIntersection = Vec2(0.0f, 0.0f);
			bool intersect = DoLinesOverlap(raycastLine, polyEdge, &possibleIntersection);
			if (intersect)
			{
				m_invisibleRaysIntersections.push_back(possibleIntersection);
			}
		}
	}

	m_raycastTime = (GetCurrentTimeSeconds() - m_raycastTimerStart) * 1000.0;
}

// -----------------------------------------------------------------------
void Game::MakeInvisibleRaycastsWithSpatialHashing()
{
	m_raycastTimerStart = GetCurrentTimeSeconds();

	m_invisibleRays.clear();
	m_invisibleRays.reserve(m_numberOfInvisibleRaycasts);
	m_invisibleRaysIntersections.clear();
	for (int i = 0; i < m_numberOfInvisibleRaycasts; ++i)
	{
		Vec2 start = g_theRandomNumberGenerator->GetRandomVec2InRange(Vec2(0.0f, 0.0f), Vec2(Map::WIDTH, Map::HEIGHT));
		Vec2 end = g_theRandomNumberGenerator->GetRandomVec2InRange(Vec2(0.0f, 0.0f), Vec2(Map::WIDTH, Map::HEIGHT));

		Line raycastLine = Line(start, end, 0.25f);
		m_invisibleRays.push_back(raycastLine);

		std::vector<ConvexPoly2D*> polysToChecks;
		m_spatialHashingDisc->RaycastAgainstMyselfAndChildren(&polysToChecks, start, end);

		bool inside = false;
		for (ConvexPoly2D* poly : polysToChecks)
		{
			if (poly->IsPointInside(start))
			{
				m_invisibleRaysIntersections.push_back(start);
				inside = true;
			}
		}

		if(!inside)
		{
			for (ConvexPoly2D* poly : polysToChecks)
			{
				for (const Line& polyEdge : poly->m_edges)
				{
					Vec2 possibleIntersection = Vec2(0.0f, 0.0f);
					bool intersect = DoLinesOverlap(raycastLine, polyEdge, &possibleIntersection);
					if (intersect)
					{
						m_invisibleRaysIntersections.push_back(possibleIntersection);
					}
				}
			}
		}
		
	}

	m_raycastTime = (GetCurrentTimeSeconds() - m_raycastTimerStart) * 1000.0;
}

// -----------------------------------------------------------------------
void Game::MakeMainRaycast()
{
	m_mainRayHit = false;

	for(ConvexPoly2D poly : m_convexPoly2Ds)
	{
		if(poly.IsPointInside(m_mainRayStart))
		{
			m_mainRayHit = true;
			m_mainRayHitPoint = m_mainRayStart;
			m_mainRayHitSurfaceNormal = (m_mainRayStart - m_mainRayEnd);
			m_mainRayHitSurfaceNormal.Normalize();
			return;
		}
	}

	float bestImpactPointDistance = 999999999999.99f;
	Vec2 bestImpactPoint;
	Line bestImpactLine;

	Line raycastLine = Line(m_mainRayStart, m_mainRayEnd);
	for(const Line& polyEdge : m_polyEdges)
	{
		Vec2 possibleIntersection = Vec2(0.0f, 0.0f);
		bool intersect = DoLinesOverlap(raycastLine, polyEdge, &possibleIntersection);
		if (intersect)
		{
			float distance = (m_mainRayStart - possibleIntersection).GetLengthSquared();
			if(distance < bestImpactPointDistance)
			{
				bestImpactPointDistance = distance;
				m_mainRayHit = true;
				bestImpactPoint = possibleIntersection;
				bestImpactLine = polyEdge;
			}
		}
	}

	if(m_mainRayHit)
	{
		m_mainRayHitPoint = bestImpactPoint;
		m_mainRayHitSurfaceNormal = (bestImpactLine.lineStart - bestImpactLine.lineEnd);
		m_mainRayHitSurfaceNormal.Normalize();
		m_mainRayHitSurfaceNormal.Rotate90Degrees();
	}
}

// -----------------------------------------------------------------------
void Game::MakeMainRaycastWithSpatialHashing()
{
	m_convexPoly2DsToCheck.clear();
	m_mainRayHit = false;
	std::vector<ConvexPoly2D*> polysToChecks;
	m_spatialHashingDisc->RaycastAgainstMyselfAndChildren(&polysToChecks, m_mainRayStart, m_mainRayEnd); 

	m_convexPoly2DsToCheck = polysToChecks;

	for (ConvexPoly2D* poly : polysToChecks)
	{
		if (poly->IsPointInside(m_mainRayStart))
		{
			m_mainRayHit = true;
			m_mainRayHitPoint = m_mainRayStart;
			m_mainRayHitSurfaceNormal = (m_mainRayStart - m_mainRayEnd);
			m_mainRayHitSurfaceNormal.Normalize();
			return;
		}
	}

	float bestImpactPointDistance = 999999999999.99f;
	Vec2 bestImpactPoint;
	Line bestImpactLine;

	Line raycastLine = Line(m_mainRayStart, m_mainRayEnd);
	for(ConvexPoly2D* poly : polysToChecks)
	{
		for (const Line& polyEdge : poly->m_edges)
		{
			Vec2 possibleIntersection = Vec2(0.0f, 0.0f);
			bool intersect = DoLinesOverlap(raycastLine, polyEdge, &possibleIntersection);
			if (intersect)
			{
				float distance = (m_mainRayStart - possibleIntersection).GetLengthSquared();
				if (distance < bestImpactPointDistance)
				{
					bestImpactPointDistance = distance;
					m_mainRayHit = true;
					bestImpactPoint = possibleIntersection;
					bestImpactLine = polyEdge;
				}
			}
		}
	}

	if (m_mainRayHit)
	{
		m_mainRayHitPoint = bestImpactPoint;
		m_mainRayHitSurfaceNormal = (bestImpactLine.lineStart - bestImpactLine.lineEnd);
		m_mainRayHitSurfaceNormal.Normalize();
		m_mainRayHitSurfaceNormal.Rotate90Degrees();
	}
}

// -----------------------------------------------------------------------
void Game::CheckForInput()
{
	if (g_theGameInput->IsF8Pressed())
	{
		MakeConvexPolys2D();
		m_frameCounter = 0;
		m_totalFrameTime = 0.0f;
		m_averageFrameTime = 0.0f;
		CreateOrRefreshSpatialHashingDiscs();
	}
	if (g_theGameInput->IsMKeyPressed())
	{
		if (m_numberOfConvexPolys != m_maxNumberOfConvexPolys)
		{
			m_numberOfConvexPolys = m_numberOfConvexPolys * 2;
			m_numberOfConvexPolys = Clamp(m_numberOfConvexPolys, m_minNumberOfConvexPolys, m_maxNumberOfConvexPolys);
			MakeConvexPolys2D();
			m_frameCounter = 0;
			m_totalFrameTime = 0.0f;
			m_averageFrameTime = 0.0f;
			CreateOrRefreshSpatialHashingDiscs();
		}
	}
	if (g_theGameInput->IsNKeyPressed())
	{
		if (m_numberOfConvexPolys != m_minNumberOfConvexPolys)
		{
			m_numberOfConvexPolys = m_numberOfConvexPolys / 2;
			m_numberOfConvexPolys = Clamp(m_numberOfConvexPolys, m_minNumberOfConvexPolys, m_maxNumberOfConvexPolys);
			MakeConvexPolys2D();
			m_frameCounter = 0;
			m_totalFrameTime = 0.0f;
			m_averageFrameTime = 0.0f;
			CreateOrRefreshSpatialHashingDiscs();
		}
	}
	if(g_theGameInput->IsLKeyPressed())
	{
		if (m_numberOfInvisibleRaycasts != m_maxNumberOfInvisibleRaycasts)
		{
			m_numberOfInvisibleRaycasts = m_numberOfInvisibleRaycasts * 2;
			m_numberOfInvisibleRaycasts = Clamp(m_numberOfInvisibleRaycasts, m_minNumberOfInvisibleRaycasts, m_maxNumberOfInvisibleRaycasts);
			m_frameCounter = 0;
			m_totalFrameTime = 0.0f;
			m_averageFrameTime = 0.0f;
		}
	}
	if (g_theGameInput->IsKKeyPressed())
	{
		if (m_numberOfInvisibleRaycasts != m_minNumberOfInvisibleRaycasts)
		{
			m_numberOfInvisibleRaycasts = m_numberOfInvisibleRaycasts / 2;
			m_numberOfInvisibleRaycasts = Clamp(m_numberOfInvisibleRaycasts, m_minNumberOfInvisibleRaycasts, m_maxNumberOfInvisibleRaycasts);
			m_frameCounter = 0;
			m_totalFrameTime = 0.0f;
			m_averageFrameTime = 0.0f;
		}
	}
	if(g_theGameInput->IsBKeyPressed())
	{
		std::vector<ConvexPoly2D> convexPolys;
		for (ConvexPoly2D& poly : m_convexPoly2Ds)
		{
			bool inside = poly.IsPointInside(m_worldMousePosition);

			if(inside)
			{
				m_mouseColor = Rgba::RED;
				for(Vec2& vert : poly.m_verts)
				{
					Vec2 displacement = vert - m_worldMousePosition;
					Vec2 direction = displacement.GetNormalized();
					vert = vert + (direction * 1.2f);
				}

				RefreshPolyEdges();
				CreateOrRefreshSpatialHashingDiscs();
			}
		}
	}
	if (g_theGameInput->IsVKeyPressed())
	{
		std::vector<ConvexPoly2D> convexPolys;
		for (ConvexPoly2D& poly : m_convexPoly2Ds)
		{
			bool inside = poly.IsPointInside(m_worldMousePosition);

			if (inside)
			{
				m_mouseColor = Rgba::RED;
				for (Vec2& vert : poly.m_verts)
				{
					Vec2 displacement = vert - m_worldMousePosition;
					Vec2 direction = displacement.GetNormalized();
					vert = vert - (direction * 1.2f);
				}

				RefreshPolyEdges();
				CreateOrRefreshSpatialHashingDiscs();
			}
		}
	}
	if (g_theGameInput->IsCKeyPressed())
	{
		std::vector<ConvexPoly2D> convexPolys;
		for (ConvexPoly2D& poly : m_convexPoly2Ds)
		{
			bool inside = poly.IsPointInside(m_worldMousePosition);

			if (inside)
			{
				m_mouseColor = Rgba::RED;
				float angle = 0.5f;
				float s = sin(angle);
				float c = cos(angle);
				for (Vec2& vert : poly.m_verts)
				{
					vert -= m_worldMousePosition;

					// Rotate;
					float xnew = vert.x * c - vert.y * s;
					float ynew = vert.x * s + vert.y * c;

					// Translate:
					vert.x = xnew + m_worldMousePosition.x;
					vert.y = ynew + m_worldMousePosition.y;
				}

				RefreshPolyEdges();
				CreateOrRefreshSpatialHashingDiscs();
			}
		}
	}
	if (g_theGameInput->IsXKeyPressed())
	{
		std::vector<ConvexPoly2D> convexPolys;
		for (ConvexPoly2D& poly : m_convexPoly2Ds)
		{
			bool inside = poly.IsPointInside(m_worldMousePosition);

			if (inside)
			{
				m_mouseColor = Rgba::RED;
				float angle = -0.5f;
				float s = sin(angle);
				float c = cos(angle);
				for (Vec2& vert : poly.m_verts)
				{
					vert -= m_worldMousePosition;

					// Rotate;
					float xnew = vert.x * c - vert.y * s;
					float ynew = vert.x * s + vert.y * c;

					// Translate:
					vert.x = xnew + m_worldMousePosition.x;
					vert.y = ynew + m_worldMousePosition.y;
				}

				RefreshPolyEdges();
				CreateOrRefreshSpatialHashingDiscs();
			}
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
	if(g_theGameInput->IsQKeyPressed())
	{
		m_spatialHashing = !m_spatialHashing;
		m_frameCounter = 0;
		m_totalFrameTime = 0.0f;
		m_averageFrameTime = 0.0f;
	}
	if (g_theGameInput->IsWKeyPressed())
	{
		m_shouldDrawSpatialHashingDiscs = !m_shouldDrawSpatialHashingDiscs;
	}
	if (g_theGameInput->IsAKeyPressed())
	{
		m_shouldRenderCheckPolys = !m_shouldRenderCheckPolys;
	}
}

// -----------------------------------------------------------------------
void Game::RefreshPolyEdges()
{
	m_polyEdges.clear();
	int polyEdges = 0;
	for (int convexPolyIndex = 0; convexPolyIndex < m_convexPoly2Ds.size(); ++convexPolyIndex)
	{
		polyEdges += m_convexPoly2Ds[convexPolyIndex].m_numVerts;
	}
	m_polyEdges.reserve(polyEdges);

	for (ConvexPoly2D& convexPoly : m_convexPoly2Ds)
	{
		convexPoly.m_edges.clear();
		for (int i = 0; i < convexPoly.m_numVerts; ++i)
		{
			int next = i + 1;
			if (i == convexPoly.m_numVerts - 1)
			{
				next = 0;
			}
			Line polyEdge = Line(convexPoly.m_verts[i], convexPoly.m_verts[next]);
			convexPoly.m_edges.push_back(polyEdge);
			m_polyEdges.push_back(polyEdge);
		}
	}
}

// -----------------------------------------------------------------------
void Game::CreateOrRefreshSpatialHashingDiscs()
{
	DELETE_POINTER(m_spatialHashingDisc);
	m_spatialHashingDisc = new SpatialHashingDisc();

	m_spatialHashingDisc->SetCenter(Vec2(Map::WIDTH, Map::HEIGHT) / 2);
	m_spatialHashingDisc->SetRadius(Map::WIDTH);
	for (int i = 0; i < m_convexPoly2Ds.size(); ++i)
	{
		m_spatialHashingDisc->m_convexPoly2Ds.push_back(&m_convexPoly2Ds[i]);
	}
	m_spatialHashingDisc->ShrinkToFit();
	m_spatialHashingDisc->m_top = m_spatialHashingDisc->m_center + (m_spatialHashingDisc->m_up * m_spatialHashingDisc->m_radius);
	m_spatialHashingDisc->m_bottom = m_spatialHashingDisc->m_center + (m_spatialHashingDisc->m_down * m_spatialHashingDisc->m_radius);
	m_spatialHashingDisc->GenerateChildren();
}

// -----------------------------------------------------------------------
void Game::DrawMainRaycast()
{
	// Main Ray;
	std::vector<Vertex_PCU> mainRaycastVerts;
	if(m_mainRayHit)
	{
		// Main Ray Split;
		AddVertsForLine2D(mainRaycastVerts, Line(m_mainRayStart, m_mainRayHitPoint, 0.25f), Rgba::GREEN);
		AddVertsForRay2D(mainRaycastVerts, Line(m_mainRayHitPoint, m_mainRayEnd, 0.25f), 10.0f, Rgba::YELLOW);

		// Impact Surface Normal;
		AddVertsForRay2D(mainRaycastVerts, Line(m_mainRayHitPoint, m_mainRayHitPoint + (m_mainRayHitSurfaceNormal * 20.0f), 0.25f), 10.0f, Rgba::RED);

		// Impact Point;
		AddVertsForDisc2D(mainRaycastVerts, m_mainRayHitPoint, 0.4f, Rgba::RED);
		AddVertsForDisc2D(mainRaycastVerts, m_mainRayHitPoint, 0.2f, Rgba::WHITE);
	}
	else
	{
		AddVertsForRay2D(mainRaycastVerts, Line(m_mainRayStart, m_mainRayEnd, 0.25f), 10.0f, Rgba::GRAY);
	}

	g_theRenderer->DrawVertexArray((int)mainRaycastVerts.size(), &mainRaycastVerts[0]);
}

// -----------------------------------------------------------------------
// Adding Debug Verts;
// -----------------------------------------------------------------------
void Game::CheckAndDrawAnyDebugVerts()
{
	std::vector<Vertex_PCU> debugVerts;

	if (m_shouldDrawInvisibleRaycasts)
	{
		AddVertsOfInvisibleRaycasts(&debugVerts);
	}

	if(m_shouldDrawInvisibleRaycastHits && m_invisibleRaysIntersections.size() > 0)
	{
		AddVertsOfInvisibleRaycastHits(&debugVerts);
	}

	if (m_shouldDrawSpatialHashingDiscs)
	{
		AddVertsOfSpatialHashingDiscs(&debugVerts);
	}

	if (debugVerts.size() > 0)
	{
		g_theRenderer->DrawVertexArray((int)debugVerts.size(), &debugVerts[0]);
	}
}

// -----------------------------------------------------------------------
void Game::AddVertsOfInvisibleRaycasts(std::vector<Vertex_PCU>* verts)
{
	for (int raycastIndex = 0; raycastIndex < m_numberOfInvisibleRaycasts; ++raycastIndex)
	{
		AddVertsForLine2D(*verts, m_invisibleRays[raycastIndex], Rgba::GRAY);
	}
}

// -----------------------------------------------------------------------
void Game::AddVertsOfInvisibleRaycastHits(std::vector<Vertex_PCU>* verts)
{
	for (int raycastIntersectionIndex = 0; raycastIntersectionIndex < m_invisibleRaysIntersections.size(); ++raycastIntersectionIndex)
	{
		AddVertsForDisc2D(*verts, m_invisibleRaysIntersections[raycastIntersectionIndex], 0.3f, Rgba::RED);
	}
}

// -----------------------------------------------------------------------
void Game::AddVertsOfSpatialHashingDiscs(std::vector<Vertex_PCU>* verts)
{
	if(m_spatialHashingDisc)
	{
		m_spatialHashingDisc->Render(verts);
	}
}

// -----------------------------------------------------------------------
void Game::DrawDebugInformation()
{
	// Background;
	AABB2 backgroundBoxInformation = AABB2::MakeFromMinsMaxs(Vec2(2.0f, 80.0f), Vec2(37.0f, 98.0f));
	AABB2 backgroundBoxControls = AABB2::MakeFromMinsMaxs(Vec2(176.0f, 78.0f), Vec2(198.0f, 98.0f));
	std::vector<Vertex_PCU> backgroundVerts;
	AddVertsForAABB2D(backgroundVerts, backgroundBoxInformation, Rgba::QUART_BLACK);
	AddVertsForAABB2D(backgroundVerts, backgroundBoxControls, Rgba::QUART_BLACK);

	g_theRenderer->BindShader("Data/Shaders/default_unlit_devconsole.shader");
	g_theRenderer->BindTextureViewWithSampler(0, nullptr);
	g_theRenderer->DrawVertexArray((int)backgroundVerts.size(), &backgroundVerts[0]);

	// Information;
	BitMapFont* font = g_theRenderer->CreateOrGetBitmapFont("SquirrelFixedFont");
	std::vector<Vertex_PCU> textVerts;
	Vec2 textStartPositionInformation = Vec2(3.0f, 95.0f);

	std::string ms    =    Stringf("Time:    %fms", m_frameTime);
	std::string avgms =    Stringf("Average: %fms", m_averageFrameTime);
	std::string polys =    Stringf("Objects:  [%d]", m_numberOfConvexPolys);
	std::string raycasts = Stringf("Raycasts: [%d]", m_numberOfInvisibleRaycasts);
	std::string raycastTime = Stringf("Raycast Time: %fms", m_raycastTime);
	std::string scheme = m_spatialHashing ? "Scheme [ON]" : "Scheme [OFF]";
	std::string visualizeScheme = m_shouldDrawSpatialHashingDiscs ? "Visualize Scheme [ON]" : "Visualize Scheme [OFF]";
	std::string showTargets = m_shouldRenderCheckPolys ? "Show Targets [ON]" : "Show Targets [OFF]";
	
	font->AddVertsForText2D(textVerts, textStartPositionInformation, 1.2f, ms, Rgba::BLACK);
	
	textStartPositionInformation -= Vec2(0.0f, 2.0f);
	font->AddVertsForText2D(textVerts, textStartPositionInformation, 1.2f, avgms, Rgba::BLACK);

	textStartPositionInformation -= Vec2(0.0f, 2.0f);
	font->AddVertsForText2D(textVerts, textStartPositionInformation, 1.2f, polys, Rgba::BLACK);
	
	textStartPositionInformation -= Vec2(0.0f, 2.0f);
	font->AddVertsForText2D(textVerts, textStartPositionInformation, 1.2f, raycasts, Rgba::BLACK);

	textStartPositionInformation -= Vec2(0.0f, 2.0f);
	font->AddVertsForText2D(textVerts, textStartPositionInformation, 1.2f, raycastTime, Rgba::BLACK);

	textStartPositionInformation -= Vec2(0.0f, 2.0f);
	font->AddVertsForText2D(textVerts, textStartPositionInformation, 1.2f, scheme, Rgba::BLACK);

	textStartPositionInformation -= Vec2(0.0f, 2.0f);
	font->AddVertsForText2D(textVerts, textStartPositionInformation, 1.2f, visualizeScheme, Rgba::BLACK);

	textStartPositionInformation -= Vec2(0.0f, 2.0f);
	font->AddVertsForText2D(textVerts, textStartPositionInformation, 1.2f, showTargets, Rgba::BLACK);

	// Controls;
	Vec2 textStartPositionControls = Vec2(177.0f, 95.0f);

	font->AddVertsForText2D(textVerts, textStartPositionControls, 1.2f, "Objects:   [N, M]", Rgba::BLACK);

	textStartPositionControls -= Vec2(0.0f, 2.0f);
	font->AddVertsForText2D(textVerts, textStartPositionControls, 1.2f, "Raycasts:  [K, L]", Rgba::BLACK);

	textStartPositionControls -= Vec2(0.0f, 2.0f);
	font->AddVertsForText2D(textVerts, textStartPositionControls, 1.2f, "Scale:     [V, B]", Rgba::BLACK);

	textStartPositionControls -= Vec2(0.0f, 2.0f);
	font->AddVertsForText2D(textVerts, textStartPositionControls, 1.2f, "Rotate:    [X, C]", Rgba::BLACK);

	textStartPositionControls -= Vec2(0.0f, 2.0f);
	font->AddVertsForText2D(textVerts, textStartPositionControls, 1.2f, "Ray Start: [S]", Rgba::BLACK);

	textStartPositionControls -= Vec2(0.0f, 2.0f);
	font->AddVertsForText2D(textVerts, textStartPositionControls, 1.2f, "Ray End:   [E]", Rgba::BLACK);

	textStartPositionControls -= Vec2(0.0f, 2.0f);
	font->AddVertsForText2D(textVerts, textStartPositionControls, 1.2f, "Toggle Scheme: [Q]", Rgba::BLACK);

	textStartPositionControls -= Vec2(0.0f, 2.0f);
	font->AddVertsForText2D(textVerts, textStartPositionControls, 1.2f, "Show Scheme: [W]", Rgba::BLACK);

	textStartPositionControls -= Vec2(0.0f, 2.0f);
	font->AddVertsForText2D(textVerts, textStartPositionControls, 1.2f, "Show Targets: [A]", Rgba::BLACK);

	if ((int)textVerts.size() > 0)
	{
		g_theRenderer->BindShader("Data/Shaders/default_unlit_devconsole.shader");
		g_theRenderer->BindTextureView(0u, font->GetTextureView());
		g_theRenderer->DrawVertexArray((int)textVerts.size(), &textVerts[0]);
	}
}

// -----------------------------------------------------------------------
// static;
// -----------------------------------------------------------------------
void CallImageAndMeshLoadThread()
{
	g_theApp->m_theGame->ImageAndMeshLoadThread();
}