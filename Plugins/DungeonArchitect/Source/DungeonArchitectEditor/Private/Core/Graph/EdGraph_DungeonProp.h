//$ Copyright 2015 Ali Akbar, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//
#pragma once
#include "DungeonProp.h"
#include "EdGraph_DungeonProp.generated.h"

class UDungeonThemeAsset;

struct FDungeonDataTypes {
	static const FString PinType_Mesh;
	static const FString PinType_Marker;
};

struct FDungeonGraphBuildError {
	class UEdGraphNode_DungeonBase* NodeWithError;
	FString ErrorMessage;
	FString ErrorTooltip;
};

UCLASS()
class UEdGraph_DungeonProp : public UEdGraph {
	GENERATED_UCLASS_BODY()

public:
	void RebuildGraph(UDungeonThemeAsset* DataAsset, TArray<FPropTypeData>& OutProps, TArray<FDungeonGraphBuildError>& OutErrors);

	void RecreateDefaultMarkerNodes(TSubclassOf<class UDungeonBuilder> BuilderClass);

	/** Add a listener for OnGraphChanged events */
	FDelegateHandle AddOnNodePropertyChangedHandler(const FOnGraphChanged::FDelegate& InHandler);

	/** Remove a listener for OnGraphChanged events */
	void RemoveOnNodePropertyChangedHandler(FDelegateHandle Handle);

	void NotifyNodePropertyChanged(const FEdGraphEditAction& InAction);

	bool ContainsFreeSpace(int32 X, int32 Y, float Distance);

private:
	/** A delegate that broadcasts a notification whenever the graph has changed. */
	FOnGraphChanged OnNodePropertyChanged;
};
