//$ Copyright 2015 Ali Akbar, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//
#include "DungeonArchitectEditorPrivatePCH.h"
#include "GridDungeonEdModeHandler.h"
#include "GridDungeonEdModeRenderer.h"
#include "Tools/GridDungeonEdTool.h"
#include "UI/SGridDungeonEdit.h"
#include "ScopedTransaction.h"
#include "Core/Dungeon.h"
#include "GridDungeonToolData.h"

DEFINE_LOG_CATEGORY(GridDungeonEdModeHandlerLog);

TSharedPtr<FDungeonEdModeRenderer> UGridDungeonEdModeHandler::CreateRenderer()
{
	return MakeShareable(new FGridDungeonEdModeRenderer(this));
}

void UGridDungeonEdModeHandler::OnUpdate(const FSceneView* View, FViewport* Viewport)
{
	UDungeonEdModeHandler::OnUpdate(View, Viewport);
}

void UGridDungeonEdModeHandler::SetActiveTool(TSharedPtr<IDungeonEdTool> NewTool)
{
	if (!NewTool.IsValid()) {
		UE_LOG(GridDungeonEdModeHandlerLog, Warning, TEXT("Tool is not valid"));
		ActiveTool = nullptr;
		return;
	}

	if (NewTool->GetToolFamily() != FGridDungeonEdTool::ToolFamily) {
		UE_LOG(GridDungeonEdModeHandlerLog, Warning, TEXT("Tool is not supported with this builder"));
		ActiveTool = nullptr;
		return;
	}

	UDungeonEdModeHandler::SetActiveTool(NewTool);
}

TSharedPtr<SWidget> UGridDungeonEdModeHandler::CreateToolkitWidget()
{
	return SNew(SGridDungeonEdit);
}

void UGridDungeonEdModeHandler::ApplyPaintData(const FString& TransactionName, const TArray<FIntVector>& StrokeData, bool bRemove)
{
	if (!ActiveDungeon) return;

	const FScopedTransaction Transaction(FText::FromString(*TransactionName));
	ActiveDungeon->Modify();
	bool bDataModelChanged = false;

	UDungeonToolData* ToolData = ActiveDungeon->GetToolData();
	UGridDungeonToolData* GridToolData = Cast<UGridDungeonToolData>(ToolData);

	TArray<FIntVector> &PaintedCells = GridToolData->PaintedCells;
	for (const FIntVector StrokeCell : StrokeData) {
		bool bContains = PaintedCells.Contains(StrokeCell);
		if (bRemove && bContains) {
			PaintedCells.Remove(StrokeCell);
		}

		else if (!bRemove && !bContains) {
			PaintedCells.Add(StrokeCell);
		}
	}
}	