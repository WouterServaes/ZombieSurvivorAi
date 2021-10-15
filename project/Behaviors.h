/*=============================================================================*/
// Copyright 2020-2021 Elite Engine
/*=============================================================================*/
// Behaviors.h: Implementation of certain reusable behaviors for a BT
/*=============================================================================*/
#ifndef ELITE_APPLICATION_BEHAVIOR_TREE_BEHAVIORS
#define ELITE_APPLICATION_BEHAVIOR_TREE_BEHAVIORS
//-----------------------------------------------------------------
// Includes & Forward Declarations
//-----------------------------------------------------------------
#include "stdafx.h"
#include "Exam_HelperStructs.h"
#include "IExamInterface.h"

#pragma region MainConditions
//-----------------------------------------------------------------
// Conditions
//-----------------------------------------------------------------

// true if there's a house in sight
bool NewHouseInSight(Elite::Blackboard* pBlackboard)
{
	Agent* pAgent{ nullptr };
	if (!pBlackboard->GetData("Agent", pAgent)) return false;
	return pAgent->GetNewHouse();
}

// true if there's at least "MinAmountHousesToExplore" discovered houses
bool MinAmountHousesFound(Elite::Blackboard* pBlackboard)
{
	Agent* pAgent{ nullptr };
	if (!pBlackboard->GetData("Agent", pAgent)) return false;

	return pAgent->GetSavedHouses()->size() >= pAgent->GetMinAmountHousesToExplore();
}

bool AgentExploringHouse(Elite::Blackboard* pBlackboard)
{
	Agent* pAgent{ nullptr };
	if (!pBlackboard->GetData("Agent", pAgent)) return false;
	return pAgent->GetIsExploringHouse();
}

bool CanLeaveHouse(Elite::Blackboard* pBlackboard)
{
	Agent* pAgent{ nullptr };
	if (!pBlackboard->GetData("Agent", pAgent)) return false;

	return pAgent->GetCanLeaveHouse();
}

bool NeedItem(Elite::Blackboard* pBlackboard)
{
	Agent* pAgent{ nullptr };
	if (!pBlackboard->GetData("Agent", pAgent)) return false;
	return pAgent->NeedItem();
}

bool NeededItemInMemory(Elite::Blackboard* pBlackboard)
{
	Agent* pAgent{ nullptr };
	if (!pBlackboard->GetData("Agent", pAgent)) return false;
	return pAgent->NeededItemInMemory();
}

bool NoNeededItemInMemory(Elite::Blackboard* pBlackboard)
{
	Agent* pAgent{ nullptr };
	if (!pBlackboard->GetData("Agent", pAgent)) return false;

	return !pAgent->NeededItemInMemory();
}

bool ItemInFov(Elite::Blackboard* pBlackboard)
{
	std::vector<EntityInfo>* pEntitiesInFov{ nullptr };
	IExamInterface* pInterface{ nullptr };

	if (!(pBlackboard->GetData("EntitiesInFov", pEntitiesInFov) && pBlackboard->GetData("Interface", pInterface))) return false;

	for (const auto& e : *pEntitiesInFov)
		if (e.Type == eEntityType::ITEM)
		{
			ItemInfo temp{};
			pInterface->Item_GetInfo(e, temp);
			if (temp.Type != eItemType::GARBAGE)
			{
				return true;
			}
		}
	return false;
}

bool ShouldGoToItemInFov(Elite::Blackboard* pBlackboard)
{
	Agent* pAgent{ nullptr };
	std::vector<EntityInfo>* pEntitiesInFov{ nullptr };
	IExamInterface* pInterface{ nullptr };

	if (!(pBlackboard->GetData("EntitiesInFov", pEntitiesInFov) && pBlackboard->GetData("Interface", pInterface) && pBlackboard->GetData("Agent", pAgent))) return false;

	std::vector<eItemType> tempV{};
	for (const auto& e : *pEntitiesInFov)
		if (e.Type == eEntityType::ITEM)
		{
			ItemInfo temp{};
			pInterface->Item_GetInfo(e, temp);
			if (temp.Type != eItemType::GARBAGE)
			{
				tempV.push_back(temp.Type);
			}
		}

	if (pAgent->ShouldPickUpNotNeededItem())
	{
		return true;
	}
	else if (pAgent->NeedItem())
	{
		const auto& neededType{ pAgent->NeededItemType() };
		for (const auto& t : tempV)
		{
			if (t == neededType)
				return true;
		}
	}

	return false;
}

