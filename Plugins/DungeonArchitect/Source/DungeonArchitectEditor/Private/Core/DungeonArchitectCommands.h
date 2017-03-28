//$ Copyright 2015 Ali Akbar, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//
#pragma once

class DUNGEONARCHITECTEDITOR_API FDungeonArchitectCommands : public TCommands<FDungeonArchitectCommands>
{
public:
	FDungeonArchitectCommands();

	virtual void RegisterCommands() override;

public:

	TSharedPtr< FUICommandInfo > OpenDungeonEditor;

	/** Commands for the dungeon editor mode toolbar. */
	TSharedPtr< FUICommandInfo > ModePaint;
	TSharedPtr< FUICommandInfo > ModeRectangle;
	TSharedPtr< FUICommandInfo > ModeBorder;
	TSharedPtr< FUICommandInfo > ModeSelect;

	TSharedPtr< FUICommandInfo > UpgradeThemeFile;
	
};