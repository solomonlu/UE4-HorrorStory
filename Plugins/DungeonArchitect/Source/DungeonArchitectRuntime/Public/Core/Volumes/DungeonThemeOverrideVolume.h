//$ Copyright 2015 Ali Akbar, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//


#pragma once
#include "../Dungeon.h"
#include "DungeonVolume.h"
#include "DungeonThemeAsset.h"
#include "DungeonThemeOverrideVolume.generated.h"

/**
*
*/
UCLASS(Blueprintable)
class DUNGEONARCHITECTRUNTIME_API ADungeonThemeOverrideVolume : public ADungeonVolume
{
	GENERATED_BODY()

public:
	ADungeonThemeOverrideVolume(const FObjectInitializer& ObjectInitializer);

public:
	/** Indicates if the bounds of this volume should be reversed, i.e. everything outside this volume is overridden */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Dungeon)
	bool Reversed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Dungeon)
	UDungeonThemeAsset* ThemeOverride;
};
