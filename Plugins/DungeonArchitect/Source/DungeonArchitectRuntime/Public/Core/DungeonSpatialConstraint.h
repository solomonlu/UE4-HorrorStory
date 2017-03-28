//$ Copyright 2015 Ali Akbar, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//
#pragma once
#include "DungeonSpatialConstraint.generated.h"

UCLASS(abstract, Blueprintable, DefaultToInstanced, BlueprintType, EditInlineNew)
class DUNGEONARCHITECTRUNTIME_API UDungeonSpatialConstraint : public UObject {
	GENERATED_UCLASS_BODY()
public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Advanced")
	bool bApplyBaseRotation;

	/** Should the constraints be rotated to fit the spatial configuration? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Advanced")
	bool bRotateToFitConstraint;

};
