#include "ThirdParty/RakNet/RakNetInterface.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Game/Gameplay/Game.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Game/Gameplay/Camera/OrbitCamera.hpp"
#include "Engine/Core/WindowContext.hpp"
#include "Game/Gameplay/UI/UIWidget.hpp"
#include "Game/Framework/App.hpp"
#include "Game/Gameplay/Map/Map.hpp"
#include "Game/Framework/GameCommon.hpp"
#include "Engine/Math/Ray.hpp"

#include "Game/Gameplay/Input/PlayerController.hpp"
#include "Game/Gameplay/RTSCommand.hpp"
#include "Game/Gameplay/Input/ReplayController.hpp"
#include "Game/Gameplay/EntityDefinition.hpp"
#include "Engine/Renderer/Prop.hpp"
#include "Engine/Renderer/Model.hpp"
#include "Game/Gameplay/Lobby/LobbyConsole.hpp"
#include "Engine/Profile/Profile.hpp"

PlayerController* g_thePlayerController = nullptr;

// -----------------------------------------------------------------------
PlayerController::PlayerController( Game* theGame )
{
	m_theGame = theGame;
	m_gameState = m_theGame->GetGameState();
	SetGameCamera( m_theGame->GetGameCamera() );

	targetCameraDistance = m_theGameCamera->m_distance;
	previousCameraDistance = m_theGameCamera->m_distance;

	m_arrowKeys[ARROWKEYS_UPARROW] = false;
	m_arrowKeys[ARROWKEYS_DOWNARROW] = false;
	m_arrowKeys[ARROWKEYS_LEFTARROW] = false;
	m_arrowKeys[ARROWKEYS_RIGHTARROW] = false;

	m_bracketKeys[BRACKETKEYS_LEFT] = false;
	m_bracketKeys[BRACKETKEYS_RIGHT] = false;

	m_mouseMode = MOUSEVISIBILITY_SHOW;
}

// -----------------------------------------------------------------------
PlayerController::~PlayerController()
{

}

