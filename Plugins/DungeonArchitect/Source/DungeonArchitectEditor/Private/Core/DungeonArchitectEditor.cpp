//$ Copyright 2015 Ali Akbar, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//
#include "DungeonArchitectEditorPrivatePCH.h"
#include "Toolkits/IToolkitHost.h"
#include "DungeonArchitectEditor.h"
#include "Editor/WorkspaceMenuStructure/Public/WorkspaceMenuStructureModule.h"
#include "GraphEditor.h"
#include "GraphEditorActions.h"
#include "EdGraphUtilities.h"
#include "SNodePanel.h"
#include "SoundCueGraphEditorCommands.h"
#include "SDockTab.h"
#include "SSingleObjectDetailsPanel.h"
#include "SAdvancedPreviewDetailsTab.h"
#include "GenericCommands.h"
#include "Core/Graph/EdGraph_DungeonProp.h"
#include "BlueprintEditorUtils.h"
#include "ScopedTransaction.h"
#include "Core/Graph/EdGraphNode_DungeonMarker.h"
#include "Core/Graph/EdGraphNode_DungeonMesh.h"
#include "EditorSupportDelegates.h"
#include "Core/Graph/EdGraphNode_DungeonBase.h"
#include "PropertyEditorModule.h"
#include "IDetailsView.h"
#include "Widgets/SGraphPalette_PropActions.h" 
#include "Widgets/SDungeonEditorViewport.h"
#include "EdGraph/EdGraph.h"
#include "DungeonBuilder.h"


#define LOCTEXT_NAMESPACE "FDungeonArchitectEditor" 

DEFINE_LOG_CATEGORY_STATIC(DungeonEditor, Log, All);

const FName DungeonEditorAppName = FName(TEXT("DungeonEditorApp"));
TSharedPtr<FDungeonEditorThumbnailPool> FDungeonEditorThumbnailPool::Instance;

struct FDungeonEditorTabs
{
	// Tab identifiers
	static const FName DetailsID;
	static const FName ActionsID;
	static const FName ViewportID;
    static const FName PreviewID;
    static const FName PreviewSettingsID;
	static const FName MarkersID;
};


//////////////////////////////////////////////////////////////////////////

const FName FDungeonEditorTabs::DetailsID(TEXT("Details"));
const FName FDungeonEditorTabs::ViewportID(TEXT("Viewport"));
const FName FDungeonEditorTabs::PreviewID(TEXT("Preview"));
const FName FDungeonEditorTabs::PreviewSettingsID(TEXT("PreviewSettings"));
const FName FDungeonEditorTabs::ActionsID(TEXT("Actions"));
const FName FDungeonEditorTabs::MarkersID(TEXT("Markers"));


FName FDungeonArchitectEditor::GetToolkitFName() const
{
	return FName("DungeonEditor");
}

FText FDungeonArchitectEditor::GetBaseToolkitName() const
{
	return LOCTEXT("DungeonEditorAppLabel", "Dungeon Editor");
}

FText FDungeonArchitectEditor::GetToolkitName() const
{
	const bool bDirtyState = PropBeingEdited->GetOutermost()->IsDirty();

	FFormatNamedArguments Args;
	Args.Add(TEXT("DungeonName"), FText::FromString(PropBeingEdited->GetName()));
	Args.Add(TEXT("DirtyState"), bDirtyState ? FText::FromString(TEXT("*")) : FText::GetEmpty());
	return FText::Format(LOCTEXT("DungeonEditorAppLabel", "{DungeonName}{DirtyState}"), Args);
}

FString FDungeonArchitectEditor::GetWorldCentricTabPrefix() const
{
	return TEXT("DungeonEditor");
}

FString FDungeonArchitectEditor::GetDocumentationLink() const
{
	return TEXT("Dungeon/DungeonEditor");
}

void FDungeonArchitectEditor::NotifyPreChange(UProperty* PropertyAboutToChange)
{

}

void FDungeonArchitectEditor::NotifyPostChange(const FPropertyChangedEvent& PropertyChangedEvent, UProperty* PropertyThatChanged)
{

}

FLinearColor FDungeonArchitectEditor::GetWorldCentricTabColorScale() const
{
	return FLinearColor::White;
}

//////////////////////////////////////////////////////////////////////////
// FDungeonArchitectEditor

