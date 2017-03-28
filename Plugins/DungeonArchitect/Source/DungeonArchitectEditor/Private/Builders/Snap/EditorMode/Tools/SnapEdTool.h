//$ Copyright 2015 Ali Akbar, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//
#pragma once
#include "Core/EditorMode/IDungeonEdTool.h"

class USnapEdToolData;
class USnapEdModeHandler;
class FSnapEdModeRenderer;

class FSnapEdTool : public IDungeonEdTool, public FGCObject {
public:
	FSnapEdTool(USnapEdModeHandler* ModeHandler);
	virtual ~FSnapEdTool() {}
	virtual void Initialize() override;
	virtual void Update(const FSceneView* View, FViewport* Viewport) override;
	virtual void Destroy() override;
	virtual void ApplyBrush(FEditorViewportClient* ViewportClient) override {}
	virtual bool InputKey(FEditorViewportClient* InViewportClient, FViewport* InViewport, FKey InKey, EInputEvent InEvent) override { return false; }
	virtual void OnUndo() override;
	virtual void OnRedo() override;
	virtual UObject* GetToolModel() const override;
	virtual void AddReferencedObjects(FReferenceCollector& Collector) override;

	TSharedPtr<FSnapEdModeRenderer> GetSnapRenderer();
	virtual FName GetToolFamily() const override;

	static FName ToolFamily;
	
protected:
	USnapEdModeHandler* ModeHandler;

};
