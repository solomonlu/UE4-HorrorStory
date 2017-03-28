//$ Copyright 2015 Ali Akbar, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "DungeonArchitectRuntimePrivatePCH.h"
#include "GridDungeonTransformLogic.h"

void UGridDungeonTransformLogic::GetNodeOffset_Implementation(UGridDungeonModel* Model, UGridDungeonConfig* Config, const FCell& Cell, const FRandomStream& RandomStream, int32 GridX, int32 GridY, FTransform& Offset)
{
	Offset = FTransform::Identity;
}