TSharedRef<SDockTab> FDungeonArchitectEditor::SpawnTab_Details(const FSpawnTabArgs& Args)
{
	FPropertyEditorModule& PropertyEditorModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
	const FDetailsViewArgs DetailsViewArgs(false, false, true, FDetailsViewArgs::HideNameArea, true, this);
	TSharedRef<IDetailsView> PropertyEditorRef = PropertyEditorModule.CreateDetailView(DetailsViewArgs);
	PropertyEditor = PropertyEditorRef;

	// Spawn the tab
	return SNew(SDockTab)
		.Label(LOCTEXT("DetailsTab_Title", "Details"))
		[
			PropertyEditorRef
		];
}

TSharedRef<SDockTab> FDungeonArchitectEditor::SpawnTab_Actions(const FSpawnTabArgs& Args)
{
	ActionPalette = SNew(SGraphPalette_PropActions, SharedThis(this));

	TSharedRef<SDockTab> SpawnedTab = SNew(SDockTab)
		.Icon(FEditorStyle::GetBrush("Kismet.Tabs.Palette"))
		.Label(LOCTEXT("ActionsPaletteTitle", "Actions"))
		[
			SNew(SBox)
			.AddMetaData<FTagMetaData>(FTagMetaData(TEXT("ActionsPalette")))
			[
				ActionPalette.ToSharedRef()
			]
		];

	return SpawnedTab;
}


TSharedRef<SDockTab> FDungeonArchitectEditor::SpawnTab_Markers(const FSpawnTabArgs& Args)
{
	/*
	TArray<UEdGraphNode_DungeonMarker*> MarkerItems;
	GetMarkerNodes(MarkerItems);
	MarkersListView = SNew(SMarkerListView)
		.ItemHeight(24)
		.ListItemsSource(&MarkerItems)
		.OnGenerateRow(this, &FDungeonArchitectEditor::GenerateMakerListRow);
		*/
	TSharedRef<SDockTab> SpawnedTab = SNew(SDockTab)
		.Icon(FEditorStyle::GetBrush("Kismet.Tabs.Palette"))
		.Label(LOCTEXT("MarkersTitle", "Markers"))
		[
			SNew(SBox)
			.AddMetaData<FTagMetaData>(FTagMetaData(TEXT("Markers")))
			[
				//MarkersListView.ToSharedRef()
				SNullWidget::NullWidget
			]
		];

	return SpawnedTab;
}

bool FDungeonArchitectEditor::IsTickableInEditor() const
{
	return true;
}

void FDungeonArchitectEditor::Tick(float DeltaTime)
{
	if (bGraphStateChanged) {
		bGraphStateChanged = false;
		HandleGraphChanged();
	}
}

bool FDungeonArchitectEditor::IsTickable() const
{
	return true;
}

TStatId FDungeonArchitectEditor::GetStatId() const
{
	return TStatId();
}

TSharedRef<SDockTab> FDungeonArchitectEditor::SpawnTab_Viewport(const FSpawnTabArgs& Args)
{

	return SNew(SDockTab)
		.Label(LOCTEXT("PropMeshGraph", "Mesh Graph"))
		.TabColorScale(GetTabColorScale())
		[
			GraphEditor.ToSharedRef()
		];

}

TSharedRef<SDockTab> FDungeonArchitectEditor::SpawnTab_Preview(const FSpawnTabArgs& Args)
{
	TSharedRef<SDockTab> SpawnedTab =
		SNew(SDockTab)
			.Label(LOCTEXT("PropPreview", "Preview"))
			.TabColorScale(GetTabColorScale())
			[
				PreviewViewport.ToSharedRef()
			];

	PreviewViewport->SetParentTab(SpawnedTab);
	return SpawnedTab;
}


