//$ Copyright 2015 Ali Akbar, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//
#include "DungeonArchitectRuntimePrivatePCH.h"
#include "PooledDungeonSceneProvider.h"
#include "Core/Dungeon.h"
#include "Core/Utils/DungeonModelHelper.h"
#include "Core/Utils/Profiler.h"
#include "Engine/DestructibleMesh.h"
#include "PhysicsEngine/DestructibleActor.h"

DEFINE_LOG_CATEGORY(PooledDungeonSceneProvider);

void FPooledDungeonSceneProvider::OnDungeonBuildStart(UDungeonConfig* Config)
{
	FDungeonSceneProvider::OnDungeonBuildStart(Config);

    NodeActorPool.Reset();
    const FName DungeonTag = UDungeonModelHelper::GetDungeonIdTag(Dungeon);
    
    // Collect all actors that have a "Dungeon" tag
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
		AActor* Actor = *ActorItr;
		FName NodeId;
		if (UDungeonModelHelper::GetNodeId(DungeonTag, Actor, NodeId)) {
			if (!NodeActorPool.Contains(NodeId)) {
				NodeActorPool.Add(NodeId, TArray<AActor*>());
			}
			NodeActorPool[NodeId].AddUnique(Actor);
		}
    }
}

void FPooledDungeonSceneProvider::OnDungeonBuildStop()
{
    // Destroy every unused object in the actor pool, since the dungeon building has ended
    TArray<TArray<AActor*>> ActorValues;
    NodeActorPool.GenerateValueArray(ActorValues);
    for (TArray<AActor*> ActorArray : ActorValues) {
        for (AActor* Actor : ActorArray) {
            TSharedPtr<SceneProviderCommand> Command = MakeShareable(new SceneProviderCommand_DestroyActor(Actor));
            GameThreadCommands.Add(Command);
        }
    }
    
    NodeActorPool.Reset();

	// Sort the game commands based on priority
	ApplyExecutionWeights();
	GameThreadCommands.Sort(SceneProviderCommand::WeightSortPredicate);
}

AActor* FPooledDungeonSceneProvider::ReuseFromPool(const FName& NodeId, const FTransform& transform)
{
    // Check if we have a free actor of this type in the pool
    if (NodeActorPool.Contains(NodeId)) {
        TArray<AActor*>& ActorList = NodeActorPool[NodeId];
        if (ActorList.Num() > 0) {
            // A free actor is available in the pool. Grab this actor and remove it from the pool
            AActor* Actor = ActorList.Pop();
            
            // Reset its transform as requested
            TSharedPtr<SceneProviderCommand> Command = MakeShareable(new SceneProviderCommand_SetActorTransform(Actor, transform));
            GameThreadCommands.Add(Command);
            
            // Indicates that the object was reused from pool
            return Actor;
        }
    }
    
    // No free objects of this type in the pool
    return NULL;
}

void FPooledDungeonSceneProvider::AddStaticMesh(UDungeonMesh* Mesh, const FTransform& transform, const FName& NodeId)
{
    if (Mesh->StaticMesh == nullptr) return;
    
    // Check if we have a free actor of this type in the pool. If so use it.
    AActor* Actor = ReuseFromPool(NodeId, transform);
    
    if (Actor) {
        AStaticMeshActor* StaticMesh = Cast<AStaticMeshActor>(Actor);
        if (StaticMesh) {
            StaticMesh->GetStaticMeshComponent()->StaticMesh = Mesh->StaticMesh;
			
        }
    }
    else {
        // No free actor exists.  Create a new actor later in the game thread
        TSharedPtr<SceneProviderCommand> Command = MakeShareable(new SceneProviderCommand_CreateMesh(Dungeon, Mesh, transform, NodeId));
        GameThreadCommands.Add(Command);
    }
}

void FPooledDungeonSceneProvider::AddDestructibleMesh(UDestructibleMesh* DestructibleMesh, const FTransform& transform, const FName& NodeId)
{
	if (!DestructibleMesh) return;

	// Check if we have a free actor of this type in the pool. If so use it.
	AActor* Actor = ReuseFromPool(NodeId, transform);

	if (Actor) {
		ADestructibleActor* Destructible = Cast<ADestructibleActor>(Actor);
		if (Destructible) {
			Destructible->GetDestructibleComponent()->SetDestructibleMesh(DestructibleMesh);
		}
	}
	else {
		// No free actor exists.  Create a new actor later in the game thread
		TSharedPtr<SceneProviderCommand> Command = MakeShareable(new SceneProviderCommand_CreateDestructibleMesh(Dungeon, DestructibleMesh, transform, NodeId));
		GameThreadCommands.Add(Command);
	}
}

