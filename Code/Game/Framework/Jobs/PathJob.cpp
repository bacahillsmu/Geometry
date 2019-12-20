#include "Game/Framework/Jobs/PathJob.hpp"
#include "Game/Gameplay/Pathing/Pathing.hpp"

PathJob::~PathJob()
{
	delete m_pather; 
	m_pather = nullptr;
}

void PathJob::Execute()
{
	// ... Create a path;
	m_pathCreation = m_pather->CreatePath(m_startTile, m_endTiles, m_tileDimensions);
	m_pathCreation->m_path->pop_back();
	std::reverse(m_pathCreation->m_path->begin(), m_pathCreation->m_path->end());

	
}
