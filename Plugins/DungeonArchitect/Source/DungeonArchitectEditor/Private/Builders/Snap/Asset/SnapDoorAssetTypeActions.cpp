//$ Copyright 2015 Ali Akbar, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//
#include "DungeonArchitectEditorPrivatePCH.h"
#include "SnapDoorAssetTypeActions.h"
#include "SnapDoorAsset.h"

#include "AssetToolsModule.h"
#include "ContentBrowserModule.h"
#include "DungeonArchitectEditorModule.h"
#include "../DoorEditor/SnapDoorEditor.h"

#define LOCTEXT_NAMESPACE "AssetTypeActions"

//////////////////////////////////////////////////////////////////////////
// FDungeonThemeAssetTypeActions

FText FSnapDoorAssetTypeActions::GetName() const
{
	return LOCTEXT("FSnapDoorAssetTypeActionsName", "Dungeon Snap Door");
}

FColor FSnapDoorAssetTypeActions::GetTypeColor() const
{
	return FColor(153, 102, 51);	// Brown
}

UClass* FSnapDoorAssetTypeActions::GetSupportedClass() const
{
	return USnapDoorAsset::StaticClass();
}

void FSnapDoorAssetTypeActions::OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<class IToolkitHost> EditWithinLevelEditor)
{
	const EToolkitMode::Type Mode = EditWithinLevelEditor.IsValid() ? EToolkitMode::WorldCentric : EToolkitMode::Standalone;
	for (auto ObjIt = InObjects.CreateConstIterator(); ObjIt; ++ObjIt)
	{
		USnapDoorAsset* DoorAsset = Cast<USnapDoorAsset>(*ObjIt);
		if (DoorAsset) {
			TSharedRef<FSnapDoorEditor> DoorEditor(new FSnapDoorEditor);
			DoorEditor->InitSnapDoorEditor(Mode, EditWithinLevelEditor, DoorAsset);
			//TSharedRef<FDungeonArchitectEditor> NewDungeonEditor(new FDungeonArchitectEditor());
			//NewDungeonEditor->InitDungeonEditor(Mode, EditWithinLevelEditor, PropData);
		}
	}
}

uint32 FSnapDoorAssetTypeActions::GetCategories()
{
	return IDungeonArchitectEditorModule::Get().GetDungeonAssetCategoryBit();
}

void FSnapDoorAssetTypeActions::GetActions(const TArray<UObject*>& InObjects, FMenuBuilder& MenuBuilder)
{
}


#undef LOCTEXT_NAMESPACE
