#pragma once
#include "Engine/Core/EngineCommon.hpp"
#include "Game/Gameplay/GameHandle.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Game/Gameplay/Entity.hpp"

#include <vector>

enum GameState : int;
class Game;
class OrbitCamera;
class UIWidget;
class Map;
class Prop;

enum CommandState
{
	NEUTRALSTATE,
	BUILDSTATE,
	ATTACKSTATE,
	FOLLOWSTATE,
	MOVESTATE,
	GATHERSTATE


};

enum ArrowKeys
{
	ARROWKEYS_UPARROW,
	ARROWKEYS_DOWNARROW,
	ARROWKEYS_LEFTARROW,
	ARROWKEYS_RIGHTARROW
};

enum BracketKeys
{
	BRACKETKEYS_LEFT,
	BRACKETKEYS_RIGHT
};

enum MouseVisibility
{
	MOUSEVISIBILITY_SHOW,
	MOUSEVISIBILITY_HIDE
};

// Called each frame - translates raw input (keyboard/mouse/etc) to input for the Game to use;
class PlayerController
{

public:

	PlayerController( Game* theGame );
	~PlayerController();

	void UpdateInput( float deltaSeconds );
	void UpdatePlaceHolderProp();
	void RenderPlaceHolderProp();

	Vec2 GetFrameScroll();
	float GetFrameZoomDistance();	
	float GetFramePan();

	bool IsRotating();

	// Pause
	bool IsPaused();

	// Selection Box;
	Vec2 GetMouseSelectionBoxStart();
	Vec2 GetMouseSelectionBoxEnd();
	void UpdateSelectionBox(Frustum& frustum);
	void SelectUnits(Frustum& frustum);

	// Mouse Clicks
	void ClickLeftMouse();
	void ClickRightMouse();
	void ClickMiddleMouse();
	void ReleaseLeftMouse();
	void ReleaseRightMouse();
	void ReleaseMiddleMouse();
	void LeftClickUIWidget( UIWidget* uiWidget );

	// Mouse States
	void ShowOrHideMouseCheck();
	bool IsLeftMouseClicked();
	bool WasLeftMouseReleased();
	bool IsRightMouseClicked();
	bool WasRightMouseReleased();
	bool IsMiddleMouseClicked();

	// Mouse Wheel
	void StoreWheelMovement( float wheelDelta );
	int GetMouseWheelDirection();

	// Key Pressed
	void HandleKeyPressed( unsigned char asKey );
	void HandleKeyReleased( unsigned char asKey );

	// Camera;
	void SetGameCamera( OrbitCamera* gameCamera );
	void UpdateTargetDistanceOfCamera();

	// Game State;
	void SetGameState(GameState gameState);
	GameState GetGameState();

	// Map;
	Map* GetGameMap();
	inline Vec3 GetTerrainSpot() { return m_hoveredTerrainSpot; }

	// Raycast;
	void RaycastToMapOrEntity(Ray3* ray, float* entityTime, float* mapTime);

public:

	Map* m_theMap = nullptr;
	IntVec2 m_clientDimensions = IntVec2(0, 0);
	IntVec2 m_clientMousePosition = IntVec2(0, 0);
	IntVec2 m_clientMousePosition2 = IntVec2(0, 0);

	// Raycast;
	Vec3 m_hoveredTerrainSpot = Vec3(0.0f, 0.0f, 0.0f);
	std::vector<GameHandle> m_selectedEntities;
	std::vector<GameHandle> m_hoveredEntities;

	//Entity* m_hoveredEntity = nullptr;
	
	// Pause;
	bool m_isPaused = false;

	// Keyboard Input;
	bool m_shiftKeyDown = false;
	bool m_arrowKeys[4];
	bool m_bracketKeys[2];
	Vec2 m_keyboardScroll = Vec2( 0.0f, 0.0f );
	float m_cameraPan = 0.0f;
	float m_keyboardScrollSpeed = 2.0f;
	float m_cameraPanSpeed = 70.0f;

	// Mouse Input;
	MouseVisibility m_mouseMode = MOUSEVISIBILITY_SHOW;
	bool m_leftMouseClicked = false;
	bool m_leftMouseReleased = false;
	bool m_rightMouseClicked = false;
	bool m_rightMouseReleased = false;
	bool m_middleMouseClicked = false;
	bool m_middleMouseReleased = false;
	float m_wheelDelta = 0.0f;

	// Selection Box;
	Vec2 m_mouseSelectionBoxStart = Vec2::VNULL;
	Vec2 m_mouseSelectionBoxEnd = Vec2::VNULL;

	// Mouse Edge Scroll;
	float m_edgePanSpeed = 2.0f;
	float m_edgePanDistance = 32.0f;

	// Zoom;
	float targetCameraDistance = 0.0f;
	float previousCameraDistance = 0.0f;
	float percentTransition = 1.0f;

	// The Game that we are attached to;
	Game* m_theGame = nullptr;
	OrbitCamera* m_theGameCamera = nullptr;

	// GameState
	GameState m_gameState;

	// UI Widget;
	UIWidget* m_hoveredButtonWidget = nullptr;

	// Building Something;
	Entity* m_buildingEntity = nullptr;
	Prop* m_placeholderProp = nullptr;
	Prop* m_placedPlaceHolderProp = nullptr;
	EntityDefinition* m_placeHolderDefinition = nullptr;
	EntityDefinition* m_placedPlaceHolderDefinition = nullptr;
	bool m_canBuildPlaceHolderProp = false;

	CommandState m_commandState = NEUTRALSTATE;
	std::vector<GameHandle> m_stateEntities;
};