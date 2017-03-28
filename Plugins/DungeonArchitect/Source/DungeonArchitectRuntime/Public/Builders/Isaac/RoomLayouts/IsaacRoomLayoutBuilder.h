//$ Copyright 2015 Ali Akbar, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "../IsaacDungeonModel.h"
#include "IsaacRoomLayoutBuilder.generated.h"

UCLASS(EditInlineNew, DefaultToInstanced, BlueprintType, Blueprintable, abstract, HideDropDown)
class DUNGEONARCHITECTRUNTIME_API UIsaacRoomLayoutBuilder : public UObject {
	GENERATED_BODY()
public:
	virtual FIsaacRoomLayout GenerateLayout(FIsaacRoomPtr room, FRandomStream& random, int roomWidth, int roomHeight);

};