bool HaveItemsInMemory(Elite::Blackboard* pBlackboard)
{
	Agent* pAgent{ nullptr };
	if (!pBlackboard->GetData("Agent", pAgent)) return false;
	return pAgent->HaveItemsInMemory();
}

bool NeededItemInGrabRange(Elite::Blackboard* pBlackboard)
{
	Agent* pAgent{ nullptr };
	if (!(pBlackboard->GetData("Agent", pAgent))) return false;

	ItemInfo closestItem{};

	if (!pAgent->GetClosestItem(pAgent->NeededItemType(), closestItem))
		return false;

	return pAgent->DistanceTo(closestItem.Location) <= pAgent->GetAgentInfo().GrabRange;
}

bool AnyItemInGrabRange(Elite::Blackboard* pBlackboard)
{
	Agent* pAgent{ nullptr };
	std::vector<EntityInfo>* pEntitiesInFov{ nullptr };
	IExamInterface* pInterface{ nullptr };
	if (!(pBlackboard->GetData("Agent", pAgent) && pBlackboard->GetData("EntitiesInFov", pEntitiesInFov) && pBlackboard->GetData("Interface", pInterface))) return false;

	for (const auto& e : *pEntitiesInFov)
	{
		ItemInfo item{};
		if (!pInterface->Item_GetInfo(e, item)) continue;

		if (item.Type == eItemType::GARBAGE) continue;

		if (pAgent->DistanceTo(item.Location) <= pAgent->GetAgentInfo().GrabRange)
		{
			pInterface->Item_Grab(e, item);
			pAgent->SetItemInGrabRange(item);
			return true;
		}
	}
	return false;
}

bool PickUpAnyItem(Elite::Blackboard* pBlackboard)
{
	Agent* pAgent{ nullptr };
	if (!(pBlackboard->GetData("Agent", pAgent))) return false;
	return pAgent->ShouldPickUpNotNeededItem();
}
bool SeeEnemy(Elite::Blackboard* pBlackboard)
{
	std::vector<EntityInfo>* pEntities{ nullptr };
	if (!pBlackboard->GetData("EntitiesInFov", pEntities)) return false;

	for (const auto& e : *pEntities)
		if (e.Type == eEntityType::ENEMY)
			return true;

	return false;
}

bool CanTargetEnemy(Elite::Blackboard* pBlackboard)
{
	Agent* pAgent{ nullptr };
	if (!(pBlackboard->GetData("Agent", pAgent))) return false;
	return pAgent->CanAttack();
}

bool ShouldRunFromEnemies(Elite::Blackboard* pBlackboard)
{
	Agent* pAgent{ nullptr };
	if (!(pBlackboard->GetData("Agent", pAgent))) return false;
	return !pAgent->CanAttack();
}

bool WasBitten(Elite::Blackboard* pBlackboard)
{
	Agent* pAgent{ nullptr };
	if (!(pBlackboard->GetData("Agent", pAgent))) return false;
	return pAgent->GetAgentInfo().WasBitten;
}

bool IsRunning(Elite::Blackboard* pBlackboard)
{
	Agent* pAgent{ nullptr };
	if (!(pBlackboard->GetData("Agent", pAgent))) return false;
	return pAgent->IsRunningFromEnemies();
}

bool SeePurgeZone(Elite::Blackboard* pBlackboard)
{
	Agent* pAgent{ nullptr };
	if (!(pBlackboard->GetData("Agent", pAgent))) return false;

	return pAgent->GetCurrentPurgeZoneInfo() != nullptr;
}
#pragma endregion

