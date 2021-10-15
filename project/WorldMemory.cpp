#include "stdafx.h"
#include "WorldMemory.h"

WorldMemory::WorldMemory(const Elite::Vector2& dimensions, const Elite::Vector2& center, const Elite::Vector2& gridResolution)
	:m_GridResolution{ gridResolution }
	, m_WorldDimensions{ dimensions }
	, m_SquareSize{ dimensions.x / gridResolution.x, dimensions.y / gridResolution.y }
{
	m_pSavedHouses = new std::vector<HouseInfo_Extended>{};
	m_pSavedItems = new std::vector<MemorizedItem>{};
	InitializeGrid();
}

void WorldMemory::InitializeGrid()
{
	auto worldBot{ -m_WorldDimensions.y / 2 }
		, worldTop{ m_WorldDimensions.y / 2 }
		, worldLeft{ -m_WorldDimensions.x / 2 }
	, worldRight{ m_WorldDimensions.x / 2 };

	for (size_t idxRow{}; idxRow < m_GridResolution.y; ++idxRow)
	{
		Elite::Vector2 gridPoint{};
		//remap value from 0 -> world dimension to -dimension / 2 -> dimension / 2 because the center of the world is 0,0
		//low2 + (value - low1) * (high2 - low2) / (high1 - low1)
		gridPoint.y = worldBot + (m_SquareSize.y * idxRow) * (worldTop - worldBot) / m_WorldDimensions.y;
		for (size_t idxColumn{}; idxColumn < m_GridResolution.x; ++idxColumn)
		{
			gridPoint.x = worldLeft + (m_SquareSize.x * idxColumn) * (worldRight - worldLeft) / m_WorldDimensions.x;
			m_WorldGrid.push_back({ idxColumn, idxRow, gridPoint });
		}
	}
}

size_t WorldMemory::GetGridSqrIdxAtPos(const Elite::Vector2& pos) const
{
	for (size_t idxRow{}; idxRow < m_GridResolution.y; ++idxRow)
		for (size_t idxColumn{}; idxColumn < m_GridResolution.x; ++idxColumn)
		{
			auto idx{ idxColumn + (idxRow * size_t(m_GridResolution.x)) };
			auto gridSquareBotLeft{ m_WorldGrid[idx].botLeft };

			if (pos.x >= gridSquareBotLeft.x
				&& pos.x <= gridSquareBotLeft.x + m_SquareSize.x
				&& pos.y >= gridSquareBotLeft.y
				&& pos.y <= gridSquareBotLeft.y + m_SquareSize.y)
			{
				return idx;
			}
		}
	return -1;
}

void WorldMemory::DrawTrail(IExamInterface* pInterface)
{
	size_t sqrIdx{};
	for (size_t idxRow{}; idxRow < m_GridResolution.y; ++idxRow)
		for (size_t idxColumn{}; idxColumn < m_GridResolution.x; ++idxColumn)
		{
			sqrIdx = idxColumn + (idxRow * size_t(m_GridResolution.x));
			if (m_WorldGrid[idxColumn + (idxRow * size_t(m_GridResolution.x))].hasBeenHere == true
				&& m_WorldGrid[idxColumn + (idxRow * size_t(m_GridResolution.x))].hasHouse == false)
				DrawSquare(pInterface, sqrIdx, { 1, 0, 0 });
		}
}

void WorldMemory::DrawHouses(IExamInterface* pInterface)
{
	size_t sqrIdx{};
	for (size_t idxRow{}; idxRow < m_GridResolution.y; ++idxRow)
		for (size_t idxColumn{}; idxColumn < size_t(m_GridResolution.x); ++idxColumn)
		{
			sqrIdx = idxColumn + (idxRow * size_t(m_GridResolution.x));
			if (m_WorldGrid[idxColumn + (idxRow * size_t(m_GridResolution.x))].hasEntrance)
				DrawSquare(pInterface, sqrIdx, { 1, 1, 0 });
			else if (m_WorldGrid[idxColumn + (idxRow * size_t(m_GridResolution.x))].hasHouseWall)
				DrawSquare(pInterface, sqrIdx, { 1, 1, 1 });
			else if (m_WorldGrid[idxColumn + (idxRow * size_t(m_GridResolution.x))].hasHouse)
				DrawSquare(pInterface, sqrIdx, { 0, 1, 0 });
		}
}

void WorldMemory::DrawItems(IExamInterface* pInterface)
{
	Elite::Vector3 color{};
	for (const auto& item : *m_pSavedItems)
	{
		if (item.itemType == eItemType::FOOD)
			color = { 0, 1, 0 };
		else if (item.itemType == eItemType::MEDKIT)
			color = { 1, 0, 0 };
		else if (item.itemType == eItemType::PISTOL)
			color = { 0, 0, 0 };

		pInterface->Draw_Circle(item.entityInfo.Location, 1.5f, color);
	}
}

