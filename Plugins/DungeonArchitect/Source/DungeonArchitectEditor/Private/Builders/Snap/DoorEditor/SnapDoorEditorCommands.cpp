//$ Copyright 2015 Ali Akbar, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//
#include "DungeonArchitectEditorPrivatePCH.h"
#include "SnapDoorEditorCommands.h"

#define LOCTEXT_NAMESPACE "FSnapDoorEditorCommands" 

FSnapDoorEditorCommands::FSnapDoorEditorCommands()
	: TCommands<FSnapDoorEditorCommands>("SnapDoorEditor", NSLOCTEXT("Contexts", "SnapDoorEditor", "Snap Door Editor"), NAME_None, FEditorStyle::GetStyleSetName())
{
}

void FSnapDoorEditorCommands::RegisterCommands()
{
	UI_COMMAND(Rebuild, "Rebuild", "Rebuilds the door", EUserInterfaceActionType::Button, FInputChord());
	UI_COMMAND(ModeDoor, "Door Mesh", "Door Mesh", EUserInterfaceActionType::ToggleButton, FInputChord());
	UI_COMMAND(ModeClosed, "Closed Mesh", "Mesh to block off the wall if no door is preset", EUserInterfaceActionType::ToggleButton, FInputChord());
}

#undef LOCTEXT_NAMESPACE