TSharedRef<SDockTab> FDungeonArchitectEditor::SpawnTab_PreviewSettings(const FSpawnTabArgs& Args)
{
    TSharedPtr<FAdvancedPreviewScene> PreviewScene;
    
    if (PreviewViewport.IsValid()) {
        PreviewScene = PreviewViewport->GetAdvancedPreview();
    }
    
    
    TSharedPtr<SWidget> SettingsWidget = SNullWidget::NullWidget;
    if (PreviewScene.IsValid()) {
        TSharedPtr<SAdvancedPreviewDetailsTab> PreviewSettingsWidget = SNew(SAdvancedPreviewDetailsTab)
        .PreviewScenePtr(PreviewScene.Get());
        
        PreviewSettingsWidget->Refresh();
        
        SettingsWidget = PreviewSettingsWidget;
    }
    
    TSharedRef<SDockTab> SpawnedTab = SNew(SDockTab)
    .Icon(FEditorStyle::GetBrush("Kismet.Tabs.Palette"))
    .Label(LOCTEXT("PreviewSettingsTitle", "Preview Settings"))
    [
     SNew(SBox)
     .AddMetaData<FTagMetaData>(FTagMetaData(TEXT("Preview Settings")))
     [
      SettingsWidget.ToSharedRef()
      ]
     ];
    
    return SpawnedTab;
}

TSharedRef<SGraphEditor> FDungeonArchitectEditor::CreateGraphEditorWidget(UEdGraph* InGraph) {
	// Create the appearance info
	FGraphAppearanceInfo AppearanceInfo;
	AppearanceInfo.CornerText = LOCTEXT("AppearanceCornerText", "Dungeon Architect");

	GraphEditorCommands = MakeShareable(new FUICommandList);
	{
		GraphEditorCommands->MapAction(FGenericCommands::Get().SelectAll,
			FExecuteAction::CreateSP(this, &FDungeonArchitectEditor::SelectAllNodes),
			FCanExecuteAction::CreateSP(this, &FDungeonArchitectEditor::CanSelectAllNodes)
			);

		GraphEditorCommands->MapAction(FGenericCommands::Get().Delete,
			FExecuteAction::CreateSP(this, &FDungeonArchitectEditor::DeleteSelectedNodes),
			FCanExecuteAction::CreateSP(this, &FDungeonArchitectEditor::CanDeleteNodes)
			);

		GraphEditorCommands->MapAction(FGenericCommands::Get().Copy,
			FExecuteAction::CreateSP(this, &FDungeonArchitectEditor::CopySelectedNodes),
			FCanExecuteAction::CreateSP(this, &FDungeonArchitectEditor::CanCopyNodes)
			);

		GraphEditorCommands->MapAction(FGenericCommands::Get().Paste,
			FExecuteAction::CreateSP(this, &FDungeonArchitectEditor::PasteNodes),
			FCanExecuteAction::CreateSP(this, &FDungeonArchitectEditor::CanPasteNodes)
			);

		GraphEditorCommands->MapAction(FGenericCommands::Get().Cut,
			FExecuteAction::CreateSP(this, &FDungeonArchitectEditor::CutSelectedNodes),
			FCanExecuteAction::CreateSP(this, &FDungeonArchitectEditor::CanCutNodes)
			);

		GraphEditorCommands->MapAction(FGenericCommands::Get().Duplicate,
			FExecuteAction::CreateSP(this, &FDungeonArchitectEditor::DuplicateNodes),
			FCanExecuteAction::CreateSP(this, &FDungeonArchitectEditor::CanDuplicateNodes)
			);

	}

	SGraphEditor::FGraphEditorEvents InEvents;
	InEvents.OnSelectionChanged = SGraphEditor::FOnSelectionChanged::CreateSP(this, &FDungeonArchitectEditor::OnSelectedNodesChanged);
	InEvents.OnNodeDoubleClicked = FSingleNodeEvent::CreateSP(this, &FDungeonArchitectEditor::OnNodeDoubleClicked);

	// Make title bar
	TSharedRef<SWidget> TitleBarWidget =
		SNew(SBorder)
		.BorderImage(FEditorStyle::GetBrush(TEXT("Graph.TitleBackground")))
		.HAlign(HAlign_Fill)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.HAlign(HAlign_Center)
			.FillWidth(1.f)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("UpdateGraphLabel", "Mesh Graph"))
				.TextStyle(FEditorStyle::Get(), TEXT("GraphBreadcrumbButtonText"))
			]
		];

	TSharedRef<SGraphEditor> _GraphEditor = SNew(SGraphEditor_Dungeon)
		.AdditionalCommands(GraphEditorCommands)
		.Appearance(AppearanceInfo)
		//.TitleBar(TitleBarWidget)
		.GraphToEdit(InGraph)
		.GraphEvents(InEvents)
		;

	
	return _GraphEditor;
}

