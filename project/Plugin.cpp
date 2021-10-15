#include "stdafx.h"
#include "Plugin.h"
#include "IExamInterface.h"
#include "Behaviors.h"
#include "BehaviorTrees.cpp"

//Called only once, during initialization
void Plugin::Initialize(IBaseInterface* pInterface, PluginInfo& info)
{
	//Retrieving the interface
	//This interface gives you access to certain actions the AI_Framework can perform for you
	m_pInterface = static_cast<IExamInterface*>(pInterface);

	//Bit information about the plugin
	//Please fill this in!!
	info.BotName = "Agent007";
	info.Student_FirstName = "Wouter";
	info.Student_LastName = "Servaes";
	info.Student_Class = "2DAE02";
	m_Target = { 0.f, 0.f };
	m_pAgent = new Agent({ 0.f, 0.f }, m_pInterface);
	m_pHousesInFov = new std::vector<HouseInfo>{};
	m_pEntitiesInFov = new std::vector<EntityInfo>{};
	InitBlackboard();
	InitBehaviorTree();
}

void Plugin::InitBehaviorTree()
{
	m_pAgent->SetBTStructure(DefaultBT(m_pBlackboard));
}

void Plugin::InitBlackboard()
{
	m_pBlackboard = new Elite::Blackboard();
	m_pBlackboard->AddData("Agent", m_pAgent);
	m_pBlackboard->AddData("Interface", m_pInterface);
	m_pBlackboard->AddData("EntitiesInFov", m_pEntitiesInFov);
	m_pBlackboard->AddData("HousesInFov", m_pHousesInFov);
}

//Called only once
void Plugin::DllInit()
{
	//Called when the plugin is loaded
}

//Called only once
void Plugin::DllShutdown()
{
	SAFE_DELETE(m_pAgent);
	SAFE_DELETE(m_pEntitiesInFov);
	SAFE_DELETE(m_pHousesInFov);
	//Called when the plugin gets unloaded
}

//Called only once, during initialization
void Plugin::InitGameDebugParams(GameDebugParams& params)
{
	params.AutoFollowCam = true; //Automatically follow the AI? (Default = true)
	params.RenderUI = true; //Render the IMGUI Panel? (Default = true)
	params.SpawnEnemies = true; //Do you want to spawn enemies? (Default = true)
	params.EnemyCount = 20; //How many enemies? (Default = 20)
	params.GodMode = false; //GodMode > You can't die, can be usefull to inspect certain behaviours (Default = false)
	params.AutoGrabClosestItem = false; //A call to Item_Grab(...) returns the closest item that can be grabbed. (EntityInfo argument is ignored)
	params.LevelFile = "LevelOne.gppl";
	//params.LevelFile = "LevelTwo.gppl";
}

//Only Active in DEBUG Mode
//(=Use only for Debug Purposes)
void Plugin::Update(float dt)
{
	//Demo Event Code
	//In the end your AI should be able to walk around without external input
	if (m_pInterface->Input_IsMouseButtonUp(Elite::InputMouseButton::eLeft))
	{
		//Update target based on input
		Elite::MouseData mouseData = m_pInterface->Input_GetMouseData(Elite::InputType::eMouseButton, Elite::InputMouseButton::eLeft);
		const Elite::Vector2 pos = Elite::Vector2(static_cast<float>(mouseData.X), static_cast<float>(mouseData.Y));
		m_Target = m_pInterface->Debug_ConvertScreenToWorld(pos);
	}
	else if (m_pInterface->Input_IsKeyboardKeyDown(Elite::eScancode_Space))
	{
		m_CanRun = true;
	}
	else if (m_pInterface->Input_IsKeyboardKeyDown(Elite::eScancode_Left))
	{
		m_AngSpeed -= Elite::ToRadians(10);
	}
	else if (m_pInterface->Input_IsKeyboardKeyDown(Elite::eScancode_Right))
	{
		m_AngSpeed += Elite::ToRadians(10);
	}
	else if (m_pInterface->Input_IsKeyboardKeyDown(Elite::eScancode_G))
	{
		m_GrabItem = true;
	}
	else if (m_pInterface->Input_IsKeyboardKeyDown(Elite::eScancode_U))
	{
		m_UseItem = true;
	}
	else if (m_pInterface->Input_IsKeyboardKeyDown(Elite::eScancode_R))
	{
		m_RemoveItem = true;
	}
	else if (m_pInterface->Input_IsKeyboardKeyUp(Elite::eScancode_Space))
	{
		m_CanRun = false;
	}
}

