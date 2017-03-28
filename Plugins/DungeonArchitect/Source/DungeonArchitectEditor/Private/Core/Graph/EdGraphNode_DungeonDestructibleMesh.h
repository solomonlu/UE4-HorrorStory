//$ Copyright 2015 Ali Akbar, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//
#pragma once
#include "EdGraphNode_DungeonActorBase.h"
#include "Engine/DestructibleMesh.h"
#include "EdGraphNode_DungeonDestructibleMesh.generated.h"


UCLASS()
class DUNGEONARCHITECTEDITOR_API UEdGraphNode_DungeonDestructibleMesh : public UEdGraphNode_DungeonActorBase {
	GENERATED_UCLASS_BODY()

public:
	virtual UObject* GetNodeAssetObject(UObject* Outer) override;
	virtual UObject* GetThumbnailAssetObject() override;
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Dungeon)
	UDestructibleMesh* DestructibleMesh;

};
