//$ Copyright 2015 Ali Akbar, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//
#include "DungeonArchitectEditorPrivatePCH.h"
#include "DungeonEditorUtils.h"
#include "Core/Dungeon.h"
#include "Editor/LevelEditor/Public/LevelEditor.h"
#include "ILevelViewport.h"

ADungeon* FDungeonEditorUtils::GetDungeonActorFromLevelViewport()
{
	FLevelEditorModule& LevelEditorModule = FModuleManager::GetModuleChecked<FLevelEditorModule>("LevelEditor");
	TSharedPtr< ILevelViewport > ViewportWindow = LevelEditorModule.GetFirstActiveViewport();
	if (ViewportWindow.IsValid())
	{
		FEditorViewportClient &Viewport = ViewportWindow->GetLevelViewportClient();
		UWorld* World = Viewport.GetWorld();
		for (TActorIterator<ADungeon> Dungeon(World); Dungeon; ++Dungeon)
		{
			return *Dungeon;
		}
	}
	return nullptr;
}
