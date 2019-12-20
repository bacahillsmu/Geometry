#pragma once

#include <stdint.h>

class GameHandle
{

public:

	GameHandle();
	explicit GameHandle(uint32_t id);
	GameHandle(uint16_t cyclicId, uint16_t index);

	uint32_t GetIndexSlot();
	uint32_t GetCyclicId();

	bool operator==(const GameHandle& other);
	bool operator!=(const GameHandle& other);

	static const GameHandle INVALID;
	
	uint32_t m_handle = 0;

};