void FDungeonArchitectEditor::SelectAllNodes()
{
	GraphEditor->SelectAllNodes();
}

bool FDungeonArchitectEditor::CanSelectAllNodes() const
{
	return GraphEditor.IsValid();
}

void FDungeonArchitectEditor::DeleteSelectedNodes()
{
	TArray<UEdGraphNode*> NodesToDelete;
	const FGraphPanelSelectionSet SelectedNodes = GraphEditor->GetSelectedNodes();

	for (FGraphPanelSelectionSet::TConstIterator NodeIt(SelectedNodes); NodeIt; ++NodeIt)
	{
		NodesToDelete.Add(CastChecked<UEdGraphNode>(*NodeIt));
	}

	DeleteNodes(NodesToDelete);
}

bool FDungeonArchitectEditor::CanDeleteNode(class UEdGraphNode* Node)
{
	bool CanDelete = true;

	if (UEdGraphNode_DungeonMarker* MarkerNode = Cast<UEdGraphNode_DungeonMarker>(Node)) {
		CanDelete = MarkerNode->bUserDefined;
	}

	return CanDelete;
}

void FDungeonArchitectEditor::DeleteNodes(const TArray<class UEdGraphNode*>& NodesToDelete)
{
	if (NodesToDelete.Num() > 0)
	{

		for (int32 Index = 0; Index < NodesToDelete.Num(); ++Index)
		{
			if (!CanDeleteNode(NodesToDelete[Index])) {
				continue;
			}

			// Break all node links first so that we don't update the material before deleting
			NodesToDelete[Index]->BreakAllNodeLinks();

			FBlueprintEditorUtils::RemoveNode(NULL, NodesToDelete[Index], true);

			// TODO: Process deletion in the data model
		}
	}
}

void FDungeonArchitectEditor::DeleteSelectedDuplicatableNodes()
{
	// Cache off the old selection
	const FGraphPanelSelectionSet OldSelectedNodes = GraphEditor->GetSelectedNodes();

	// Clear the selection and only select the nodes that can be duplicated
	FGraphPanelSelectionSet RemainingNodes;
	GraphEditor->ClearSelectionSet();

	for (FGraphPanelSelectionSet::TConstIterator SelectedIter(OldSelectedNodes); SelectedIter; ++SelectedIter)
	{
		UEdGraphNode* Node = Cast<UEdGraphNode>(*SelectedIter);
		if ((Node != NULL) && Node->CanDuplicateNode())
		{
			GraphEditor->SetNodeSelection(Node, true);
		}
		else
		{
			RemainingNodes.Add(Node);
		}
	}

	// Delete the duplicatable nodes
	DeleteSelectedNodes();

	// Reselect whatever is left from the original selection after the deletion
	GraphEditor->ClearSelectionSet();

	for (FGraphPanelSelectionSet::TConstIterator SelectedIter(RemainingNodes); SelectedIter; ++SelectedIter)
	{
		if (UEdGraphNode* Node = Cast<UEdGraphNode>(*SelectedIter))
		{
			GraphEditor->SetNodeSelection(Node, true);
		}
	}
}

bool FDungeonArchitectEditor::CanDeleteNodes() const
{
	return true;
}

void FDungeonArchitectEditor::CopySelectedNodes()
{
	// Export the selected nodes and place the text on the clipboard
	const FGraphPanelSelectionSet SelectedNodes = GraphEditor->GetSelectedNodes();

	FString ExportedText;

	for (FGraphPanelSelectionSet::TConstIterator SelectedIter(SelectedNodes); SelectedIter; ++SelectedIter)
	{
		if (UEdGraphNode* Node = Cast<UEdGraphNode>(*SelectedIter))
		{
			Node->PrepareForCopying();
		}
	}

	FEdGraphUtilities::ExportNodesToText(SelectedNodes, /*out*/ ExportedText);
	FPlatformMisc::ClipboardCopy(*ExportedText);

	// Make sure the owner remains the same for the copied node
	// TODO: Check MaterialEditor.cpp for reference

}

