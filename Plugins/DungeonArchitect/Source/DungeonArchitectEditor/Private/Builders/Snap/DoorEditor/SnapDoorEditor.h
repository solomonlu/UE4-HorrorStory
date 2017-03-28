//$ Copyright 2015 Ali Akbar, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "Toolkits/AssetEditorToolkit.h"
#include "Toolkits/AssetEditorManager.h"
#include "ISnapDoorEditor.h"
#include "EditorUndoClient.h"
#include "SnapDoorAsset.h"
#include "IDetailsView.h"
#include "Tickable.h"
#include "GraphEditor.h"

class FSnapDoorEditor : public FNotifyHook, public ISnapDoorEditor, public FTickableGameObject
{
public:
	~FSnapDoorEditor();
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

	virtual void InitSnapDoorEditor(const EToolkitMode::Type Mode, const TSharedPtr< class IToolkitHost >& InitToolkitHost, USnapDoorAsset* TreeTheme);
	USnapDoorAsset* GetDoorBeingEdited() const { return DoorBeingEdited; }

	void OnPropertyChanged(UObject* ObjectBeingModified, FPropertyChangedEvent& PropertyChangedEvent);

	void RebuildDoor();

protected:
	void ExtendMenu();
	void BindCommands();

	void HandleRebuildActionExecute();

protected:
	/** Called when "Save" is clicked for this asset */
	virtual void SaveAsset_Execute() override;
	void UpdateOriginalDoorAsset();

protected:
	USnapDoorAsset* DoorBeingEdited;

	TSharedRef<SDockTab> SpawnTab_Preview3D(const FSpawnTabArgs& Args);
	TSharedRef<SDockTab> SpawnTab_Details(const FSpawnTabArgs& Args);

	/** Preview Viewport widget */
	TSharedPtr<class SSnapDoorPreview3DViewport> PreviewViewport;

	/** Properties widget */
	TSharedPtr<class SSnapDoorEdit> PropertyEditor;

};

