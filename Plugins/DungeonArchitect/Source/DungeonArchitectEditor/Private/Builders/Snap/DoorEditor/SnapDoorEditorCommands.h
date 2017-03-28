//$ Copyright 2015 Ali Akbar, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once

/**
* Holds the UI commands for the landscape theme editor
*/
class FSnapDoorEditorCommands
	: public TCommands<FSnapDoorEditorCommands>
{
public:

	/**
	* Default constructor.
	*/
	FSnapDoorEditorCommands();

public:

	// TCommands interface

	virtual void RegisterCommands() override;

public:

	/** Toggles the red channel */
	TSharedPtr<FUICommandInfo> Rebuild;
	TSharedPtr<FUICommandInfo> ModeDoor;
	TSharedPtr<FUICommandInfo> ModeClosed;

};

