#include "stdafx.h"
#include "Agent.h"

Agent::Agent(const Elite::Vector2& spawnPos, const IExamInterface* pInterface)
	:m_StartHp{ pInterface->Agent_GetInfo().Health }
	, m_StartEnergy{ pInterface->Agent_GetInfo().Energy }
	, m_InvCapacity{ int(pInterface->Inventory_GetCapacity()) }
{
	
	m_AgentInfo.Position = spawnPos;
	m_pWorldMemory = new WorldMemory(pInterface->World_GetInfo().Dimensions, pInterface->World_GetInfo().Center, { 100, 100 });
	m_pSteeringOutput = new SteeringPlugin_Output();

	m_pSeek = new Seek();
	m_pFlee = new Flee();
	m_pWander = new Wander();
	m_pFace = new Face();

	m_Inventory.resize(m_InvCapacity);
	ItemInfo temp;
	temp.Type = eItemType::GARBAGE;
	std::fill(m_Inventory.begin(), m_Inventory.end(), temp);
}
Agent::~Agent()
{
	SAFE_DELETE(m_pPurgeZone);
	SAFE_DELETE(m_pSeek);
	SAFE_DELETE(m_pFlee);
	SAFE_DELETE(m_pWander);
	SAFE_DELETE(m_pFace);
	SAFE_DELETE(m_pWorldMemory);
	SAFE_DELETE(m_pBTStructure);
}

void Agent::Update(float deltaT, IExamInterface* pInterface)
{
	if ((m_AgentInfo.RunMode && m_AgentInfo.IsInHouse) || m_AgentInfo.Stamina == 0) m_AgentInfo.RunMode = false;
	NeedPistol(pInterface);
	UseMedkit(pInterface);
	UseFood(pInterface);
	m_pBTStructure->Update(deltaT);
	WasInHouse();
	ExploreHouseTimer(deltaT);
	m_pWorldMemory->UpdateHasBeenHere(m_AgentInfo.Position);
}

void Agent::Render(IExamInterface* pInterface, bool renderDebug) const
{
	pInterface->Draw_Circle(m_AgentInfo.Position, m_AgentInfo.AgentSize, m_AgentColor);
	if (!renderDebug) return;
	DrawHouseMemory(pInterface);
	DrawTrailMemory(pInterface);
	DrawItemMemory(pInterface);
}

SteeringPlugin_Output* Agent::CalcSteering()
{
	if (m_pCurrentBehavior)
		m_pSteeringOutput = &m_pCurrentBehavior->CalcSteering(m_AgentInfo);

	m_pSteeringOutput->RunMode = m_AgentInfo.RunMode;
	return m_pSteeringOutput;
}

void Agent::SetToSeek(const Elite::Vector2& seekPos)
{
	m_pSeek->SetTarget(seekPos);
	m_pCurrentBehavior = m_pSeek;
}

void Agent::SetToFlee(const Elite::Vector2& fleePos)
{
	m_pFlee->SetTarget(fleePos);
	m_pCurrentBehavior = m_pFlee;
}

void Agent::ExploreWorldGrid()
{
	Elite::Vector2 target{};

	if (m_pWorldMemory->RandomNextSqrTarget(m_AgentInfo.Position, target))
		SetToSeek(target);
	else
		SetToWander();
}

void Agent::WasInHouse()
{
	if (!m_WasInHouse)
	{
		if (m_AgentInfo.IsInHouse)
		{
			m_IsExploringHouse = true;
			m_WasInHouse = true;
			m_pWorldMemory->AddHouseEntrance(m_AgentInfo.Position);
		}
	}
	else
	{
		if (!m_AgentInfo.IsInHouse)
		{
			m_ExploringNewHouse = false;
			m_WasInHouse = false;
			m_pWorldMemory->AddHouseEntrance(m_AgentInfo.Position);
			if (m_pWorldMemory->GetAmountOfSavedHouses() >= m_MinHousesToDiscover)
				if (m_CurrentHouseTargetIdx < m_MinHousesToDiscover)
					m_CurrentHouseTargetIdx++;
				else
					m_CurrentHouseTargetIdx = 0;
		}
	}
}

