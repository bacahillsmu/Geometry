#include "Game/Gameplay/EntityDefinition.hpp"
#include "Engine/Core/XmlUtils.hpp"
#include "Game/Gameplay/RTSTask.hpp"
#include "Engine/Renderer/Prop.hpp"


std::map<std::string, EntityDefinition*> EntityDefinition::s_entityDefinitions;

// -----------------------------------------------------------------------
EntityDefinition::EntityDefinition()
{
	delete m_prop;
	m_prop = nullptr;
}

// -----------------------------------------------------------------------
EntityDefinition::~EntityDefinition()
{

	std::map<std::string, EntityDefinition*>::iterator it;
	for (it = s_entityDefinitions.begin(); it != s_entityDefinitions.end(); it++)
	{
		delete it->second;
		it->second = nullptr;
	}
	s_entityDefinitions.clear();

	std::map<std::string, RTSTaskInfo*>::iterator taskit;
	for (taskit = m_availableTasks.begin(); taskit != m_availableTasks.end(); taskit++)
	{
		RTSTaskInfo* rtsTaskInfo = taskit->second;
		delete rtsTaskInfo->rtsTask;
		rtsTaskInfo->rtsTask = nullptr;
		delete taskit->second;
		taskit->second = nullptr;
	}
	m_availableTasks.clear();
}

// -----------------------------------------------------------------------
void EntityDefinition::LoadUnitsFromXML( const char* filename )
{
	tinyxml2::XMLDocument entityXMLDoc;
	entityXMLDoc.LoadFile(filename);

	if(entityXMLDoc.ErrorID() != tinyxml2::XML_SUCCESS)
	{
		printf("Error with XML Doc: %s\n", filename);
		printf("ErrorID:      %i\n", entityXMLDoc.ErrorID());
		printf("ErrorLineNum: %i\n", entityXMLDoc.ErrorLineNum());
		printf("ErrorLineNum: \"%s\"\n", entityXMLDoc.ErrorName());
	}
	else
	{
		printf("Success with XML Doc: %s\n", filename);

		XmlElement* rootElement = entityXMLDoc.RootElement();

		XmlElement* unitElement = rootElement->FirstChildElement("unit");
		while(unitElement)
		{
			EntityDefinition* entityDefinition = new EntityDefinition();

			// Load Unit stats;
			entityDefinition->m_entityType		= ParseXmlAttribute(*unitElement, "type", "");
			entityDefinition->m_health			= ParseXmlAttribute(*unitElement, "health", 0.0f);
			entityDefinition->m_supply			= ParseXmlAttribute(*unitElement, "supply", 0.0f);
			entityDefinition->m_armor			= ParseXmlAttribute(*unitElement, "armor", 0.0f);
			entityDefinition->m_attack			= ParseXmlAttribute(*unitElement, "attack", 0.0f);
			entityDefinition->m_range			= ParseXmlAttribute(*unitElement, "range", 0.0f);
			entityDefinition->m_attackSpeed		= ParseXmlAttribute(*unitElement, "attackSpeed", 0.0f);
			entityDefinition->m_speed			= ParseXmlAttribute(*unitElement, "speed", 0.0f);
			entityDefinition->m_buildtime		= ParseXmlAttribute(*unitElement, "buildtime", 0.0f);
			entityDefinition->m_physicsRadius	= ParseXmlAttribute(*unitElement, "physicsRadius", 0.0f);
			entityDefinition->m_height			= ParseXmlAttribute(*unitElement, "height", 0.0f);
			entityDefinition->m_selectable		= ParseXmlAttribute(*unitElement, "selectable", "false") == "true" ? true : false;
			entityDefinition->m_providesSupply	= ParseXmlAttribute(*unitElement, "providesSupply", "false") == "true" ? true : false;
			entityDefinition->m_canBeRepaired	= ParseXmlAttribute(*unitElement, "canBeRepaired", "false") == "true" ? true : false;

			XmlElement* costElement = unitElement->FirstChildElement("cost");
			if(costElement)
			{
				entityDefinition->m_woodCost = ParseXmlAttribute(*costElement, "wood", 0);
				entityDefinition->m_mineralCost = ParseXmlAttribute(*costElement, "mineral", 0);
			}

			XmlElement* modelElement = unitElement->FirstChildElement("model");
			if(modelElement)
			{
				entityDefinition->m_prop = new Prop(g_theRenderer);
				entityDefinition->m_prop->Load(ParseXmlAttribute(*modelElement, "filename", "").c_str());
			}

			XmlElement* occupancyElement = unitElement->FirstChildElement("occupancy");
			if(occupancyElement)
			{
				IntVec2 offset = IntVec2(0, 0);
				IntVec2 occupancySize = IntVec2(0, 0);
				entityDefinition->m_offset = ParseXmlAttribute(*occupancyElement, "offset", offset);
				entityDefinition->m_occupancySize = ParseXmlAttribute(*occupancyElement, "size", occupancySize);
			}

			// Load Unit Commands;
			XmlElement* tasksElement = unitElement->FirstChildElement("tasks");
			XmlElement* task = tasksElement->FirstChildElement("task");
			if(!task)
			{
				RTSTaskInfo* rtsTaskInfo = new RTSTaskInfo();
				RTSTask* newTask = nullptr;
				std::string taskType = "NONE";
				std::string hotkeyLabel = "NONE";

				rtsTaskInfo->taskType = taskType;
				rtsTaskInfo->rtsTask = newTask;
				rtsTaskInfo->hotkeyLabel = hotkeyLabel;
				entityDefinition->m_availableTasks[taskType] = rtsTaskInfo;
			}
			while (task)
			{
				RTSTaskInfo* rtsTaskInfo = new RTSTaskInfo();
				RTSTask* newTask = nullptr;
				std::string taskType = ParseXmlAttribute(*task, "type", "");
				std::string hotkeyLabel = ParseXmlAttribute(*task, "hotkeyLabel", "");

				if(taskType == "move")
				{
					newTask = new MoveTask();
				}
				else if(taskType == "attack")
				{
					newTask = new AttackTask();					
				}
				else if(taskType == "follow")
				{
					newTask = new FollowTask();					
				}
				else if(taskType == "gather")
				{
					newTask = new GatherTask();
				}
				else if(taskType == "buildtowncenter")
				{
					newTask = new BuildTask();
				}
				else if(taskType == "buildhut")
				{
					newTask = new BuildTask();
				}
				else if(taskType == "buildtower")
				{
					newTask = new BuildTask();
				}
				else if(taskType == "buildgoblinhut")
				{
					newTask = new BuildTask();
				}
				else if(taskType == "buildgoblintower")
				{
					newTask = new BuildTask();
				}
				else if(taskType == "repair")
				{
					newTask = new RepairTask();
				}
				else if(taskType == "trainpeon")
				{
					newTask = new TrainTask();
				}
				else if(taskType == "trainwarrior")
				{
					newTask = new TrainTask();
				}
				else if(taskType == "traingoblin")
				{
					newTask = new TrainTask();
				}

				rtsTaskInfo->taskType = taskType;
				rtsTaskInfo->rtsTask = newTask;
				rtsTaskInfo->hotkeyLabel = hotkeyLabel;
				entityDefinition->m_availableTasks[taskType] = rtsTaskInfo;
				task = task->NextSiblingElement();
			}

			s_entityDefinitions[entityDefinition->m_entityType] = entityDefinition;
			unitElement = unitElement->NextSiblingElement();
		}		
	}
}

