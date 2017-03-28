//$ Copyright 2015 Ali Akbar, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once


class DUNGEONARCHITECTRUNTIME_API IDungeonEditorService {
public:
    virtual ~IDungeonEditorService() {}
	virtual AActor* CreateGroupActor(UWorld* World, const TArray<AActor*>& MergedActorList, const FTransform& Transform) = 0;

	static TSharedPtr<IDungeonEditorService> Get();
	static void Set(TSharedPtr<IDungeonEditorService> InInstance);
private:
	static TSharedPtr<IDungeonEditorService> Instance;
};
