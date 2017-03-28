//$ Copyright 2015 Ali Akbar, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//
#include "DungeonArchitectRuntimePrivatePCH.h"
#include "SnapDoorAsset.h"

USnapDoorAsset::USnapDoorAsset(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	DoorMesh = ObjectInitializer.CreateDefaultSubobject<USnapDoorMeshInfo>(this, "DoorMesh");
	ClosedMesh = ObjectInitializer.CreateDefaultSubobject<USnapDoorMeshInfo>(this, "ClosedMesh");
}

