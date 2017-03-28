//$ Copyright 2015 Ali Akbar, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//
#pragma once

#include "SnapModuleInfo.h"
#include "SnapModule.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSnapModule, Log, All);

class FDungeonSceneProvider;
class ASnapDoor;
typedef TMap<ASnapDoor*, FName> DoorTemplateToInstance_t;

UCLASS()
class DUNGEONARCHITECTRUNTIME_API USnapModule : public UObject {
	GENERATED_UCLASS_BODY()
public:
	
	void Initialize(UWorld* InModuleWorld, const TArray<UClass*>& InIgnoredModuleActorType);
	void SpawnModule(const FTransform& ModuleTransform, TSharedPtr<FDungeonSceneProvider> SceneProvider, DoorTemplateToInstance_t& OutDoorTemplateToInstance);

public:
	UPROPERTY()
	UWorld* ModuleWorld;

	UPROPERTY()
	TArray<FName> ModuleActorIds;

	UPROPERTY()
	TArray<UClass*> IgnoredModuleActorType;

	UPROPERTY()
	TArray<ASnapDoor*> SnapDoors;

	UPROPERTY()
	FBox Bounds;

	UPROPERTY()
	FSnapModuleMetaData ModuleInfo;

	bool ContainsDoorType(const FString& DesiredDoorPath);
	void GetDoorsOfType(const FString& DesiredDoorPath, TArray<ASnapDoor*>& OutDoors);

private:
	void GetFilteredWorldActors(TArray<AActor*>& OutActors);

	/** Finds the ASnapModuleInfo actor in the module world and copies the data to the ModuleInfo structure */
	void FindModuleInfo();
	
};

