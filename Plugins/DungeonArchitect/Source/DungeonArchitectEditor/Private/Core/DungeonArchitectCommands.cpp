//$ Copyright 2015 Ali Akbar, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//
#include "DungeonArchitectEditorPrivatePCH.h"
#include "DungeonArchitectCommands.h"
#include "Customizations/DungeonArchitectStyle.h"

#define LOCTEXT_NAMESPACE "ContentBrowser"

FDungeonArchitectCommands::FDungeonArchitectCommands() : TCommands<FDungeonArchitectCommands>(
	TEXT("DungeonArchitect"),
	NSLOCTEXT("DungeonArchitect", "DungeonArchitect", "Dungeon Architect"),
	NAME_None,
	FDungeonArchitectStyle::GetStyleSetName())
{
}

void FDungeonArchitectCommands::RegisterCommands() {
	UI_COMMAND(OpenDungeonEditor, "Open Dungeon Editor", "Opens the dungeon editor that lets you define props meshes for the dungeons", EUserInterfaceActionType::Button, FInputGesture(EKeys::Enter));

	UI_COMMAND(ModePaint, "Paint", "Paint", EUserInterfaceActionType::ToggleButton, FInputGesture());
	UI_COMMAND(ModeRectangle, "Rectangle", "Rectangle", EUserInterfaceActionType::ToggleButton, FInputGesture());
	UI_COMMAND(ModeBorder, "Border", "Border", EUserInterfaceActionType::ToggleButton, FInputGesture());
	UI_COMMAND(ModeSelect, "Select", "Select", EUserInterfaceActionType::ToggleButton, FInputGesture());

	UI_COMMAND(UpgradeThemeFile, "Upgrade Theme File", "Upgrade Theme File", EUserInterfaceActionType::Button, FInputGesture());
}

#undef LOCTEXT_NAMESPACE