//Update
//This function calculates the new SteeringOutput, called once per frame
SteeringPlugin_Output Plugin::UpdateSteering(float dt)
{
	//Use the Interface (IAssignmentInterface) to 'interface' with the AI_Framework
	
	m_pAgent->SetAgentInfo(m_pInterface->Agent_GetInfo());

	m_pHousesInFov->clear();
	*m_pHousesInFov = GetHousesInFOV();
	m_pEntitiesInFov->clear();
	*m_pEntitiesInFov = GetEntitiesInFOV();

	if (m_pHousesInFov->size() > 0)
		for (const auto& house : *m_pHousesInFov)
			m_pAgent->AddHouseToMemory(house);

	bool purgeZone{ false };
	if (m_pEntitiesInFov->size() > 0)
		for (const auto& entity : *m_pEntitiesInFov)
		{
			if (entity.Type == eEntityType::ITEM)
			{
				ItemInfo itemInfo{};
				m_pInterface->Item_GetInfo(entity, itemInfo);
				if (itemInfo.Type != eItemType::GARBAGE)
					m_pAgent->AddItemToMemory(itemInfo, m_pInterface);
			}
			else if (entity.Type == eEntityType::PURGEZONE)
			{
				PurgeZoneInfo pZInfo{};
				m_pInterface->PurgeZone_GetInfo(entity, pZInfo);
				m_pAgent->SetCurrentPurgeZone(pZInfo);
				purgeZone = true;
			}
		}

	if (!purgeZone) m_pAgent->NotInPurgeZone();
	m_pAgent->Update(dt, m_pInterface);
	PrintPurgeZone();

	return *m_pAgent->CalcSteering();
}

//This function should only be used for rendering debug elements
void Plugin::Render(float dt) const
{
	//This Render function should only contain calls to Interface->Draw_... functions
	m_pInterface->Draw_SolidCircle(m_Target, .7f, { 0, 0 }, { 1, 0, 0 });

	m_pAgent->Render(m_pInterface, true); //renders agent itself + debug rendering (world memory (traveled path, discovered houses, found items))
	//bool is for rendering world memory
}

vector<HouseInfo> Plugin::GetHousesInFOV() const
{
	vector<HouseInfo> vHousesInFOV = {};

	HouseInfo hi = {};
	for (int i = 0;; ++i)
	{
		if (m_pInterface->Fov_GetHouseByIndex(i, hi))
		{
			vHousesInFOV.push_back(hi);
			continue;
		}

		break;
	}

	return vHousesInFOV;
}

vector<EntityInfo> Plugin::GetEntitiesInFOV() const
{
	vector<EntityInfo> vEntitiesInFOV = {};

	EntityInfo ei = {};
	for (int i = 0;; ++i)
	{
		if (m_pInterface->Fov_GetEntityByIndex(i, ei))
		{
			vEntitiesInFOV.push_back(ei);
			continue;
		}

		break;
	}

	return vEntitiesInFOV;
}

void Plugin::PrintPurgeZone()
{
	auto vEntitiesInFOV = GetEntitiesInFOV(); //uses m_pInterface->Fov_GetEntityByIndex(...)

	for (auto& e : vEntitiesInFOV)
	{
		if (e.Type == eEntityType::PURGEZONE)
		{
			PurgeZoneInfo zoneInfo;
			m_pInterface->PurgeZone_GetInfo(e, zoneInfo);
			std::cout << "Purge Zone in FOV:" << e.Location.x << ", " << e.Location.y << " ---EntityHash: " << e.EntityHash << "---Radius: " << zoneInfo.Radius << std::endl;
		}
	}
}