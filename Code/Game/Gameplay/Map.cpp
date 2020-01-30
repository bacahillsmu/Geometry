#include "Game/Gameplay/Map.hpp"

// Commons ----------------------------------------------------------------------------------------
#include "Engine/Core/EngineCommon.hpp"
#include "Game/Framework/GameCommon.hpp"

// Engine Includes --------------------------------------------------------------------------------
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Input/InputSystem.hpp"

// Game Includes ----------------------------------------------------------------------------------
#include "Game/Framework/App.hpp"
#include "Game/Input/GameInput.hpp"





// ------------------------------------------------------------------------------------------------
Map::Map()
{

}

// ------------------------------------------------------------------------------------------------
Map::~Map()
{

}

// ------------------------------------------------------------------------------------------------
// Flow;
// ------------------------------------------------------------------------------------------------
void Map::Startup()
{
	// Calculate the size and placement of the fields;
	Vec2 field1Center = Vec2(Map::WIDTH * 0.5f, Map::HEIGHT * 0.2f);
	Vec2 field1Min = Vec2(field1Center.x - Map::WIDTH * 0.3f, 0.0f);
	Vec2 field1Max = Vec2(field1Center.x + Map::WIDTH * 0.3f, 0.0f + Map::HEIGHT * 0.4f);
	m_field1 = AABB2::MakeFromMinsMaxs(field1Min, field1Max);

	Vec2 field2Center = Vec2(Map::WIDTH * 0.5f, Map::HEIGHT * 0.8f);
	Vec2 field2Min = Vec2(field2Center.x - Map::WIDTH * 0.3f, 0.0f + Map::HEIGHT * 0.6f);
	Vec2 field2Max = Vec2(field2Center.x + Map::WIDTH * 0.3f, Map::HEIGHT);
	m_field2 = AABB2::MakeFromMinsMaxs(field2Min, field2Max);

	// Calculate the size of a unit slot; The placement will be determined by the slot;
	float fieldWidth = m_field1.maxs.x - m_field1.mins.x;
	float fieldHeight = m_field1.maxs.y - m_field1.mins.y;
	m_unitSlotDimensions = Vec2(fieldWidth * 0.25f, fieldHeight * 0.5f);
	m_unitSlotsPerField = 8;
	float anEigth = 1.0f / (float)m_unitSlotsPerField;

	// Calculate the placement of the AABB2s of each unit slot for both fields;
	// Making my field;
	Vec2 firstRowSlotCenter = Vec2(m_field1.mins.x + fieldWidth * anEigth, m_field1.mins.y + fieldHeight * 0.25f);
	Vec2 secondRowSlotCenter = Vec2(m_field1.mins.x + fieldWidth * anEigth, firstRowSlotCenter.y + m_unitSlotDimensions.y);
	for(int i = 0; i < m_unitSlotsPerField / 2; ++i)
	{
		Vec2 currentCenter1 = Vec2(firstRowSlotCenter.x + (i * m_unitSlotDimensions.x), firstRowSlotCenter.y);
		Vec2 currentCenter2 = Vec2(secondRowSlotCenter.x + (i * m_unitSlotDimensions.x), secondRowSlotCenter.y);
		AABB2 slot1 = AABB2(currentCenter1, Vec2(m_unitSlotDimensions.x * 0.5f, m_unitSlotDimensions.y * 0.5f));
		AABB2 slot2 = AABB2(currentCenter2, Vec2(m_unitSlotDimensions.x * 0.5f, m_unitSlotDimensions.y * 0.5f));

		m_field1Slots.push_back(slot1);
		m_field1Slots.push_back(slot2);
	}

	// Making my opponents field;
	Vec2 firstRowSlotCenter2 = Vec2(m_field2.mins.x + fieldWidth * anEigth, m_field2.mins.y + fieldHeight * 0.25f);
	Vec2 secondRowSlotCenter2 = Vec2(m_field2.mins.x + fieldWidth * anEigth, firstRowSlotCenter2.y + m_unitSlotDimensions.y);
	for (int i = 0; i < m_unitSlotsPerField / 2; ++i)
	{
		float slotNumber = (float)i + 1;
		Vec2 currentCenter1 = Vec2(firstRowSlotCenter2.x + (i * m_unitSlotDimensions.x), firstRowSlotCenter2.y);
		Vec2 currentCenter2 = Vec2(secondRowSlotCenter2.x + (i * m_unitSlotDimensions.x), secondRowSlotCenter2.y);
		AABB2 slot1 = AABB2(currentCenter1, Vec2(m_unitSlotDimensions.x * 0.5f, m_unitSlotDimensions.y * 0.5f));
		AABB2 slot2 = AABB2(currentCenter2, Vec2(m_unitSlotDimensions.x * 0.5f, m_unitSlotDimensions.y * 0.5f));

		m_field2Slots.push_back(slot1);
		m_field2Slots.push_back(slot2);
	}
}

