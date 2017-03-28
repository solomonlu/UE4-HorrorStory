//$ Copyright 2015 Ali Akbar, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "DungeonArchitectRuntimePrivatePCH.h"
#include "DungeonSceneProvider.h"
#include "Core/Actors/DungeonParticleSystem.h"
#include "Particles/Emitter.h"
#include "Core/Dungeon.h"
#include "Core/Utils/DungeonModelHelper.h"
#include "Core/Utils/Profiler.h"
#include "Engine/DestructibleMesh.h"
#include "PhysicsEngine/DestructibleActor.h"

DEFINE_LOG_CATEGORY(DungeonSceneProvider);


FDungeonSceneProvider::FDungeonSceneProvider(ADungeon* pDungeon) : Dungeon(pDungeon), BuildPriorityLocation(FVector::ZeroVector), CurrentGTCommandExecutionIndex(0)
{
}

void FDungeonSceneProvider::OnDungeonBuildStart(UDungeonConfig* Config)
{
	this->LastBuildConfig = Config;
}

void FDungeonSceneProvider::AddStaticMesh(UDungeonMesh* Mesh, const FTransform& transform, const FName& NodeId)
{
	SceneProviderCommand_CreateMesh Command(Dungeon, Mesh, transform, NodeId);
	Command.Execute(GetDungeonWorld());
}

void FDungeonSceneProvider::AddDestructibleMesh(UDestructibleMesh* DestructibleMesh, const FTransform& transform, const FName& NodeId)
{
	SceneProviderCommand_CreateDestructibleMesh Command(Dungeon, DestructibleMesh, transform, NodeId);
	Command.Execute(GetDungeonWorld());
}

void FDungeonSceneProvider::AddLight(UPointLightComponent* LightTemplate, const FTransform& transform, const FName& NodeId)
{
	SceneProviderCommand_AddLight Command(Dungeon, LightTemplate, transform, NodeId);
	Command.Execute(GetDungeonWorld());
}

void FDungeonSceneProvider::AddParticleSystem(UParticleSystem* ParticleTemplate, const FTransform& transform, const FName& NodeId)
{
	SceneProviderCommand_AddParticleSystem Command(Dungeon, ParticleTemplate, transform, NodeId);
	Command.Execute(GetDungeonWorld());
}

void FDungeonSceneProvider::AddActorFromTemplate(UClass* ClassTemplate, const FTransform& transform, const FName& NodeId)
{
	SceneProviderCommand_AddActor Command(Dungeon, ClassTemplate, transform, NodeId);
	Command.Execute(GetDungeonWorld());
}

void FDungeonSceneProvider::AddClonedActor(AActor* ActorTemplate, const FTransform& transform, const FName& NodeId)
{
	SceneProviderCommand_CloneActor Command(Dungeon, ActorTemplate, transform, NodeId);
	Command.Execute(GetDungeonWorld());
}

void FDungeonSceneProvider::AddGroupActor(const TArray<FName>& ActorNodeIds, const FTransform& transform, const FName& NodeId)
{
	SceneProviderCommand_CreateGroupActor Command(Dungeon, ActorNodeIds, transform, NodeId);
	Command.Execute(GetDungeonWorld());
}

void FDungeonSceneProvider::ProcessUnsupportedObject(UObject* Object, const FTransform& transform, const FName& NodeId)
{
}

void FDungeonSceneProvider::ExecuteCustomCommand(TSharedPtr<SceneProviderCommand> SceneCommand)
{
	SceneCommand->Execute(GetDungeonWorld());
}

void FDungeonSceneProvider::RunGameThreadCommands(float MaxBuildTimePerFrameMs)
{
    const double MAX_FRAME_TIME = MaxBuildTimePerFrameMs;
    
    FDateTime frameStartTime = FDateTime::Now();
    while (CurrentGTCommandExecutionIndex < GameThreadCommands.Num()) {
        TSharedPtr<SceneProviderCommand> Command = GameThreadCommands[CurrentGTCommandExecutionIndex];
        Command->Execute(GetDungeonWorld());
        CurrentGTCommandExecutionIndex++;
        
        double elapsedMilli = (FDateTime::Now() - frameStartTime).GetTotalMilliseconds();
        if (MAX_FRAME_TIME > 0 && elapsedMilli >= MAX_FRAME_TIME) {
            break;
        }
    }
    
    if (CurrentGTCommandExecutionIndex >= GameThreadCommands.Num()) {
        GameThreadCommands.Reset();
    }
}

void FDungeonSceneProvider::ApplyExecutionWeights() {
    for (TSharedPtr<SceneProviderCommand> Command : GameThreadCommands) {
        Command->UpdateExecutionPriority(BuildPriorityLocation);
    }
}

bool FDungeonSceneProvider::IsRunningGameThreadCommands() const {
    return CurrentGTCommandExecutionIndex < GameThreadCommands.Num();
}

UWorld* FDefaultDungeonSceneProvider::GetDungeonWorld()
{
	return World;
}

void SceneProviderCommand_DestroyActor::Execute(UWorld* World)
{
	if (Actor) {
		Actor->Destroy();
		Actor = nullptr;
	}
}