// -----------------------------------------------------------------------
void PlayerController::UpdateInput( float deltaSeconds )
{
	PROFILE_FUNCTION();

	// Get the current Map from the Game;
	m_theMap = GetGameMap();

	// Get client information from WindowsContext;	
	m_clientDimensions = g_theWindowContext->GetClientDimensions();
	m_clientMousePosition = g_theWindowContext->GetClientMousePosition();
	m_clientMousePosition2 = g_theWindowContext->GetClientMousePosition();
	m_clientMousePosition2.y = (int)m_clientDimensions.y - m_clientMousePosition2.y;

	switch(m_gameState)
	{
		case GAMESTATE_PLAY:
		{
			Vec2 bottomLeft = Vec2(GetMin(m_mouseSelectionBoxStart.x, m_mouseSelectionBoxEnd.x), GetMin(m_mouseSelectionBoxStart.y, m_mouseSelectionBoxEnd.y));
			Vec2 topRight = Vec2(GetMax(m_mouseSelectionBoxStart.x, m_mouseSelectionBoxEnd.x), GetMax(m_mouseSelectionBoxStart.y, m_mouseSelectionBoxEnd.y));
			Frustum frustum = m_theGameCamera->GetWorldFrustumFromClientRegion(bottomLeft, topRight, m_clientDimensions);
			
			m_theMap->UnhoverAllEntities();
			m_hoveredEntities.clear();
			
			Ray3 ray = m_theGameCamera->GetRayFromClientPosition(m_clientMousePosition, m_clientDimensions);
			float entityTime;
			float mapTime;
			RaycastToMapOrEntity(&ray, &entityTime, &mapTime);
			UpdateSelectionBox(frustum);

			if(m_commandState == BUILDSTATE)
			{
				if(WasLeftMouseReleased() && m_canBuildPlaceHolderProp && m_theMap->CanAffordEntity(m_placeHolderDefinition))
				{
					m_theMap->PayCostEntity(m_placeHolderDefinition);
					BuildCommand* buildCommand = new BuildCommand();
					buildCommand->m_commandFrameIssued = g_theApp->m_commandFrame.GetCommandFrame();
					buildCommand->m_theGame = m_theGame;
					buildCommand->m_team = m_theGame->GetCurrentTeam();
					buildCommand->m_buildPosition = m_hoveredTerrainSpot;
					buildCommand->m_unit = m_buildingEntity->GetGameHandle();
					if(m_placeHolderDefinition->m_entityType == "TownHall")
					{
						buildCommand->m_commandType = BUILDTOWNHALL;
					}
					else if(m_placeHolderDefinition->m_entityType == "Hut")
					{
						buildCommand->m_commandType = BUILDHUT;
					}
					else if(m_placeHolderDefinition->m_entityType == "Tower")
					{
						buildCommand->m_commandType = BUILDTOWER;
					}
					else if(m_placeHolderDefinition->m_entityType == "GoblinHut")
					{
						buildCommand->m_commandType = BUILDGOBLINHUT;
					}
					else if(m_placeHolderDefinition->m_entityType == "GoblinTower")
					{
						buildCommand->m_commandType = BUILDGOBLINTOWER;
					}
					buildCommand->m_theMap = m_theGame->GetGameMap();
					buildCommand->m_buildPosition = m_placeholderProp->m_model->m_modelMatrix.GetT();
					m_theGame->EnQueueCommand(buildCommand);
					g_theReplayController->RecordCommand(buildCommand);

					m_placedPlaceHolderProp = m_placeholderProp;
					m_placedPlaceHolderDefinition = m_placeHolderDefinition;
					m_canBuildPlaceHolderProp = false;
					m_placeholderProp = nullptr;
					m_placeHolderDefinition = nullptr;
					m_buildingEntity = nullptr;
					m_commandState = NEUTRALSTATE;
					m_mouseSelectionBoxStart = Vec2::VNULL;
					m_mouseSelectionBoxEnd = Vec2::VNULL;
				}
				else if((WasLeftMouseReleased() && !m_canBuildPlaceHolderProp) || (WasLeftMouseReleased() && !m_theMap->CanAffordEntity(m_placeHolderDefinition)) || WasRightMouseReleased())
				{
					m_canBuildPlaceHolderProp = false;
					m_placeholderProp = nullptr;
					m_placeHolderDefinition = nullptr;
					m_buildingEntity = nullptr;
					m_commandState = NEUTRALSTATE;
					m_mouseSelectionBoxStart = Vec2::VNULL;
					m_mouseSelectionBoxEnd = Vec2::VNULL;
				}
			}
			else if(m_commandState == ATTACKSTATE)
			{
				if(WasLeftMouseReleased() && m_hoveredEntities.size() > 0)
				{
					GameHandle handleToAttack = m_hoveredEntities.front();
					for(GameHandle handle: m_stateEntities)
					{
						Entity* entity = m_theMap->FindEntity(handle);

						std::map< std::string, RTSTaskInfo* >::iterator found = entity->m_entityDefinition->m_availableTasks.find("attack");
						if(found != entity->m_entityDefinition->m_availableTasks.end())
						{
							AttackCommand* attackCommand = new AttackCommand();
							attackCommand->m_commandFrameIssued = g_theApp->m_commandFrame.GetCommandFrame();
							attackCommand->m_unit = entity->GetGameHandle();
							attackCommand->m_entityToAttack = handleToAttack;
							attackCommand->m_theGame = m_theGame;
							attackCommand->m_commandType = 3;
							attackCommand->m_theMap = m_theGame->GetGameMap();
							m_theGame->EnQueueCommand(attackCommand);
							g_theReplayController->RecordCommand(attackCommand);
						}
					}
					m_stateEntities.clear();
					m_commandState = NEUTRALSTATE;
					m_mouseSelectionBoxStart = Vec2::VNULL;
					m_mouseSelectionBoxEnd = Vec2::VNULL;
				}
				else if(WasLeftMouseReleased() && m_hoveredEntities.size() == 0)
				{
					m_commandState = NEUTRALSTATE;
					m_mouseSelectionBoxStart = Vec2::VNULL;
					m_mouseSelectionBoxEnd = Vec2::VNULL;
				}
				else if(WasRightMouseReleased())
				{
					m_commandState = NEUTRALSTATE;
					m_mouseSelectionBoxStart = Vec2::VNULL;
					m_mouseSelectionBoxEnd = Vec2::VNULL;
				}
			}
			else if(m_commandState == FOLLOWSTATE)
			{
				if(WasLeftMouseReleased() && m_hoveredEntities.size() > 0)
				{
					GameHandle handleToFollow = m_hoveredEntities.front();
					for(GameHandle handle: m_stateEntities)
					{
						Entity* entity = m_theMap->FindEntity(handle);

						std::map< std::string, RTSTaskInfo* >::iterator found = entity->m_entityDefinition->m_availableTasks.find("follow");
						if(found != entity->m_entityDefinition->m_availableTasks.end())
						{
							FollowCommand* followCommand = new FollowCommand();
							followCommand->m_commandFrameIssued = g_theApp->m_commandFrame.GetCommandFrame();
							followCommand->m_unit = entity->GetGameHandle();
							followCommand->m_entityToFollow = handleToFollow;
							followCommand->m_theGame = m_theGame;
							followCommand->m_commandType = 5;
							followCommand->m_theMap = m_theGame->GetGameMap();
							m_theGame->EnQueueCommand(followCommand);
							g_theReplayController->RecordCommand(followCommand);
						}
					}
					m_stateEntities.clear();
					m_commandState = NEUTRALSTATE;
					m_mouseSelectionBoxStart = Vec2::VNULL;
					m_mouseSelectionBoxEnd = Vec2::VNULL;
				}
				else if(WasLeftMouseReleased() && m_hoveredEntities.size() == 0)
				{
					m_commandState = NEUTRALSTATE;
					m_mouseSelectionBoxStart = Vec2::VNULL;
					m_mouseSelectionBoxEnd = Vec2::VNULL;
				}
				else if(WasRightMouseReleased())
				{
					m_commandState = NEUTRALSTATE;
					m_mouseSelectionBoxStart = Vec2::VNULL;
					m_mouseSelectionBoxEnd = Vec2::VNULL;
				}
			}
			else if(m_commandState == MOVESTATE)
			{
				if(WasLeftMouseReleased())
				{
					for(GameHandle handle: m_stateEntities)
					{
						Entity* entity = m_theMap->FindEntity(handle);

						std::map< std::string, RTSTaskInfo* >::iterator found = entity->m_entityDefinition->m_availableTasks.find("follow");
						if(found != entity->m_entityDefinition->m_availableTasks.end())
						{
							MoveCommand* moveCommand = new MoveCommand();
							moveCommand->m_commandFrameIssued = g_theApp->m_commandFrame.GetCommandFrame();
							moveCommand->m_unit = entity->GetGameHandle();
							moveCommand->m_theGame = m_theGame;
							moveCommand->m_commandType = 2;
							moveCommand->m_theMap = m_theGame->GetGameMap();
							moveCommand->m_movePosition = m_hoveredTerrainSpot;
							m_theGame->EnQueueCommand(moveCommand);
							g_theReplayController->RecordCommand(moveCommand);
						}
					}
					m_stateEntities.clear();
					m_commandState = NEUTRALSTATE;
					m_mouseSelectionBoxStart = Vec2::VNULL;
					m_mouseSelectionBoxEnd = Vec2::VNULL;
				}
				else if(WasRightMouseReleased())
				{
					m_commandState = NEUTRALSTATE;
					m_mouseSelectionBoxStart = Vec2::VNULL;
					m_mouseSelectionBoxEnd = Vec2::VNULL;
				}
			}
			else if(m_commandState == GATHERSTATE)
			{
				if(WasLeftMouseReleased() && m_hoveredEntities.size() > 0)
				{
					for(GameHandle handle: m_stateEntities)
					{
						GameHandle handleToGather = m_hoveredEntities.front();						
						Entity* gatherEntity = m_theMap->FindEntity(handleToGather);
						if(gatherEntity && gatherEntity->m_isResource)
						{
							Entity* entity = m_theMap->FindEntity(handle);
							std::map< std::string, RTSTaskInfo* >::iterator found = entity->m_entityDefinition->m_availableTasks.find("gather");
							if(found != entity->m_entityDefinition->m_availableTasks.end())
							{
								GatherCommand* gatherCommand = new GatherCommand();
								gatherCommand->m_commandFrameIssued = g_theApp->m_commandFrame.GetCommandFrame();
								gatherCommand->m_unit = entity->GetGameHandle();
								gatherCommand->m_theGame = m_theGame;
								gatherCommand->m_commandType = 9;
								gatherCommand->m_theMap = m_theGame->GetGameMap();
								gatherCommand->m_entityToGather = handleToGather;
								m_theGame->EnQueueCommand(gatherCommand);
								g_theReplayController->RecordCommand(gatherCommand);
							}
						}
					}
					m_stateEntities.clear();
					m_commandState = NEUTRALSTATE;
					m_mouseSelectionBoxStart = Vec2::VNULL;
					m_mouseSelectionBoxEnd = Vec2::VNULL;
				}
				else if(WasRightMouseReleased())
				{
					m_commandState = NEUTRALSTATE;
					m_mouseSelectionBoxStart = Vec2::VNULL;
					m_mouseSelectionBoxEnd = Vec2::VNULL;
				}
			}
			else if(m_commandState == NEUTRALSTATE)
			{
				SelectUnits(frustum);
				
				// If we have a selected Entity, check to see if we RightClicked and move Entity;
				if(m_selectedEntities.size() > 0 && WasRightMouseReleased())
				{
					for(GameHandle selection: m_selectedEntities)
					{
						// Set our selected Entity;
						Entity* selectedEntity = m_theMap->FindEntity(selection);
						if(!m_shiftKeyDown)
						{
							selectedEntity->ClearTasks();
						}

						RightClickCommand* rightClickCommand = new RightClickCommand();
						rightClickCommand->m_commandFrameIssued = g_theApp->m_commandFrame.GetCommandFrame();
						rightClickCommand->m_commandType = 6;
						rightClickCommand->m_theGame = m_theGame;
						rightClickCommand->m_theMap = m_theGame->GetGameMap();
						rightClickCommand->m_unit = selection;					
						rightClickCommand->m_rightClickPosition = ray.PointAtTime(mapTime);
						if(m_hoveredEntities.size() > 0)
						{
							rightClickCommand->m_rightClickEntity = m_hoveredEntities.front();
						}
						m_theGame->EnQueueCommand(rightClickCommand);
						g_theReplayController->RecordCommand(rightClickCommand);
					}
				}
			}
		}
	}	

	// Reset
	m_keyboardScroll = Vec2( 0.0f, 0.0f );
	m_cameraPan = 0.0f;	
	m_hoveredButtonWidget = nullptr;

	// Rotation
	if(m_bracketKeys[BRACKETKEYS_RIGHT])
	{
		m_cameraPan = m_cameraPanSpeed;
	}

	if(m_bracketKeys[BRACKETKEYS_LEFT])
	{
		m_cameraPan = -m_cameraPanSpeed;
	}

	// Zoom
	percentTransition += deltaSeconds * 2.0f;
	percentTransition = Clamp(percentTransition, 0.0f, 1.0f);

	// Scroll
	ShowOrHideMouseCheck();
	if(IsMiddleMouseClicked())
	{
		IntVec2 relativeMousePosition = g_theWindowContext->GetClientMouseRelativePosition();
		if(relativeMousePosition.x < 0)
		{
			m_keyboardScroll.y = m_keyboardScrollSpeed;
		}

		if(relativeMousePosition.x > 0)
		{
			m_keyboardScroll.y = -m_keyboardScrollSpeed;
		}

		if(relativeMousePosition.y < 0)
		{
			m_keyboardScroll.x = -m_keyboardScrollSpeed;
		}

		if(relativeMousePosition.y > 0)
		{
			m_keyboardScroll.x = m_keyboardScrollSpeed;
		}
	}
	else if(m_arrowKeys[ARROWKEYS_UPARROW]
		 || m_arrowKeys[ARROWKEYS_DOWNARROW]
		 || m_arrowKeys[ARROWKEYS_RIGHTARROW]
		 || m_arrowKeys[ARROWKEYS_LEFTARROW])
	{
		if(m_arrowKeys[ARROWKEYS_UPARROW])
		{
			m_keyboardScroll.x = m_keyboardScrollSpeed;
		}

		if(m_arrowKeys[ARROWKEYS_DOWNARROW])
		{
			m_keyboardScroll.x = -m_keyboardScrollSpeed;
		}

		if(m_arrowKeys[ARROWKEYS_RIGHTARROW])
		{
			m_keyboardScroll.y = m_keyboardScrollSpeed;
		}

		if(m_arrowKeys[ARROWKEYS_LEFTARROW])
		{
			m_keyboardScroll.y = -m_keyboardScrollSpeed;
		}
	}
	else
	{
		// This logic is confusing/backwards. (0, 0) being in the top left. And me too lazy to fix it;
		IntVec2 mousePosition = g_theWindowContext->GetClientMousePosition();
		IntVec2 clientMins = g_theWindowContext->GetClientMins();
		IntVec2 clientMaxs = g_theWindowContext->GetClientMaxs();

		float mousepositionx = (float)mousePosition.x;
		float mousepositiony = (float)mousePosition.y;

		Vec2 clientsmins = Vec2((float)clientMins.x, (float)clientMaxs.y);
		Vec2 clientsmaxs = Vec2((float)clientMaxs.x, (float)clientMins.y);

		mousepositiony = abs(mousepositiony - clientsmaxs.y);

		Vec2 betterMousePosition = Vec2(mousepositionx, mousepositiony);

		// First check if we are within bounds of an UI element that we are in a state for;
		switch(m_gameState)
		{

			case GAMESTATE_TITLE:
			{
				UIWidget* uiWidget = m_theGame->GetUIWidgetOfCurrentState();
				if(uiWidget)
				{
					for(auto& child: uiWidget->GetChildren())
					{
						if(child->GetUIFlags() == UIWIDGETFLAGS_BUTTON)
						{
							UIButton* childButton = static_cast<UIButton*>(child);
							if(childButton->GetBounds().IsPointInside(betterMousePosition))
							{
								m_hoveredButtonWidget = child;
								childButton->m_shouldHighlight = true;
								return;
							}
							else
							{
								childButton->m_shouldHighlight = false;
							}
						}					
					}
				}

				break;
			}

			case GAMESTATE_LOBBY:
			{
				UIWidget* uiWidget = m_theGame->GetUIWidgetOfCurrentState();
				if(uiWidget)
				{
					for(auto& child: uiWidget->GetChildren())
					{
						if(child->GetUIFlags() == UIWIDGETFLAGS_BUTTON)
						{
							UIButton* childButton = static_cast<UIButton*>(child);
							if(childButton->GetBounds().IsPointInside(betterMousePosition))
							{
								m_hoveredButtonWidget = child;
								childButton->m_shouldHighlight = true;
								return;
							}
							else
							{
								childButton->m_shouldHighlight = false;
							}
						}					
					}
				}

				break;
			}

			case GAMESTATE_PLAY:
			{
				UIWidget* uiWidget = m_theGame->GetUIWidgetOfCurrentState();
				if(uiWidget)
				{
					for(auto& child: uiWidget->GetChildren())
					{
						if(child->GetUIFlags() == UIWIDGETFLAGS_BUTTON)
						{
							UIButton* childButton = static_cast<UIButton*>(child);
							if(childButton->GetBounds().IsPointInside(betterMousePosition))
							{
								m_hoveredButtonWidget = child;
								childButton->m_shouldHighlight = true;
								return;
							}
							else
							{
								childButton->m_shouldHighlight = false;
							}
						}					
					}

// 					if(!m_theGame->m_isPaused)
// 					{
// 						for(auto& child: m_theGame->m_playTaskWidget->GetChildren())
// 						{
// 							if(child->GetUIFlags() == UIWIDGETFLAGS_BUTTON)
// 							{
// 								UIButton* childButton = static_cast<UIButton*>(child);
// 								if(childButton->GetBounds().IsPointInside(betterMousePosition))
// 								{
// 									m_hoveredButtonWidget = child;
// 									childButton->m_shouldHighlight = true;
// 									return;
// 								}
// 								else
// 								{
// 									childButton->m_shouldHighlight = false;
// 								}								
// 							}					
// 						}
// 					}					
				}

				break;
			}

			case GAMESTATE_EDIT:
			{
				UIWidget* uiWidget = m_theGame->GetUIWidgetOfCurrentState();
				if(uiWidget)
				{
					for(auto& child: uiWidget->GetChildren())
					{
						if(child->GetUIFlags() == UIWIDGETFLAGS_BUTTON)
						{
							UIButton* childButton = static_cast<UIButton*>(child);
							if(childButton->GetBounds().IsPointInside(betterMousePosition))
							{
								m_hoveredButtonWidget = child;
								childButton->m_shouldHighlight = true;
								return;
							}
							else
							{
								childButton->m_shouldHighlight = false;
							}
						}					
					}
				}

				break;
			}
		}

		// This stuff will get skipped if we are hovering over a UI Widget;
		if(mousePosition.x <= clientMins.x + m_edgePanDistance)
		{
			m_keyboardScroll.y = -m_keyboardScrollSpeed;
		}

		if(mousePosition.x >= clientMaxs.x - m_edgePanDistance)
		{
			m_keyboardScroll.y = m_keyboardScrollSpeed;
		}

		if(mousePosition.y >= clientMins.y - m_edgePanDistance)
		{
			m_keyboardScroll.x = -m_keyboardScrollSpeed;
		}

		if(mousePosition.y <= clientMaxs.y + m_edgePanDistance)
		{
			m_keyboardScroll.x = m_keyboardScrollSpeed;
		}
	}

	m_leftMouseReleased = false;
	m_rightMouseReleased = false;
	m_middleMouseReleased = false;
}

