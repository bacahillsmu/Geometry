#pragma once

/*
Players will be owned by the Match;
*/
class Player
{

public:

	Player(int playerID_);
	~Player();

	// Flow;
	void Update(float deltaSeconds_);

public:

	int m_playerID = -1;

};