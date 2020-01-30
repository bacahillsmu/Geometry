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

	int SlotForPosition(const Vec2& position_);
	Vec2 CenterPositionOfSlot(int slot_);

public:

	static constexpr float WIDTH = 200.0f;
	static constexpr float HEIGHT = 100.0f;

public:

	AABB2 m_field1;
	AABB2 m_field2;
	Vec2 m_unitSlotDimensions;
	int m_unitSlotsPerField = 0;
	std::vector<AABB2> m_field1Slots;
	std::vector<AABB2> m_field2Slots;



};