// -----------------------------------------------------------------------
void PlayerController::UpdatePlaceHolderProp()
{
	if(m_placeholderProp)
	{
		m_placeholderProp->m_model->m_modelMatrix.SetT(m_hoveredTerrainSpot);
		Vec3 position = m_placeholderProp->m_model->m_modelMatrix.GetT();
		IntVec2 tile = IntVec2((int)floor(position.x), (int)floor(position.y));
		m_placeholderProp->m_model->m_modelMatrix.SetT(Vec3((float)tile.x, (float)tile.y, position.z));

		IntVec2 offset = m_placeHolderDefinition->m_offset;
		tile.x -= (int)offset.x;
		tile.y -= (int)offset.y;

		std::vector<bool> statuses;

		for(int y = 0; y < m_placeHolderDefinition->m_occupancySize.y; y++)
		{
			for(int x = 0; x < m_placeHolderDefinition->m_occupancySize.x; x++)
			{
				IntVec2 occupiedTile = IntVec2(0, 0);
				occupiedTile.x = tile.x + x;
				occupiedTile.y = tile.y + y;
				int tileIndex = GetIndexFromCoord(occupiedTile, m_theMap->m_tileDimensions);
				//int tileIndex = m_theMap->GetTileIndexFromTileCoord(occupiedTile);
				int status = m_theMap->GetOccupiedTileIndexStatus(tileIndex);
				if(status == -1)
				{
					statuses.push_back(true);
					m_theMap->SetOccupiedTileIndexStatus(tileIndex, 0);
				}
				else if(status == 1)
				{
					statuses.push_back(false);
					m_theMap->SetOccupiedTileIndexStatus(tileIndex, 2);
				}
				
			}
		}
		
		m_canBuildPlaceHolderProp = true;
		for(int f = 0; f < statuses.size(); f++)
		{
			if(statuses[f] == false)
			{
				m_canBuildPlaceHolderProp = false;
				break;
			}
		}
	}

	if(m_placedPlaceHolderProp)
	{
		Vec3 position = m_placedPlaceHolderProp->m_model->m_modelMatrix.GetT();
		IntVec2 tile = IntVec2((int)position.x, (int)position.y);
		//m_placedPlaceHolderProp->m_model->m_modelMatrix.SetT(Vec3(tile.x, tile.y, position.z));

		IntVec2 offset = m_placedPlaceHolderDefinition->m_offset;
		tile.x -= (int)offset.x;
		tile.y -= (int)offset.y;

		for(int y = 0; y < m_placedPlaceHolderDefinition->m_occupancySize.y; y++)
		{
			for(int x = 0; x < m_placedPlaceHolderDefinition->m_occupancySize.x; x++)
			{
				IntVec2 occupiedTile = IntVec2(0, 0);
				occupiedTile.x = tile.x + x;
				occupiedTile.y = tile.y + y;
				int tileIndex = GetIndexFromCoord(occupiedTile, m_theMap->m_tileDimensions);
				//int tileIndex = m_theMap->GetTileIndexFromTileCoord(occupiedTile);
				int status = m_theMap->GetOccupiedTileIndexStatus(tileIndex);
				if(status == -1)
				{
					m_theMap->SetOccupiedTileIndexStatus(tileIndex, 0);
				}
				else if(status == 1)
				{
					m_theMap->SetOccupiedTileIndexStatus(tileIndex, 1);
				}
			}
		}
	}

	
}

