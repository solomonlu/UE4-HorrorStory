//$ Copyright 2015 Ali Akbar, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//
#pragma once
#include "EdGraphNode_DungeonActorBase.h"
#include "Core/Actors/DungeonMesh.h"
#include "EdGraphNode_DungeonMesh.generated.h"


UCLASS()
class DUNGEONARCHITECTEDITOR_API UEdGraphNode_DungeonMesh : public UEdGraphNode_DungeonActorBase {
	GENERATED_UCLASS_BODY()

public:
	virtual UObject* GetNodeAssetObject(UObject* Outer) override;
	virtual UObject* GetThumbnailAssetObject() override;

	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& e) override;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Dungeon)
	UStaticMesh* Mesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Dungeon)
	TArray<FMaterialOverride> MaterialOverrides;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh Details")
	UStaticMeshComponent* AdvancedOptions;
};
