//$ Copyright 2015 Ali Akbar, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//
#include "DungeonArchitectRuntimePrivatePCH.h"
#include "SnapDoorMeshInfo.h"

DEFINE_LOG_CATEGORY(LogSnapDoorMeshInfo);

USnapDoorMeshInfo::USnapDoorMeshInfo(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	MeshType = EUSnapDoorMeshType::StaticMesh;
	//StaticMesh = nullptr;
	BlueprintClass = nullptr;
	MeshComponent = ObjectInitializer.CreateDefaultSubobject<UStaticMeshComponent>(this, "StaticMeshComponent");
}

