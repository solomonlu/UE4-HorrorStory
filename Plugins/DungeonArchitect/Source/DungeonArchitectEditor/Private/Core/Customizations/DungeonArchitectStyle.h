//$ Copyright 2015 Ali Akbar, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//
#pragma once

/**
* Implements the visual style of Dungeon Architect plugin
*/
class FDungeonArchitectStyle
{
public:
	static void Initialize();

	static void Shutdown();

	/** @return The Slate style set for Fortnite Editor */
	static const ISlateStyle& Get();

	static FName GetStyleSetName();

private:
	static TSharedRef< class FSlateStyleSet > Create();
	static FString InContent(const FString& RelativePath, const ANSICHAR* Extension);

private:

	static TSharedPtr< class FSlateStyleSet > StyleInstance;
};