void WorldMemory::DrawSquare(IExamInterface* pInterface, size_t squareIndex, const Elite::Vector3& color)
{
	Elite::Vector2 points[4]{};
	points[0] = m_WorldGrid[squareIndex].botLeft;
	points[1] = Elite::Vector2{ m_WorldGrid[squareIndex].botLeft.x + m_SquareSize.x, m_WorldGrid[squareIndex].botLeft.y };
	points[2] = Elite::Vector2{ m_WorldGrid[squareIndex].botLeft.x + m_SquareSize.x, m_WorldGrid[squareIndex].botLeft.y + m_SquareSize.y };
	points[3] = Elite::Vector2{ m_WorldGrid[squareIndex].botLeft.x, m_WorldGrid[squareIndex].botLeft.y + m_SquareSize.y };

	pInterface->Draw_SolidPolygon(&points[0], 4, color);
}

bool WorldMemory::RandomNextSqrTarget(const Elite::Vector2& currentPos, Elite::Vector2& newTarget)
{
	if (m_CurrentExploreTarget != Elite::Vector2{ 0.f, 0.f })
		if (Elite::Distance(currentPos, m_CurrentExploreTarget) > 1.f)
		{
			newTarget = m_CurrentExploreTarget;
			return false;
		}

	auto sqrIdx{ GetGridSqrIdxAtPos(currentPos) };

	std::vector<size_t> sqrsAroundCurrentSqr{};
	//X = sqrIdx, 0 -> 9 positions in sqrsAroundCurrentSqr
	//3  2  1
	//4  X  0
	//5  6  7

	int radius{ 3 };
	size_t rowIdx{ m_WorldGrid[sqrIdx].row }
	, columnIdx{ m_WorldGrid[sqrIdx].column };

	if (columnIdx != m_GridResolution.x)
	{
		sqrsAroundCurrentSqr.push_back(sqrIdx + radius); // mid right

		if (rowIdx != m_GridResolution.y)
			sqrsAroundCurrentSqr.push_back((columnIdx + radius) + ((rowIdx + radius) * size_t(m_GridResolution.x))); //top right
	}

	if (rowIdx != m_GridResolution.y)
	{
		sqrsAroundCurrentSqr.push_back(columnIdx + ((rowIdx + radius) * size_t(m_GridResolution.x))); //top mid

		if (columnIdx != 0)
			sqrsAroundCurrentSqr.push_back((columnIdx - radius) + ((rowIdx + radius) * size_t(m_GridResolution.x))); //top left
	}

	if (columnIdx != 0)
	{
		sqrsAroundCurrentSqr.push_back(sqrIdx - radius); //mid left
		if (rowIdx != 0)
			sqrsAroundCurrentSqr.push_back((columnIdx - radius) + ((rowIdx - radius) * size_t(m_GridResolution.x))); // bot left
	}

	if (rowIdx != 0)
	{
		sqrsAroundCurrentSqr.push_back(columnIdx + ((rowIdx - radius) * size_t(m_GridResolution.x))); // bot mid
		if (columnIdx != m_GridResolution.x)
			sqrsAroundCurrentSqr.push_back((columnIdx + radius) + ((rowIdx - radius) * size_t(m_GridResolution.x))); //bot right
	}

	std::vector<size_t> posNewSquares;
	for (auto idx : sqrsAroundCurrentSqr)
		if (!m_WorldGrid[idx].hasBeenHere && !m_WorldGrid[idx].hasHouse)
			posNewSquares.push_back(idx);

	if (posNewSquares.size() == 0) 
		return false;

	auto newSqrIdx{ (posNewSquares.size() > 0) ? Elite::randomInt(posNewSquares.size()) : Elite::randomInt(sqrsAroundCurrentSqr.size()) };
	newTarget = GetMidOfSquare(posNewSquares[newSqrIdx]);
	m_CurrentExploreTarget = newTarget;
	m_WorldGrid[sqrIdx].hasBeenHere = true;
	return true;
}

bool WorldMemory::AddHouseToMemory(const HouseInfo* pHouseInfo)
{
	AddHouseToSqrs(pHouseInfo->Center, pHouseInfo->Size);
	return AddHouseToVector(pHouseInfo);
}

