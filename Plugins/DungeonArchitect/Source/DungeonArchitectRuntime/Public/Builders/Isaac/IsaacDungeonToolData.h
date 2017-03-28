//$ Copyright 2015 Ali Akbar, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//
#pragma once
#include "Core/DungeonToolData.h"
#include "IsaacDungeonToolData.generated.h"


UCLASS(Blueprintable)
class DUNGEONARCHITECTRUNTIME_API UIsaacDungeonToolData : public UDungeonToolData {
	GENERATED_BODY()

public:

	// The cells painted by the "Paint" tool
	UPROPERTY()
	TArray<FIntVector> PaintedCells;

};