Elite::Vector2 Agent::GetNextHousePos()
{
	if (m_pWorldMemory->GetAmountOfSavedHouses() >= m_MinHousesToDiscover)
		return m_CurrentHouseCenter = m_pWorldMemory->GetHouseAtIndex(m_CurrentHouseTargetIdx)->Center;
	
	return m_CurrentHouseCenter = m_pWorldMemory->GetHouseAtIndex(m_pWorldMemory->GetSavedHouses()->size() - 1)->Center;
}

Elite::Vector2 Agent::GetHouseEntracePos() const
{
	auto house{ m_pWorldMemory->GetHouseAtPosition(m_AgentInfo.Position) };

	Elite::Vector2 closestEntrance{};

	for (const auto& entrance : house->entrancePos)
		if (closestEntrance == Elite::Vector2{ 0.f, 0.f } ||
			Elite::Distance(m_AgentInfo.Position, entrance) < Elite::Distance(m_AgentInfo.Position, closestEntrance))
			closestEntrance = entrance;

	return closestEntrance;
}

void Agent::AddHouseToMemory(const HouseInfo& house)
{
	if (m_pWorldMemory->AddHouseToMemory(&house))
	{
		m_ExploringNewHouse = true;
		m_NewHouse = true;
	}
	else
		m_NewHouse = false;
}

void Agent::ExploreHouseTimer(float deltaT)
{
	if (m_IsExploringHouse)
	{
		if (Elite::Distance(m_AgentInfo.Position, m_CurrentHouseCenter) > .5f) return;
		if (m_ElapsedExploreTime >= m_MaxExploreTime)
		{
			m_IsExploringHouse = false;
			m_ElapsedExploreTime = 0.f;
		}
		else
			m_ElapsedExploreTime += deltaT;
	}
}

bool Agent::NeededItemInMemory() const
{
	return m_pWorldMemory->HaveItemTypeInMemory(m_NeededItem);
}

bool Agent::AddItemToInventory(const ItemInfo& item, IExamInterface* pInterface)
{
	int freeSpot{};
	for (const auto& i : m_Inventory)
	{
		if (i.Type == eItemType::GARBAGE)
			break;

		freeSpot++;
	}
	if (pInterface->Inventory_AddItem(freeSpot, item))
	{
		m_Inventory[freeSpot] = item;

		m_pWorldMemory->RemoveItemFromMemory(item);
		return true;
	}
	return false;
}

void Agent::UseMedkit(IExamInterface* pInterface)
{
	if (m_AgentInfo.Health <= m_StartHp / 2.f)
	{
		int slotId{};
		int medkitSlotId{};

		auto it{ std::find_if(m_Inventory.begin(), m_Inventory.end(), [&slotId, &medkitSlotId](const ItemInfo& itemInfo) {
			if (itemInfo.Type == eItemType::MEDKIT)
			{
				medkitSlotId = slotId;
				return true;
			}
			slotId++;
			return false;
			}) };

		if (it != m_Inventory.end())
		{
			ItemInfo temp;
			temp.Type = eItemType::GARBAGE;
			m_Inventory[medkitSlotId] = temp;
			pInterface->Inventory_UseItem(medkitSlotId);
			pInterface->Inventory_RemoveItem(medkitSlotId);
			if (m_NeedItem && m_NeededItem == eItemType::MEDKIT)
				m_NeedItem = false;
		}
		else
		{
			m_NeededItem = eItemType::MEDKIT;
			m_NeedItem = true;
		}
	}
}

