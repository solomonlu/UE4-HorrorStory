//$ Copyright 2015 Ali Akbar, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "DungeonArchitectRuntimePrivatePCH.h"
#include "FloorPlanTransformLogic.h"
#include "FloorPlanModel.h"

void UFloorPlanTransformLogic::GetNodeOffset_Implementation(UFloorPlanModel* Model, UFloorPlanConfig* Config, const FRandomStream& RandomStream, int32 GridX, int32 GridY, FTransform& Offset)
{
	Offset = FTransform::Identity;
}