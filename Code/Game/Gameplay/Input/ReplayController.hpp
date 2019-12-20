#pragma once


#include <string>
#include <vector>

class Game;
class RTSCommand;

struct ReplayInfo
{
	int commandFrame;
	RTSCommand* rtsCommand;
};

class ReplayController
{

public:

	ReplayController( Game* theGame );
	~ReplayController();

	void RecordCommand(RTSCommand* rtsCommand);
	void LoadReplayFile(std::string filename);
	void SaveReplayFile(std::string filename);

public:

	Game* m_theGame = nullptr;
	std::vector<ReplayInfo> m_replayInfo;

};