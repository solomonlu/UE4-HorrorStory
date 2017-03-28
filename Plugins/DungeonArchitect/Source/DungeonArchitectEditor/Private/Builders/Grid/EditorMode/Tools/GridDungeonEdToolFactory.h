//$ Copyright 2015 Ali Akbar, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//
#pragma once
#include "GridDungeonEdTool.h"

class FGridDungeonEdToolFactory {
public:
	static TSharedPtr<FGridDungeonEdTool> Create(const FDungeonEdToolID& ToolType, UGridDungeonEdModeHandler* ModeHandler);
};
