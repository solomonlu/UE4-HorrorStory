//$ Copyright 2015 Ali Akbar, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//
#pragma once
#include "EdGraphNode_DungeonActorBase.h"
#include "EdGraphNode_DungeonParticleSystem.generated.h"

UCLASS()
class DUNGEONARCHITECTEDITOR_API UEdGraphNode_DungeonParticleSystem : public UEdGraphNode_DungeonActorBase {
	GENERATED_BODY()

public:
	virtual UObject* GetNodeAssetObject(UObject* Outer) override { return ParticleSystem; }

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Dungeon)
	UParticleSystem* ParticleSystem;
};
