//$ Copyright 2015-2016, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//
#include "DungeonArchitectEditorPrivatePCH.h"
#include "Toolkits/IToolkitHost.h"
#include "SnapDoorEditor.h"
#include "SNodePanel.h"
#include "SDockTab.h"
#include "SSingleObjectDetailsPanel.h"
#include "GenericCommands.h"
#include "BlueprintEditorUtils.h"
#include "ScopedTransaction.h"
#include "EditorSupportDelegates.h"
#include "IDetailsView.h"
#include "Preview3D/SSnapDoorPreview3DViewport.h"
#include "SnapDoorEditorCommands.h"
#include "SnapDoor.h"
#include "SSnapDoorEdit.h"


#define LOCTEXT_NAMESPACE "FSnapDoorEditor" 

DEFINE_LOG_CATEGORY_STATIC(SnapDoorEditor, Log, All);

const FName LAThemeEditorAppName = FName(TEXT("LAThemeEditorApp"));

struct FSnapDoorEditorTabs
{
	// Tab identifiers
	static const FName Preview3DID;
	static const FName DetailsID;
};


//////////////////////////////////////////////////////////////////////////

const FName FSnapDoorEditorTabs::DetailsID(TEXT("Details"));
const FName FSnapDoorEditorTabs::Preview3DID(TEXT("Preview3D"));

FName FSnapDoorEditor::GetToolkitFName() const
{
	return FName("SnapDoorEditor");
}

FText FSnapDoorEditor::GetBaseToolkitName() const
{
	return LOCTEXT("LAThemeEditorAppLabel", "Tree Theme Editor");
}

FText FSnapDoorEditor::GetToolkitName() const
{
	const bool bDirtyState = DoorBeingEdited->GetOutermost()->IsDirty();

	FFormatNamedArguments Args;
	Args.Add(TEXT("ThemeName"), FText::FromString(DoorBeingEdited->GetName()));
	Args.Add(TEXT("DirtyState"), bDirtyState ? FText::FromString(TEXT("*")) : FText::GetEmpty());
	return FText::Format(LOCTEXT("SnapDoorEditorAppLabel", "{ThemeName}{DirtyState}"), Args);
}

FString FSnapDoorEditor::GetWorldCentricTabPrefix() const
{
	return TEXT("TreeArchitectThemeEditor");
}

FString FSnapDoorEditor::GetDocumentationLink() const
{
	return TEXT("TreeArchitect/ThemeEditor");
}

void FSnapDoorEditor::NotifyPreChange(UProperty* PropertyAboutToChange)
{

}

void FSnapDoorEditor::NotifyPostChange(const FPropertyChangedEvent& PropertyChangedEvent, UProperty* PropertyThatChanged)
{

}

FLinearColor FSnapDoorEditor::GetWorldCentricTabColorScale() const
{
	return FLinearColor::White;
}

//////////////////////////////////////////////////////////////////////////
// FSnapDoorEditor

TSharedRef<SDockTab> FSnapDoorEditor::SpawnTab_Details(const FSpawnTabArgs& Args)
{
	PropertyEditor = SNew(SSnapDoorEdit)
		.SnapDoorEditor(SharedThis(this))
		.ObjectToEdit(DoorBeingEdited);

	// Spawn the tab
	return SNew(SDockTab)
		.Label(LOCTEXT("DetailsTab_Title", "Details"))
		[
			PropertyEditor.ToSharedRef()
		];
}

bool FSnapDoorEditor::IsTickableInEditor() const
{
	return true;
}

void FSnapDoorEditor::Tick(float DeltaTime)
{
}

bool FSnapDoorEditor::IsTickable() const
{
	return true;
}

TStatId FSnapDoorEditor::GetStatId() const
{
	return TStatId();
}

TSharedRef<SDockTab> FSnapDoorEditor::SpawnTab_Preview3D(const FSpawnTabArgs& Args)
{
	TSharedRef<SDockTab> SpawnedTab =
		SNew(SDockTab)
		.Label(LOCTEXT("Preview3D", "Preview 3D"))
		.TabColorScale(GetTabColorScale())
		[
			PreviewViewport.ToSharedRef()
		];

	PreviewViewport->SetParentTab(SpawnedTab);
	return SpawnedTab;
}

void FSnapDoorEditor::SaveAsset_Execute()
{
	UE_LOG(SnapDoorEditor, Log, TEXT("Saving snap door asset %s"), *GetEditingObjects()[0]->GetName());

	UpdateOriginalDoorAsset();

	UPackage* Package = DoorBeingEdited->GetOutermost();
	if (Package)
	{
		TArray<UPackage*> PackagesToSave;
		PackagesToSave.Add(Package);

		FEditorFileUtils::PromptForCheckoutAndSave(PackagesToSave, false, false);
	}

}

void FSnapDoorEditor::UpdateOriginalDoorAsset()
{
	FEditorDelegates::RefreshEditor.Broadcast();
	FEditorSupportDelegates::RedrawAllViewports.Broadcast();
}

FSnapDoorEditor::~FSnapDoorEditor()
{

}

