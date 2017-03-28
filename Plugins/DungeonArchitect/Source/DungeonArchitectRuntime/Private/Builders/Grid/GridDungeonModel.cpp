//$ Copyright 2015 Ali Akbar, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//
#include "DungeonArchitectRuntimePrivatePCH.h"
#include "GridDungeonModel.h"

void UGridDungeonModel::BuildCellLookup()
 {
	CellLookup.Reset();
	FCell* CellArray = Cells.GetData();
	 for (int i = 0; i < Cells.Num(); i++) {
		 FCell* cell = CellArray + i;
		 if (cell) {
			 CellLookup.Add(cell->Id, cell);
		 }
	 }
}

bool UGridDungeonModel::ContainsStairAtLocation(int x, int y)
{
	TArray<TArray<FStairInfo>> StairLists;
	CellStairs.GenerateValueArray(StairLists);

	for (const auto& StairList : StairLists)
	{
		for (const auto& Stair : StairList)
		{
			if (Stair.IPosition.X == x && Stair.IPosition.Y == y)
			{
				return true;
			}
		}
	}
	return false;
}

FGridCellInfo UGridDungeonModel::GetGridCellLookup(int x, int y) const
{
	if (!GridCellInfoLookup.Contains(x) || !GridCellInfoLookup[x].Contains(y)) {
		return FGridCellInfo();
	}
	return GridCellInfoLookup[x][y];
}

void UGridDungeonModel::Cleanup()
{
	CellLookup.Reset();
}

void UGridDungeonModel::Reset()
{
	Cells.Reset();
	Doors.Reset();
	CellStairs.Reset();
	CellLookup.Reset();
	GridCellInfoLookup.Reset();
	DoorManager = FDoorManager();
	BuildState = DungeonModelBuildState::Initial;
}

bool operator==(const FCellDoor& A, const FCellDoor& B)
{
	return A.AdjacentCells[0] == B.AdjacentCells[0]
		&& A.AdjacentCells[1] == B.AdjacentCells[1]
		&& A.AdjacentTiles[0] == B.AdjacentTiles[0]
		&& A.AdjacentTiles[1] == B.AdjacentTiles[1]
		;
	;
}
