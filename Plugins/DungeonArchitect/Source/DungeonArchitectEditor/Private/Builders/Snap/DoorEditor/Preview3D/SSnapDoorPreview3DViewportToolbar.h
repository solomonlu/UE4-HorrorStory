//$ Copyright 2015 Ali Akbar, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "SViewportToolBar.h"


/**
* A level viewport toolbar widget that is placed in a viewport
*/
class DUNGEONARCHITECTEDITOR_API SSnapDoorPreview3DViewportToolbar : public SViewportToolBar
{
public:
	SLATE_BEGIN_ARGS(SSnapDoorPreview3DViewportToolbar){}
		SLATE_ARGUMENT(TSharedPtr<class SSnapDoorPreview3DViewport>, Viewport)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	TSharedRef<SWidget> GeneratePropertiesMenu() const;
	TSharedRef<SWidget> GenerateCameraMenu() const;
	TSharedPtr<FExtender> GetViewMenuExtender();

private:
	/** The viewport that we are in */
	TWeakPtr<class SSnapDoorPreview3DViewport> Viewport;

};


/**
* Class containing commands for level viewport actions
*/
class DUNGEONARCHITECTEDITOR_API FSnapDoorEditorViewportCommands : public TCommands<FSnapDoorEditorViewportCommands> {
public:

	FSnapDoorEditorViewportCommands()
	: TCommands<FSnapDoorEditorViewportCommands>
		(
			TEXT("LAThemeEditorViewport"), // Context name for fast lookup
			NSLOCTEXT("Contexts", "SnapDoorViewport", "Snap Door Viewport"), // Localized context name for displaying
			NAME_None, //TEXT("EditorViewport"), // Parent context name.  
			FEditorStyle::GetStyleSetName() // Icon Style Set
		)
	{
	}

	TSharedPtr< FUICommandInfo > ToggleDebugData;

public:
	/** Registers our commands with the binding system */
	virtual void RegisterCommands() override;

};