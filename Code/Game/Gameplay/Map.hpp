#pragma once

#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/Vec2.hpp"


class Map
{

public:

	Map();
	~Map();

	// Flow;
	void Startup();
	void Update(float deltaSeconds_);
	void Render();

public:

	static constexpr float WIDTH = 200.0f;
	static constexpr float HEIGHT = 100.0f;

public:

	Vec2 m_worldMousePosition;



};