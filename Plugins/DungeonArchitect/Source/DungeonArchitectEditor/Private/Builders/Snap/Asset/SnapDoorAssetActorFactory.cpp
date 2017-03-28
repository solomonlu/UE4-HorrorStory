//$ Copyright 2015 Ali Akbar, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//
#include "DungeonArchitectEditorPrivatePCH.h"
#include "SnapDoorAssetActorFactory.h"
#include "SnapDoorAsset.h"
#include "SnapDoor.h"

USnapDoorAssetActorFactory::USnapDoorAssetActorFactory(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	DisplayName = NSLOCTEXT("SnapDoor", "SnapDoorFactoryDisplayName", "Add a Snap Door");
	NewActorClass = ASnapDoor::StaticClass();
}

AActor* USnapDoorAssetActorFactory::SpawnActor(UObject* Asset, ULevel* InLevel, const FTransform& Transform, EObjectFlags InObjectFlags, const FName Name)
{
	FActorSpawnParameters SpawnInfo;
	SpawnInfo.OverrideLevel = InLevel;
	SpawnInfo.ObjectFlags = InObjectFlags;
	SpawnInfo.Name = Name;

	AActor* Actor = InLevel->OwningWorld->SpawnActor(ASnapDoor::StaticClass(), &Transform, SpawnInfo);
	ASnapDoor* SnapDoor = Cast<ASnapDoor>(Actor);
	SnapDoor->DoorAsset = Cast<USnapDoorAsset>(Asset);
	SnapDoor->VisualMode = ESnapDoorMode::Door;
	SnapDoor->BuildDoor();
	return SnapDoor;
}

bool USnapDoorAssetActorFactory::CanCreateActorFrom(const FAssetData& AssetData, FText& OutErrorMsg)
{
	UObject* Asset = AssetData.GetAsset();
	return Asset && Asset->IsA(USnapDoorAsset::StaticClass());
}
