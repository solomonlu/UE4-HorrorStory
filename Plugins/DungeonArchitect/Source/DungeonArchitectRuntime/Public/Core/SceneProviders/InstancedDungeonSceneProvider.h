//$ Copyright 2015 Ali Akbar, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//
#pragma once
#include "PooledDungeonSceneProvider.h"

class ADungeonInstancedMeshActor;

struct InstanceMeshContext {
    InstanceMeshContext() : InstancedActor(nullptr) {}
    ADungeonInstancedMeshActor* InstancedActor;
	ADungeon* Dungeon;
};


/** Pools the actors in the scene and reuses them if possible, while rebuilding */
class DUNGEONARCHITECTRUNTIME_API FInstancedDungeonSceneProvider : public FPooledDungeonSceneProvider {
public:
    FInstancedDungeonSceneProvider(ADungeon* pDungeon, UWorld* pWorld) : FPooledDungeonSceneProvider(pDungeon, pWorld) { }
    virtual ~FInstancedDungeonSceneProvider() {}
	virtual void OnDungeonBuildStart(UDungeonConfig* Config) override;
    virtual void OnDungeonBuildStop() override;
    virtual void AddStaticMesh(UDungeonMesh* Mesh, const FTransform& transform, const FName& NodeId) override;
    
private:
    TSharedPtr<InstanceMeshContext> Context;
    
};

