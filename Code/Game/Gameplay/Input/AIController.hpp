#pragma once


class Game;
class Map;



class AIController
{

public:

	AIController(Game* game);
	~AIController();

	void UpdateThoughts();

	void LazyGoblinCheck();
	void TrainGoblin();
	void GoblinsGather();
	void GoblinsAttack();


	// Helpers;
	void CreateFirstTownCenter();

public:

	Game* m_theGame = nullptr;
	Map* m_theMap = nullptr;
	int m_aiTeam = 1;


	bool m_createdFirstTownCenter = false;
	bool m_issuedAttack = false;

};