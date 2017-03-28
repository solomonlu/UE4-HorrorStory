//$ Copyright 2015 Ali Akbar, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//
#include "DungeonArchitectRuntimePrivatePCH.h"
#include "SnapModuleInfo.h"

DEFINE_LOG_CATEGORY(LogSnapModuleInfo)

ASnapModuleInfo::ASnapModuleInfo(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	USceneComponent* SceneComponent = ObjectInitializer.CreateDefaultSubobject<USceneComponent>(this, "Scene");
	UBillboardComponent* Sprite = ObjectInitializer.CreateDefaultSubobject<UBillboardComponent>(this, "Sprite");
	Sprite->bHiddenInGame = true;
	Sprite->SetupAttachment(SceneComponent);

	RootComponent = SceneComponent;
}
