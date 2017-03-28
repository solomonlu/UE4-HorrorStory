//$ Copyright 2015 Ali Akbar, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "DungeonArchitectRuntimePrivatePCH.h"
#include "DungeonNegationVolume.h"
#include "../DungeonModel.h"

ADungeonNegationVolume::ADungeonNegationVolume(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer), Reversed(false), AffectsUserDefinedCells(true)
{
}
