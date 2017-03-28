//$ Copyright 2015 Ali Akbar, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//


#pragma once
#include "GridDungeonModel.h"
#include "Core/Rules/DungeonSelectorLogic.h"
#include "GridDungeonSelectorLogic.generated.h"


/**
*
*/
UCLASS(Blueprintable, HideDropDown)
class DUNGEONARCHITECTRUNTIME_API UGridDungeonSelectorLogic : public UDungeonSelectorLogic
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = Dungeon)
	bool IsOnCorner(UGridDungeonModel* Model, int32 GridX, int32 GridY);

	UFUNCTION(BlueprintCallable, Category = Dungeon)
	bool IsPillarOnCorner(UGridDungeonModel* Model, int32 GridX, int32 GridY, FTransform& OutCornerOffset);

	UFUNCTION(BlueprintCallable, Category = Dungeon)
	bool IsPassageTooNarrow(UGridDungeonModel* Model, int32 GridX, int32 GridY);

	UFUNCTION(BlueprintCallable, Category = Dungeon)
	bool ContainsStair(UGridDungeonModel* Model, const FCell& Cell, int32 GridX, int32 GridY);

	/** called when something enters the sphere component */
	UFUNCTION(BlueprintNativeEvent, Category = "Dungeon")
	bool SelectNode(UGridDungeonModel* Model, UGridDungeonConfig* Config, const FCell& Cell, const FRandomStream& RandomStream, int32 GridX, int32 GridY);
	virtual bool SelectNode_Implementation(UGridDungeonModel* Model, UGridDungeonConfig* Config, const FCell& Cell, const FRandomStream& RandomStream, int32 GridX, int32 GridY);

private:
	FCell* GetCell(UGridDungeonModel* Model, int32 GridX, int32 GridY);
	bool IsDifferentCell(FCell* Cell0, FCell* Cell1);

private:
	

};
