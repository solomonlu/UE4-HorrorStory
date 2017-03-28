//$ Copyright 2015 Ali Akbar, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//
#pragma once
#include "Core/DungeonModel.h"
#include "SnapDungeonLayoutNode.h"
#include "SnapDungeonModel.generated.h"

class USnapModule;
class ASnapDoor;
class USnapDungeonLayoutNode;

UCLASS(Blueprintable)
class DUNGEONARCHITECTRUNTIME_API USnapDungeonModel : public UDungeonModel
{
	GENERATED_UCLASS_BODY()
public:

	virtual void Cleanup() override;
	virtual void Reset() override;

public:
	UPROPERTY()
	TArray<USnapModule*> SnapModules;

	UPROPERTY()
	TArray<USnapModule*> StartSnapModules;

	UPROPERTY()
	TArray<USnapModule*> EndSnapModules;

	UPROPERTY()
	TArray<USnapModule*> BranchEndSnapModules;

	UPROPERTY()
	USnapDungeonLayoutNode* LayoutRootNode;
};
