//$ Copyright 2015 Ali Akbar, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//
#include "DungeonArchitectRuntimePrivatePCH.h"
#include "GridDungeonConfig.h"

DEFINE_LOG_CATEGORY(GridDungeonConfigLog);

UGridDungeonConfig::UGridDungeonConfig(const FObjectInitializer& ObjectInitializer) :
	Super(ObjectInitializer),
	NumCells(100),
	GridCellSize(FVector(400, 400, 200)),
	MinCellSize(2),
	MaxCellSize(5),
	RoomAreaThreshold(15),
	RoomAspectDelta(0.4f),
	SpanningTreeLoopProbability(0.15f),
    StairConnectionTollerance(3),
    DoorProximitySteps(2),
	HeightVariationProbability(0.2f),
	NormalMean(0),
	NormalStd(0.3f),
	MaxAllowedStairHeight(2),
	LaneWidth(2),
	FloorHeight(0),
	InitialRoomRadius(15)
{

}
