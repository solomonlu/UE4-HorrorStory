//$ Copyright 2015 Ali Akbar, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "Core/DungeonModel.h"
#include "Core/DungeonProp.h"
#include "Core/Utils/DungeonEditorViewportProperties.h"
#include "DungeonPropDataAsset.generated.h"

/** A Dungeon Theme asset lets you design the look and feel of you dungeon with an intuitive graph based approach */
UCLASS(Blueprintable)
class DUNGEONARCHITECTRUNTIME_API UDungeonPropDataAsset : public UObject {
	GENERATED_UCLASS_BODY()

public:

	UPROPERTY()
	TArray<FPropTypeData> Props;

#if WITH_EDITORONLY_DATA
	/** EdGraph based representation */
	UPROPERTY()
	class UEdGraph* UpdateGraph;

	UPROPERTY()
	UDungeonEditorViewportProperties* PreviewViewportProperties;
#endif // WITH_EDITORONLY_DATA

	static void AddReferencedObjects(UObject* InThis, FReferenceCollector& Collector);
};



