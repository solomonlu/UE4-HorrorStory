//$ Copyright 2015 Ali Akbar, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//
#include "DungeonArchitectRuntimePrivatePCH.h"
#include "SnapModule.h"
#include "DungeonSceneProvider.h"
#include "SnapDoor.h"
#include "AssetUtils.h"
#include "SnapModuleInfo.h"

USnapModule::USnapModule(const FObjectInitializer& ObjectInitializer) 
	: Super(ObjectInitializer) 
{

}

void USnapModule::Initialize(UWorld* InModuleWorld, const TArray<UClass*>& InIgnoredModuleActorType)
{
	this->ModuleWorld = InModuleWorld;
	this->IgnoredModuleActorType = InIgnoredModuleActorType;

	TArray<AActor*> Actors;
	GetFilteredWorldActors(Actors);

	FindModuleInfo();

	// Find the bounding box of the module
	Bounds = FBox(EForceInit::ForceInitToZero);
	for (AActor* Actor : Actors) {
		Bounds += Actor->GetComponentsBoundingBox();
	}

	// Find the snap doors
	SnapDoors.Reset();
	for (AActor* Actor : Actors) {
		if (Actor->IsA<ASnapDoor>()) {
			SnapDoors.Add(Cast<ASnapDoor>(Actor));
		}
	}
}

void USnapModule::GetFilteredWorldActors(TArray<AActor*>& OutActors)
{
	OutActors.Reset();
	if (!ModuleWorld) return;

	for (int32 i = 0; i < ModuleWorld->GetNumLevels(); i++) {
		ULevel* Level = ModuleWorld->GetLevel(i);
		if (!Level) continue;
		for (AActor* Actor : Level->Actors) {
			if (!Actor || !Actor->GetRootComponent()) {
				continue;
			}
			if (IgnoredModuleActorType.Contains(Actor->GetClass())) {
				// Ignore this actor 
				continue;
			}

			if (Actor->IsChildActor()) {
				// We don't want to clone a child actor. They will be built by the cloned parent
				continue;
			}
			OutActors.Add(Actor);
		}
	}
}

void USnapModule::FindModuleInfo()
{
	for (int32 i = 0; i < ModuleWorld->GetNumLevels(); i++) {
		ULevel* Level = ModuleWorld->GetLevel(i);
		if (!Level) continue;
		for (AActor* Actor : Level->Actors) {
			if (Actor && Actor->IsA(ASnapModuleInfo::StaticClass())) {
				ASnapModuleInfo* ModuleInfoActor = Cast<ASnapModuleInfo>(Actor);
				ModuleInfo = ModuleInfoActor->ModuleInfo;
				return;
			}
		}
	}

	// Not found. Reset to default
	ModuleInfo = FSnapModuleMetaData();
}

void USnapModule::SpawnModule(const FTransform& ModuleTransform, TSharedPtr<FDungeonSceneProvider> SceneProvider, DoorTemplateToInstance_t& OutDoorTemplateToInstance)
{
	ModuleActorIds.Reset();
	OutDoorTemplateToInstance.Reset();

	TArray<AActor*> Actors;
	GetFilteredWorldActors(Actors);
	for (AActor* Actor : Actors) {
		FTransform ModuleActorTransform = Actor->GetActorTransform();
		FTransform ActorTransform;
		FTransform::Multiply(&ActorTransform, &ModuleActorTransform, &ModuleTransform);
		FName ActorId = *FGuid::NewGuid().ToString();	// TODO: Implement me for better caching
		SceneProvider->AddClonedActor(Actor, ActorTransform, ActorId);

		ModuleActorIds.Add(ActorId);

		if (Actor->IsA<ASnapDoor>()) {
			OutDoorTemplateToInstance.Add(Cast<ASnapDoor>(Actor), ActorId);
		}
	}

	FName GroupNodeId = *FGuid::NewGuid().ToString();
	FTransform GroupTransform = FTransform::Identity; 
	FVector ModuleCenter = ModuleTransform.GetRotation() * Bounds.GetCenter();
	GroupTransform.SetLocation(ModuleCenter + ModuleTransform.GetLocation());
	SceneProvider->AddGroupActor(ModuleActorIds, GroupTransform, GroupNodeId);
}

bool USnapModule::ContainsDoorType(const FString& DesiredDoorPath)
{
	for (TActorIterator<ASnapDoor> DoorIt(ModuleWorld); DoorIt; ++DoorIt)
	{
		// Same as with the Object Iterator, access the subclass instance with the * or -> operators.
		ASnapDoor *Door = *DoorIt;
		FString DoorPath = Door->GetDoorAssetPath();
		if (DoorPath == DesiredDoorPath || DesiredDoorPath.IsEmpty()) {
			return true;
		}
	}
	return false;
}

void USnapModule::GetDoorsOfType(const FString& DesiredDoorPath, TArray<ASnapDoor*>& OutDoors)
{
	OutDoors.Reset();
	for (TActorIterator<ASnapDoor> DoorIt(ModuleWorld); DoorIt; ++DoorIt)
	{
		// Same as with the Object Iterator, access the subclass instance with the * or -> operators.
		ASnapDoor *Door = *DoorIt;
		FString DoorPath = Door->GetDoorAssetPath();
		if (DoorPath == DesiredDoorPath || DesiredDoorPath.IsEmpty()) {
			OutDoors.Add(Door);
		}
	}
}
