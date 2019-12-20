#pragma once
#include "Engine/Math/IntRange.hpp"
#include "Engine/Math/IntVec2.hpp"

#include <string>
#include <map>

class RTSTask;

enum ResourceType
{
	RESOURCETYPE_NONE,
	RESOURCETYPE_WOOD,
	RESOURCETYPE_MINERAL
};

struct RTSTaskInfo
{
	std::string taskType = "";
	std::string hotkeyLabel = "";
	RTSTask* rtsTask = nullptr;
};

class RTSTask;
class Prop;

class EntityDefinition
{

public:

	EntityDefinition();
	~EntityDefinition();

	// Load;
	static void LoadUnitsFromXML(const char* filename);
	static void LoadResourcesFromXML(const char* filename);

	// Stats;
	std::string m_entityType = "";
	float m_health			= 0.0f;
	float m_supply			= 0.0f;
	float m_armor			= 0.0f;
	float m_attack			= 0.0f;
	float m_range			= 0.0f;
	float m_attackSpeed		= 0.0f;
	float m_speed			= 0.0f;
	float m_buildtime		= 0.0f;	
	float m_physicsRadius	= 0.0f;
	float m_height			= 0.0f;
	bool m_selectable		= false;
	bool m_resource			= false;
	bool m_providesSupply	= false;
	bool m_canBeRepaired	= false;
	IntVec2 m_offset		= IntVec2(0, 0);
	IntVec2 m_occupancySize = IntVec2(0, 0);
	int m_woodCost			= 0;
	int m_mineralCost		= 0;
	ResourceType m_resourceType = RESOURCETYPE_NONE;

	// Model, if any;
	Prop* m_prop = nullptr;

	// Map to hold available Tasks for this Entity;
	std::map<std::string, RTSTaskInfo*> m_availableTasks;

	// Static Map to hold Definitions;
	static std::map<std::string, EntityDefinition*> s_entityDefinitions;

};