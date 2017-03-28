//$ Copyright 2015 Ali Akbar, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//


#pragma once
#include "../Dungeon.h"
#include "DungeonVolume.h"
#include "FloorPlanDoorVolume.generated.h"

/**
*
*/
UCLASS(Blueprintable)
class DUNGEONARCHITECTRUNTIME_API AFloorPlanDoorVolume : public ADungeonVolume
{
	GENERATED_UCLASS_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Dungeon)
	FString DoorMarker;
};
