//$ Copyright 2015 Ali Akbar, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//


#pragma once
#include "GridDungeonModel.h"
#include "Core/Rules/DungeonTransformLogic.h"
#include "GridDungeonTransformLogic.generated.h"

struct FCell;
class UGridDungeonModel;

/**
*
*/
UCLASS(Blueprintable, HideDropDown)
class DUNGEONARCHITECTRUNTIME_API UGridDungeonTransformLogic : public UDungeonTransformLogic
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintNativeEvent, Category = "Dungeon")
	void GetNodeOffset(UGridDungeonModel* Model, UGridDungeonConfig* Config, const FCell& Cell, const FRandomStream& RandomStream, int32 GridX, int32 GridY, FTransform& Offset);
	virtual void GetNodeOffset_Implementation(UGridDungeonModel* Model, UGridDungeonConfig* Config, const FCell& Cell, const FRandomStream& RandomStream, int32 GridX, int32 GridY, FTransform& Offset);

};