#pragma region MainActions
//-----------------------------------------------------------------
// Actions
//-----------------------------------------------------------------

//set behavior to seek, seek target is NextTargetPos from blackboard
Elite::BehaviorState ToWander(Elite::Blackboard* pBlackboard)
{
	Agent* pAgent{ nullptr };
	if (!(pBlackboard->GetData("Agent", pAgent))) return Elite::Failure;

	pAgent->SetToWander();
	return  Elite::Success;
}

//agents travels the world grid
Elite::BehaviorState Explore(Elite::Blackboard* pBlackboard)
{
	Agent* pAgent{ nullptr };
	if (!(pBlackboard->GetData("Agent", pAgent))) return Elite::Failure;

	pAgent->ExploreWorldGrid();

	return  Elite::Success;
}

//Finds the closest exit of the house the agent is currently in and goes to this exit
Elite::BehaviorState LeaveHouse(Elite::Blackboard* pBlackboard)
{
	Agent* pAgent{ nullptr };
	IExamInterface* pInterface{ nullptr };

	if (!(pBlackboard->GetData("Agent", pAgent) && pBlackboard->GetData("Interface", pInterface))) return Elite::Failure;

	pAgent->SetToSeek(pInterface->NavMesh_GetClosestPathPoint(pAgent->GetHouseEntracePos()));
	return Elite::Success;
}

//goes to the center of the newly discovered house
Elite::BehaviorState GoToHouse(Elite::Blackboard* pBlackboard)
{
	Agent* pAgent{ nullptr };
	IExamInterface* pInterface{ nullptr };
	if (!(pBlackboard->GetData("Agent", pAgent) && pBlackboard->GetData("Interface", pInterface))) return Elite::Failure;

	pAgent->SetToSeek(pInterface->NavMesh_GetClosestPathPoint(pAgent->GetNextHousePos()));
	return  Elite::Success;
}

Elite::BehaviorState GoToItemInFov(Elite::Blackboard* pBlackboard)
{
	Agent* pAgent{ nullptr };
	std::vector<EntityInfo>* pEntitiesInFov{ nullptr };
	IExamInterface* pInterface{ nullptr };
	if (!(pBlackboard->GetData("Agent", pAgent) && pBlackboard->GetData("EntitiesInFov", pEntitiesInFov) && pBlackboard->GetData("Interface", pInterface))) return Elite::Failure;

	Elite::Vector2 closestItem{ pEntitiesInFov->at(0).Location };

	bool itemFound{ false };

	for (const auto& e : *pEntitiesInFov)
		if (e.Type == eEntityType::ITEM)
		{
			ItemInfo temp{};
			pInterface->Item_GetInfo(e, temp);
			if (temp.Type == eItemType::GARBAGE)
				continue;

			if (pAgent->NeedItem() && pAgent->NeededItemType() == temp.Type)
			{
				closestItem = temp.Location;
				itemFound = true;
				break;
			}

			if (pAgent->DistanceTo(e.Location) <= pAgent->DistanceTo(closestItem))
			{
				itemFound = true;
				closestItem = e.Location;
			}
		}

	if (itemFound)
	{
		pAgent->SetToSeek(closestItem);
		return  Elite::Success;
	}

	return Elite::Failure;
}

Elite::BehaviorState GoToNeededItem(Elite::Blackboard* pBlackboard)
{
	Agent* pAgent{ nullptr };
	IExamInterface* pInterface{ nullptr };
	if (!(pBlackboard->GetData("Agent", pAgent) &&
		pBlackboard->GetData("Interface", pInterface))) return Elite::Failure;

	Elite::Vector2 closestItem{};

	pAgent->GetClosestItemPosition(pAgent->NeededItemType(), closestItem);

	pAgent->SetToSeek(pInterface->NavMesh_GetClosestPathPoint(closestItem));
	return  Elite::Success;
}