// ------------------------------------------------------------------------------------------------
void Map::Update(float deltaSeconds_)
{

}

// ------------------------------------------------------------------
void Map::Render()
{
	std::vector<Vertex_PCU> boxVerts;
	AddVertsForAABB2D(boxVerts, m_field1, Rgba::GREEN);
	AddVertsForAABB2D(boxVerts, m_field2, Rgba::RED);

	AddVertsForAABB2D(boxVerts, m_field1Slots[0], Rgba::BLUE);
	AddVertsForAABB2D(boxVerts, m_field2Slots[0], Rgba::BLUE);
	AddVertsForAABB2D(boxVerts, m_field1Slots[1], Rgba::BROWN);
	AddVertsForAABB2D(boxVerts, m_field2Slots[1], Rgba::BROWN);
	AddVertsForAABB2D(boxVerts, m_field1Slots[2], Rgba::GRAY);
	AddVertsForAABB2D(boxVerts, m_field2Slots[2], Rgba::GRAY);
	AddVertsForAABB2D(boxVerts, m_field1Slots[3], Rgba::MAGENTA);
	AddVertsForAABB2D(boxVerts, m_field2Slots[3], Rgba::MAGENTA);
	AddVertsForAABB2D(boxVerts, m_field1Slots[4], Rgba::YELLOW);
	AddVertsForAABB2D(boxVerts, m_field2Slots[4], Rgba::YELLOW);
	AddVertsForAABB2D(boxVerts, m_field1Slots[5], Rgba::TEAL);
	AddVertsForAABB2D(boxVerts, m_field2Slots[5], Rgba::TEAL);
	AddVertsForAABB2D(boxVerts, m_field1Slots[6], Rgba::BLACK);
	AddVertsForAABB2D(boxVerts, m_field2Slots[6], Rgba::BLACK);
	AddVertsForAABB2D(boxVerts, m_field1Slots[7], Rgba::WHITE);
	AddVertsForAABB2D(boxVerts, m_field2Slots[7], Rgba::WHITE);

	g_theRenderer->BindShader("Data/Shaders/default_unlit_devconsole.shader");
	g_theRenderer->BindTextureViewWithSampler(0, nullptr);
	//g_theRenderer->BindTextureViewWithSampler(0, g_theRenderer->CreateOrGetTextureViewFromFile("Data/Sprites/Jobs/Knight/Knight1M-S.png"));
	g_theRenderer->DrawVertexArray((int)boxVerts.size(), &boxVerts[0]);
}

// ------------------------------------------------------------------
int Map::SlotForPosition(const Vec2& position_)
{
	//-------------------//
	// | 1 | 3 | 5 | 7 | //
	// | 0 | 2 | 4 | 6 | //
	//-------------------//

	// We need to think about what we are clicking on, my field vs. their field;
	// I don't think we will ever actually need to click their field;
	for(int slotIndex = 0; slotIndex < m_field1Slots.size(); ++slotIndex)
	{
		if(m_field1Slots[slotIndex].IsPointInside(position_))
		{
			return slotIndex;
		}
	}

	return -1;
}

// ------------------------------------------------------------------
Vec2 Map::CenterPositionOfSlot(int slot_)
{
	return m_field1Slots[slot_].center;
}

