//$ Copyright 2015 Ali Akbar, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//
#pragma once
#include "GridDungeonEdTool.h"

class FGridDungeonEdToolPaint : public FGridDungeonEdTool {
public:
	FGridDungeonEdToolPaint(UGridDungeonEdModeHandler* ModeHandler);
	virtual void Render(const FSceneView* View, FViewport* Viewport, FPrimitiveDrawInterface* PDI) override;
	virtual bool InputKey(FEditorViewportClient* InViewportClient, FViewport* InViewport, FKey InKey, EInputEvent InEvent) override;
	virtual void ApplyBrush(FEditorViewportClient* ViewportClient) override;

	virtual FDungeonEdToolID GetToolType() const { return ToolID; }
	static FName ToolID;

private:
	UMaterial* OverlayMaterial;
	bool bRemoveMode;
	bool bPainting;
};
