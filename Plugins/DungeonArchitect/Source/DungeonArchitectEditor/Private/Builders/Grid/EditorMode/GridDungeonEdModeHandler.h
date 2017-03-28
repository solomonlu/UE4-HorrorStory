//$ Copyright 2015 Ali Akbar, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//
#pragma once
#include "Core/EditorMode/DungeonEdModeHandler.h"
#include "GridDungeonEdModeHandler.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(GridDungeonEdModeHandlerLog, Log, All);

UCLASS()
class UGridDungeonEdModeHandler : public UDungeonEdModeHandler {
	GENERATED_BODY()

public:
	virtual TSharedPtr<FDungeonEdModeRenderer> CreateRenderer() override;
	void ApplyPaintData(const FString& TransactionName, const TArray<FIntVector>& StrokeData, bool bRemove);
	virtual void SetActiveTool(TSharedPtr<IDungeonEdTool> NewTool);
	virtual TSharedPtr<SWidget> CreateToolkitWidget() override;

protected:
	virtual void OnUpdate(const FSceneView* View, FViewport* Viewport) override;
};
