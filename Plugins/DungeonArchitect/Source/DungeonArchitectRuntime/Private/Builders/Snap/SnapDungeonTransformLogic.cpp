//$ Copyright 2015 Ali Akbar, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "DungeonArchitectRuntimePrivatePCH.h"
#include "SnapDungeonTransformLogic.h"
#include "SnapDungeonModel.h"

void USnapDungeonTransformLogic::GetNodeOffset_Implementation(USnapDungeonModel* Model, FTransform& Offset)
{
	Offset = FTransform::Identity;
}