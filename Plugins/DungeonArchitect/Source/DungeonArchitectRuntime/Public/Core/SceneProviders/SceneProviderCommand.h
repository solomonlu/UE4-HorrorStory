//$ Copyright 2015 Ali Akbar, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//
#pragma once
#include "Core/DungeonModel.h"
#include "Core/Actors/DungeonMesh.h"
#include "Engine/DestructibleMesh.h"

class ADungeon;

class DUNGEONARCHITECTRUNTIME_API SceneProviderCommand {
public:
    SceneProviderCommand(ADungeon* pDungeon) : ExecutionPriority(INT_MAX), Dungeon(pDungeon) {}
    virtual ~SceneProviderCommand() {}
    virtual void Execute(UWorld* World) = 0;
    
    inline static bool WeightSortPredicate(TSharedPtr<SceneProviderCommand> cmd1, TSharedPtr<SceneProviderCommand> cmd2) {
        return (cmd1->ExecutionPriority < cmd2->ExecutionPriority);
    }
    
    float ExecutionPriority;
    
    virtual void UpdateExecutionPriority(const FVector& BuildPosition) {}
    
	FORCEINLINE FName CreateNodeTagFromId(const FName& NodeId) {
		return *FString("NODE-").Append(NodeId.ToString());
	}

protected:
    void AddNodeTag(AActor* Actor, const FName& NodeId);
	void MoveToFolder(AActor* Actor);
    void UpdateExecutionPriorityByDistance(const FVector& BuildPosition, const FTransform& CommandTransform) {
        FVector Location = CommandTransform.GetLocation();
        ExecutionPriority = (Location - BuildPosition).SizeSquared();
    }
    
    template<typename T>
    T* AddActor(UWorld* World, const FTransform& transform, const FName& NodeId) {
        T* Actor = World->SpawnActor<T>(T::StaticClass());
        Actor->SetActorTransform(transform);
        Actor->Tags.Add(FName("Dungeon"));
		AddNodeTag(Actor, NodeId);
		MoveToFolder(Actor);
        return Actor;
    }
    
    ADungeon* Dungeon;
};

class DUNGEONARCHITECTRUNTIME_API SceneProviderCommand_CreateMesh : public SceneProviderCommand {
public:
    SceneProviderCommand_CreateMesh(ADungeon* pDungeon, UDungeonMesh* pMesh, const FTransform& ptransform, const FName& pNodeId) : SceneProviderCommand(pDungeon), Mesh(pMesh), transform(ptransform), NodeId(pNodeId) {}
    virtual void Execute(UWorld* World) override;
	static void SetMeshComponentAttributes(UStaticMeshComponent* StaticMeshComponent, UStaticMeshComponent* StaticMeshTemplate);
    
    virtual void UpdateExecutionPriority(const FVector& BuildPosition) override {
        UpdateExecutionPriorityByDistance(BuildPosition, transform);
    }
    
private:
    UDungeonMesh* Mesh;
    FTransform transform;
    FName NodeId;
};

class DUNGEONARCHITECTRUNTIME_API SceneProviderCommand_CreateDestructibleMesh : public SceneProviderCommand {
public:
	SceneProviderCommand_CreateDestructibleMesh(ADungeon* pDungeon, UDestructibleMesh* pDestructibleMesh, const FTransform& ptransform, const FName& pNodeId) 
		: SceneProviderCommand(pDungeon), DestructibleMesh(pDestructibleMesh), transform(ptransform), NodeId(pNodeId) {}
	virtual void Execute(UWorld* World) override;
	static void SetMeshComponentAttributes(UDestructibleMesh* StaticMeshComponent, UDestructibleMesh* StaticMeshTemplate);

	virtual void UpdateExecutionPriority(const FVector& BuildPosition) override {
		UpdateExecutionPriorityByDistance(BuildPosition, transform);
	}

private:
	UDestructibleMesh* DestructibleMesh;
	FTransform transform;
	FName NodeId;
};

class DUNGEONARCHITECTRUNTIME_API SceneProviderCommand_AddLight : public SceneProviderCommand {
public:
    SceneProviderCommand_AddLight(ADungeon* pDungeon, UPointLightComponent* pLightTemplate, const FTransform& ptransform, const FName& pNodeId) : SceneProviderCommand(pDungeon), LightTemplate(pLightTemplate), transform(ptransform), NodeId(pNodeId) {}
    virtual void Execute(UWorld* World) override;
	static void SetSpotLightAttributes(USpotLightComponent* SpotLightComponent, USpotLightComponent* SpotLightTemplate);
	static void SetPointLightAttributes(UPointLightComponent* PointLightComponent, UPointLightComponent* LightTemplate);
    
    virtual void UpdateExecutionPriority(const FVector& BuildPosition) override {
        UpdateExecutionPriorityByDistance(BuildPosition, transform);
    }
    
private:
    UPointLightComponent* LightTemplate;
    FTransform transform;
    FName NodeId;
};

