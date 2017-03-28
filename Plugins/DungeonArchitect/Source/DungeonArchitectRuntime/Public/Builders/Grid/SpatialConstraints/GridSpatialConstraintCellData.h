//$ Copyright 2015 Ali Akbar, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//
#pragma once
#include "GridSpatialConstraintCellData.generated.h"

UENUM(BlueprintType)
enum class EGridSpatialCellOccupation : uint8
{
	DontCare	= 0		UMETA(DisplayName = "Ignore"),
	Occupied	= 1		UMETA(DisplayName = "Occupied"),
	Empty		= 2		UMETA(DisplayName = "Empty")
};

USTRUCT()
struct DUNGEONARCHITECTRUNTIME_API FGridSpatialConstraintCellData {
	GENERATED_USTRUCT_BODY()

	FGridSpatialConstraintCellData() : OccupationConstraint(EGridSpatialCellOccupation::DontCare) {}

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EGridSpatialCellOccupation OccupationConstraint;
};
