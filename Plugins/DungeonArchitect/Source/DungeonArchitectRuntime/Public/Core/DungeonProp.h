//$ Copyright 2015 Ali Akbar, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//
#pragma once
#include "Rules/DungeonSelectorLogic.h"
#include "Rules/DungeonTransformLogic.h"
#include "DungeonSpatialConstraint.h"
#include "DungeonProp.generated.h"

USTRUCT(Blueprintable)
struct DUNGEONARCHITECTRUNTIME_API FPropSocket {
	GENERATED_USTRUCT_BODY()

	int32 Id;
	FString SocketType;
	FTransform Transform;
	bool IsConsumed;


	FORCEINLINE bool operator==(const FPropSocket& other) const {
		return other.Id == Id;
	}
};

/** Props can emit new sockets when they are inserted, to add more child props relative to them */
USTRUCT(Blueprintable)
struct DUNGEONARCHITECTRUNTIME_API FPropChildSocketData {
	GENERATED_USTRUCT_BODY()

	FPropChildSocketData() : Offset(FTransform::Identity) {}

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = DungeonProp)
	FString SocketType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = DungeonProp)
	FTransform Offset;
};

USTRUCT(Blueprintable)
struct DUNGEONARCHITECTRUNTIME_API FPropTypeData {
	GENERATED_USTRUCT_BODY()

	FPropTypeData() : AttachToSocket(TEXT("None")), Probability(1), bUseSelectionLogic(false), bLogicOverridesAffinity(true), bUseTransformLogic(false), ConsumeOnAttach(false), Offset(FTransform::Identity)
	{
	}
    
    UPROPERTY()
    FName NodeId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = DungeonProp)
	UObject* AssetObject;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = DungeonProp)
	FString AttachToSocket;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = DungeonProp)
	float Probability;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = DungeonProp)
	bool bUseSelectionLogic;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = DungeonProp)
	bool bLogicOverridesAffinity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = DungeonProp)
	TArray<UDungeonSelectorLogic*> SelectionLogics;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = DungeonProp)
	bool bUseTransformLogic;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = DungeonProp)
	TArray<UDungeonTransformLogic*> TransformLogics;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = DungeonProp)
	bool ConsumeOnAttach;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = DungeonProp)
	FTransform Offset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = DungeonProp)
	bool bUseSpatialConstraint;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = DungeonProp)
	UDungeonSpatialConstraint* SpatialConstraint;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = DungeonProp)
	TArray<FPropChildSocketData> ChildSockets;
};