bool FDungeonArchitectEditor::CanCopyNodes() const
{
	// If any of the nodes can be duplicated then we should allow copying
	const FGraphPanelSelectionSet SelectedNodes = GraphEditor->GetSelectedNodes();
	for (FGraphPanelSelectionSet::TConstIterator SelectedIter(SelectedNodes); SelectedIter; ++SelectedIter)
	{
		UEdGraphNode* Node = Cast<UEdGraphNode>(*SelectedIter);
		if ((Node != NULL) && Node->CanDuplicateNode())
		{
			return true;
		}
	}
	return false;
}

void FDungeonArchitectEditor::PasteNodes()
{

	PasteNodesHere(GraphEditor->GetPasteLocation());
}

void FDungeonArchitectEditor::PasteNodesHere(const FVector2D& Location)
{
	// Undo/Redo support
	const FScopedTransaction Transaction(NSLOCTEXT("DungeonArchitect", "DungeonEditorPaste", "Dungeon Editor: Paste"));
	// TODO: Notify the data model of modification
	//Material->MaterialGraph->Modify();
	//Material->Modify();

	// Clear the selection set (newly pasted stuff will be selected)
	GraphEditor->ClearSelectionSet();

	// Grab the text to paste from the clipboard.
	FString TextToImport;
	FPlatformMisc::ClipboardPaste(TextToImport);

	// Import the nodes
	if (!PropBeingEdited) return;
	TSet<UEdGraphNode*> PastedNodes;
	FEdGraphUtilities::ImportNodesFromText(PropBeingEdited->UpdateGraph, TextToImport, /*out*/ PastedNodes);

	//Average position of nodes so we can move them while still maintaining relative distances to each other
	FVector2D AvgNodePosition(0.0f, 0.0f);

	for (TSet<UEdGraphNode*>::TIterator It(PastedNodes); It; ++It)
	{
		UEdGraphNode* Node = *It;
		AvgNodePosition.X += Node->NodePosX;
		AvgNodePosition.Y += Node->NodePosY;
	}

	if (PastedNodes.Num() > 0)
	{
		float InvNumNodes = 1.0f / float(PastedNodes.Num());
		AvgNodePosition.X *= InvNumNodes;
		AvgNodePosition.Y *= InvNumNodes;
	}

	for (TSet<UEdGraphNode*>::TIterator It(PastedNodes); It; ++It)
	{
		UEdGraphNode* Node = *It;
		if (UEdGraphNode_DungeonMesh* MeshNode = Cast<UEdGraphNode_DungeonMesh>(Node))
		{
			// TODO: Handle
		}
		else if (UEdGraphNode_DungeonMarker* MarkerNode = Cast<UEdGraphNode_DungeonMarker>(Node))
		{
			// TODO: Handle
		}

		// Select the newly pasted stuff
		GraphEditor->SetNodeSelection(Node, true);

		Node->NodePosX = (Node->NodePosX - AvgNodePosition.X) + Location.X;
		Node->NodePosY = (Node->NodePosY - AvgNodePosition.Y) + Location.Y;

		Node->SnapToGrid(SNodePanel::GetSnapGridSize());

		// Give new node a different Guid from the old one
		Node->CreateNewGuid();
	}

	// TODO: Implement
	//UpdatePropAfterGraphChange();

	// Update UI
	GraphEditor->NotifyGraphChanged();
}

bool FDungeonArchitectEditor::CanPasteNodes() const
{
	FString ClipboardContent;
	FPlatformMisc::ClipboardPaste(ClipboardContent);

	return FEdGraphUtilities::CanImportNodesFromText(PropBeingEdited->UpdateGraph, ClipboardContent);
}

void FDungeonArchitectEditor::CutSelectedNodes()
{
	CopySelectedNodes();
	// Cut should only delete nodes that can be duplicated
	DeleteSelectedDuplicatableNodes();
}

bool FDungeonArchitectEditor::CanCutNodes() const
{
	return CanCopyNodes() && CanDeleteNodes();
}

void FDungeonArchitectEditor::DuplicateNodes()
{
	// Copy and paste current selection
	CopySelectedNodes();
	PasteNodes();
}

bool FDungeonArchitectEditor::CanDuplicateNodes() const
{
	return CanCopyNodes();
}

void FDungeonArchitectEditor::OnGraphChanged(const FEdGraphEditAction& Action)
{
	bGraphStateChanged = true;
}

