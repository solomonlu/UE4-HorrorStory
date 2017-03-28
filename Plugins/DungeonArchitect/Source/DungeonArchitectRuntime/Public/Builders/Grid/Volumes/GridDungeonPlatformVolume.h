//$ Copyright 2015 Ali Akbar, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//


#pragma once
#include "../Dungeon.h"
#include "DungeonVolume.h"
#include "GridDungeonModel.h"
#include "GridDungeonPlatformVolume.generated.h"

/**
*
*/
UCLASS(Blueprintable)
class DUNGEONARCHITECTRUNTIME_API AGridDungeonPlatformVolume : public ADungeonVolume
{
	GENERATED_BODY()

public:
	AGridDungeonPlatformVolume(const FObjectInitializer& ObjectInitializer);

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Dungeon)
	FCellType CellType;		// TODO: abstract this
};