void FSnapDoorEditor::RegisterTabSpawners(const TSharedRef<class FTabManager>& InTabManager)
{
	WorkspaceMenuCategory = InTabManager->AddLocalWorkspaceMenuCategory(LOCTEXT("WorkspaceMenu_SnapDoorEditor", "Snap Door Editor"));
	auto WorkspaceMenuCategoryRef = WorkspaceMenuCategory.ToSharedRef();

	FAssetEditorToolkit::RegisterTabSpawners(InTabManager);

	InTabManager->RegisterTabSpawner(FSnapDoorEditorTabs::DetailsID, FOnSpawnTab::CreateSP(this, &FSnapDoorEditor::SpawnTab_Details))
		.SetDisplayName(LOCTEXT("DetailsTabLabel", "Details"))
		.SetGroup(WorkspaceMenuCategoryRef)
		.SetIcon(FSlateIcon(FEditorStyle::GetStyleSetName(), "LevelEditor.Tabs.Details"));

	InTabManager->RegisterTabSpawner(FSnapDoorEditorTabs::Preview3DID, FOnSpawnTab::CreateSP(this, &FSnapDoorEditor::SpawnTab_Preview3D))
		.SetDisplayName(LOCTEXT("Preview3DTab", "Preview 3D"))
		.SetGroup(WorkspaceMenuCategoryRef)
		.SetIcon(FSlateIcon(FEditorStyle::GetStyleSetName(), "LevelEditor.Tabs.Viewports"));
}

void FSnapDoorEditor::UnregisterTabSpawners(const TSharedRef<class FTabManager>& InTabManager)
{
	FAssetEditorToolkit::UnregisterTabSpawners(InTabManager);

	InTabManager->UnregisterTabSpawner(FSnapDoorEditorTabs::Preview3DID);
	InTabManager->UnregisterTabSpawner(FSnapDoorEditorTabs::DetailsID);
}

void FSnapDoorEditor::ExtendMenu() {
	struct Local
	{
		static void FillToolbar(FToolBarBuilder& ToolbarBuilder)
		{
			ToolbarBuilder.BeginSection("Tree");
			{
				ToolbarBuilder.AddToolBarButton(FSnapDoorEditorCommands::Get().Rebuild);
			}
			ToolbarBuilder.EndSection();
		}
	};

	TSharedPtr<FExtender> ToolbarExtender = MakeShareable(new FExtender);
	ToolbarExtender->AddToolBarExtension(
		"Asset",
		EExtensionHook::After,
		GetToolkitCommands(),
		FToolBarExtensionDelegate::CreateStatic(&Local::FillToolbar)
		);
	AddToolbarExtender(ToolbarExtender);
}


void FSnapDoorEditor::BindCommands()
{

	const FSnapDoorEditorCommands& Commands = FSnapDoorEditorCommands::Get();

	ToolkitCommands->MapAction(
		Commands.Rebuild,
		FExecuteAction::CreateSP(this, &FSnapDoorEditor::HandleRebuildActionExecute));
}

void FSnapDoorEditor::HandleRebuildActionExecute()
{
	RebuildDoor();
}

void FSnapDoorEditor::RebuildDoor()
{
	if (PropertyEditor.IsValid()) {
		ESnapDoorMode DoorVisualMode = PropertyEditor->GetDoorVisualMode();
		ASnapDoor* Door = PreviewViewport->GetDoor();
		Door->DoorAsset = DoorBeingEdited;
		Door->VisualMode = DoorVisualMode;
		Door->BuildDoor();
	}
}

void FSnapDoorEditor::InitSnapDoorEditor(const EToolkitMode::Type Mode, const TSharedPtr< class IToolkitHost >& InitToolkitHost, USnapDoorAsset* DoorAsset) {

	// Initialize the asset editor and spawn nothing (dummy layout)
	FAssetEditorManager::Get().CloseOtherEditors(DoorAsset, this);
	DoorBeingEdited = DoorAsset;

	FSnapDoorEditorCommands::Register();
	BindCommands();
	ExtendMenu();

	PreviewViewport = SNew(SSnapDoorPreview3DViewport)
		.SnapDoorEditor(SharedThis(this))
		.DoorAsset(DoorBeingEdited);


	// Default layout
	const TSharedRef<FTabManager::FLayout> StandaloneDefaultLayout = FTabManager::NewLayout("Standalone_TreeArchitectEditor_Layout_v1")
		->AddArea
		(
			FTabManager::NewPrimaryArea()
			->SetOrientation(Orient_Vertical)
			->Split
			(
				FTabManager::NewStack()
				->SetSizeCoefficient(0.1f)
				->SetHideTabWell(true)
				->AddTab(GetToolbarTabId(), ETabState::OpenedTab)
				)
			->Split
			(
				FTabManager::NewSplitter()
				->SetOrientation(Orient_Horizontal)
				->SetSizeCoefficient(1.0f)
				->Split
				(

					FTabManager::NewStack()
					->SetSizeCoefficient(0.5f)
					->AddTab(FSnapDoorEditorTabs::DetailsID, ETabState::OpenedTab)
				)

				->Split
				(
					FTabManager::NewStack()
					->SetSizeCoefficient(0.5f)
					->SetHideTabWell(true)
					->AddTab(FSnapDoorEditorTabs::Preview3DID, ETabState::OpenedTab)
				)
			)
		);

	// Initialize the asset editor and spawn nothing (dummy layout)
	InitAssetEditor(Mode, InitToolkitHost, LAThemeEditorAppName, StandaloneDefaultLayout, /*bCreateDefaultStandaloneMenu=*/ true, /*bCreateDefaultToolbar=*/ true, DoorAsset);
	
	RebuildDoor(); 
}

void FSnapDoorEditor::OnPropertyChanged(UObject* ObjectBeingModified, FPropertyChangedEvent& PropertyChangedEvent)
{
	RebuildDoor();
}

#undef LOCTEXT_NAMESPACE