void FDungeonArchitectEditor::OnNodePropertyChanged(const FEdGraphEditAction& Action)
{
	// The nodes that were modified and requires a clean rebuild by the scene provider
	TSet<FName> NodeObjectsToRebuild;

	// Flag the node id to generate it cleanly in the scene provider
	for (const UEdGraphNode* Node : Action.Nodes) {
		const UEdGraphNode_DungeonActorBase* ActorNode = Cast<const UEdGraphNode_DungeonActorBase>(Node);
		if (ActorNode) {
			FName NodeId(*ActorNode->NodeGuid.ToString());
			NodeObjectsToRebuild.Add(NodeId);
		}
	}

	PreviewViewport->SetNodesToRebuild(NodeObjectsToRebuild);

	bGraphStateChanged = true;
}

void FDungeonArchitectEditor::HandleGraphChanged()
{
	UpdateOriginalPropAsset();
	if (PreviewViewport.IsValid()) {
		PreviewViewport->RebuildMeshes();
	}
	if (ActionPalette.IsValid()) {
		ActionPalette->Refresh();
	}
}

void FDungeonArchitectEditor::SaveAsset_Execute()
{
	UE_LOG(DungeonEditor, Log, TEXT("Saving dungeon theme %s"), *GetEditingObjects()[0]->GetName());

	UpdateOriginalPropAsset();

	UPackage* Package = PropBeingEdited->GetOutermost();
	if (Package)
	{
		TArray<UPackage*> PackagesToSave;
		PackagesToSave.Add(Package);

		FEditorFileUtils::PromptForCheckoutAndSave(PackagesToSave, false, false);
	}

	PreviewViewport->RebuildMeshes();
}

void FDungeonArchitectEditor::UpdateOriginalPropAsset()
{
	if (UEdGraph_DungeonProp* DungeonGraphEditor = Cast<UEdGraph_DungeonProp>(PropBeingEdited->UpdateGraph)) {
		TArray<FPropTypeData> Props;
		TArray<FDungeonGraphBuildError> CompileErrors;
		DungeonGraphEditor->RebuildGraph(PropBeingEdited, Props, CompileErrors);
		if (CompileErrors.Num() == 0) {
			PropBeingEdited->Props = Props;
		}
		else {
			// TODO: Display the error messages on the graph nodes
		}
	}
	FEditorDelegates::RefreshEditor.Broadcast();
	FEditorSupportDelegates::RedrawAllViewports.Broadcast();
}

void FDungeonArchitectEditor::OnSelectedNodesChanged(const TSet<class UObject*>& NewSelection)
{
	if (NewSelection.Num() > 0) {
		TArray<UObject*> SelectedObjects;
		for (UObject* Object : NewSelection) {
			SelectedObjects.Add(Object);
		}
		PropertyEditor->SetObjects(SelectedObjects);
	} 
	else {
		ShowObjectDetails(nullptr, true);
	}
}

void FDungeonArchitectEditor::OnNodeDoubleClicked(class UEdGraphNode* Node)
{

}

FDungeonArchitectEditor::~FDungeonArchitectEditor()
{
	if (GraphEditor->GetCurrentGraph()) {
		GraphEditor->GetCurrentGraph()->RemoveOnGraphChangedHandler(OnGraphChangedDelegateHandle);
		UEdGraph_DungeonProp* ThemeGraph = Cast<UEdGraph_DungeonProp>(GraphEditor->GetCurrentGraph());
		if (ThemeGraph) {
			ThemeGraph->RemoveOnNodePropertyChangedHandler(OnNodePropertyChangedDelegateHandle);
		}
	}
}

