//$ Copyright 2015 Ali Akbar, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//
#pragma once
#include "DungeonSceneProvider.h"

DECLARE_LOG_CATEGORY_EXTERN(PooledDungeonSceneProvider, Log, All);


/** Pools the actors in the scene and reuses them if possible, while rebuilding */
class DUNGEONARCHITECTRUNTIME_API FPooledDungeonSceneProvider : public FDungeonSceneProvider {
public:
    FPooledDungeonSceneProvider(ADungeon* pDungeon, UWorld* pWorld) : FDungeonSceneProvider(pDungeon), World(pWorld) { }
    virtual ~FPooledDungeonSceneProvider() {}
	virtual void OnDungeonBuildStart(UDungeonConfig* Config) override;
    virtual void OnDungeonBuildStop() override;
    virtual void AddStaticMesh(UDungeonMesh* Mesh, const FTransform& transform, const FName& NodeId) override;
	virtual void AddDestructibleMesh(UDestructibleMesh* DestructibleMesh, const FTransform& transform, const FName& NodeId);
    virtual void AddLight(UPointLightComponent* LightTemplate, const FTransform& transform, const FName& NodeId) override;
    virtual void AddParticleSystem(UParticleSystem* ParticleTemplate, const FTransform& transform, const FName& NodeId) override;
	virtual void AddActorFromTemplate(UClass* ClassTemplate, const FTransform& transform, const FName& NodeId) override;
	virtual void AddClonedActor(AActor* ActorTemplate, const FTransform& transform, const FName& NodeId) override;
	virtual void AddGroupActor(const TArray<FName>& ActorNodeIds, const FTransform& transform, const FName& NodeId) override;
	virtual void ExecuteCustomCommand(TSharedPtr<SceneProviderCommand> SceneCommand) override;

	virtual UWorld* GetDungeonWorld();
    
protected:
    /** Reuses an object from pool if available. Returns the actor that was reused, NULL otherwise */
    AActor* ReuseFromPool(const FName& NodeId, const FTransform& transform);
    
protected:
    UWorld* World;
    TMap<FName, TArray<AActor*>> NodeActorPool;
};

