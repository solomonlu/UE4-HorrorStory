//$ Copyright 2015 Ali Akbar, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//
#pragma once

#include "SnapDoorMeshInfo.h"
#include "SnapDoorAsset.generated.h"

UCLASS(Blueprintable)
class DUNGEONARCHITECTRUNTIME_API USnapDoorAsset : public UObject
{
	GENERATED_UCLASS_BODY()

public:

	UPROPERTY()
	USnapDoorMeshInfo* DoorMesh;

	UPROPERTY()
	USnapDoorMeshInfo* ClosedMesh;

};