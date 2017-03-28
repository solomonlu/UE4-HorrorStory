//$ Copyright 2015 Ali Akbar, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//


#pragma once

#include "Toolkits/AssetEditorToolkit.h"
#include "Toolkits/AssetEditorManager.h"
#include "EditorUndoClient.h"
#include "IDungeonArchitectEditor.h"
#include "DungeonThemeAsset.h"
#include "Widgets/SGraphNode_DungeonActor.h"
#include "IDetailsView.h"
#include "Widgets/SMarkerListView.h"
#include "Tickable.h"

class FDungeonEditorThumbnailPool : public FAssetThumbnailPool {
public:
	FDungeonEditorThumbnailPool(int NumObjectsInPool) : FAssetThumbnailPool(NumObjectsInPool) {}

	static TSharedPtr<FDungeonEditorThumbnailPool> Get() { return Instance; }
	static void Create() {
		Instance = MakeShareable(new FDungeonEditorThumbnailPool(512));
	}
private:
	static TSharedPtr<FDungeonEditorThumbnailPool> Instance;
};

class SGraphEditor_Dungeon : public SGraphEditor {
public:
	// SWidget implementation
	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;
	// End SWidget implementation
};

/*-----------------------------------------------------------------------------
FDungeonArchitectEditor
-----------------------------------------------------------------------------*/
class FDungeonArchitectEditor : public IDungeonArchitectEditor, public FNotifyHook, public FTickableGameObject
{
public:
	~FDungeonArchitectEditor();
	// IToolkit interface
	virtual void RegisterTabSpawners(const TSharedRef<class FTabManager>& TabManager) override;
	virtual void UnregisterTabSpawners(const TSharedRef<class FTabManager>& TabManager) override;
	// End of IToolkit interface

	// FAssetEditorToolkit
	
	virtual FName GetToolkitFName() const override;
	virtual FText GetBaseToolkitName() const override;
	virtual FText GetToolkitName() const override;
	virtual FLinearColor GetWorldCentricTabColorScale() const override;
	virtual FString GetWorldCentricTabPrefix() const override;
	virtual FString GetDocumentationLink() const override;
	// End of FAssetEditorToolkit

	// FNotifyHook interface
	virtual void NotifyPreChange(UProperty* PropertyAboutToChange) override;
	virtual void NotifyPostChange(const FPropertyChangedEvent& PropertyChangedEvent, UProperty* PropertyThatChanged) override;
	// End of FNotifyHook interface

	// FTickableGameObject Interface
	virtual bool IsTickableInEditor() const override;
	virtual void Tick(float DeltaTime) override;
	virtual bool IsTickable() const override;
	virtual TStatId GetStatId() const override;
	// End of FTickableGameObject Interface

	void InitDungeonEditor(const EToolkitMode::Type Mode, const TSharedPtr< class IToolkitHost >& InitToolkitHost, UDungeonThemeAsset* PropData);
	UDungeonThemeAsset* GetPropBeingEdited() const { return PropBeingEdited; }

	FORCEINLINE TSharedPtr<SGraphEditor> GetGraphEditor() const { return GraphEditor; }

	void ShowObjectDetails(UObject* ObjectProperties, bool bForceRefresh = false);

	void RecreateDefaultMarkerNodes();

protected:
	void ExtendMenu();
	TSharedRef<class SGraphEditor> CreateGraphEditorWidget(UEdGraph* InGraph);

	/** Select every node in the graph */
	void SelectAllNodes();
	/** Whether we can select every node */
	bool CanSelectAllNodes() const;

	/** Deletes all the selected nodes */
	void DeleteSelectedNodes();

	bool CanDeleteNode(class UEdGraphNode* Node);

	/** Delete an array of Material Graph Nodes and their corresponding expressions/comments */
	void DeleteNodes(const TArray<class UEdGraphNode*>& NodesToDelete);

	/** Delete only the currently selected nodes that can be duplicated */
	void DeleteSelectedDuplicatableNodes();

	/** Whether we are able to delete the currently selected nodes */
	bool CanDeleteNodes() const;

	/** Copy the currently selected nodes */
	void CopySelectedNodes();
	/** Whether we are able to copy the currently selected nodes */
	bool CanCopyNodes() const;

	/** Paste the contents of the clipboard */
	void PasteNodes();
	virtual bool CanPasteNodes() const;
	virtual void PasteNodesHere(const FVector2D& Location);

	/** Cut the currently selected nodes */
	void CutSelectedNodes();
	/** Whether we are able to cut the currently selected nodes */
	bool CanCutNodes() const;

	/** Duplicate the currently selected nodes */
	void DuplicateNodes();
	/** Whether we are able to duplicate the currently selected nodes */
	bool CanDuplicateNodes() const;
	
	void OnGraphChanged(const FEdGraphEditAction& Action);
	void OnNodePropertyChanged(const FEdGraphEditAction& Action);
	void HandleGraphChanged();

protected:
	/** Called when "Save" is clicked for this asset */
	virtual void SaveAsset_Execute() override;
	void UpdateOriginalPropAsset();

	/** Called when the selection changes in the GraphEditor */
	void OnSelectedNodesChanged(const TSet<class UObject*>& NewSelection);

	/** Called when a node is double clicked */
	void OnNodeDoubleClicked(class UEdGraphNode* Node);

protected:
	TSharedPtr<SGraphEditor> GraphEditor;
	TSharedPtr<FUICommandList> GraphEditorCommands;
	TSharedPtr<IDetailsView> PropertyEditor;
	UDungeonThemeAsset* PropBeingEdited;
	TSharedRef<SDockTab> SpawnTab_Viewport(const FSpawnTabArgs& Args);
	TSharedRef<SDockTab> SpawnTab_Preview(const FSpawnTabArgs& Args);
	TSharedRef<SDockTab> SpawnTab_Details(const FSpawnTabArgs& Args);
    TSharedRef<SDockTab> SpawnTab_Actions(const FSpawnTabArgs& Args);
    TSharedRef<SDockTab> SpawnTab_Markers(const FSpawnTabArgs& Args);
    TSharedRef<SDockTab> SpawnTab_PreviewSettings(const FSpawnTabArgs& Args);


	/** Palette of Node actions to perform on the graph */
	TSharedPtr<class SGraphPalette_PropActions> ActionPalette;

	/** List of markers registered on the graph */
	TSharedPtr<SMarkerListView> MarkersListView;

	/** Preview Viewport widget */
	TSharedPtr<class SDungeonEditorViewport> PreviewViewport;

	/** Handle to the registered OnGraphChanged delegate. */
	FDelegateHandle OnGraphChangedDelegateHandle;

	/** Handle to the registered OnNodePropertyChanged delegate. */
	FDelegateHandle OnNodePropertyChangedDelegateHandle;

	bool bGraphStateChanged;
};
