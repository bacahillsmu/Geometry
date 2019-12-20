#include "Game/Framework/CommandFrame.hpp"




// -----------------------------------------------------------------------
CommandFrame::CommandFrame()
{
	m_fixedDeltaSeconds = 1.0f / m_commandFrameFps;
}

// -----------------------------------------------------------------------
CommandFrame::~CommandFrame()
{

}

// -----------------------------------------------------------------------
unsigned int CommandFrame::GetCommandFrame()
{
	return m_commandFrameNumber;
}