void WorldMemory::AddHouseToSqrs(const Elite::Vector2& housePos, const Elite::Vector2& houseSize)
{
	const auto botLeftIdx{ GetGridSqrIdxAtPos(housePos - (houseSize / 2.f)) }
	, topRightIdx{ GetGridSqrIdxAtPos(housePos + (houseSize / 2.f)) };
	//bottom row = walls
	//top row = walls (bot)
	for (size_t r{ m_WorldGrid[botLeftIdx].row }; r <= m_WorldGrid[topRightIdx].row; ++r)
		for (size_t c{ m_WorldGrid[botLeftIdx].column }; c <= m_WorldGrid[topRightIdx].column; ++c)
		{
			if (r == m_WorldGrid[botLeftIdx].row || r == m_WorldGrid[topRightIdx].row
				|| c == m_WorldGrid[botLeftIdx].column || c == m_WorldGrid[topRightIdx].column)
				m_WorldGrid[c + (r * size_t(m_GridResolution.x))].hasHouseWall = true;

			m_WorldGrid[c + (r * size_t(m_GridResolution.x))].hasHouse = true;
		}
}

void WorldMemory::AddHouseEntrance(const Elite::Vector2& entrancePos)
{
	auto sqrIdx{ GetGridSqrIdxAtPos(entrancePos) };
	if (!m_WorldGrid[sqrIdx].hasEntrance)
	{
		const auto houseIdx{ IdxOfHouseAtPosition(entrancePos) };

		const auto botLeftIdx{ GetGridSqrIdxAtPos(m_pSavedHouses->at(houseIdx).Center - (m_pSavedHouses->at(houseIdx).Size / 2.f)) },
			topRightIdx{ GetGridSqrIdxAtPos(m_pSavedHouses->at(houseIdx).Center + (m_pSavedHouses->at(houseIdx).Size / 2.f)) };

		//+1 / -1 because grid and houses are not aligned
		if ((m_WorldGrid[sqrIdx].row == m_WorldGrid[botLeftIdx].row || m_WorldGrid[sqrIdx].row + 1 == m_WorldGrid[botLeftIdx].row || m_WorldGrid[sqrIdx].row - 1 == m_WorldGrid[botLeftIdx].row)
			|| (m_WorldGrid[sqrIdx].row == m_WorldGrid[topRightIdx].row || m_WorldGrid[sqrIdx].row + 1 == m_WorldGrid[topRightIdx].row || m_WorldGrid[sqrIdx].row - 1 == m_WorldGrid[topRightIdx].row))
		{
			m_WorldGrid[GetGridSqrIdxAtPos({ entrancePos.x, entrancePos.y + (m_SquareSize.y / 2) })].hasEntrance = true;
			m_WorldGrid[GetGridSqrIdxAtPos({ entrancePos.x, entrancePos.y - (m_SquareSize.y / 2) })].hasEntrance = true;
		}
		else if ((m_WorldGrid[sqrIdx].column == m_WorldGrid[botLeftIdx].column || m_WorldGrid[sqrIdx].column + 1 == m_WorldGrid[botLeftIdx].column || m_WorldGrid[sqrIdx].column - 1 == m_WorldGrid[botLeftIdx].column)
			|| (m_WorldGrid[sqrIdx].column == m_WorldGrid[topRightIdx].column || m_WorldGrid[sqrIdx].column + 1 == m_WorldGrid[topRightIdx].column || m_WorldGrid[sqrIdx].column - 1 == m_WorldGrid[topRightIdx].column))
		{
			m_WorldGrid[GetGridSqrIdxAtPos({ entrancePos.x + (m_SquareSize.x / 2), entrancePos.y })].hasEntrance = true;
			m_WorldGrid[GetGridSqrIdxAtPos({ entrancePos.x - (m_SquareSize.x / 2), entrancePos.y })].hasEntrance = true;
		}

		m_WorldGrid[sqrIdx].hasEntrance = true;
		m_pSavedHouses->at(houseIdx).entrancePos.push_back(entrancePos);
	}
}

size_t WorldMemory::IdxOfHouseAtPosition(const Elite::Vector2& position) const
{
	size_t closesHouseIdx{ 0 };
	for (size_t idx{}; idx < m_pSavedHouses->size(); ++idx)
		if (Elite::Distance(position, m_pSavedHouses->at(idx).Center) <= Elite::Distance(position, m_pSavedHouses->at(closesHouseIdx).Center))
			closesHouseIdx = idx;

	return closesHouseIdx;
}

void  WorldMemory::AddItemToMemory(const ItemInfo& itemInfo, IExamInterface* pInterface)
{
	auto it{ std::find_if(m_pSavedItems->begin(), m_pSavedItems->end(), [itemInfo](const MemorizedItem& itemInVector) {
		return (itemInVector.entityInfo.ItemHash == itemInfo.ItemHash);
		}) };

	if (it == m_pSavedItems->end())
	{
		MemorizedItem tempMemItem{};
		tempMemItem.entityInfo = itemInfo;
		tempMemItem.itemType = itemInfo.Type;
		m_pSavedItems->push_back(tempMemItem);
	}
}

