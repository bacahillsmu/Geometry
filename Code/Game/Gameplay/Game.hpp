#pragma once
#include "Engine/Core/Rgba.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Async/AsyncQueue.hpp"

#include "Engine/Renderer/Shader.hpp"

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
class UIWidget;
class UILabel;
class UIButton;
class UIRadioGroup;
class Prop;
struct IntVec2;
struct Camera;
class RTSCommand;

namespace RakNet 
{
	struct Packet;
	struct RakNetGUID;
	struct SystemAddress;
}

struct ImageLoading
{
	ImageLoading();
	ImageLoading(std::string name);
	
	std::string imageName;
	Image* image;
};

struct CPUMeshLoading
{
	CPUMeshLoading();
	CPUMeshLoading(std::string name);

	std::string meshName;
	CPUMesh* cpuMesh;
};


enum GameState
{
	GAMESTATE_UNKNOWN = -1,
	
	GAMESTATE_INIT,
	GAMESTATE_LOADING,
	GAMESTATE_TITLE,
	GAMESTATE_LOBBY,
	GAMESTATE_PLAYMAPLOAD,
	GAMESTATE_EDITMAPLOAD,
	GAMESTATE_PLAY,
	GAMESTATE_EDIT,

	GAMESTATE_COUNT
};

class Game
{
	
public:

	// Constructor/Deconstructor
	Game();
	~Game();

	// Game Flow;
	void Init();
	void Startup();
	void BeginFrame();
	void PreRenderStateStartup();
	void Render();
	void Update(float deltaSeconds);
	void EndFrame();

	// Async Learning bullshit;
	void ContinueLoading();
	bool DoneLoading();
	void LoadingTitleRender();
	void StartLoadingTexture(std::string nameOfTexture);
	void StartLoadingGPUMesh(std::string nameOfGPUMesh);
	void ImageAndMeshLoadThread();
	void EndLoadingThreads();

	// Game State Startups;
	void TitleStartup();
	void LobbyStartup();
	void PlayMapLoadStartup();
	void EditMapLoadStartup();

	// Other Load Play Stuff;
	void LoadPlayModels();

	// Other Load Edit Stuff;
	void LoadEditModels();

	// Game State Renders;
	void InitRender();
	void TitleRender();
	void LobbyRender();
	void PlayMapLoadRender();
	void EditMapLoadRender();
	void PlayRender();
	void EditRender();
	void PauseRender();

	// Selection Box;
	void RenderSelectionBox();

	// Game State Updates;
	void TitleUpdate(float deltaSeconds);
	void LobbyUpdate(float deltaSeconds);
	void PlayMapLoadUpdate();
	void EditMapLoadUpdate();
	void PlayUpdate( float deltaSeconds );
	void EditUpdate( float deltaSeconds );

	// Load UI Widgets;
	void LoadPauseWidgets();

	// Assets
	void StartLoadingAssets();
	void SetGameAssets();
	void EnqueueWorkForTexturesAndGPUMeshes();
	void LoadGameShaders();
	void LoadGameFonts();
	void LoadGameMaterials();
	void CreateLoadingScreen();

	// Cameras;
	void CreateCameras();

	// Game Camera;
	void UpdateGameCamera( float deltaSeconds );
	void UpdateFocalPointPosition( float deltaSeconds );
	void UpdateGameCameraPan( float deltaSeconds );
	void UpdateGameCameraZoom();
	void UpdateGameCameraPosition();
	OrbitCamera* GetGameCamera();

	// GameState;
	void SetGameState( GameState gameState );
	void CheckAndChangeGameState(); 
	GameState GetGameState();

	// Lobby;
	void DrawLobbyPlayers();
	void DrawLobbyPlayersBoxes();
	void DrawLobbyPlayersNames();

	// UI Widgets;
	UIWidget* GetUIWidgetOfCurrentState();
	void DeleteUIWidgets();

	// Map;
	Map*& GetMap() { return m_map; }
	Map* GetGameMap() { return m_map; }

	// Queue
	void EnQueueCommand(RTSCommand* command);
	void ProcessCommands(); // Process and free up memory;
	void ProcessNetworkCommands();
	void ClearCommands();   // Just free up memory;

	// Team;
	void SetCurrentTeam(int teamID);
	int GetCurrentTeam();

	// Sound;
	void StopAllMusic();

	// RakNet;
	void OnIncomingPacket(RakNet::Packet* packet);
	void SetupConnectionInformation();
	void ReadMessageIntoRTSCommand(const char* message);

private:

	std::vector<RTSCommand*> m_myRTSCommands;
	std::vector<RTSCommand*> m_themRTSCommands;


public:
	
	int m_currentTeam					= 0;
	
	bool m_readFromReplay				= false;
	bool m_titleStartupDone				= false;
	bool m_lobbyStartupDone				= false;
	bool m_playStartupDone				= false;
	bool m_editStartupDone				= false;
	bool m_isPaused						= false;
	bool m_setupConnectionInfoComplete	= false;
	bool m_isConnected					= false;
	bool m_playMapLoadDone				= false;
	bool m_connectedPlayMapLoadDone		= false;
	bool m_connectedPlayMapLoadSent		= false;

	float m_fadeTimer = 0.0f;
	float m_fadeTime = 1.5f;