// -----------------------------------------------------------------------
void PlayerController::RenderPlaceHolderProp()
{
	if(m_placeholderProp)
	{
		g_theRenderer->BindModelMatrix( m_placeholderProp->m_model->m_modelMatrix );
		g_theRenderer->BindMaterial( m_placeholderProp->m_model->m_material );
		g_theRenderer->BindShader("Data/Shaders/PlaceHolderShader.shader");
		g_theRenderer->DrawMesh(m_placeholderProp->m_model->m_mesh);
	}

	if(m_placedPlaceHolderProp)
	{
		
		
		g_theRenderer->BindModelMatrix( m_placedPlaceHolderProp->m_model->m_modelMatrix );
		g_theRenderer->BindMaterial( m_placedPlaceHolderProp->m_model->m_material );
		g_theRenderer->BindShader("Data/Shaders/PlaceHolderShader.shader");
		g_theRenderer->DrawMesh(m_placedPlaceHolderProp->m_model->m_mesh); 

		
	}	
}

// -----------------------------------------------------------------------
Vec2 PlayerController::GetFrameScroll()
{
	return m_keyboardScroll;
}

float PlayerController::GetFrameZoomDistance()
{
	// Check for movement and apply movement (if any) to our target distance;
	UpdateTargetDistanceOfCamera();

	return Lerp(previousCameraDistance, targetCameraDistance, percentTransition);
}

void PlayerController::UpdateTargetDistanceOfCamera()
{
	// Get any wheel movement;
	float wheelDirection = (float)GetMouseWheelDirection() * 0.5f;

	// If any movement, set our previous to the current;
	if(wheelDirection != 0.0f)
	{
		previousCameraDistance = m_theGameCamera->m_distance;
		percentTransition = 0.0f;
	}

	// Change our target based on movement (if any);
	targetCameraDistance -= wheelDirection;
	targetCameraDistance = Clamp(targetCameraDistance, m_theGameCamera->m_minDistance, m_theGameCamera->m_maxDistance);
}

void PlayerController::SetGameState( GameState gameState )
{
	m_gameState = gameState;
}

GameState PlayerController::GetGameState()
{
	return m_gameState;
}

// -----------------------------------------------------------------------
// Map
// -----------------------------------------------------------------------
Map* PlayerController::GetGameMap()
{
	PROFILE_FUNCTION();

	return m_theGame->GetGameMap();
}

void PlayerController::RaycastToMapOrEntity(Ray3* ray, float* entityTime, float* mapTime)
{
	// See if Raycast from the mouse hits an Entity or the Map first;
	Entity* entity = m_theMap->RaycastEntity(entityTime, ray);
	if(m_theMap->RaycastTerrain(mapTime, ray))
	{
		if(*entityTime < *mapTime && entity)
		{
			m_hoveredEntities.push_back(entity->GetGameHandle());
		}

		if(*mapTime < INFINITY)
		{
			m_hoveredTerrainSpot = ray->PointAtTime(*mapTime);
		}
	}			
}

// -----------------------------------------------------------------------
float PlayerController::GetFramePan()
{
	return m_cameraPan;
}

// -----------------------------------------------------------------------
bool PlayerController::IsRotating()
{
	return m_cameraPan > 0.0f ? true : false;
}

