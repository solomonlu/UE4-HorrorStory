//$ Copyright 2015 Ali Akbar, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//
#pragma once
#include "EdGraphNode_DungeonActorBase.h"
#include "EdGraphNode_DungeonActorTemplate.generated.h"

UCLASS()
class DUNGEONARCHITECTEDITOR_API UEdGraphNode_DungeonActorTemplate : public UEdGraphNode_DungeonActorBase {
	GENERATED_BODY()

public:
	virtual UObject* GetNodeAssetObject(UObject* Outer) override { return ClassTemplate; }

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Dungeon)
	UClass* ClassTemplate;
};
