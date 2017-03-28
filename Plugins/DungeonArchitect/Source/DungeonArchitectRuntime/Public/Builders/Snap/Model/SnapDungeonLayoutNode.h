//$ Copyright 2015 Ali Akbar, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//
#pragma once

#include "SnapModule.h"
//#include "SnapDungeonLayoutLink.h"
#include "SnapDungeonLayoutNode.generated.h"


UENUM()
enum class ESnapDungeonLayoutLink : uint8 {
	MainBranch = 0		UMETA(DisplayName = "Main Branch"),
	SideBranch = 1		UMETA(DisplayName = "Side Branch"),
};


USTRUCT()
struct DUNGEONARCHITECTRUNTIME_API FSnapDungeonLayoutLink
{
	GENERATED_BODY()

public:
	UPROPERTY()
	FTransform DoorTransform;

	UPROPERTY()
	ESnapDungeonLayoutLink LinkType;
};

UCLASS(Blueprintable)
class DUNGEONARCHITECTRUNTIME_API USnapDungeonLayoutNode : public UObject
{
	GENERATED_UCLASS_BODY()

public:
	//UPROPERTY()
	//USnapModule* Module;

	UPROPERTY()
	FTransform ModuleTransform;

	UPROPERTY()
	FBox Bounds;

	UPROPERTY()
	TArray<FSnapDungeonLayoutLink> Links;

	UPROPERTY()
	TArray<USnapDungeonLayoutNode*> LinkedNodes;
};
