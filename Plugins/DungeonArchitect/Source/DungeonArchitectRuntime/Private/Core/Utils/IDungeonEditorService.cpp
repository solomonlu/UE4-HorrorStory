//$ Copyright 2015 Ali Akbar, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//
#include "DungeonArchitectRuntimePrivatePCH.h"
#include "Core/Utils/IDungeonEditorService.h"

TSharedPtr<IDungeonEditorService> IDungeonEditorService::Instance;

TSharedPtr<IDungeonEditorService> IDungeonEditorService::Get()
{
	return Instance;
}

void IDungeonEditorService::Set(TSharedPtr<IDungeonEditorService> InInstance)
{
	Instance = InInstance;
}
