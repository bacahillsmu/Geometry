#include "Game/Gameplay/GameHandle.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"



GameHandle::GameHandle()
{

}

GameHandle::GameHandle( uint32_t id )
{
	m_handle = id;
}

GameHandle::GameHandle( uint16_t cyclicId, uint16_t index )
{
	uint32_t hiWord = cyclicId;
	hiWord <<= 16;
	m_handle = hiWord | index;
}

uint32_t GameHandle::GetIndexSlot()
{
	return m_handle & 0xffff;
}

uint32_t GameHandle::GetCyclicId()
{
	return (m_handle & 0xffff0000) >> 16;
}

bool GameHandle::operator!=( const GameHandle& other )
{
	return m_handle != other.m_handle;
}

bool GameHandle::operator==( const GameHandle& other )
{
	return m_handle == other.m_handle;
}


GameHandle const GameHandle::INVALID = GameHandle(0);