bool PlayerController::IsPaused()
{
	return m_isPaused;
}

// -----------------------------------------------------------------------
// Selection Box
// -----------------------------------------------------------------------
Vec2 PlayerController::GetMouseSelectionBoxStart()
{
	return m_mouseSelectionBoxStart;
}

// -----------------------------------------------------------------------
Vec2 PlayerController::GetMouseSelectionBoxEnd()
{
	return m_mouseSelectionBoxEnd;
}

// -----------------------------------------------------------------------
void PlayerController::UpdateSelectionBox(Frustum& frustum)
{
	

	// Check if our Frustum is Hovering any Entities;
	for(Entity* entity: m_theMap->GetAllValidEntities())
	{
		if(entity)
		{
			if(frustum.ContainsPoint(entity->GetPosition()))
			{
				m_hoveredEntities.push_back(entity->GetGameHandle());
			}
		}
	}

	// Highlight the Hovered Entities;
	if(m_hoveredEntities.size() > 0)
	{
		for(GameHandle hoveredEntityGameHandle: m_hoveredEntities)
		{
			Entity* entity = m_theMap->FindEntity(hoveredEntityGameHandle);
			if(entity)
			{
				entity->SetHovered(true);
			}
		}
	}

	// Clicking and starting a Selection Box;
	if(IsLeftMouseClicked() && m_mouseSelectionBoxStart == Vec2::VNULL)
	{
		Vec2 position = Vec2((float)m_clientMousePosition2.x, (float)m_clientMousePosition2.y);
		m_mouseSelectionBoxStart = position;
	}

	// Continuing a Selection Box;
	if(IsLeftMouseClicked() && m_mouseSelectionBoxStart != Vec2::VNULL)
	{
		Vec2 position = Vec2((float)m_clientMousePosition2.x, (float)m_clientMousePosition2.y);
		m_mouseSelectionBoxEnd = position;
	}
}

// -----------------------------------------------------------------------
void PlayerController::SelectUnits(Frustum& frustum)
{
	// Letting go of the Selection Box;
	if(WasLeftMouseReleased() && m_mouseSelectionBoxStart != Vec2::VNULL)
	{
		if(m_hoveredEntities.size() != 0)
		{
			if(!m_shiftKeyDown)
			{
				m_selectedEntities.clear();
				m_theMap->UnSelectAllEntities();

				for(GameHandle hoveredEntityGameHandle: m_hoveredEntities)
				{
					Entity* entity = m_theMap->FindEntity(hoveredEntityGameHandle);
					if(entity->IsOnTeam(m_theGame->GetCurrentTeam()))
					{
						m_selectedEntities.push_back(entity->GetGameHandle());
						entity->SetSelected(true);
					}						
				}
			}
			else
			{
				for(Entity* entity: m_theMap->GetAllValidEntities())
				{
					if(frustum.ContainsPoint(entity->GetPosition()))
					{
						if(entity->IsOnTeam(m_theGame->GetCurrentTeam()))
						{
							m_selectedEntities.push_back(entity->GetGameHandle());
							entity->SetSelected(true);
						}				
					}

					if(m_hoveredEntities.size() == 1)
					{
						if(entity->IsOnTeam(m_theGame->GetCurrentTeam()))
						{
							m_selectedEntities.push_back(entity->GetGameHandle());
							entity->SetSelected(true);
						}

						break;
					}			
				}
			}

			// 			for(GameHandle selection: m_selectedEntities)
			// 			{
			// 				Entity* selectedEntity = m_theMap->FindEntity(selection);
			// 				selectedEntity->SetSelected(true);
			// 			}
		}

		m_mouseSelectionBoxStart = Vec2::VNULL;
		m_mouseSelectionBoxEnd = Vec2::VNULL;
	}
}

// -----------------------------------------------------------------------
// Mouse
// -----------------------------------------------------------------------
void PlayerController::ClickLeftMouse()
{
	m_leftMouseClicked = true;	
}

void PlayerController::ClickRightMouse()
{
	m_rightMouseClicked = true;
}

void PlayerController::ClickMiddleMouse()
{
	m_middleMouseClicked = true;
}

void PlayerController::ReleaseLeftMouse()
{
	m_leftMouseClicked = false;
	m_leftMouseReleased = true;
	if(m_hoveredButtonWidget)
	{
		LeftClickUIWidget( m_hoveredButtonWidget );
	}
}

void PlayerController::ReleaseRightMouse()
{
	m_rightMouseClicked = false;
	m_rightMouseReleased = true;
}

void PlayerController::ReleaseMiddleMouse()
{
	m_middleMouseClicked = false;
	m_middleMouseReleased = true;
}

void PlayerController::LeftClickUIWidget( UIWidget* uiWidget )
{
	UIButton* buttonPressed = static_cast<UIButton*>(uiWidget);
	switch(m_gameState)
	{
		case GAMESTATE_TITLE:
		{
			if(buttonPressed->m_action == "none")
			{
				return;
			}
			else if(buttonPressed->m_action == "lobbyload")
			{
				buttonPressed->ToggleHighlight();
				g_theLobbyConsole->m_isOpen = true;
				SetGameState(GAMESTATE_LOBBY);
			}
			else if(buttonPressed->m_action == "editmapload")
			{
				buttonPressed->ToggleHighlight();
				SetGameState(GAMESTATE_EDITMAPLOAD);
			}
			else if(buttonPressed->m_action == "quit")
			{
				buttonPressed->ToggleHighlight();
				g_theApp->HandleCloseApplication();
			}

			break;
		}

		case GAMESTATE_LOBBY:
		{
			if(buttonPressed->m_action == "playmapload")
			{
				buttonPressed->ToggleHighlight();
				g_theLobbyConsole->m_isOpen = false;
				SetGameState(GAMESTATE_PLAYMAPLOAD);
			}
			else if(buttonPressed->m_action == "backtotitle")
			{
				buttonPressed->ToggleHighlight();
				SetGameState(GAMESTATE_TITLE);
			}
		}

		case GAMESTATE_PLAY:
		{
			if(buttonPressed->m_action == "none")
			{
				return;
			}
			else if(buttonPressed->m_action == "quit")
			{
				buttonPressed->ToggleHighlight();
				g_theApp->HandleCloseApplication();
			}
			else if(buttonPressed->m_action == "quitandsavereplay")
			{
				buttonPressed->ToggleHighlight();
				g_theReplayController->SaveReplayFile("test1");
				g_theApp->HandleCloseApplication();
			}
			else if(buttonPressed->m_action == "mainmenu")
			{
				buttonPressed->ToggleHighlight();
				m_isPaused = false;
				SetGameState(GAMESTATE_TITLE);
			}

			else if(buttonPressed->m_action == "resume")
			{
				buttonPressed->ToggleHighlight();
				m_isPaused = false;
			}

			else if(buttonPressed->m_action == "move" && !m_theGame->m_isPaused)
			{
				buttonPressed->ToggleHighlight();


			}

			else if(buttonPressed->m_action == "attack" && !m_theGame->m_isPaused)
			{
				buttonPressed->ToggleHighlight();


			}
		}

		case GAMESTATE_EDIT:
		{
			if(buttonPressed->m_action == "none")
			{
				return;
			}
			else if(buttonPressed->m_action == "dirttexture")
			{
				buttonPressed->SelectButtonFromRadioGroup();
			}
			else if(buttonPressed->m_action == "dirtstonetexture")
			{
				buttonPressed->SelectButtonFromRadioGroup();
			}
			else if(buttonPressed->m_action == "grasstexture")
			{
				buttonPressed->SelectButtonFromRadioGroup();
			}
			else if(buttonPressed->m_action == "grassrubbletexture")
			{
				buttonPressed->SelectButtonFromRadioGroup();
			}
			else if(buttonPressed->m_action == "stonetexture")
			{
				buttonPressed->SelectButtonFromRadioGroup();
			}
			else if(buttonPressed->m_action == "quit")
			{
				buttonPressed->ToggleHighlight();
				g_theApp->HandleCloseApplication();
			}
			else if(buttonPressed->m_action == "resume")
			{
				buttonPressed->ToggleHighlight();
				m_isPaused = false;
			}
			else if(buttonPressed->m_action == "mainmenu")
			{
				buttonPressed->ToggleHighlight();
				m_isPaused = false;
				SetGameState(GAMESTATE_TITLE);
			}

			break;
		}
	}
}