void FDungeonArchitectEditor::RegisterTabSpawners(const TSharedRef<class FTabManager>& InTabManager)
{
	WorkspaceMenuCategory = InTabManager->AddLocalWorkspaceMenuCategory(LOCTEXT("WorkspaceMenu_DungeonEditor", "Dungeon Editor"));
	auto WorkspaceMenuCategoryRef = WorkspaceMenuCategory.ToSharedRef();

	FAssetEditorToolkit::RegisterTabSpawners(InTabManager);

	InTabManager->RegisterTabSpawner(FDungeonEditorTabs::DetailsID, FOnSpawnTab::CreateSP(this, &FDungeonArchitectEditor::SpawnTab_Details))
		.SetDisplayName(LOCTEXT("DetailsTabLabel", "Details"))
		.SetGroup(WorkspaceMenuCategoryRef)
		.SetIcon(FSlateIcon(FEditorStyle::GetStyleSetName(), "LevelEditor.Tabs.Details"));

	InTabManager->RegisterTabSpawner(FDungeonEditorTabs::ViewportID, FOnSpawnTab::CreateSP(this, &FDungeonArchitectEditor::SpawnTab_Viewport))
		.SetDisplayName(LOCTEXT("ViewportTab", "Viewport"))
		.SetGroup(WorkspaceMenuCategoryRef)
		.SetIcon(FSlateIcon(FEditorStyle::GetStyleSetName(), "LevelEditor.Tabs.Viewports"));

	InTabManager->RegisterTabSpawner(FDungeonEditorTabs::PreviewID, FOnSpawnTab::CreateSP(this, &FDungeonArchitectEditor::SpawnTab_Preview))
		.SetDisplayName(LOCTEXT("PreviewTab", "Preview"))
		.SetGroup(WorkspaceMenuCategoryRef)
		.SetIcon(FSlateIcon(FEditorStyle::GetStyleSetName(), "LevelEditor.Tabs.Viewports"));
    
	InTabManager->RegisterTabSpawner(FDungeonEditorTabs::PreviewSettingsID, FOnSpawnTab::CreateSP(this, &FDungeonArchitectEditor::SpawnTab_PreviewSettings))
        .SetDisplayName(LOCTEXT("PreviewSettingsTabLabel", "Preview Settings"))
        .SetGroup(WorkspaceMenuCategoryRef)
        .SetIcon(FSlateIcon(FEditorStyle::GetStyleSetName(), "LevelEditor.Tabs.Details"));

	InTabManager->RegisterTabSpawner(FDungeonEditorTabs::ActionsID, FOnSpawnTab::CreateSP(this, &FDungeonArchitectEditor::SpawnTab_Actions))
		.SetDisplayName(LOCTEXT("ActionsTabLabel", "Actions"))
		.SetGroup(WorkspaceMenuCategoryRef)
		.SetIcon(FSlateIcon(FEditorStyle::GetStyleSetName(), "LevelEditor.Tabs.Details"));

	InTabManager->RegisterTabSpawner(FDungeonEditorTabs::MarkersID, FOnSpawnTab::CreateSP(this, &FDungeonArchitectEditor::SpawnTab_Markers))
		.SetDisplayName(LOCTEXT("MarkersTabLabel", "Markers"))
		.SetGroup(WorkspaceMenuCategoryRef)
		.SetIcon(FSlateIcon(FEditorStyle::GetStyleSetName(), "LevelEditor.Tabs.Details"));
}

void FDungeonArchitectEditor::UnregisterTabSpawners(const TSharedRef<class FTabManager>& InTabManager)
{
	FAssetEditorToolkit::UnregisterTabSpawners(InTabManager);

	InTabManager->UnregisterTabSpawner(FDungeonEditorTabs::ViewportID);
	InTabManager->UnregisterTabSpawner(FDungeonEditorTabs::PreviewID);
	InTabManager->UnregisterTabSpawner(FDungeonEditorTabs::PreviewSettingsID);
	InTabManager->UnregisterTabSpawner(FDungeonEditorTabs::ActionsID);
	InTabManager->UnregisterTabSpawner(FDungeonEditorTabs::MarkersID);
	InTabManager->UnregisterTabSpawner(FDungeonEditorTabs::DetailsID);
}

void FDungeonArchitectEditor::ExtendMenu() {

}

