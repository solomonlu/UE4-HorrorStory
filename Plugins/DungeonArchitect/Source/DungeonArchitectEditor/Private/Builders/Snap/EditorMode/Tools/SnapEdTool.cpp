//$ Copyright 2015 Ali Akbar, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//
#include "DungeonArchitectEditorPrivatePCH.h"
#include "SnapEdTool.h"
#include "../SnapEdModeRenderer.h"
#include "../SnapEdModeHandler.h"

FName FSnapEdTool::ToolFamily = "Snap";

FSnapEdTool::FSnapEdTool(USnapEdModeHandler* InModeHandler) : ModeHandler(InModeHandler)
{
}

void FSnapEdTool::Initialize()
{
}

void FSnapEdTool::Update(const FSceneView* View, FViewport* Viewport)
{

}

void FSnapEdTool::Destroy()
{
}

void FSnapEdTool::OnUndo()
{
}

void FSnapEdTool::OnRedo()
{
}

void FSnapEdTool::AddReferencedObjects(FReferenceCollector& Collector)
{
}

UObject* FSnapEdTool::GetToolModel() const
{
	return nullptr;
}

TSharedPtr<FSnapEdModeRenderer> FSnapEdTool::GetSnapRenderer()
{
	if (!ModeHandler) {
		return nullptr;
	}
	return StaticCastSharedPtr<FSnapEdModeRenderer>(ModeHandler->GetRenderer());
}

FName FSnapEdTool::GetToolFamily() const
{
	return ToolFamily;
}
