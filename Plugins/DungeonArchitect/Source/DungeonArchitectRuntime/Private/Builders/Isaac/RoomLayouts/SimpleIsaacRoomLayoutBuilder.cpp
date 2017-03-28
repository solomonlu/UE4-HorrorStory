//$ Copyright 2015 Ali Akbar, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "DungeonArchitectRuntimePrivatePCH.h"
#include "SimpleIsaacRoomLayoutBuilder.h"


FIsaacRoomLayout USimpleIsaacRoomLayoutBuilder::GenerateLayout(FIsaacRoomPtr room, FRandomStream& random, int roomWidth, int roomHeight)
{
	return Super::GenerateLayout(room, random, roomWidth, roomHeight);
}

