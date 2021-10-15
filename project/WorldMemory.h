#pragma once
#include "IExamInterface.h"

struct HouseInfo_Extended :HouseInfo
{
	std::vector<Elite::Vector2> entrancePos{};
	size_t houseIdx{};
};


class WorldMemory
{
public:
	WorldMemory(const Elite::Vector2& dimensions, const Elite::Vector2& center, const Elite::Vector2& gridResolution);
	~WorldMemory()
	{
		SAFE_DELETE(m_pSavedHouses);
		SAFE_DELETE(m_pSavedItems);
	};

	//===========exploration of grid
	bool RandomNextSqrTarget(const Elite::Vector2& currentPos, Elite::Vector2& newTarget);	//sets m_CurrentExploreTarget to the middle of a surrounding square that has not yet been explored
		
	//===========memory get/set
	//houses
	bool AddHouseToMemory(const HouseInfo* pHouseInfo); //tries to add house to memory
	void AddHouseEntrance(const Elite::Vector2& entrancePos); //adds an entrance to the house and marks this entrance on the grid
	std::vector<HouseInfo_Extended>* GetSavedHouses() const { return m_pSavedHouses; }; //get all saved houses
	const HouseInfo_Extended* GetHouseAtIndex(size_t idx) { return &m_pSavedHouses->at(idx); };
	const HouseInfo_Extended* GetHouseAtPosition(const Elite::Vector2& position) { return GetHouseAtIndex(IdxOfHouseAtPosition(position)); };
	int GetAmountOfSavedHouses() {return m_pSavedHouses->size();};
	//items
	void AddItemToMemory(const ItemInfo& itemInfo, IExamInterface* pInterface);	//add an item to memory
	void GetClosestItemPosition(const Elite::Vector2& position, const eItemType& itemType, Elite::Vector2& closestItemPosition); //get the location of the closest item of some item type
	bool GetClosestItem(const Elite::Vector2& position, const eItemType& itemType, ItemInfo& closestItem);
	bool HaveItemsInMemory() const { return m_pSavedItems->size(); };
	bool HaveItemTypeInMemory(const eItemType& itemType)const;
	void RemoveItemFromMemory(const ItemInfo& item);
	//exploration
	void WorldMemory::UpdateHasBeenHere(const Elite::Vector2& agentPos);
	//===========drawing
	void DrawTrail(IExamInterface* pInterface);  //draw all the squares the agent has explored
	void DrawHouses(IExamInterface* pInterface); //draw all squares makred with a house
	void DrawItems(IExamInterface* pInterface); //draws circles around items
private:
	//=========== 
	size_t IdxOfHouseAtPosition(const Elite::Vector2& position) const; //get the index of a house from the house vector 
	void InitializeGrid(); //initializes the grid

	//=========== no category
	size_t GetGridSqrIdxAtPos(const Elite::Vector2& pos) const; //get the idx in the m_WorldGrid from a position in the world
	void DrawSquare(IExamInterface* pInterface, size_t squareIndex, const Elite::Vector3& color); //fills a square in a color
	Elite::Vector2 GetMidOfSquare(size_t idx) const { //get the mid position of a square
		return m_WorldGrid[idx].botLeft + (m_SquareSize / 2.f);
	};

	//===========memory
	bool AddHouseToVector(const HouseInfo* pHouseInfo); //adds a house to the house vector if the house isnt in there 
	void AddHouseToSqrs(const Elite::Vector2& housePos, const Elite::Vector2& houseSize); //marks the grid on the place of the house

	//===========
	
	struct GridSquare //represent a square in the grid
	{
		size_t column, row;	//the column idx / row idx of this square
		Elite::Vector2 botLeft;	//the bottom left position of this square
		bool hasBeenHere{ false };	//has the agent been here
		bool hasHouse{ false };	//does this square contain a house
		bool hasEntrance{ false };	//does this square contain an entrance
		bool hasHouseWall{ false };
	};

	struct MemorizedItem
	{
		ItemInfo entityInfo{};
		eItemType itemType{};
	};
	std::vector<GridSquare> m_WorldGrid{};	//world grid vector

	const Elite::Vector2 m_WorldDimensions //dimensions of the world
		, m_SquareSize //size of one square in the grid
		, m_GridResolution; //resolution of the grid (amount of columns/rows)


	Elite::Vector2 m_CurrentExploreTarget{0.f, 0.f}; //current exploration target, not always the agent's target

	//== saved items/houses
	std::vector<MemorizedItem>* m_pSavedItems{ nullptr }; //all known non-garbage items in the world
	std::vector<HouseInfo_Extended>* m_pSavedHouses{nullptr}; //all known houses in the world
};