void FDungeonArchitectEditor::InitDungeonEditor(const EToolkitMode::Type Mode, const TSharedPtr< class IToolkitHost >& InitToolkitHost, UDungeonThemeAsset* PropData) {
	// Initialize the asset editor and spawn nothing (dummy layout)
	FAssetEditorManager::Get().CloseOtherEditors(PropData, this);
	PropBeingEdited = PropData;
	if (!PropBeingEdited->UpdateGraph) {
		UEdGraph_DungeonProp* DungeonGraph = NewObject<UEdGraph_DungeonProp>(PropBeingEdited, UEdGraph_DungeonProp::StaticClass(), NAME_None, RF_Transactional);
		DungeonGraph->RecreateDefaultMarkerNodes(PropBeingEdited->PreviewViewportProperties->BuilderClass);
		PropBeingEdited->UpdateGraph = DungeonGraph;
	}

	GraphEditor = CreateGraphEditorWidget(PropBeingEdited->UpdateGraph);

	PreviewViewport = SNew(SDungeonEditorViewport)
		.DungeonEditor(SharedThis(this))
		.ObjectToEdit(PropBeingEdited);

	
	// Default layout
	const TSharedRef<FTabManager::FLayout> StandaloneDefaultLayout = FTabManager::NewLayout("Standalone_DungeonPropEditor_Layout_v6")
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
				->SetSizeCoefficient(0.63f)
				->Split
				(
					FTabManager::NewStack()
					->SetSizeCoefficient(0.8f)
					->SetHideTabWell(true)
					->AddTab(FDungeonEditorTabs::ViewportID, ETabState::OpenedTab)
				)
				->Split
				(
					FTabManager::NewSplitter()
					->SetOrientation(Orient_Vertical)
					->SetSizeCoefficient(0.37f)
					->Split
					(
						//FTabManager::NewStack()
						FTabManager::NewSplitter()
						->SetOrientation(Orient_Horizontal)
						->SetSizeCoefficient(0.33f)
						->Split
						(
							FTabManager::NewStack()
							->SetSizeCoefficient(0.6f)
							->AddTab(FDungeonEditorTabs::DetailsID, ETabState::OpenedTab)
						)
						->Split
						(
							FTabManager::NewStack()
							->SetSizeCoefficient(0.6f)
							->AddTab(FDungeonEditorTabs::PreviewSettingsID, ETabState::OpenedTab)
							->AddTab(FDungeonEditorTabs::ActionsID, ETabState::OpenedTab)
						)
					)
					->Split
					(
						FTabManager::NewStack()
						->SetSizeCoefficient(0.667f)
						->AddTab(FDungeonEditorTabs::PreviewID, ETabState::OpenedTab)
					)
				)
			)
		);

	// Initialize the asset editor and spawn nothing (dummy layout)
	InitAssetEditor(Mode, InitToolkitHost, DungeonEditorAppName, StandaloneDefaultLayout, /*bCreateDefaultStandaloneMenu=*/ true, /*bCreateDefaultToolbar=*/ true, PropData);

	ExtendMenu();

	// Listen for graph changed event
	OnGraphChangedDelegateHandle = GraphEditor->GetCurrentGraph()->AddOnGraphChangedHandler(FOnGraphChanged::FDelegate::CreateRaw(this, &FDungeonArchitectEditor::OnGraphChanged));
	bGraphStateChanged = true;

	UEdGraph_DungeonProp* ThemeGraph = Cast<UEdGraph_DungeonProp>(GraphEditor->GetCurrentGraph());
	if (ThemeGraph) {
		OnNodePropertyChangedDelegateHandle = ThemeGraph->AddOnNodePropertyChangedHandler(FOnGraphChanged::FDelegate::CreateRaw(this, &FDungeonArchitectEditor::OnNodePropertyChanged));
	}

	// Show the dungeon properties
	ShowObjectDetails(nullptr, true);
}

void FDungeonArchitectEditor::ShowObjectDetails(UObject* ObjectProperties, bool bForceRefresh)
{
	if (!ObjectProperties) {
		// No object selected.  Show the dungeon properties by default
		ObjectProperties = PropBeingEdited ? PropBeingEdited->PreviewViewportProperties : nullptr;
	}

	PropertyEditor->SetObject(ObjectProperties, bForceRefresh);
}

void FDungeonArchitectEditor::RecreateDefaultMarkerNodes()
{
	if (PropBeingEdited && PropBeingEdited->UpdateGraph && PropBeingEdited->PreviewViewportProperties) {
		if (UEdGraph_DungeonProp* DungeonGraph = Cast<UEdGraph_DungeonProp>(PropBeingEdited->UpdateGraph)) {
			TSubclassOf<UDungeonBuilder> BuilderClass = PropBeingEdited->PreviewViewportProperties->BuilderClass;
			if (BuilderClass) {
				DungeonGraph->RecreateDefaultMarkerNodes(BuilderClass);
			}
		}
	}
}

void SGraphEditor_Dungeon::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	SGraphEditor::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);

	//FDungeonEditorThumbnailPool::Get()->Tick(InDeltaTime);
}

#undef LOCTEXT_NAMESPACE