void EntityDefinition::LoadResourcesFromXML( const char* filename )
{
	tinyxml2::XMLDocument entityXMLDoc;
	entityXMLDoc.LoadFile(filename);

	if(entityXMLDoc.ErrorID() != tinyxml2::XML_SUCCESS)
	{
		printf("Error with XML Doc: %s\n", filename);
		printf("ErrorID:      %i\n", entityXMLDoc.ErrorID());
		printf("ErrorLineNum: %i\n", entityXMLDoc.ErrorLineNum());
		printf("ErrorLineNum: \"%s\"\n", entityXMLDoc.ErrorName());
	}
	else
	{
		printf("Success with XML Doc: %s\n", filename);

		XmlElement* rootElement = entityXMLDoc.RootElement();
		XmlElement* resourceElement = rootElement->FirstChildElement("resource");
		while(resourceElement)
		{
			EntityDefinition* entityDefinition = new EntityDefinition();
			entityDefinition->m_prop = new Prop(g_theRenderer);

			// Load Unit stats;
			entityDefinition->m_resource = true;
			entityDefinition->m_selectable = true;
			entityDefinition->m_health = ParseXmlAttribute(*resourceElement, "health", 0.0f);
			entityDefinition->m_physicsRadius = ParseXmlAttribute(*resourceElement, "physicsRadius", 0.0f);
			entityDefinition->m_entityType = ParseXmlAttribute(*resourceElement, "type", "");
			entityDefinition->m_resourceType = ParseXmlAttribute(*resourceElement, "resourceType", "") == "wood" ? RESOURCETYPE_WOOD : RESOURCETYPE_NONE;
			entityDefinition->m_prop->Load(ParseXmlAttribute(*resourceElement, "filename", "").c_str());

			XmlElement* occupancyElement = resourceElement->FirstChildElement("occupancy");
			if(occupancyElement)
			{
				IntVec2 offset = IntVec2(0, 0);
				IntVec2 occupancy = IntVec2(0, 0);
				entityDefinition->m_offset = ParseXmlAttribute(*occupancyElement, "offset", occupancy);
				entityDefinition->m_occupancySize = ParseXmlAttribute(*occupancyElement, "size", occupancy);
			}

			s_entityDefinitions[entityDefinition->m_entityType] = entityDefinition;
			resourceElement = resourceElement->NextSiblingElement();
		}
	}
}