void PlayerController::ShowOrHideMouseCheck()
{
	if(g_thePlayerController->IsMiddleMouseClicked() && m_mouseMode == MOUSEVISIBILITY_SHOW)
	{
		g_theWindowContext->SetMouseMode(MOUSE_MODE_RELATIVE);
		g_theWindowContext->HideMouse();
		m_mouseMode = MOUSEVISIBILITY_HIDE;	
	}
	else if(!g_thePlayerController->IsMiddleMouseClicked() && m_mouseMode == MOUSEVISIBILITY_HIDE)
	{
		g_theWindowContext->SetMouseMode(MOUSE_MODE_ABSOLUTE);
		g_theWindowContext->ShowMouse();
		m_mouseMode = MOUSEVISIBILITY_SHOW;
	}
}

// -----------------------------------------------------------------------
// Mouse States
// -----------------------------------------------------------------------
bool PlayerController::IsLeftMouseClicked()
{
	return m_leftMouseClicked;
}

bool PlayerController::WasLeftMouseReleased()
{
	return m_leftMouseReleased;
}

bool PlayerController::IsRightMouseClicked()
{
	return m_rightMouseClicked;
}

bool PlayerController::WasRightMouseReleased()
{
	return m_rightMouseReleased;
}

bool PlayerController::IsMiddleMouseClicked()
{
	return m_middleMouseClicked;
}

// -----------------------------------------------------------------------
// Mouse Wheel
// -----------------------------------------------------------------------
void PlayerController::StoreWheelMovement( float wheelDelta )
{
	m_wheelDelta = wheelDelta;	
}

int PlayerController::GetMouseWheelDirection()
{
	int direction = 0;

	if(m_wheelDelta != 0.0f)
	{
		direction = m_wheelDelta > 0.0f ? 1 : -1;
	}

	m_wheelDelta = 0.0f;
	return direction;
}

