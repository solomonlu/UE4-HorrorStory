//$ Copyright 2015 Ali Akbar, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "IsaacRoomLayoutBuilder.h"
#include "SimpleIsaacRoomLayoutBuilder.generated.h"


UCLASS(Blueprintable)
class DUNGEONARCHITECTRUNTIME_API USimpleIsaacRoomLayoutBuilder : public UIsaacRoomLayoutBuilder {
	GENERATED_BODY()
public:
	virtual FIsaacRoomLayout GenerateLayout(FIsaacRoomPtr room, FRandomStream& random, int roomWidth, int roomHeight) override;

};

