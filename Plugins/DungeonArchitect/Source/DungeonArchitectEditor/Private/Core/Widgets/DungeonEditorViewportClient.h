//$ Copyright 2015 Ali Akbar, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//
#pragma once
#include "SDungeonEditorViewport.h"
#include "IDungeonArchitectEditor.h"
#include "PreviewScene.h"

class FDungeonPreviewScene;

/** Viewport Client for the preview viewport */
class DUNGEONARCHITECTEDITOR_API FDungeonEditorViewportClient : public FEditorViewportClient, public TSharedFromThis<FDungeonEditorViewportClient>
{
public:
	FDungeonEditorViewportClient(TWeakPtr<IDungeonArchitectEditor> InDungeonEditor, TWeakPtr<SDungeonEditorViewport> InDungeonEditorViewport, FPreviewScene& InPreviewScene, UDungeonThemeAsset* InProp);

	// FEditorViewportClient interface
	virtual void Tick(float DeltaSeconds) override;
	virtual void ProcessClick(class FSceneView& View, class HHitProxy* HitProxy, FKey Key, EInputEvent Event, uint32 HitX, uint32 HitY) override;
	// End of FEditorViewportClient interface
private:
	TWeakPtr<IDungeonArchitectEditor> InDungeonEditor;
	TWeakPtr<SDungeonEditorViewport> InDungeonEditorViewport;
	UDungeonThemeAsset* InProp;
};

