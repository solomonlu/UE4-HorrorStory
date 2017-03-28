//$ Copyright 2015 Ali Akbar, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//
#include "DungeonArchitectEditorPrivatePCH.h"
#include "DungeonEditorService.h"

AActor* FDungeonEditorService::CreateGroupActor(UWorld* World, const TArray<AActor*>& MergedActorList, const FTransform& Transform)
{
	FActorSpawnParameters SpawnParams;
	AGroupActor* GroupActor = World->SpawnActor<AGroupActor>(Transform.GetLocation(), FRotator(Transform.GetRotation()), SpawnParams);
	//GroupActor->SetActorTransform(Transform);
	for (AActor* Actor : MergedActorList) {
		GroupActor->Add(*Actor);
	}
	return GroupActor;
}

