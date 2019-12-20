#pragma once
#include "Engine/Math/Vec3.hpp"
#include "Game/Gameplay/GameHandle.hpp"
#include "Game/Gameplay/Map/Map.hpp"
#include "Engine/Core/NamedStrings.hpp"

#include <string>
#include <map>

enum CommandType
{
	CREATEPEON,
	CREATEWARRIOR,
	MOVE,
	ATTACK,
	DIE,
	FOLLOW,
	RIGHTCLICK,
	CREATEPINETREE,
	CREATETOWNHALL,
	GATHER,
	CREATEPINEBARK,
	CREATEPINESTUMP,
	REPAIR,
	TRAINPEON,
	TRAINWARRIOR,
	BUILDTOWNHALL,
	TRAINGOBLIN,
	CREATEGOBLIN,
	BUILDHUT,
	BUILDTOWER,
	BUILDGOBLINHUT,
	BUILDGOBLINTOWER,
	CREATEFULLMINERAL,
	CREATEMIDDLEMINERAL,
	CREATEEMPTYMINERAL,
	CREATEGOBLINHUT
};

// -----------------------------------------------------------------------
class RTSCommand
{

public:

	virtual ~RTSCommand() {}
	virtual void Execute() = 0;
	virtual void AppendDataToXMLElement(tinyxml2::XMLElement& element) = 0;

	unsigned int m_commandFrameIssued = 0;
	GameHandle m_unit;
	int m_commandType	= -1;
	Game* m_theGame		= nullptr;
	Map* m_theMap		= nullptr;
	
		
};

// -----------------------------------------------------------------------
class CreateCommand : public RTSCommand
{

public:

	virtual void Execute() override;
	virtual void AppendDataToXMLElement(tinyxml2::XMLElement& element) override;

public:

	bool m_justConstructed	= false;
	float m_percentHealth	= 1.0f;
	int m_team				= 0;
	Vec3 m_createPosition	= Vec3(0.0f, 0.0f, 0.0f);

};

// -----------------------------------------------------------------------
class RightClickCommand : public RTSCommand
{

public:

	virtual void Execute() override;
	virtual void AppendDataToXMLElement(tinyxml2::XMLElement& element) override;

	Vec3 m_rightClickPosition = Vec3(0.0f, 0.0f, 0.0f);
	GameHandle m_rightClickEntity;
};

// -----------------------------------------------------------------------
class MoveCommand : public RTSCommand
{

public:

	virtual void Execute() override;
	virtual void AppendDataToXMLElement(tinyxml2::XMLElement& element) override;

	Vec3 m_movePosition = Vec3(0.0f, 0.0f, 0.0f);
};

// -----------------------------------------------------------------------
class BuildCommand : public RTSCommand
{

public:

	virtual void Execute() override;
	virtual void AppendDataToXMLElement(tinyxml2::XMLElement& element) override;

	int m_team				= 0;
	Vec3 m_buildPosition	= Vec3(0.0f, 0.0f, 0.0f);
};

// -----------------------------------------------------------------------
class AttackCommand : public RTSCommand
{

public:

	virtual void Execute() override;
	virtual void AppendDataToXMLElement(tinyxml2::XMLElement& element) override;

	GameHandle m_entityToAttack;
};

// -----------------------------------------------------------------------
class FollowCommand : public RTSCommand
{

public:

	virtual void Execute() override;
	virtual void AppendDataToXMLElement(tinyxml2::XMLElement& element) override;

	GameHandle m_entityToFollow;
};

// -----------------------------------------------------------------------
class GatherCommand : public RTSCommand
{

public:

	virtual void Execute() override;
	virtual void AppendDataToXMLElement(tinyxml2::XMLElement& element) override;

	GameHandle m_entityToGather;
};



// -----------------------------------------------------------------------
class DieCommand : public RTSCommand
{

public:

	virtual void Execute() override;
	virtual void AppendDataToXMLElement(tinyxml2::XMLElement& element) override;

public:

	bool m_instantDeath = false;
	float m_deathTimerThreshold = 3.0f;

};

// -----------------------------------------------------------------------
class RepairCommand : public RTSCommand
{

public:

	virtual void Execute() override;
	virtual void AppendDataToXMLElement(tinyxml2::XMLElement& element) override;

public:

	GameHandle m_entityToRepair;

};

// -----------------------------------------------------------------------
class TrainCommand : public RTSCommand
{

public:

	virtual void Execute() override;
	virtual void AppendDataToXMLElement(tinyxml2::XMLElement& element) override;

public:

	//EntityDefinition* m_entityDefinitionToTrain = nullptr;
	int m_team = 0;
	std::string m_training = "";
	std::string m_entityToTrain = "";

};