bool WorldMemory::AddHouseToVector(const HouseInfo* pHouseInfo)
{
	auto it{ std::find_if(m_pSavedHouses->begin(), m_pSavedHouses->end(), [pHouseInfo](const HouseInfo_Extended& house) {
		if (pHouseInfo->Center == house.Center &&
			pHouseInfo->Size == house.Size)
			return true;
		return false;
		}) };

	if (it == m_pSavedHouses->end())
	{
		HouseInfo_Extended tempHouse{};
		tempHouse.Center = pHouseInfo->Center;
		tempHouse.Size = pHouseInfo->Size;
		m_pSavedHouses->push_back(tempHouse);
		m_pSavedHouses->at(m_pSavedHouses->size() - 1).houseIdx = m_pSavedHouses->size() - 1;
		return true;
	}
	return false;
}

void WorldMemory::GetClosestItemPosition(const Elite::Vector2& position, const eItemType& itemType, Elite::Vector2& closestItemPosition)
{
	std::vector<Elite::Vector2> allItemPositions{};
	for (const auto& item : *m_pSavedItems)
		if (item.itemType == itemType)
			allItemPositions.push_back(item.entityInfo.Location);

	Elite::Vector2 closestPos{};

	for (const auto& pos : allItemPositions)
		if (closestPos == Elite::Vector2{ 0.f, 0.f } ||
			Elite::Distance(position, pos) < Elite::Distance(pos, closestPos))
			closestPos = pos;

	closestItemPosition = closestPos;
}

bool WorldMemory::GetClosestItem(const Elite::Vector2& position, const eItemType& itemType, ItemInfo& closestItem)
{
	if (m_pSavedItems->size() == 0) return false;

	std::vector<ItemInfo> allItemsOfType{};
	for (const auto& item : *m_pSavedItems)
		if (item.itemType == itemType)
			allItemsOfType.push_back(item.entityInfo);

	if (allItemsOfType.size() == 0) return false;
	ItemInfo tempClosestItem{};

	for (const auto& item : allItemsOfType)
		if (tempClosestItem.Location == Elite::Vector2{ 0.f, 0.f } ||
			Elite::Distance(position, item.Location) < Elite::Distance(item.Location, tempClosestItem.Location))
			tempClosestItem = item;

	closestItem = tempClosestItem;
	return true;
}

bool WorldMemory::HaveItemTypeInMemory(const eItemType& itemType)const
{
	for (const auto& item : *m_pSavedItems)
		if (item.itemType == itemType)
			return true;
	return false;
}

void WorldMemory::RemoveItemFromMemory(const ItemInfo& item)
{
	auto it(std::remove_if(m_pSavedItems->begin(), m_pSavedItems->end(), [item](const MemorizedItem& memoryItem)
		{
			return (memoryItem.entityInfo.ItemHash == item.ItemHash);
		}
	));
	m_pSavedItems->erase(it, m_pSavedItems->end());
}

void WorldMemory::UpdateHasBeenHere(const Elite::Vector2& agentPos)
{
	const auto gridIdx{ GetGridSqrIdxAtPos(agentPos) };
	const auto idxColumn{ m_WorldGrid[gridIdx].column }
	, idxRow{ m_WorldGrid[gridIdx].row };

	m_WorldGrid[gridIdx].hasBeenHere = true;

	if (idxRow > m_GridResolution.y - 1)m_WorldGrid[(idxColumn + 0) + ((idxRow + 1) * size_t(m_GridResolution.x))].hasBeenHere = true;
	if (idxRow > 1) m_WorldGrid[(idxColumn + 0) + ((idxRow - 1) * size_t(m_GridResolution.x))].hasBeenHere = true;

	if (idxColumn < m_GridResolution.x - 1)
	{
		m_WorldGrid[(idxColumn + 1) + ((idxRow + 0) * size_t(m_GridResolution.x))].hasBeenHere = true;
		if (idxRow > m_GridResolution.y - 1)m_WorldGrid[(idxColumn + 1) + ((idxRow + 1) * size_t(m_GridResolution.x))].hasBeenHere = true;
		if (idxRow > 1) m_WorldGrid[(idxColumn + 1) + ((idxRow - 1) * size_t(m_GridResolution.x))].hasBeenHere = true;
	}

	if (idxColumn > 1)
	{
		m_WorldGrid[(idxColumn - 1) + ((idxRow + 0) * size_t(m_GridResolution.x))].hasBeenHere = true;
		if (idxRow > m_GridResolution.y - 1)m_WorldGrid[(idxColumn - 1) + ((idxRow + 1) * size_t(m_GridResolution.x))].hasBeenHere = true;
		if (idxRow > 1) m_WorldGrid[(idxColumn - 1) + ((idxRow - 1) * size_t(m_GridResolution.x))].hasBeenHere = true;
	}
}