	//IntVec2 m_mousePosition				= IntVec2(0, 0);
	Vec2 m_clientMins					= Vec2(0.0f, 0.0f);
	Vec2 m_clientMaxs					= Vec2(0.0f, 0.0f);
	
	

	BitMapFont* m_squirrelFont			= nullptr;
	
	ColorTargetView* m_colorTargetView = nullptr;
	
	
	
	Rgba m_clearColor					= Rgba(0.5f, 0.5f, 0.5f, 1.0f);

	std::vector<std::string> m_mouseInformationText;

	// Game State;
	GameState m_gameState				= GAMESTATE_INIT;

	// Game Camera;
	OrbitCamera* m_gameMainCamera		= nullptr;	
	float m_fieldOfView					= 45.0f;
	float m_cameraZoom					= 0.0f;
	Vec3 m_cameraPosition				= Vec3( 0.0f, 0.0f, -10.0f );
	Vec3 m_cameraEuler					= Vec3( 0.0f, 0.0f, 0.0f );
	Vec3 m_focalPoint					= Vec3( 0.0f, 0.0f, 0.0f );

	// UI Camera;
	Camera* m_uiCamera					= nullptr;

	// Title UI Widgets;
	UIWidget* m_titleWidget				= nullptr;
	UIButton* m_titlePlayButton			= nullptr;
	UILabel*  m_titlePlayLabel			= nullptr;
	UIButton* m_editPlayButton			= nullptr;
	UILabel*  m_editPlayLabel			= nullptr;
	UIButton* m_quitPlayButton			= nullptr;
	UILabel*  m_quitPlayLabel			= nullptr;

	// Lobby UI Widgets;
	UIWidget* m_lobbyWidget				= nullptr;
	UIButton* m_lobbyPlayButton			= nullptr;
	UILabel*  m_lobbyPlayLabel			= nullptr;
	UIButton* m_lobbyBackButton			= nullptr;
	UILabel*  m_lobbyBackLabel			= nullptr;

	// Pause UI Widgets;
	UIWidget* m_pauseWidget				= nullptr;
	UIButton* m_pauseResumeButton		= nullptr;
	UILabel*  m_pauseResumeLabel		= nullptr;
	UIButton* m_pauseQuitButton			= nullptr;
	UILabel*  m_pauseQuitLabel			= nullptr;
	UIButton* m_pauseQuitAndSaveButton	= nullptr;
	UILabel*  m_pauseQuitAndSaveLabel	= nullptr;

	// Play UI Widgets;
	UIWidget* m_playWidget				= nullptr;
	UIButton* m_playMainMenuButton		= nullptr;
	UILabel*  m_playMainMenuLabel		= nullptr;

	// Edit UI Widgets;
	UIWidget* m_editWidget				= nullptr;
	UIButton* m_editTexture1Button		= nullptr;
	UILabel*  m_editTexture1Label		= nullptr;
	UIButton* m_editTexture2Button		= nullptr;
	UILabel*  m_editTexture2Label		= nullptr;
	UIButton* m_editTexture3Button		= nullptr;
	UILabel*  m_editTexture3Label		= nullptr;
	UIButton* m_editTexture4Button		= nullptr;
	UILabel*  m_editTexture4Label		= nullptr;
	UIButton* m_editTexture5Button		= nullptr;
	UILabel*  m_editTexture5Label		= nullptr;

	// Radio Groups;
	UIRadioGroup* m_editRadioGroup		= nullptr;


	// Map;
	Map* m_map = nullptr;

	// TextureViews;
	TextureView* m_loadingScreen		= nullptr;
	TextureView* m_titleScreen			= nullptr;
	TextureView* m_lobbyScreen			= nullptr;
	TextureView* m_playMapLoading		= nullptr;
	TextureView* m_editMapLoading		= nullptr;


	// Shaders

	// Materials
	Material* m_tonemapMaterial			= nullptr;
	
	// GPU Mesh;
	GPUMesh* m_cubeMesh					= nullptr;
	GPUMesh* m_sphereMesh				= nullptr;
	GPUMesh* m_titleMesh				= nullptr;
	

	// Model Matrixs;
	Matrix4x4 m_titleModel;
	Matrix4x4 m_lobbyModel;
	
	// Replay;
	std::string m_replayFilename		= "";	

	// Sound;
	bool m_mainMenuMusicPlaying = false;
	bool m_playMusicPlaying = false;
	size_t m_mainMenuMusicPlaybackID;
	size_t m_playMusicPlaybackID;

	// RakNet;
	unsigned int m_commandFrameToProcess	= 0;
	unsigned int m_commandFrameDelay		= 15;
	unsigned int m_myCommandFrameComplete	= 0;
	unsigned int m_themCommandFrameComplete = 0;
	unsigned int m_myHashedFrame			= 0;
	unsigned int m_themHashedFrame			= 0;

	// Async;
	int m_objectLoading = 0;
	int m_objectsToLoad = 100;
	AsyncQueue<ImageLoading> imageLoadingFromDiscQueue;
	AsyncQueue<ImageLoading> imageCreatingTextureQueue;
	AsyncQueue<CPUMeshLoading> cpuLoadingFromDiscQueue;
	AsyncQueue<CPUMeshLoading> cpuCreatingGPUMeshQueue;
	std::vector<std::thread> m_threads;

	double m_loadStartTime = 0.0;
	double m_loadEndTime = 0.0;
	
};

void CallImageAndMeshLoadThread();