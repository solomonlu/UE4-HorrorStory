//$ Copyright 2015 Ali Akbar, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//
#include "DungeonArchitectRuntimePrivatePCH.h"
#include "IsaacDungeonModel.h"

void UIsaacDungeonModel::Cleanup()
{
	Rooms.Reset();
}

void UIsaacDungeonModel::Reset()
{
	Cleanup();
}