//$ Copyright 2015 Ali Akbar, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//
#include "DungeonArchitectEditorPrivatePCH.h"
#include "GridDungeonEdToolPaint.h"
#include "Core/EditorMode/DungeonEdModeRenderer.h"
#include "DynamicMeshBuilder.h"
#include "Core/EditorMode/DungeonEdMode.h"
#include "GridDungeonEdTool.h"
#include "../GridDungeonEdModeRenderer.h"
#include "Dungeon.h"
#include "../GridDungeonEdModeHandler.h"
#include "GridDungeonEdToolData.h"


FName FGridDungeonEdToolPaint::ToolID = "Tool_Grid_Paint";

FGridDungeonEdToolPaint::FGridDungeonEdToolPaint(UGridDungeonEdModeHandler* ModeHandler)
	: FGridDungeonEdTool(ModeHandler), bPainting(false)
{
	OverlayMaterial = LoadObject<UMaterial>(nullptr, TEXT("/DungeonArchitect/Materials/M_DungeonEdModeOverlay.M_DungeonEdModeOverlay"), nullptr, LOAD_None, nullptr);
}

void FGridDungeonEdToolPaint::Render(const FSceneView* View, FViewport* Viewport, FPrimitiveDrawInterface* PDI)
{
	// Allocate the material proxy and register it so it can be deleted properly once the rendering is done with it.
	FDynamicColoredMaterialRenderProxy* SelectedColorInstance = new FDynamicColoredMaterialRenderProxy(OverlayMaterial->GetRenderProxy(false), FLinearColor::White);
	PDI->RegisterDynamicResource(SelectedColorInstance);

	const bool RightMouseButtonDown = Viewport->KeyState(EKeys::RightMouseButton) ? true : false;
	const bool bDrawCursor = !RightMouseButtonDown;	// Draw when we are not panning / moving the camera

	if (bDrawCursor) {
		FDynamicMeshBuilder MeshBuilder;
		// Draw the cursor that follows the mouse in grid coordinates
		FIntVector BrushSize(ToolModel->BrushSize, ToolModel->BrushSize, 1);
		auto GridRenderer = GetGridRenderer();
		if (GridRenderer.IsValid()) {
			GridRenderer->DrawCursor(View, Viewport, PDI, GridCursorPosition, MeshBuilder, BrushSize);
		}
		MeshBuilder.Draw(PDI, FMatrix::Identity, SelectedColorInstance, SDPG_Foreground, true, false);
	}
}

bool FGridDungeonEdToolPaint::InputKey(FEditorViewportClient* InViewportClient, FViewport* InViewport, FKey InKey, EInputEvent InEvent)
{
	bRemoveMode = InViewportClient->IsShiftPressed();
	ADungeon* Dungeon = ModeHandler->GetActiveDungeon();

	if (Dungeon) {
		if (InKey == EKeys::LeftMouseButton) {
			if (InEvent == EInputEvent::IE_Pressed) {
				bPainting = true;
			}
			else if (InEvent == EInputEvent::IE_Released) {
				// Apply the stroke data
				ModeHandler->ApplyPaintData(FString("Apply Paint Brush"), PaintedStrokeData, bRemoveMode);

				// Rebuild only when the user release the left mouse button
				Dungeon->BuildDungeon();

				// Clear the stroke data for next paint operation
				PaintedStrokeData.Reset();
				bPainting = false;
			}
		}
	}

	return false;
}

void FGridDungeonEdToolPaint::ApplyBrush(FEditorViewportClient* ViewportClient)
{
	ADungeon* Dungeon = ModeHandler->GetActiveDungeon();
	if (!Dungeon || !bPainting) return;

	for (int dx = 0; dx < ToolModel->BrushSize; dx++) {
		for (int dy = 0; dy < ToolModel->BrushSize; dy++) {
			FIntVector Position = GridCursorPosition + FIntVector(dx, dy, 0);
			PaintedStrokeData.AddUnique(Position);
		}
	}
}
