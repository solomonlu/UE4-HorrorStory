//$ Copyright 2015 Ali Akbar, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//
#pragma once
#include "SSnapDoorPreview3DViewport.h"
#include "ISnapDoorEditor.h"
#include "PreviewScene.h"

//FSnapDoorPreview3DViewportClient

/** Viewport Client for the preview viewport */
class DUNGEONARCHITECTEDITOR_API FSnapDoorPreview3DViewportClient : public FEditorViewportClient, public TSharedFromThis<FSnapDoorPreview3DViewportClient>
{
public:
	FSnapDoorPreview3DViewportClient(TWeakPtr<ISnapDoorEditor> InLAThemeEditor, TWeakPtr<SSnapDoorPreview3DViewport> InLAThemeEditorViewport, FPreviewScene& InPreviewScene, USnapDoorAsset* InProp);

	// FEditorViewportClient interface
	virtual void Tick(float DeltaSeconds) override;
	// End of FEditorViewportClient interface
private:
	TWeakPtr<ISnapDoorEditor> InLAThemeEditor;
	TWeakPtr<SSnapDoorPreview3DViewport> InLAThemeEditorViewport;
	USnapDoorAsset* InProp;
};

