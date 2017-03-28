//$ Copyright 2015 Ali Akbar, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//


#pragma once
#include "../Dungeon.h"
#include "DungeonVolume.h"
#include "DungeonMirrorVolume.generated.h"

/**
*
*/
UCLASS(Blueprintable)
class DUNGEONARCHITECTRUNTIME_API ADungeonMirrorVolume : public ADungeonVolume
{
	GENERATED_BODY()

public:
	ADungeonMirrorVolume(const FObjectInitializer& ObjectInitializer);

};
