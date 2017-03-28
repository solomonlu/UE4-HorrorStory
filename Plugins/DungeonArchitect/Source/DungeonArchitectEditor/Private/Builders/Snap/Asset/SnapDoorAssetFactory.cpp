//$ Copyright 2015 Ali Akbar, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//
#include "DungeonArchitectEditorPrivatePCH.h"
#include "SnapDoorAssetFactory.h"
#include "SnapDoorAsset.h"

USnapDoorAssetFactory::USnapDoorAssetFactory(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer) {
	SupportedClass = USnapDoorAsset::StaticClass();
	bCreateNew = true;
	bEditAfterNew = true;
}

bool USnapDoorAssetFactory::CanCreateNew() const {
	return true;
}

UObject* USnapDoorAssetFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) {
	USnapDoorAsset* NewAsset = NewObject<USnapDoorAsset>(InParent, Class, Name, Flags | RF_Transactional);
	return NewAsset;
}
