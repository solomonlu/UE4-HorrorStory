//$ Copyright 2015 Ali Akbar, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//
#pragma once

#include "PreviewScene.h"
#include "SEditorViewport.h"
#include "SnapDoorAsset.h"
#include "../SnapDoorEditor.h"
#include "GraphEditAction.h"

class ASnapDoor;

/**
* StaticMesh Editor Preview viewport widget
*/
class DUNGEONARCHITECTEDITOR_API SSnapDoorPreview3DViewport : public SEditorViewport, public FGCObject
{
public:
	SLATE_BEGIN_ARGS(SSnapDoorPreview3DViewport){}
		SLATE_ARGUMENT(TWeakPtr<FSnapDoorEditor>, SnapDoorEditor)
		SLATE_ARGUMENT(USnapDoorAsset*, DoorAsset)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	~SSnapDoorPreview3DViewport();

	// FGCObject interface
	virtual void AddReferencedObjects(FReferenceCollector& Collector) override;
	// End of FGCObject interface

	/** Set the parent tab of the viewport for determining visibility */
	void SetParentTab(TSharedRef<SDockTab> InParentTab) { ParentTab = InParentTab; }

	// SWidget Interface
	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime);
	// End of SWidget Interface

	EVisibility GetToolbarVisibility() const;

	void SpawnDoorActor();
	
	ASnapDoor* GetDoor() const { return Door; }
	UWorld* GetWorld() const;
protected:
	/** SEditorViewport interface */
	virtual TSharedRef<FEditorViewportClient> MakeEditorViewportClient() override;
	virtual EVisibility OnGetViewportContentVisibility() const override;
	virtual void BindCommands() override;
	virtual void OnFocusViewportToSelection() override;
	virtual TSharedPtr<SWidget> MakeViewportToolbar() override;

	void OnToggleDebugData();
	
private:
	/** Determines the visibility of the viewport. */
	bool IsVisible() const;


private:
	TWeakPtr<FSnapDoorEditor> LAThemeEditorPtr;
	USnapDoorAsset* DoorAsset;

	/** The parent tab where this viewport resides */
	TWeakPtr<SDockTab> ParentTab;

	/** Level viewport client */
	TSharedPtr<class FSnapDoorPreview3DViewportClient> EditorViewportClient;

	/** The scene for this viewport. */
	TSharedPtr<FPreviewScene> PreviewScene;

	/** The landscape actor in the preview 3D viewport */
	ASnapDoor* Door;

	USkyLightComponent* Skylight;
	UAtmosphericFogComponent* AtmosphericFog;

};

