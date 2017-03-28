//$ Copyright 2015 Ali Akbar, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//
#pragma once
#include "Core/DungeonModel.h"
#include "SceneProviderCommand.h"

DECLARE_LOG_CATEGORY_EXTERN(DungeonSceneProvider, Log, All);

class UDestructibleMesh;

class DUNGEONARCHITECTRUNTIME_API FDungeonSceneProvider {
public:
    FDungeonSceneProvider() : Dungeon(nullptr), BuildPriorityLocation(FVector::ZeroVector), CurrentGTCommandExecutionIndex(0) {}
	FDungeonSceneProvider(ADungeon* pDungeon);
	virtual void OnDungeonBuildStart(UDungeonConfig* Config);
	virtual void OnDungeonBuildStop() {}
	virtual void AddStaticMesh(UDungeonMesh* Mesh, const FTransform& transform, const FName& NodeId);
	virtual void AddDestructibleMesh(UDestructibleMesh* DestructibleMesh, const FTransform& transform, const FName& NodeId);
	virtual void AddLight(UPointLightComponent* LightTemplate, const FTransform& transform, const FName& NodeId);
	virtual void AddParticleSystem(UParticleSystem* ParticleTemplate, const FTransform& transform, const FName& NodeId);
	virtual void AddActorFromTemplate(UClass* ClassTemplate, const FTransform& transform, const FName& NodeId);
	virtual void AddClonedActor(AActor* ActorTemplate, const FTransform& transform, const FName& NodeId);
	virtual void AddGroupActor(const TArray<FName>& ActorNodeIds, const FTransform& transform, const FName& NodeId);
	virtual void ProcessUnsupportedObject(UObject* Object, const FTransform& transform, const FName& NodeId);
	virtual void ExecuteCustomCommand(TSharedPtr<SceneProviderCommand> SceneCommand);

	virtual UWorld* GetDungeonWorld() = 0;
    
	// Run all the queued commands. This will be called from the game thread. The commands might have been inserted from the background thread
	void RunGameThreadCommands(float MaxBuildTimePerFrameMs);
    bool IsRunningGameThreadCommands() const;
    void SetBuildPriorityLocation(const FVector& Location) {
        this->BuildPriorityLocation = Location;
    }
    
    void ApplyExecutionWeights();
    
protected:
    ADungeon* Dungeon;

	UDungeonConfig* LastBuildConfig;
    
    // The location to start building from and spreading out from here, while building asynchronously
    FVector BuildPriorityLocation;

	// Commands to be run in the game thread. Push a command here if we cannot perform a specific tasks in the background thread
	TArray<TSharedPtr<SceneProviderCommand>> GameThreadCommands;
    
private:
    int32 CurrentGTCommandExecutionIndex;
};

class DUNGEONARCHITECTRUNTIME_API FDefaultDungeonSceneProvider : public FDungeonSceneProvider {
public:
	FDefaultDungeonSceneProvider(ADungeon* pDungeon, UWorld* pWorld) : FDungeonSceneProvider(pDungeon), World(pWorld) {}
	virtual UWorld* GetDungeonWorld() override;

private:
	UWorld* World;
};

/** Ignores all requests to create objects on the scene */
class DUNGEONARCHITECTRUNTIME_API FNullDungeonSceneProvider : public FDefaultDungeonSceneProvider {
public:
	FNullDungeonSceneProvider(ADungeon* pDungeon, UWorld* World) : FDefaultDungeonSceneProvider(pDungeon, World) {}
	virtual void AddStaticMesh(UDungeonMesh* Mesh, const FTransform& transform, const FName& NodeId) override {}
	virtual void AddLight(UPointLightComponent* LightTemplate, const FTransform& transform, const FName& NodeId) override {}
	virtual void AddParticleSystem(UParticleSystem* ParticleTemplate, const FTransform& transform, const FName& NodeId) override {}
	virtual void AddActorFromTemplate(UClass* ClassTemplate, const FTransform& transform, const FName& NodeId) override {}
};