// -----------------------------------------------------------------------
// Key Presses
// -----------------------------------------------------------------------
void PlayerController::HandleKeyPressed( unsigned char asKey )
{
	if(asKey == Key::UPARROW)
	{
		m_arrowKeys[ARROWKEYS_UPARROW] = true;
	}

	if(asKey == Key::DOWNARROW)
	{
		m_arrowKeys[ARROWKEYS_DOWNARROW] = true;
	}

	if(asKey == Key::RIGHTARROW)
	{
		m_arrowKeys[ARROWKEYS_RIGHTARROW] = true;
	}

	if(asKey == Key::LEFTARROW)
	{
		m_arrowKeys[ARROWKEYS_LEFTARROW] = true;
	}

	if(asKey == Key::LEFTBRACKET)
	{
		m_bracketKeys[BRACKETKEYS_LEFT] = true;
	}

	if(asKey == Key::RIGHTBRACKET)
	{
		m_bracketKeys[BRACKETKEYS_RIGHT] = true;
	}

	if(asKey == Key::F9)
	{
		m_isPaused = true;
	}	

	if(asKey == Key::SHIFT)
	{
		m_shiftKeyDown = true;
	}

	// Suicide Command;
	if(asKey == Key::PERIOD)
	{
		for(GameHandle handle: m_selectedEntities)
		{
			Entity* entity = m_theMap->FindEntity(handle);
			if(entity)
			{
				entity->m_health = 0.0f;
			}
		}
	}

	if(asKey == Key::F1)
	{
		m_theGame->SetCurrentTeam(0);
		m_theMap->UnhoverAllEntities();
		m_hoveredEntities.clear();
		m_theMap->UnSelectAllEntities();
	}

	if(asKey == Key::F2)
	{
		m_theGame->SetCurrentTeam(1);
		m_theMap->UnhoverAllEntities();
		m_hoveredEntities.clear();
		m_theMap->UnSelectAllEntities();
	}

	// Attack Command;
	if(asKey == 'A')
	{
		if(m_selectedEntities.size() > 0)
		{
			m_commandState = ATTACKSTATE;

			for(GameHandle handle: m_selectedEntities)
			{
				m_stateEntities.push_back(handle);
			}
		}	
	}

	// Follow Command;
	if(asKey == 'F')
	{
		if(m_selectedEntities.size() > 0)
		{
			m_commandState = FOLLOWSTATE;

			for(GameHandle handle: m_selectedEntities)
			{
				m_stateEntities.push_back(handle);
			}
		}	
	}

	// Move Command;
	if(asKey == 'M')
	{
		if(m_selectedEntities.size() > 0)
		{
			m_commandState = MOVESTATE;

			for(GameHandle handle: m_selectedEntities)
			{
				m_stateEntities.push_back(handle);
			}
		}	
	}

	// Gather Command;
	if(asKey == 'X')
	{
		if(m_selectedEntities.size() > 0)
		{
			m_commandState = GATHERSTATE;

			for(GameHandle handle: m_selectedEntities)
			{
				m_stateEntities.push_back(handle);
			}
		}	
	}

	// Building a Town Center
	if(asKey == 'C')
	{
		if(m_selectedEntities.size() > 0)
		{
			m_commandState = BUILDSTATE;
			GameHandle unit = m_selectedEntities.front();
			m_buildingEntity = m_theMap->FindEntity(unit);
			std::map< std::string, RTSTaskInfo* >::iterator found = m_buildingEntity->m_entityDefinition->m_availableTasks.find("buildtowncenter");
			if(found != m_buildingEntity->m_entityDefinition->m_availableTasks.end())
			{
				std::map< std::string, EntityDefinition* >::iterator building = EntityDefinition::s_entityDefinitions.find("TownHall");
				if(building != EntityDefinition::s_entityDefinitions.end())
				{
					EntityDefinition* entityDefintion = building->second;
					m_placeholderProp = entityDefintion->m_prop;
					m_placeHolderDefinition = entityDefintion;
					m_canBuildPlaceHolderProp = true;
				}
			}
		}
	}

	// Building a Hut
	if(asKey == 'H')
	{
		if(m_selectedEntities.size() > 0)
		{
			m_commandState = BUILDSTATE;
			GameHandle unit = m_selectedEntities.front();
			m_buildingEntity = m_theMap->FindEntity(unit);
			if(m_buildingEntity->m_entityDefinition->m_entityType == "Peon")
			{
				std::map< std::string, RTSTaskInfo* >::iterator found = m_buildingEntity->m_entityDefinition->m_availableTasks.find("buildhut");
				if(found != m_buildingEntity->m_entityDefinition->m_availableTasks.end())
				{
					std::map< std::string, EntityDefinition* >::iterator building = EntityDefinition::s_entityDefinitions.find("Hut");
					if(building != EntityDefinition::s_entityDefinitions.end())
					{
						EntityDefinition* entityDefintion = building->second;
						m_placeholderProp = entityDefintion->m_prop;
						m_placeHolderDefinition = entityDefintion;
						m_canBuildPlaceHolderProp = true;
					}
				}
			}
			else if(m_buildingEntity->m_entityDefinition->m_entityType == "Goblin")
			{
				std::map< std::string, RTSTaskInfo* >::iterator found = m_buildingEntity->m_entityDefinition->m_availableTasks.find("buildgoblinhut");
				if(found != m_buildingEntity->m_entityDefinition->m_availableTasks.end())
				{
					std::map< std::string, EntityDefinition* >::iterator building = EntityDefinition::s_entityDefinitions.find("GoblinHut");
					if(building != EntityDefinition::s_entityDefinitions.end())
					{
						EntityDefinition* entityDefintion = building->second;
						m_placeholderProp = entityDefintion->m_prop;
						m_placeHolderDefinition = entityDefintion;
						m_canBuildPlaceHolderProp = true;
					}
				}
			}
			
			
		}
	}

	// Building a Tower;
	if(asKey == 'T')
	{
		if(m_selectedEntities.size() > 0)
		{
			m_commandState = BUILDSTATE;
			GameHandle unit = m_selectedEntities.front();
			m_buildingEntity = m_theMap->FindEntity(unit);
			if(m_buildingEntity->m_entityDefinition->m_entityType == "Peon")
			{
				std::map< std::string, RTSTaskInfo* >::iterator found = m_buildingEntity->m_entityDefinition->m_availableTasks.find("buildtower");
				if(found != m_buildingEntity->m_entityDefinition->m_availableTasks.end())
				{
					std::map< std::string, EntityDefinition* >::iterator building = EntityDefinition::s_entityDefinitions.find("Tower");
					if(building != EntityDefinition::s_entityDefinitions.end())
					{
						EntityDefinition* entityDefintion = building->second;
						m_placeholderProp = entityDefintion->m_prop;
						m_placeHolderDefinition = entityDefintion;
						m_canBuildPlaceHolderProp = true;
					}
				}
			}
			else if(m_buildingEntity->m_entityDefinition->m_entityType == "Goblin")
			{
				std::map< std::string, RTSTaskInfo* >::iterator found = m_buildingEntity->m_entityDefinition->m_availableTasks.find("buildgoblintower");
				if(found != m_buildingEntity->m_entityDefinition->m_availableTasks.end())
				{
					std::map< std::string, EntityDefinition* >::iterator building = EntityDefinition::s_entityDefinitions.find("GoblinTower");
					if(building != EntityDefinition::s_entityDefinitions.end())
					{
						EntityDefinition* entityDefintion = building->second;
						m_placeholderProp = entityDefintion->m_prop;
						m_placeHolderDefinition = entityDefintion;
						m_canBuildPlaceHolderProp = true;
					}
				}
			}
			
		}
	}

	// Training a Peon;
	if(asKey == 'P')
	{
		if(m_selectedEntities.size() > 0)
		{
			GameHandle unit = m_selectedEntities.front();
			Entity* building = m_theMap->FindEntity(unit);
			std::map< std::string, RTSTaskInfo* >::iterator found = building->m_entityDefinition->m_availableTasks.find("trainpeon");
			if(found != building->m_entityDefinition->m_availableTasks.end())
			{
				if(building->m_buildingFinishedConstruction)
				{
					std::map< std::string, EntityDefinition* >::iterator peon = EntityDefinition::s_entityDefinitions.find("Peon");
					if(peon != EntityDefinition::s_entityDefinitions.end())
					{
						//EntityDefinition* entityDefintion = peon->second;
						TrainCommand* trainCommand = new TrainCommand();
						trainCommand->m_commandFrameIssued = g_theApp->m_commandFrame.GetCommandFrame();
						//trainCommand->m_entityDefinitionToTrain = entityDefintion;
						trainCommand->m_training = "trainpeon";
						trainCommand->m_entityToTrain = "Peon";
						trainCommand->m_commandType = 13;
						trainCommand->m_team = m_theGame->GetCurrentTeam();
						trainCommand->m_theGame = m_theGame;
						trainCommand->m_theMap = m_theGame->GetGameMap();
						trainCommand->m_unit = unit;
						m_theGame->EnQueueCommand(trainCommand);
						g_theReplayController->RecordCommand(trainCommand);
					}
				}				
			}
		}
	}

	// Training a Warrior
	if(asKey == 'W')
	{
		if(m_selectedEntities.size() > 0)
		{
			GameHandle unit = m_selectedEntities.front();
			Entity* building = m_theMap->FindEntity(unit);
			std::map< std::string, RTSTaskInfo* >::iterator found = building->m_entityDefinition->m_availableTasks.find("trainwarrior");
			if(found != building->m_entityDefinition->m_availableTasks.end())
			{
				if(building->m_buildingFinishedConstruction)
				{
					std::map< std::string, EntityDefinition* >::iterator warrior = EntityDefinition::s_entityDefinitions.find("Warrior");
					if(warrior != EntityDefinition::s_entityDefinitions.end())
					{
						//EntityDefinition* entityDefintion = warrior->second;
						TrainCommand* trainCommand = new TrainCommand();
						trainCommand->m_commandFrameIssued = g_theApp->m_commandFrame.GetCommandFrame();
						//trainCommand->m_entityDefinitionToTrain = entityDefintion;
						trainCommand->m_commandType = 14;
						trainCommand->m_training = "trainwarrior";
						trainCommand->m_entityToTrain = "Warrior";
						trainCommand->m_team = m_theGame->GetCurrentTeam();
						trainCommand->m_theGame = m_theGame;
						trainCommand->m_theMap = m_theGame->GetGameMap();
						trainCommand->m_unit = unit;
						m_theGame->EnQueueCommand(trainCommand);
						g_theReplayController->RecordCommand(trainCommand);
					}
				}
			}
		}
	}

	// Training a Goblin
	if(asKey == 'G')
	{
		if(m_selectedEntities.size() > 0)
		{
			GameHandle unit = m_selectedEntities.front();
			Entity* building = m_theMap->FindEntity(unit);
			std::map< std::string, RTSTaskInfo* >::iterator found = building->m_entityDefinition->m_availableTasks.find("traingoblin");
			if(found != building->m_entityDefinition->m_availableTasks.end())
			{
				if(building->m_buildingFinishedConstruction)
				{
					std::map< std::string, EntityDefinition* >::iterator goblin = EntityDefinition::s_entityDefinitions.find("Goblin");
					if(goblin != EntityDefinition::s_entityDefinitions.end())
					{
						//EntityDefinition* entityDefintion = warrior->second;
						TrainCommand* trainCommand = new TrainCommand();
						trainCommand->m_commandFrameIssued = g_theApp->m_commandFrame.GetCommandFrame();
						//trainCommand->m_entityDefinitionToTrain = entityDefintion;
						trainCommand->m_commandType = 16;
						trainCommand->m_training = "traingoblin";
						trainCommand->m_entityToTrain = "Goblin";
						trainCommand->m_team = m_theGame->GetCurrentTeam();
						trainCommand->m_theGame = m_theGame;
						trainCommand->m_theMap = m_theGame->GetGameMap();
						trainCommand->m_unit = unit;
						m_theGame->EnQueueCommand(trainCommand);
						g_theReplayController->RecordCommand(trainCommand);
					}
				}
			}
		}
	}

	// Cheat Spawn a Peon
	if(asKey == 'E')
	{
		CreateCommand* createCommand = new CreateCommand();
		createCommand->m_commandFrameIssued = g_theApp->m_commandFrame.GetCommandFrame();
		createCommand->m_theGame = m_theGame;
		createCommand->m_team = m_theGame->GetCurrentTeam();
		createCommand->m_commandType = 0;
		createCommand->m_theMap = m_theGame->GetGameMap();
		createCommand->m_createPosition = m_hoveredTerrainSpot;
		m_theGame->EnQueueCommand(createCommand);
		g_theReplayController->RecordCommand(createCommand);
	}

	// Cheat Spawn a Warrior;
	if(asKey == 'I')
	{
		CreateCommand* createCommand = new CreateCommand();
		createCommand->m_commandFrameIssued = g_theApp->m_commandFrame.GetCommandFrame();
		createCommand->m_theGame = m_theGame;
		createCommand->m_team = m_theGame->GetCurrentTeam();
		createCommand->m_commandType = 1;
		createCommand->m_theMap = m_theGame->GetGameMap();
		createCommand->m_createPosition = m_hoveredTerrainSpot;
		m_theGame->EnQueueCommand(createCommand);
		g_theReplayController->RecordCommand(createCommand);
	}

	// Cheat Spawn a Goblin;
	if(asKey == 'O')
	{
		CreateCommand* createCommand = new CreateCommand();
		createCommand->m_commandFrameIssued = g_theApp->m_commandFrame.GetCommandFrame();
		createCommand->m_theGame = m_theGame;
		createCommand->m_team = m_theGame->GetCurrentTeam();
		createCommand->m_commandType = 17;
		createCommand->m_theMap = m_theGame->GetGameMap();
		createCommand->m_createPosition = m_hoveredTerrainSpot;
		m_theGame->EnQueueCommand(createCommand);
		g_theReplayController->RecordCommand(createCommand);
	}

	// Stop. Clear all Commands;
	if(asKey == 'S')
	{
		if(m_selectedEntities.size() > 0)
		{
			for(GameHandle gameHandle: m_selectedEntities)
			{
				Entity* entity = m_theMap->FindEntity(gameHandle);
				delete entity->m_pathCreation;
				entity->m_pathCreation = nullptr;
				entity->ClearTasks();
				entity->SetAnimationState(ANIMATION_IDLE);
			}
		}
	}

	if(asKey == 'L')
	{
		Vec3 position = m_hoveredTerrainSpot;
		IntVec2 tile = IntVec2((int)floor(position.x), (int)floor(position.y));

		std::map< std::string, EntityDefinition* >::iterator tree = EntityDefinition::s_entityDefinitions.find("pinewhole");
		EntityDefinition* treeDefinition = tree->second;

		IntVec2 offset = treeDefinition->m_offset;
		tile.x -= (int)offset.x;
		tile.y -= (int)offset.y;

		int tileIndex = GetIndexFromCoord(tile, m_theMap->m_tileDimensions);
		int status = m_theMap->GetOccupiedTileIndexStatus(tileIndex);

		if(status == -1)
		{
			CreateCommand* createCommand = new CreateCommand();
			createCommand->m_commandFrameIssued = g_theApp->m_commandFrame.GetCommandFrame();
			createCommand->m_theGame = m_theGame;
			createCommand->m_team = 99;
			createCommand->m_commandType = 7;
			createCommand->m_theMap = m_theGame->GetGameMap();
			createCommand->m_createPosition = m_hoveredTerrainSpot;
			m_theGame->EnQueueCommand(createCommand);
			g_theReplayController->RecordCommand(createCommand);
		}
	}

	if(asKey == 'M')
	{
		Vec3 position = m_hoveredTerrainSpot;
		IntVec2 tile = IntVec2((int)floor(position.x), (int)floor(position.y));

		std::map< std::string, EntityDefinition* >::iterator mineral = EntityDefinition::s_entityDefinitions.find("fullmineral");
		EntityDefinition* mineralDefinition = mineral->second;

		IntVec2 offset = mineralDefinition->m_offset;
		tile.x -= (int)offset.x;
		tile.y -= (int)offset.y;

		int tileIndex = GetIndexFromCoord(tile, m_theMap->m_tileDimensions);
		//int tileIndex = m_theMap->GetTileIndexFromTileCoord(tile);
		int status = m_theMap->GetOccupiedTileIndexStatus(tileIndex);

		if(status == -1)
		{
			CreateCommand* createCommand = new CreateCommand();
			createCommand->m_commandFrameIssued = g_theApp->m_commandFrame.GetCommandFrame();
			createCommand->m_theGame = m_theGame;
			createCommand->m_team = 99;
			createCommand->m_commandType = 22;
			createCommand->m_theMap = m_theGame->GetGameMap();
			createCommand->m_createPosition = m_hoveredTerrainSpot;
			m_theGame->EnQueueCommand(createCommand);
			g_theReplayController->RecordCommand(createCommand);
		}
	}
}

