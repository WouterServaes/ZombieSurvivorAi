#pragma once
#include "IExamInterface.h"
#include "SteeringBehaviors.h"
#include "WorldMemory.h"

class Agent
{
public:
	Agent(const Elite::Vector2& spawnPos, const IExamInterface* pInterface);
	~Agent();
	void Update(float deltaT, IExamInterface* pInterface);
	void Render(IExamInterface* pInterface, bool renderDebug)const;

	//=== agent info
	void SetAgentInfo(AgentInfo pAgentInfo) { m_AgentInfo = pAgentInfo; };
	void SetBTStructure(Elite::IDecisionMaking* pBTStructure) { m_pBTStructure = pBTStructure; };
	AgentInfo GetAgentInfo() const { return m_AgentInfo; };

	//=== world memory
	//houses
	void AddHouseToMemory(const HouseInfo& house);
	std::vector<HouseInfo_Extended>* GetSavedHouses() const { return m_pWorldMemory->GetSavedHouses(); };
	Elite::Vector2 GetNextHousePos();
	Elite::Vector2 GetHouseEntracePos() const;
	bool GetNewHouse() const { return m_NewHouse || m_ExploringNewHouse; };
	int GetMinAmountHousesToExplore()const { return m_MinHousesToDiscover; };
	bool GetIsExploringHouse()const { return m_IsExploringHouse; };
	bool GetCanLeaveHouse()const { return m_WasInHouse; };
	//items
	void AddItemToMemory(const ItemInfo& itemInfo, IExamInterface* pInterface) { m_pWorldMemory->AddItemToMemory(itemInfo, pInterface); };
	void GetClosestItemPosition(const eItemType& itemType, Elite::Vector2& closestItemPos) { m_pWorldMemory->GetClosestItemPosition(m_AgentInfo.Position, itemType, closestItemPos); };
	bool GetClosestItem(const eItemType& itemType, ItemInfo& closestItem) { return m_pWorldMemory->GetClosestItem(m_AgentInfo.Position, itemType, closestItem); };
	bool HaveItemsInMemory() const { return m_pWorldMemory->HaveItemsInMemory(); };
	bool NeedItem()const { return (m_NeedItem || (m_Inventory.size() < m_InvCapacity)); };
	eItemType NeededItemType() const { return m_NeededItem; };
	bool NeededItemInMemory() const;
	bool AddItemToInventory(const ItemInfo& item, IExamInterface* pInterface);
	void SetItemInGrabRange(const ItemInfo& item) { m_ItemInGrabRange = item; };
	ItemInfo& GetItemInGrabRange() { return m_ItemInGrabRange; };
	bool ShouldPickUpNotNeededItem();

	//=== steering
	SteeringPlugin_Output* CalcSteering();

	//=== set current behavior to different behavior
	void SetToSeek(const Elite::Vector2& seekPos);
	void SetToFlee(const Elite::Vector2& fleePos);
	void SetToWander() { m_pCurrentBehavior = m_pWander; };
	void ExploreWorldGrid();

	//=== enemies
	//attack
	bool CanAttack()const { return m_HasGun && m_AgentInfo.Health > m_StartHp/2.f; };
	void AttackEnemy(const EnemyInfo& enemy, IExamInterface* pInterface);
	//run
	void RunFromEnemies(const std::vector<EnemyInfo>& enemies);
	void SetRunModeOn();
	bool IsRunningFromEnemies()const { return m_pSteeringOutput->RunMode; };

	//=== purgeZone
	void SetCurrentPurgeZone(const PurgeZoneInfo& Pz) { m_pPurgeZone = new PurgeZoneInfo(Pz); };
	void NotInPurgeZone() { SAFE_DELETE( m_pPurgeZone); };
	const PurgeZoneInfo* GetCurrentPurgeZoneInfo() const { return m_pPurgeZone; };
	void RunFromPurgeZone();

	//=== no category
	float DistanceTo(const Elite::Vector2& other) { return Elite::Distance(m_AgentInfo.Position, other); }

private:
	//=== draw world memory (for debug)
	void DrawHouseMemory(IExamInterface* pInterface) const { m_pWorldMemory->DrawHouses(pInterface); };
	void DrawTrailMemory(IExamInterface* pInterface) const { m_pWorldMemory->DrawTrail(pInterface); };
	void DrawItemMemory(IExamInterface* pInterface) const { m_pWorldMemory->DrawItems(pInterface); };

	//== agent info
	Elite::Vector3 m_AgentColor{ 1.f, 0.f, 0.f };
	AgentInfo m_AgentInfo{};
	Elite::IDecisionMaking* m_pBTStructure{ nullptr };
	eItemType m_NeededItem{ eItemType::PISTOL };
	bool m_NeedItem{ false };

	//==houses
	void WasInHouse();
	WorldMemory* m_pWorldMemory{ nullptr };
	bool m_WasInHouse{ false },
		m_NewHouse{ false },
		m_ExploringNewHouse{ false };
	size_t m_CurrentHouseTargetIdx{ 0 };
	int m_MinHousesToDiscover{ 4 };

	Elite::Vector2 m_ExploreDirection{ 1.f, 1.f };

	//=== steering
	SteeringPlugin_Output* m_pSteeringOutput{ nullptr };
	SteeringBehaviors* m_pCurrentBehavior{ nullptr };

	//-- steering behaviors
	SteeringBehaviors* m_pSeek{ nullptr };
	SteeringBehaviors* m_pFlee{ nullptr };
	SteeringBehaviors* m_pWander{ nullptr };
	SteeringBehaviors* m_pFace{ nullptr };

	//-- house exploring
	float m_MaxExploreTime{ 1.f }, m_ElapsedExploreTime{};
	bool m_IsExploringHouse{ false };
	void ExploreHouseTimer(float deltaT);
	Elite::Vector2 m_CurrentHouseCenter{};

	//== items
	std::vector<ItemInfo> m_Inventory;
	float m_StartHp, m_StartEnergy;
	bool m_HasGun{ false };
	int m_InvCapacity;
	void UseMedkit(IExamInterface* pInterface);
	void UseFood(IExamInterface* pInterface);
	void NeedPistol(IExamInterface* pInterface);
	void RemoveEmptyGuns(IExamInterface* pInterface);
	ItemInfo m_ItemInGrabRange{};



	//purge
	PurgeZoneInfo *m_pPurgeZone{};
};