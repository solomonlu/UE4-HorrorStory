//$ Copyright 2015 Ali Akbar, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//
#pragma once
#include "Core/Utils/IDungeonEditorService.h"

class DUNGEONARCHITECTEDITOR_API FDungeonEditorService : public IDungeonEditorService {
public:
	virtual AActor* CreateGroupActor(UWorld* World, const TArray<AActor*>& MergedActorList, const FTransform& Transform) override;

};

