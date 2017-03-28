//$ Copyright 2015 Ali Akbar, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "DungeonArchitectRuntimePrivatePCH.h"
#include "GridDungeonPlatformVolume.h"

AGridDungeonPlatformVolume::AGridDungeonPlatformVolume(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer),
		CellType(FCellType::Corridor)
{
}
