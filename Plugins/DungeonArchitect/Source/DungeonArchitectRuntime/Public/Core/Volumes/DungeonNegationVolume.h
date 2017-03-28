//$ Copyright 2015 Ali Akbar, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//


#pragma once
#include "../Dungeon.h"
#include "DungeonVolume.h"
#include "DungeonThemeAsset.h"
#include "DungeonNegationVolume.generated.h"

/**
*
*/
UCLASS(Blueprintable)
class DUNGEONARCHITECTRUNTIME_API ADungeonNegationVolume : public ADungeonVolume
{
	GENERATED_BODY()

public:
	ADungeonNegationVolume(const FObjectInitializer& ObjectInitializer);

public:
	/** Indicates if the negation should be reversed, i.e. everything outside this volume is negated */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Dungeon)
	bool Reversed;

	/** Indicates if the volume affects user defined cells (e.g. placed with platform volumes, paint tool etc) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Dungeon)
	bool AffectsUserDefinedCells;

};
