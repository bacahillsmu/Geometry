#pragma once
#include "Game/Gameplay/Game.hpp"


// World
constexpr float WORLD_WIDTH = 200.f;
constexpr float WORLD_HEIGHT = 100.f;
constexpr float ASPECT = WORLD_WIDTH / WORLD_HEIGHT;
constexpr float ASPECTED_WORLD_WIDTH = ASPECT * WORLD_WIDTH;
constexpr float ASPECTED_WORLD_HEIGHT = ASPECT * WORLD_HEIGHT;
constexpr float WORLD_CENTER_X = WORLD_WIDTH / 2.f;
constexpr float WORLD_CENTER_Y = WORLD_HEIGHT / 2.f;

// Camera
constexpr float CAMERA_SHAKE_REDUCTION_PER_SECOND = 1.0f;
constexpr float CAMERA_SHAKE_MAX = 2.0f;

// Time Constants
// constexpr float MIN_FPS = 10.0f;
// constexpr float MAX_DS = 1.0f / MIN_FPS;
