//$ Copyright 2015 Ali Akbar, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//


#pragma once
#include "DungeonMesh.generated.h"

USTRUCT()
struct DUNGEONARCHITECTRUNTIME_API FMaterialOverride {
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Dungeon)
	int32 index;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Dungeon)
	UMaterialInterface* Material;
};

/**
*
*/
UCLASS()
class DUNGEONARCHITECTRUNTIME_API UDungeonMesh : public UObject
{
	GENERATED_UCLASS_BODY()

public:
	UPROPERTY()
	UStaticMesh* StaticMesh;

	UPROPERTY()
	TArray<FMaterialOverride> MaterialOverrides;

	UPROPERTY()
	uint32 HashCode;

	UPROPERTY()
	UStaticMeshComponent* Template;

	void CalculateHashCode();
};

