//$ Copyright 2015 Ali Akbar, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "DungeonArchitectRuntimePrivatePCH.h"
#include "FloorPlanRoomVolume.h"

AFloorPlanRoomVolume::AFloorPlanRoomVolume(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, bCreateWalls(true)
	, Priority(100)
{

}

