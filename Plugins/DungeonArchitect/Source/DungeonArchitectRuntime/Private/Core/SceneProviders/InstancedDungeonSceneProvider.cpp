 //$ Copyright 2015 Ali Akbar, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//
#include "DungeonArchitectRuntimePrivatePCH.h"
#include "InstancedDungeonSceneProvider.h"
#include "Core/Dungeon.h"
#include "Core/Utils/DungeonModelHelper.h"
#include "Core/Utils/Profiler.h"
#include "SceneProviderCommand.h"
#include "Core/Actors/DungeonInstancedMeshActor.h"
#include "Runtime/Engine/Classes/Components/InstancedStaticMeshComponent.h"

const FName InstancedActorTag("DUNGEON-INSTANCED-ACTOR");

class SceneProviderCommand_InstanceMeshBase : public SceneProviderCommand {
public:
    SceneProviderCommand_InstanceMeshBase(TSharedPtr<InstanceMeshContext> pContext)
		: SceneProviderCommand(pContext->Dungeon), Context(pContext) {}
    
protected:
    bool IsValid() {
        return (Context.IsValid() && Context->InstancedActor);
    }
    
protected:
    TSharedPtr<InstanceMeshContext> Context;
};


class SceneProviderCommand_InstanceMesh_FindInstancedActor : public SceneProviderCommand_InstanceMeshBase {
public:
    SceneProviderCommand_InstanceMesh_FindInstancedActor(TSharedPtr<InstanceMeshContext> pContext)
    : SceneProviderCommand_InstanceMeshBase(pContext) {}
    
    virtual void Execute(UWorld* World) override {
        // Search for an existing actor that holds the instanced static meshes
        const FName DungeonTag = UDungeonModelHelper::GetDungeonIdTag(Dungeon);
        ADungeonInstancedMeshActor* InstancedActor = nullptr;
        for (TObjectIterator<AActor> ActorItr; ActorItr; ++ActorItr) {
            if (!ActorItr->IsValidLowLevel() || ActorItr->IsPendingKill()) continue;
            if (ActorItr->ActorHasTag(DungeonTag) && ActorItr->ActorHasTag(InstancedActorTag)) {
                ADungeonInstancedMeshActor* InstancedActorPtr = Cast<ADungeonInstancedMeshActor>(*ActorItr);
                if (InstancedActorPtr && !InstancedActorPtr->IsPendingKill()) {
                    InstancedActor = InstancedActorPtr;
                    break;
                }
            }
        }
        
        if (!InstancedActor) {
            InstancedActor = World->SpawnActor<ADungeonInstancedMeshActor>();
            InstancedActor->Tags.Add(DungeonTag);
            InstancedActor->Tags.Add(InstancedActorTag);
			MoveToFolder(InstancedActor);
        }
        
        Context->InstancedActor = InstancedActor;
    }

};


class SceneProviderCommand_InstanceMesh_OnStart : public SceneProviderCommand_InstanceMeshBase {
public:
    SceneProviderCommand_InstanceMesh_OnStart(TSharedPtr<InstanceMeshContext> pContext)
    : SceneProviderCommand_InstanceMeshBase(pContext) {}
    
    virtual void Execute(UWorld* World) override {
        if (!IsValid()) { return; }
        Context->InstancedActor->OnBuildStart();
    }
};

class SceneProviderCommand_InstanceMesh_OnStop : public SceneProviderCommand_InstanceMeshBase {
public:
    SceneProviderCommand_InstanceMesh_OnStop(TSharedPtr<InstanceMeshContext> pContext)
    : SceneProviderCommand_InstanceMeshBase(pContext) {}
    
    virtual void Execute(UWorld* World) override {
        if (!IsValid()) { return; }
        Context->InstancedActor->OnBuildStop();
    }
};

class SceneProviderCommand_InstanceMesh_AddMeshInstance : public SceneProviderCommand_InstanceMeshBase {
public:
	SceneProviderCommand_InstanceMesh_AddMeshInstance(TSharedPtr<InstanceMeshContext> pContext, UDungeonMesh* pMesh, const FTransform& pTransform)
            : SceneProviderCommand_InstanceMeshBase(pContext), Mesh(pMesh), Transform(pTransform) {
        
    }
    
    virtual void Execute(UWorld* World) override {
        if (!IsValid()) { return; }
        Context->InstancedActor->AddMeshInstance(Mesh, Transform);
    }
    
    virtual void UpdateExecutionPriority(const FVector& BuildPosition) override {
        UpdateExecutionPriorityByDistance(BuildPosition, Transform);
    }
private:
	UDungeonMesh* Mesh;
    FTransform Transform;
};


void FInstancedDungeonSceneProvider::OnDungeonBuildStart(UDungeonConfig* Config) {
    FPooledDungeonSceneProvider::OnDungeonBuildStart(Config);
    Context = MakeShareable(new InstanceMeshContext());
	Context->Dungeon = Dungeon;
}

void FInstancedDungeonSceneProvider::OnDungeonBuildStop() {
    FPooledDungeonSceneProvider::OnDungeonBuildStop();

    // Insert the starting commands
    GameThreadCommands.Insert(MakeShareable(new SceneProviderCommand_InstanceMesh_FindInstancedActor(Context)), 0);
    GameThreadCommands.Insert(MakeShareable(new SceneProviderCommand_InstanceMesh_OnStart(Context)), 1);

    // Notify the creation has ended, as the last command
    GameThreadCommands.Add(MakeShareable(new SceneProviderCommand_InstanceMesh_OnStop(Context)));
    
    Context = nullptr;
}

void FInstancedDungeonSceneProvider::AddStaticMesh(UDungeonMesh* Mesh, const FTransform& transform, const FName& NodeId) {
    GameThreadCommands.Add(MakeShareable(new SceneProviderCommand_InstanceMesh_AddMeshInstance(Context, Mesh, transform)));
}


