#pragma once

#include "Engine/Math/Vec2.hpp"

#include <vector>

class Entity;
class Player;
class Map;

/*
Owned by the Game;
Responsible for owning everything needed to play a Match of the Game;

Will own Entities;
Entities are Jobs and Units;
The Player will need to ask the Match what are my Enitties;


Will own the Players. Which can be Player or AI;
Will own the Map. Which will have the field information;
Will own the Hands of the Players. 
*/
class Match
{

public:

	Match();
	~Match();

	// Flow;
	void Startup();
	void Update(float deltaSeconds_);
	void Render();


public:

	int m_maxPlayers = 2; // This will be 8 someday...
	
	Map* m_map = nullptr;

	std::vector<Entity*> m_entities;
	std::vector<Player*> m_players;


};