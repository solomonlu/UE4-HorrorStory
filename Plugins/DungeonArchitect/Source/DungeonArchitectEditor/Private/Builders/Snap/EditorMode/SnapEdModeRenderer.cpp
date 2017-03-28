//$ Copyright 2015 Ali Akbar, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//
#include "DungeonArchitectEditorPrivatePCH.h"
#include "SnapEdModeRenderer.h"
#include "Core/EditorMode/DungeonEdModeHandler.h"
#include "Dungeon.h"
#include "DungeonModelHelper.h"
#include "SnapDungeonModel.h"
#include "SnapDungeonConfig.h"
#include "SnapEdModeHandler.h"

FSnapEdModeRenderer::FSnapEdModeRenderer(USnapEdModeHandler* pModeHandler)
	: ModeHandler(pModeHandler)
{
}

void FSnapEdModeRenderer::Render(const FSceneView* View, FViewport* Viewport, FPrimitiveDrawInterface* PDI, TSharedPtr<IDungeonEdTool> ActiveTool)
{

}

void FSnapEdModeRenderer::Destroy()
{
}