void Agent::UseFood(IExamInterface* pInterface)
{
	if (m_AgentInfo.Energy <= m_StartEnergy / 2)
	{
		int slotId{};
		int foodSlotId{};
		auto it{ std::find_if(m_Inventory.begin(), m_Inventory.end(), [&slotId, &foodSlotId](const ItemInfo& itemInfo) {
			if (itemInfo.Type == eItemType::FOOD)
			{
				foodSlotId = slotId;
				return true;
			}
			slotId++;
			return false;
			}) };

		if (it != m_Inventory.end())
		{
			ItemInfo temp;
			temp.Type = eItemType::GARBAGE;
			m_Inventory[foodSlotId] = temp;
			pInterface->Inventory_UseItem(slotId);
			pInterface->Inventory_RemoveItem(slotId);
			if (m_NeedItem && m_NeededItem == eItemType::FOOD)
				m_NeedItem = false;
		}
		else
		{
			m_NeededItem = eItemType::FOOD;
			m_NeedItem = true;
		}
	}
}

void Agent::NeedPistol(IExamInterface* pInterface)
{
	auto it{ std::find_if(m_Inventory.begin(), m_Inventory.end(), [pInterface](const ItemInfo& itemInfo) {
		if (itemInfo.Type == eItemType::PISTOL)
			return true;
		return false;
		}) };

	if (it == m_Inventory.end())
	{
		m_NeedItem = true;
		m_NeededItem = eItemType::PISTOL;
		m_HasGun = false;
	}
	else
	{
		if (m_NeededItem == eItemType::PISTOL)
			m_NeedItem = false;

		m_HasGun = true;
	}
}

void Agent::AttackEnemy(const EnemyInfo& enemy, IExamInterface* pInterface)
{
	RemoveEmptyGuns(pInterface);

	if ((Elite::OrientationToVector(m_AgentInfo.Orientation).GetNormalized() - (enemy.Location - m_AgentInfo.Position).GetNormalized()).Magnitude() <= .1f)
	{
		m_AgentInfo.AngularVelocity = 0.f;
		m_pSteeringOutput->AngularVelocity = 0.f;
		int slotId{};
		int gunSlotId{};
		auto it{ std::find_if(m_Inventory.begin(), m_Inventory.end(), [pInterface, &slotId, &gunSlotId](ItemInfo& itemInfo) {
			if (itemInfo.Type == eItemType::PISTOL)
			{
				gunSlotId = slotId;
				return true;
			}

			slotId++;
			return false;
			}) };

		if (it != m_Inventory.end())
		{
			pInterface->Inventory_UseItem(slotId);
		}
	}
	else
	{
		m_pSteeringOutput->AutoOrient = false;

		m_pFace->SetTarget(enemy.Location);
		m_pCurrentBehavior = m_pFace;
	}
}

void Agent::RunFromEnemies(const std::vector<EnemyInfo>& enemies)
{
	Elite::Vector2 target{};
	m_AgentInfo.RunMode = true;
	Elite::Vector2 point{};
	for (const auto& e : enemies)
	{
		point += e.Location;
	}
	point /= enemies.size();

	m_pFlee->SetTarget(point);
	m_pCurrentBehavior = m_pFlee;
}

void Agent::SetRunModeOn()
{
	m_AgentInfo.RunMode = true;
}

void Agent::RemoveEmptyGuns(IExamInterface* pInterface)
{
	ItemInfo garbage;
	garbage.Type = eItemType::GARBAGE;
	int slotId{};
	for (auto& i : m_Inventory)
	{
		if (i.Type == eItemType::PISTOL)
		{
			if (pInterface->Weapon_GetAmmo(i) == 0)
			{
				pInterface->Inventory_RemoveItem(slotId);
				i = garbage;
			}
		}
		slotId++;
	}
}

bool Agent::ShouldPickUpNotNeededItem()
{
	int freeSpot{};
	std::for_each(m_Inventory.begin(), m_Inventory.end(), [&freeSpot](ItemInfo& item) {
		if (item.Type == eItemType::GARBAGE) freeSpot += 1;
		});

	return freeSpot >= 2;
}

void Agent::RunFromPurgeZone()
{
	m_pFlee->SetTarget(m_pPurgeZone->Center);
	m_pCurrentBehavior = m_pFlee;
	m_AgentInfo.RunMode = true;
}