class DUNGEONARCHITECTRUNTIME_API SceneProviderCommand_SetParticleTemplate : public SceneProviderCommand {
public:
	SceneProviderCommand_SetParticleTemplate(ADungeon* pDungeon, AEmitter* pEmitterActor, UParticleSystem* pParticleTemplate)
		: SceneProviderCommand(pDungeon), EmitterActor(pEmitterActor), ParticleTemplate(pParticleTemplate) {}
	virtual void Execute(UWorld* World) override;

private:
	AEmitter* EmitterActor;
	UParticleSystem* ParticleTemplate;
};

class DUNGEONARCHITECTRUNTIME_API SceneProviderCommand_AddParticleSystem : public SceneProviderCommand {
public:
    SceneProviderCommand_AddParticleSystem(ADungeon* pDungeon, UParticleSystem* pParticleTemplate, const FTransform& ptransform, const FName& pNodeId) : SceneProviderCommand(pDungeon), ParticleTemplate(pParticleTemplate), transform(ptransform), NodeId(pNodeId) {}
    virtual void Execute(UWorld* World) override;
    
    virtual void UpdateExecutionPriority(const FVector& BuildPosition) override {
        UpdateExecutionPriorityByDistance(BuildPosition, transform);
    }
    
private:
    UParticleSystem* ParticleTemplate;
    FTransform transform;
    FName NodeId;
};

class DUNGEONARCHITECTRUNTIME_API SceneProviderCommand_AddActor : public SceneProviderCommand {
public:
    SceneProviderCommand_AddActor(ADungeon* pDungeon, UClass* pClassTemplate, const FTransform& ptransform, const FName& pNodeId) : SceneProviderCommand(pDungeon), ClassTemplate(pClassTemplate), transform(ptransform), NodeId(pNodeId) {}
    virtual void Execute(UWorld* World) override;
    
    virtual void UpdateExecutionPriority(const FVector& BuildPosition) override {
        UpdateExecutionPriorityByDistance(BuildPosition, transform);
    }
    
private:
    UClass* ClassTemplate;
    FTransform transform;
    FName NodeId;
};

class DUNGEONARCHITECTRUNTIME_API SceneProviderCommand_CreateGroupActor : public SceneProviderCommand {
public:
	SceneProviderCommand_CreateGroupActor(ADungeon* pDungeon, const TArray<FName>& pActorNodeIds, const FTransform& pTransform, const FName& pNodeId)
		: SceneProviderCommand(pDungeon)
		, ActorNodeIds(pActorNodeIds)
		, transform(pTransform)
		, NodeId(pNodeId)
	{}
	virtual void Execute(UWorld* World) override;

	virtual void UpdateExecutionPriority(const FVector& BuildPosition) override {
		ExecutionPriority = INT32_MAX;
	}

private:
	TArray<FName> ActorNodeIds;
	FTransform transform;
	FName NodeId;
};


class DUNGEONARCHITECTRUNTIME_API SceneProviderCommand_CloneActor : public SceneProviderCommand {
public:
	SceneProviderCommand_CloneActor(ADungeon* pDungeon, AActor* pActorTemplate, const FTransform& ptransform, const FName& pNodeId) 
		: SceneProviderCommand(pDungeon)
		, ActorTemplate(pActorTemplate)
		, transform(ptransform)
		, NodeId(pNodeId) 
	{}

	virtual void Execute(UWorld* World) override;

	virtual void UpdateExecutionPriority(const FVector& BuildPosition) override {
		UpdateExecutionPriorityByDistance(BuildPosition, transform);
	}

private:
	AActor* ActorTemplate;
	FTransform transform;
	FName NodeId;
};



class DUNGEONARCHITECTRUNTIME_API SceneProviderCommand_SetActorTransform : public SceneProviderCommand {
public:
    SceneProviderCommand_SetActorTransform(AActor* pActor, const FTransform& ptransform) : SceneProviderCommand(nullptr), Actor(pActor), transform(ptransform) {}
    virtual void Execute(UWorld* World) override;
    
    virtual void UpdateExecutionPriority(const FVector& BuildPosition) override {
        UpdateExecutionPriorityByDistance(BuildPosition, transform);
    }
    
private:
    AActor* Actor;
    FTransform transform;
};

class DUNGEONARCHITECTRUNTIME_API SceneProviderCommand_DestroyActor : public SceneProviderCommand {
public:
    SceneProviderCommand_DestroyActor(AActor* pActor) : SceneProviderCommand(nullptr), Actor(pActor) {}
    virtual void Execute(UWorld* World) override;
private:
    AActor* Actor;
};


class DUNGEONARCHITECTRUNTIME_API SceneProviderCommand_DestroyActorWithTag : public SceneProviderCommand {
public:
	SceneProviderCommand_DestroyActorWithTag(const FName& InTag) : SceneProviderCommand(nullptr), Tag(InTag) {}
	virtual void Execute(UWorld* World) override;

	virtual void UpdateExecutionPriority(const FVector& BuildPosition) override {
		ExecutionPriority = INT32_MAX;
	}

private:
	FName Tag;
};

