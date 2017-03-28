//$ Copyright 2015 Ali Akbar, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//
#include "DungeonArchitectEditorPrivatePCH.h"
#include "DungeonArchitectEditorCustomization.h"
#include "PropertyEditing.h"
#include "SNotificationList.h"
#include "NotificationManager.h"
#include "EditorActorFolders.h"
#include "IDetailsView.h"
#include "AssetSelection.h"

#include "Core/Dungeon.h"
#include "Core/DungeonArchitectEditor.h"
#include "DungeonArchitectEditorModule.h"
#include "Core/Graph/EdGraphNode_DungeonActorBase.h"
#include "Core/Graph/EdGraphNode_DungeonMesh.h"
#include "Core/Utils/DungeonEditorViewportProperties.h"
#include "Core/Volumes/DungeonVolume.h"
#include "IPropertyChangeListener.h"
#include "DungeonDebug.h"
#include "ScopedTransaction.h"
#include "../Utils/DungeonEditorUtils.h"

#define LOCTEXT_NAMESPACE "DungeonArchitectEditorModule" 
DEFINE_LOG_CATEGORY(LogDungeonCustomization)

template<typename T>
T* GetBuilderObject(IDetailLayoutBuilder* DetailBuilder) {
	TArray<TWeakObjectPtr<UObject>> OutObjects;
	DetailBuilder->GetObjectsBeingCustomized(OutObjects);
	T* Obj = nullptr;
	if (OutObjects.Num() > 0) {
		Obj = Cast<T>(OutObjects[0].Get());
	}
	return Obj;
}

ADungeon* GetDungeon(IDetailLayoutBuilder* DetailBuilder) {
	return GetBuilderObject<ADungeon>(DetailBuilder);
}

void ShowDungeonConfigProperties(IDetailLayoutBuilder& DetailBuilder, UDungeonConfig* Config) {
	if (!Config) return;

	TArray<FName> ImportantAttributes;
	ImportantAttributes.Add("Seed");

	TArray<UObject*> Configs;
	Configs.Add(Config);
	FString ConfigName = Config->GetName();
	IDetailCategoryBuilder& ConfigCategory = DetailBuilder.EditCategory(*ConfigName);
	

	// First add the important attributes, so they show up on top of the config properties list
	for (const FName& Attrib : ImportantAttributes) {
		ConfigCategory.AddExternalProperty(Configs, Attrib);
	}

	for (TFieldIterator<UProperty> PropIt(Config->GetClass()); PropIt; ++PropIt)
	{
		UProperty* Property = *PropIt;
		FName PropertyName(*(Property->GetName()));

		// Make sure we skip the important attrib, since we have already added them above
		if (ImportantAttributes.Contains(PropertyName)) {
			// Already added
			continue;
		}

		ConfigCategory.AddExternalProperty(Configs, PropertyName);
	}
}

void FDungeonArchitectEditorCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) {
	IDetailCategoryBuilder& Category = DetailBuilder.EditCategory("Dungeon");

	Category.AddCustomRow(LOCTEXT("DungeonCommand_FilterBuildDungeon", "build dungeon"))
		.ValueContent()
		[
			SNew(SButton)
			.Text(LOCTEXT("DungeonCommand_BuildDungeon", "Build Dungeon"))
			.OnClicked(FOnClicked::CreateStatic(&FDungeonArchitectEditorCustomization::RebuildDungeon, &DetailBuilder))
		];

	Category.AddCustomRow(LOCTEXT("DungeonCommand_FilterDestroyDungeon", "destroy dungeon"))
		.ValueContent()
		[
			SNew(SButton)
			.Text(LOCTEXT("DungeonCommand_DestroyDungeon", "Destroy Dungeon"))
			.OnClicked(FOnClicked::CreateStatic(&FDungeonArchitectEditorCustomization::DestroyDungeon, &DetailBuilder))
		];

	Category.AddCustomRow(LOCTEXT("DungeonCommand_FilterRandomizeSeed", "randomize seed"))
		.ValueContent()
		[
			SNew(SButton)
			.Text(LOCTEXT("DungeonCommand_RandomizeSeed", "Randomize Seed"))
			.OnClicked(FOnClicked::CreateStatic(&FDungeonArchitectEditorCustomization::RandomizeSeed, &DetailBuilder))
		];

	
	////////////// CONFIG CATEGORY //////////////
	ADungeon* Dungeon = GetDungeon(&DetailBuilder);
	if (Dungeon) {
		UDungeonConfig* ConfigObject = Dungeon->GetConfig();
		if (ConfigObject) {
			ShowDungeonConfigProperties(DetailBuilder, ConfigObject);
		}
	}

	// Hide unsupported properties
	{
		UDungeonBuilder* Builder = Dungeon->GetBuilder();
		TArray<FString> PropertyNames;
		PropertyNames.Add("Themes");
		PropertyNames.Add("MarkerEmitters");
		PropertyNames.Add("EventListeners");
		for (const FString& PropertyName : PropertyNames) {
			TSharedRef<IPropertyHandle> PropertyHandle = DetailBuilder.GetProperty(*PropertyName);
			if (!Builder->SupportsProperty(*PropertyName)) {
				PropertyHandle->MarkHiddenByCustomization();
			}
		}
	}
}


TSharedRef<IDetailCustomization> FDungeonArchitectEditorCustomization::MakeInstance() {
	return MakeShareable(new FDungeonArchitectEditorCustomization);
}

void ShowNotification(FText Text, SNotificationItem::ECompletionState State = SNotificationItem::CS_Fail) {
	FNotificationInfo Info(Text);
	Info.bFireAndForget = true;
	Info.FadeOutDuration = 1.0f;
	Info.ExpireDuration = 2.0f;

	TWeakPtr<SNotificationItem> NotificationPtr = FSlateNotificationManager::Get().AddNotification(Info);
	if (NotificationPtr.IsValid())
	{
		NotificationPtr.Pin()->SetCompletionState(State);
	}
}

void SwitchToRealtimeMode() {
    FEditorViewportClient* client = (FEditorViewportClient*)GEditor->GetActiveViewport()->GetClient();
    if (client) {
        bool bRealtime = client->IsRealtime();
        if (!bRealtime) {
            ShowNotification(NSLOCTEXT("DungeonRealtimeMode", "DungeonRealtimeMode", "Switched viewport to Realtime mode"), SNotificationItem::CS_None);
            client->SetRealtime(true);
        }
    } else {
        ShowNotification(NSLOCTEXT("ClientNotFound", "ClientNotFound", "Warning: Cannot find active viewport"));
    }

}

void CreateDungeonItemFolder(ADungeon* Dungeon) {
	if (Dungeon && FActorFolders::IsAvailable()) {
		UWorld& World = *Dungeon->GetWorld();
		const FScopedTransaction Transaction(LOCTEXT("UndoAction_CreateFolder", "Create Folder"));
		const FName NewFolderName = FActorFolders::Get().GetDefaultFolderNameForSelection(World);

		auto& Folders = FActorFolders::Get();
		FString FullPath = Dungeon->GetName() + "_Items";
		FName Path(*FullPath);

		Folders.CreateFolder(World, Path);
		Dungeon->ItemFolderPath = Path;
	}
	else {
		// Folder manager does not exist.  Clear the folder path from the dungeon actor,
		// so they are spawned in the root folder node
		Dungeon->ItemFolderPath = FName();
	}
}

void BuildDungeon(ADungeon* Dungeon) {
	SwitchToRealtimeMode();
	CreateDungeonItemFolder(Dungeon);
	Dungeon->BuildDungeon();
}

class DungeonEditorPostBuildHandler {
public:
	void HandleEditorPostDungeonBuild(ADungeon* Dungeon) {

	}
};

