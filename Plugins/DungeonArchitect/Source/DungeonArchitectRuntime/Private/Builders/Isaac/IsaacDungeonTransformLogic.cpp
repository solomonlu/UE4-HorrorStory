//$ Copyright 2015 Ali Akbar, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "DungeonArchitectRuntimePrivatePCH.h"
#include "IsaacDungeonTransformLogic.h"
#include "IsaacDungeonModel.h"

void UIsaacDungeonTransformLogic::GetNodeOffset_Implementation(UIsaacDungeonModel* Model, FTransform& Offset)
{
	Offset = FTransform::Identity;
}