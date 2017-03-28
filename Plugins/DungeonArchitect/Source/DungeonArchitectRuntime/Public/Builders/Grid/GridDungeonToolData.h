//$ Copyright 2015 Ali Akbar, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//
#pragma once
#include "Core/DungeonToolData.h"
#include "MathUtils.h"
#include "GridDungeonToolData.generated.h"


UCLASS(Blueprintable)
class DUNGEONARCHITECTRUNTIME_API UGridDungeonToolData : public UDungeonToolData {
	GENERATED_BODY()

public:

	// The cells painted by the "Paint" tool
	UPROPERTY()
	TArray<FIntVector> PaintedCells;

	// The platform rectangles defined in the scene using the "Rectangle" tool
	UPROPERTY()
	TArray<FRectangle> Rectangles;

	// The platform borders defined in the scene using the "Border" tool
	UPROPERTY()
	TArray<FRectangle> Borders;
};