FReply FDungeonArchitectEditorCustomization::RebuildDungeon(IDetailLayoutBuilder* DetailBuilder) {
	ADungeon* Dungeon = GetDungeon(DetailBuilder);
    
	if (!Dungeon) {
		// invalid state
		return FReply::Handled();
	}

	UDungeonBuilder* Builder = Dungeon->GetBuilder();
	if (!Builder) {
		ShowNotification(NSLOCTEXT("DungeonMissingBuilder", "BuilderNotAssigned", "Dungeon Builder not assigned"));
		return FReply::Handled();
	}

	bool bContainValidThemes = false;
	if (Builder->SupportsTheming()) {
		// make sure we have at least one theme defined
		for (UDungeonThemeAsset* Theme : Dungeon->Themes) {
			if (Theme) {
				bContainValidThemes = true;
				break;
			}
		}
	}
	else {
		// themes are not supported. We don't require a theme to be defined
		bContainValidThemes = true;
	}

	if (!bContainValidThemes) {
		// Notify user that a theme has not been assigned
		ShowNotification(NSLOCTEXT("DungeonMissingTheme", "ThemeNotAssigned", "Dungeon Theme Not Assigned"));
		return FReply::Handled();
	}
	
	BuildDungeon(Dungeon);
	
	return FReply::Handled();
}

FReply FDungeonArchitectEditorCustomization::DestroyDungeon(IDetailLayoutBuilder* DetailBuilder) {
	ADungeon* Dungeon = GetDungeon(DetailBuilder);
	if (Dungeon) {
        SwitchToRealtimeMode();
		Dungeon->DestroyDungeon();
	}

	return FReply::Handled();
}


FReply FDungeonArchitectEditorCustomization::RandomizeSeed(IDetailLayoutBuilder* DetailBuilder)
{
	ADungeon* Dungeon = GetDungeon(DetailBuilder);
	if (Dungeon) {
		Dungeon->GetConfig()->Seed = FMath::Rand();
	}

	return FReply::Handled();
}

void FDungeonArchitectMeshNodeCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	IDetailCategoryBuilder& Category = DetailBuilder.EditCategory("Dungeon");

	Category.AddCustomRow(LOCTEXT("DungeonCommand_FilterBuildDungeon", "build dungeon"))
		.WholeRowContent()
		[
			SNew(SButton)
			.Text(LOCTEXT("DungeonCommand_ShowMeshNodeProperties", "Edit Advanced Properties"))
			.OnClicked(FOnClicked::CreateStatic(&FDungeonArchitectMeshNodeCustomization::EditAdvancedOptions, &DetailBuilder))
		];
}

FReply FDungeonArchitectMeshNodeCustomization::EditAdvancedOptions(IDetailLayoutBuilder* DetailBuilder)
{
	TArray<TWeakObjectPtr<UObject>> ObjectsBeingCustomized;
	DetailBuilder->GetObjectsBeingCustomized(ObjectsBeingCustomized);

	if (ObjectsBeingCustomized.Num() > 1) {
		ShowNotification(NSLOCTEXT("TooManyObjects", "TooManyObjects", "Only a single node can be edited at a time"));
		return FReply::Handled();
	}

	UObject* ObjectToEdit = nullptr;
	if (ObjectsBeingCustomized.Num() == 0) {
		ObjectToEdit = ObjectsBeingCustomized[0].Get();
	}

	if (!ObjectToEdit) {
		ShowNotification(NSLOCTEXT("ObjectNotValid", "ObjectNotValid", "Node state is not valid"));
		return FReply::Handled();
	}

	if (UEdGraphNode_DungeonMesh* MeshNode = Cast<UEdGraphNode_DungeonMesh>(ObjectToEdit)) {
		FPropertyEditorModule& PropertyEditorModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
		TMap<UObject*, UObject*> OldToNewObjectMap;
	}

	return FReply::Handled();
}

void FDungeonEditorViewportPropertiesCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	UDungeonEditorViewportProperties* ViewportProperties = GetBuilderObject<UDungeonEditorViewportProperties>(&DetailBuilder);
	if (ViewportProperties) {
		ShowDungeonConfigProperties(DetailBuilder, ViewportProperties->DungeonConfig);
	}
}

TSharedRef<IDetailCustomization> FDungeonEditorViewportPropertiesCustomization::MakeInstance()
{
	return MakeShareable(new FDungeonEditorViewportPropertiesCustomization);
}

/////////////////////////////// Volume Customization //////////////////////////
void FDungeonArchitectVolumeCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) {
	IDetailCategoryBuilder& Category = DetailBuilder.EditCategory("Dungeon");
	Category.AddCustomRow(LOCTEXT("DungeonVolumeCommand_FilterBuildDungeon", "build dungeon"))
		.ValueContent()
		[
			SNew(SButton)
			.Text(LOCTEXT("DungeonVolumeCommand_BuildDungeon", "Rebuild Dungeon"))
			.OnClicked(FOnClicked::CreateStatic(&FDungeonArchitectVolumeCustomization::RebuildDungeon, &DetailBuilder))
		];
}


TSharedRef<IDetailCustomization> FDungeonArchitectVolumeCustomization::MakeInstance()
{
	return MakeShareable(new FDungeonArchitectVolumeCustomization);
}

FReply FDungeonArchitectVolumeCustomization::RebuildDungeon(IDetailLayoutBuilder* DetailBuilder) {
	ADungeonVolume* Volume = GetBuilderObject<ADungeonVolume>(DetailBuilder);
	if (Volume && Volume->Dungeon) {
		BuildDungeon(Volume->Dungeon);
	}
	return FReply::Handled();
}

void FDungeonPropertyChangeListener::Initialize()
{
	FCoreUObjectDelegates::OnObjectPropertyChanged.AddSP(SharedThis(this), &FDungeonPropertyChangeListener::OnPropertyChanged);
}

void FDungeonPropertyChangeListener::OnPropertyChanged(UObject* Object, struct FPropertyChangedEvent& Event)
{
	if (Object->IsA(ADungeon::StaticClass())) {
		FName PropertyName = (Event.Property != NULL) ? Event.Property->GetFName() : NAME_None;
		if (PropertyName == "BuilderClass") {
			// Refresh the properties window
			UE_LOG(LogDungeonCustomization, Log, TEXT("Dungeon builder class changed"));
			FPropertyEditorModule& PropertyEditorModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
			TArray<UObject*> ObjectList;
			ObjectList.Add(Object);
			PropertyEditorModule.UpdatePropertyViews(ObjectList);
		}
	}
}

//////////////// FDungeonDebugCustomization ////////////////////
void FDungeonDebugCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	IDetailCategoryBuilder& Category = DetailBuilder.EditCategory("Debug Commands");
	for (int i = 0; i < 10; i++) {
		FText Caption = FText::FromString("Command " + FString::FromInt(i));

		Category.AddCustomRow(Caption)
			.ValueContent()
			[
				SNew(SButton)
				.Text(Caption)
				.OnClicked(FOnClicked::CreateStatic(&FDungeonDebugCustomization::ExecuteCommand, &DetailBuilder, i))
			];
	}
}

TSharedRef<IDetailCustomization> FDungeonDebugCustomization::MakeInstance()
{
	return MakeShareable(new FDungeonDebugCustomization);
}

FReply FDungeonDebugCustomization::ExecuteCommand(IDetailLayoutBuilder* DetailBuilder, int32 CommandID)
{
	ADungeonDebug* Debug = FDungeonEditorUtils::GetBuilderObject<ADungeonDebug>(DetailBuilder);
	if (Debug) {
		Debug->ExecuteDebugCommand(CommandID);
	}
	return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE
