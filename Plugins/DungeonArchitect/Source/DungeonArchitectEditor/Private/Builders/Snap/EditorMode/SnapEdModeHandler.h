//$ Copyright 2015 Ali Akbar, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//
#pragma once
#include "Core/EditorMode/DungeonEdModeHandler.h"
#include "SnapEdModeHandler.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(SnapEdModeHandlerLog, Log, All);

UCLASS()
class USnapEdModeHandler : public UDungeonEdModeHandler {
	GENERATED_BODY()

public:
	virtual TSharedPtr<FDungeonEdModeRenderer> CreateRenderer() override;
	virtual void SetActiveTool(TSharedPtr<IDungeonEdTool> NewTool);
	virtual TSharedPtr<SWidget> CreateToolkitWidget() override;

protected:
	virtual void OnUpdate(const FSceneView* View, FViewport* Viewport) override;
};
