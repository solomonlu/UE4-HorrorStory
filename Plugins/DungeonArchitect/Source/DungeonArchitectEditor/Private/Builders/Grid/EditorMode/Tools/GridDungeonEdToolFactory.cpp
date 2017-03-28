//$ Copyright 2015 Ali Akbar, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//
#include "DungeonArchitectEditorPrivatePCH.h"
#include "GridDungeonEdToolFactory.h"
#include "GridDungeonEdToolPaint.h"
#include "GridDungeonEdToolRectangle.h"
#include "GridDungeonEdToolBorder.h"

TSharedPtr<FGridDungeonEdTool> FGridDungeonEdToolFactory::Create(const FDungeonEdToolID& ToolType, UGridDungeonEdModeHandler* ModeHandler)
{
	if (ToolType == FGridDungeonEdToolPaint::ToolID) {
		return MakeShareable(new FGridDungeonEdToolPaint(ModeHandler));
	}
	else if (ToolType == FGridDungeonEdToolRectangle::ToolID) {
		return MakeShareable(new FGridDungeonEdToolRectangle(ModeHandler));
	}
	else if (ToolType == FGridDungeonEdToolBorder::ToolID) {
		return MakeShareable(new FGridDungeonEdToolBorder(ModeHandler));
	}
	return nullptr;
}
