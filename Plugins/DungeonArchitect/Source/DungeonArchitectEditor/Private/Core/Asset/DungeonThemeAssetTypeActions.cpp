//$ Copyright 2015 Ali Akbar, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//
#include "DungeonArchitectEditorPrivatePCH.h"
#include "DungeonThemeAssetTypeActions.h"
#include "DungeonThemeAsset.h"
#include "Core/DungeonArchitectEditor.h"

#include "AssetToolsModule.h"
#include "ContentBrowserModule.h"
#include "Transition/DungeonPropDataAsset.h"
#include "../DungeonArchitectCommands.h"
#include "DungeonThemeDataFactory.h"
#include "DungeonArchitectEditorModule.h"

#define LOCTEXT_NAMESPACE "AssetTypeActions"

//////////////////////////////////////////////////////////////////////////
// FDungeonThemeAssetTypeActions

FText FDungeonThemeAssetTypeActions::GetName() const
{
	return LOCTEXT("FDungeonThemeAssetTypeActionsName", "Dungeon Theme");
}

FColor FDungeonThemeAssetTypeActions::GetTypeColor() const
{
	return FColor::Cyan;
}

UClass* FDungeonThemeAssetTypeActions::GetSupportedClass() const
{
	return UDungeonThemeAsset::StaticClass();
}

void FDungeonThemeAssetTypeActions::OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<class IToolkitHost> EditWithinLevelEditor)
{
	const EToolkitMode::Type Mode = EditWithinLevelEditor.IsValid() ? EToolkitMode::WorldCentric : EToolkitMode::Standalone;
	for (auto ObjIt = InObjects.CreateConstIterator(); ObjIt; ++ObjIt)
	{
		UDungeonThemeAsset* PropData = Cast<UDungeonThemeAsset>(*ObjIt);
		if (PropData) {
			TSharedRef<FDungeonArchitectEditor> NewDungeonEditor(new FDungeonArchitectEditor());
			NewDungeonEditor->InitDungeonEditor(Mode, EditWithinLevelEditor, PropData);
		}
	}
}

uint32 FDungeonThemeAssetTypeActions::GetCategories()
{
	return EAssetTypeCategories::Basic
		| IDungeonArchitectEditorModule::Get().GetDungeonAssetCategoryBit();
}

void FDungeonThemeAssetTypeActions::GetActions(const TArray<UObject*>& InObjects, FMenuBuilder& MenuBuilder)
{
}

///////////////////////////////// V1 /////////////////////////////////////


FText FDungeonThemeAssetTypeActionsV1::GetName() const
{
	return LOCTEXT("FDungeonThemeAssetTypeActionsNameV1", "Dungeon Theme (ver 1)");
}

FColor FDungeonThemeAssetTypeActionsV1::GetTypeColor() const
{
	return FColor::Red;
}

UClass* FDungeonThemeAssetTypeActionsV1::GetSupportedClass() const
{
	return UDungeonPropDataAsset::StaticClass();
}

bool FDungeonThemeAssetTypeActionsV1::HasActions(const TArray<UObject*>& InObjects) const
{
	return true;
}

void FDungeonThemeAssetTypeActionsV1::GetActions(const TArray<UObject*>& InObjects, FMenuBuilder& MenuBuilder)
{
	if (InObjects.Num() > 0 && InObjects[0]->IsA(UDungeonPropDataAsset::StaticClass())) {
		TSharedPtr<FUICommandList> UICommandList = MakeShareable(new FUICommandList);
		const FDungeonArchitectCommands& Commands = FDungeonArchitectCommands::Get();
		UICommandList->MapAction(
			Commands.UpgradeThemeFile,
			FExecuteAction::CreateSP(this, &FDungeonThemeAssetTypeActionsV1::OnUpgradeThemeFile, InObjects[0]),
			FCanExecuteAction());


		MenuBuilder.PushCommandList(UICommandList.ToSharedRef());
		MenuBuilder.AddMenuEntry(Commands.UpgradeThemeFile);
	}
}

uint32 FDungeonThemeAssetTypeActionsV1::GetCategories()
{
	return EAssetTypeCategories::None;
}

void FDungeonThemeAssetTypeActionsV1::OnUpgradeThemeFile(UObject* SourceObject)
{
	UDungeonPropDataAsset* OldThemeAsset = Cast<UDungeonPropDataAsset>(SourceObject);
	FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools");
	UFactory* Factory = NewObject<UDungeonThemeDataFactory>();

	FAssetData AssetData(SourceObject);
	FString AssetName = AssetData.AssetName.ToString() + "_v2";
	FString PackagePath = AssetData.PackagePath.ToString();

	UObject* Asset = AssetToolsModule.Get().CreateAsset(AssetName, PackagePath, UDungeonThemeAsset::StaticClass(), Factory, FName("ContentBrowserNewAsset"));
	UDungeonThemeAsset* ThemeAsset = Cast<UDungeonThemeAsset>(Asset);
	if (!ThemeAsset) return;
	
	ThemeAsset->UpdateGraph = DuplicateObject(OldThemeAsset->UpdateGraph, ThemeAsset);
	ThemeAsset->PreviewViewportProperties = DuplicateObject(OldThemeAsset->PreviewViewportProperties, ThemeAsset);
	ThemeAsset->PreviewViewportProperties->DungeonConfig = DuplicateObject(OldThemeAsset->PreviewViewportProperties->DungeonConfig, ThemeAsset);

	// Copy over the deprecated affinity variable to the new probability variable
	for (UEdGraphNode* GraphNode : ThemeAsset->UpdateGraph->Nodes) {
		if (GraphNode->IsA<UEdGraphNode_DungeonActorBase>()) {
			UEdGraphNode_DungeonActorBase* Node = Cast<UEdGraphNode_DungeonActorBase>(GraphNode);
			Node->Probability = Node->Affinity;
		}
	}

	
}

//////////////////////////////////////////////////////////////////////////


#undef LOCTEXT_NAMESPACE