// -----------------------------------------------------------------------
// Key Release
// -----------------------------------------------------------------------
void PlayerController::HandleKeyReleased( unsigned char asKey )
{
	if(asKey == Key::UPARROW)
	{
		m_arrowKeys[ARROWKEYS_UPARROW] = false;
	}

	if(asKey == Key::DOWNARROW)
	{
		m_arrowKeys[ARROWKEYS_DOWNARROW] = false;
	}

	if(asKey == Key::RIGHTARROW)
	{
		m_arrowKeys[ARROWKEYS_RIGHTARROW] = false;
	}

	if(asKey == Key::LEFTARROW)
	{
		m_arrowKeys[ARROWKEYS_LEFTARROW] = false;
	}

	if(asKey == Key::LEFTBRACKET)
	{
		m_bracketKeys[BRACKETKEYS_LEFT] = false;
	}

	if(asKey == Key::RIGHTBRACKET)
	{
		m_bracketKeys[BRACKETKEYS_RIGHT] = false;
	}

	if(asKey == Key::SHIFT)
	{
		m_shiftKeyDown = false;
	}
}

// -----------------------------------------------------------------------
// Camera
// -----------------------------------------------------------------------
void PlayerController::SetGameCamera( OrbitCamera* gameCamera )
{
	m_theGameCamera = gameCamera;
}
