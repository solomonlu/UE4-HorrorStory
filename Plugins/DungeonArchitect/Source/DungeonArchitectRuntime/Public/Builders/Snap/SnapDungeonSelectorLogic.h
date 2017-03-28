//$ Copyright 2015 Ali Akbar, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//


#pragma once
#include "SnapDungeonModel.h"
#include "Core/Rules/DungeonSelectorLogic.h"
#include "SnapDungeonSelectorLogic.generated.h"


/**
*
*/
UCLASS(Blueprintable, HideDropDown)
class DUNGEONARCHITECTRUNTIME_API USnapDungeonSelectorLogic : public UDungeonSelectorLogic
{
	GENERATED_BODY()

public:

	/** called when something enters the sphere component */
	UFUNCTION(BlueprintNativeEvent, Category = "Dungeon")
	bool SelectNode(USnapDungeonModel* Model);
	virtual bool SelectNode_Implementation(USnapDungeonModel* Model);


};
