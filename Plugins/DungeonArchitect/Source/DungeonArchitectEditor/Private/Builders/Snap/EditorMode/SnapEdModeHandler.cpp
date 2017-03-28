//$ Copyright 2015 Ali Akbar, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//
#include "DungeonArchitectEditorPrivatePCH.h"
#include "SnapEdModeHandler.h"
#include "SnapEdModeRenderer.h"
#include "UI/SSnapEditor.h"
#include "ScopedTransaction.h"
#include "Core/Dungeon.h"

DEFINE_LOG_CATEGORY(SnapEdModeHandlerLog);

TSharedPtr<FDungeonEdModeRenderer> USnapEdModeHandler::CreateRenderer()
{
	return MakeShareable(new FSnapEdModeRenderer(this));
}

void USnapEdModeHandler::OnUpdate(const FSceneView* View, FViewport* Viewport)
{
	UDungeonEdModeHandler::OnUpdate(View, Viewport);
}

void USnapEdModeHandler::SetActiveTool(TSharedPtr<IDungeonEdTool> NewTool)
{
	UDungeonEdModeHandler::SetActiveTool(NewTool);
}

TSharedPtr<SWidget> USnapEdModeHandler::CreateToolkitWidget()
{
	return SNew(SSnapEditor);
}