void FPooledDungeonSceneProvider::AddLight(UPointLightComponent* LightTemplate, const FTransform& transform, const FName& NodeId)
{
    // Check if we have a free actor of this type in the pool. If so use it.
    if (AActor* Actor = ReuseFromPool(NodeId, transform)) {
        if (USpotLightComponent* SpotLightTemplate = Cast<USpotLightComponent>(LightTemplate)) {
            if (ASpotLight* SpotLightActor = Cast<ASpotLight>(Actor)) {
                SceneProviderCommand_AddLight::SetSpotLightAttributes(SpotLightActor->SpotLightComponent, SpotLightTemplate);
                SceneProviderCommand_AddLight::SetPointLightAttributes(SpotLightActor->SpotLightComponent, SpotLightTemplate);
            }
        }
        else if (APointLight* PointLight = Cast<APointLight>(Actor)) {
            SceneProviderCommand_AddLight::SetPointLightAttributes(PointLight->PointLightComponent, LightTemplate);
        }
    }
    else {
        // No free actor exists.  Create a new actor later in the game thread
        TSharedPtr<SceneProviderCommand> Command = MakeShareable(new SceneProviderCommand_AddLight(Dungeon, LightTemplate, transform, NodeId));
        GameThreadCommands.Add(Command);
    }
}

void FPooledDungeonSceneProvider::AddParticleSystem(UParticleSystem* ParticleTemplate, const FTransform& transform, const FName& NodeId)
{
    // Check if we have a free actor of this type in the pool. If so use it.
    if (AActor* Actor = ReuseFromPool(NodeId, transform)) {
        AEmitter* EmitterActor = Cast<AEmitter>(Actor);
        if (EmitterActor) {
			// Set the template in the game thread
			TSharedPtr<SceneProviderCommand> Command = MakeShareable(new SceneProviderCommand_SetParticleTemplate(Dungeon, EmitterActor, ParticleTemplate));
			GameThreadCommands.Add(Command);
        }
    }
    else {
        // No free actor exists.  Create a new actor later in the game thread
        TSharedPtr<SceneProviderCommand> Command = MakeShareable(new SceneProviderCommand_AddParticleSystem(Dungeon, ParticleTemplate, transform, NodeId));
        GameThreadCommands.Add(Command);
    }
}

void FPooledDungeonSceneProvider::AddActorFromTemplate(UClass* ClassTemplate, const FTransform& transform, const FName& NodeId)
{
    // Check if we have a free actor of this type in the pool. If so use it.
    AActor* Actor = ReuseFromPool(NodeId, transform);
    if (!Actor) {
        TSharedPtr<SceneProviderCommand> Command = MakeShareable(new SceneProviderCommand_AddActor(Dungeon, ClassTemplate, transform, NodeId));
        GameThreadCommands.Add(Command);
    }
}

void FPooledDungeonSceneProvider::AddClonedActor(AActor* ActorTemplate, const FTransform& transform, const FName& NodeId)
{
	// Check if we have a free actor of this type in the pool. If so use it.
	AActor* Actor = ReuseFromPool(NodeId, transform);
	if (!Actor) {
		TSharedPtr<SceneProviderCommand> Command = MakeShareable(new SceneProviderCommand_CloneActor(Dungeon, ActorTemplate, transform, NodeId));
		GameThreadCommands.Add(Command);
	}
}

void FPooledDungeonSceneProvider::AddGroupActor(const TArray<FName>& ActorNodeIds, const FTransform& transform, const FName& NodeId)
{
	// Check if we have a free actor of this type in the pool. If so use it.
	AActor* Actor = ReuseFromPool(NodeId, transform);
	if (!Actor) {
		TSharedPtr<SceneProviderCommand> Command = MakeShareable(new SceneProviderCommand_CreateGroupActor(Dungeon, ActorNodeIds, transform, NodeId));
		GameThreadCommands.Add(Command);
	}
}

void FPooledDungeonSceneProvider::ExecuteCustomCommand(TSharedPtr<SceneProviderCommand> SceneCommand)
{
	GameThreadCommands.Add(SceneCommand);
}

UWorld* FPooledDungeonSceneProvider::GetDungeonWorld()
{
    return World;
}
