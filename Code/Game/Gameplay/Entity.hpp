#pragma once

#include "Engine/Math/Vec2.hpp"



/*
All Entities will be own by the Match.

There are two types of Entities;
Cards and Units;
Cards are like cards and will be in Player's Hands or in the Market.
Units will be on the field for each Player.

*/
class Entity
{

public:

	Entity();
	virtual ~Entity();

	// Flow;
	virtual void Update(float deltaSeconds_) = 0;
	virtual void Render() = 0;

public:


	Vec2 m_position;
};

class Unit : public Entity
{

public:

	Unit();
	virtual ~Unit();

	virtual void Update(float deltaSeconds_) override;
	virtual void Render() override;


	//sprite
};

class Card : public Entity
{

public:

	Card();
	virtual ~Card();

	virtual void Update(float deltaSeconds_) override;
	virtual void Render() override;

};