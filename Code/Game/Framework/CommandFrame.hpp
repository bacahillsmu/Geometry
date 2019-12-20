#pragma once
#include "Engine/Core/Time.hpp"



class CommandFrame
{

public:

	CommandFrame();
	~CommandFrame();

	// To calculate the fixed deltaSeconds per frame;
	float m_fixedDeltaSeconds = 0.0f;
	float m_commandFrameFps = 100.0f;

	// To store the time left over if we have less than is needed this frame;
	float m_timeLeftOver = 0.0f;

	// Command Frame counts;
	unsigned int m_commandFrameNumber = 0;
	unsigned int m_restRun = 0;
	

	unsigned int GetCommandFrame();

};

