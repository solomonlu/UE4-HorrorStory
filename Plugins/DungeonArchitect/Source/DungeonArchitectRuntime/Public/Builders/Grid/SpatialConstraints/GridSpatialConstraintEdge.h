//$ Copyright 2015 Ali Akbar, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//
#pragma once
#include "Core/DungeonSpatialConstraint.h"
#include "GridSpatialConstraintCellData.h"
#include "GridSpatialConstraintEdge.generated.h"

USTRUCT()
struct DUNGEONARCHITECTRUNTIME_API FGridSpatialConstraintEdgeData {
	GENERATED_USTRUCT_BODY()
		FGridSpatialConstraintEdgeData() {
		Cells.SetNum(2);
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TArray<FGridSpatialConstraintCellData> Cells;
};


UCLASS(Blueprintable)
class DUNGEONARCHITECTRUNTIME_API UGridSpatialConstraintEdge : public UDungeonSpatialConstraint {
	GENERATED_UCLASS_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spatial Setup")
	FGridSpatialConstraintEdgeData Configuration;
};
