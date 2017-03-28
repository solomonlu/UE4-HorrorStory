//$ Copyright 2015 Ali Akbar, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//
#pragma once
#include "Core/Dungeon.h"
#include "GridDungeonEdToolData.generated.h"

UCLASS()
class UGridDungeonEdToolData : public UObject {
	GENERATED_BODY()
public:
	UGridDungeonEdToolData(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon Edit Mode")
	int32 FloorIndex;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon Edit Mode")
	int32 BrushSize;

};