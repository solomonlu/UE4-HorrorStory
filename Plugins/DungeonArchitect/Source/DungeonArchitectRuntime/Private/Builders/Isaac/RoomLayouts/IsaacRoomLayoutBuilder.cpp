//$ Copyright 2015 Ali Akbar, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "DungeonArchitectRuntimePrivatePCH.h"
#include "IsaacRoomLayoutBuilder.h"

FIsaacRoomLayout UIsaacRoomLayoutBuilder::GenerateLayout(FIsaacRoomPtr room, FRandomStream& random, int roomWidth, int roomHeight)
{
	FIsaacRoomLayout layout;
	layout.Initialize(roomWidth, roomHeight, EIsaacRoomTileType::Floor);
	return layout;
}