Elite::BehaviorState GrabNeededItem(Elite::Blackboard* pBlackboard)
{
	Agent* pAgent{ nullptr };
	IExamInterface* pInterface{ nullptr };
	std::vector<EntityInfo>* pEntitiesInFov{ nullptr };
	if (!(pBlackboard->GetData("Agent", pAgent) && pBlackboard->GetData("EntitiesInFov", pEntitiesInFov) && pBlackboard->GetData("Interface", pInterface))) return Elite::Failure;

	ItemInfo closestItem{};

	pAgent->GetClosestItem(pAgent->NeededItemType(), closestItem);

	for (const auto& e : *pEntitiesInFov)
		if (e.EntityHash == closestItem.ItemHash)
			pInterface->Item_Grab(e, closestItem); //we need to grab it before adding it

	if (pAgent->AddItemToInventory(closestItem, pInterface))
		return  Elite::Success;

	return  Elite::Failure;
}

Elite::BehaviorState AttackEnemy(Elite::Blackboard* pBlackboard)
{
	Agent* pAgent{ nullptr };
	std::vector<EntityInfo>* pEntities{ nullptr };
	IExamInterface* pInterface{ nullptr };
	if (!(pBlackboard->GetData("Agent", pAgent) && pBlackboard->GetData("EntitiesInFov", pEntities) && pBlackboard->GetData("Interface", pInterface))) return Elite::Failure;

	std::vector<EnemyInfo> enemiesInFov{};

	for (const auto& ent : *pEntities)
		if (ent.Type == eEntityType::ENEMY)
		{
			EnemyInfo temp{};
			pInterface->Enemy_GetInfo(ent, temp);
			enemiesInFov.push_back(temp);
		}

	EnemyInfo closestEnemy{};

	for (const auto& enemy : enemiesInFov)
		if (pAgent->DistanceTo(enemy.Location) < pAgent->DistanceTo(closestEnemy.Location))
			closestEnemy = enemy;

	pAgent->AttackEnemy(closestEnemy, pInterface);

	return  Elite::Success;
}

Elite::BehaviorState RunFromEnemies(Elite::Blackboard* pBlackboard)
{
	Agent* pAgent{ nullptr };
	std::vector<EntityInfo>* pEntities{ nullptr };
	IExamInterface* pInterface{ nullptr };
	if (!(pBlackboard->GetData("Agent", pAgent) && pBlackboard->GetData("EntitiesInFov", pEntities) && pBlackboard->GetData("Interface", pInterface))) return Elite::Failure;

	std::vector<EnemyInfo> enemiesInFov{};

	for (const auto& ent : *pEntities)
		if (ent.Type == eEntityType::ENEMY)
		{
			EnemyInfo temp{};
			pInterface->Enemy_GetInfo(ent, temp);
			enemiesInFov.push_back(temp);
		}
	pAgent->RunFromEnemies(enemiesInFov);

	return  Elite::Success;
}

Elite::BehaviorState Run(Elite::Blackboard* pBlackboard)
{
	Agent* pAgent{ nullptr };
	if (!(pBlackboard->GetData("Agent", pAgent))) return Elite::Failure;
	pAgent->SetRunModeOn();
	return Elite::Success;
}

Elite::BehaviorState GrabNotNeededItem(Elite::Blackboard* pBlackboard)
{
	Agent* pAgent{ nullptr };
	IExamInterface* pInterface{ nullptr };
	if (!(pBlackboard->GetData("Agent", pAgent) && pBlackboard->GetData("Interface", pInterface))) return Elite::Failure;
	if (pAgent->AddItemToInventory(pAgent->GetItemInGrabRange(), pInterface)) return Elite::Success;
	return Elite::Failure;
}

Elite::BehaviorState RunFromPurgeZone(Elite::Blackboard* pBlackboard)
{
	Agent* pAgent{ nullptr };
	if (!(pBlackboard->GetData("Agent", pAgent))) return Elite::Failure;
	pAgent->RunFromPurgeZone();
	return Elite::Success;
}
#pragma endregion

#endif