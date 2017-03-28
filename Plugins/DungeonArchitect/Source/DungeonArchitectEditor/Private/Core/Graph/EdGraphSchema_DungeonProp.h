//$ Copyright 2015 Ali Akbar, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//
#pragma once
#include "EdGraphSchema_DungeonProp.generated.h"

/** Action to add a node to the graph */
USTRUCT()
struct DUNGEONARCHITECTEDITOR_API FDungeonSchemaAction_NewNode : public FEdGraphSchemaAction
{
	GENERATED_USTRUCT_BODY();

	/** Template of node we want to create */
	UPROPERTY()
	class UEdGraphNode* NodeTemplate;


	FDungeonSchemaAction_NewNode()
		: FEdGraphSchemaAction()
		, NodeTemplate(NULL)
	{}

	FDungeonSchemaAction_NewNode(const FText& InNodeCategory, const FText& InMenuDesc, const FString& InToolTip, const int32 InGrouping)
		: FEdGraphSchemaAction(InNodeCategory, InMenuDesc, InToolTip, InGrouping)
		, NodeTemplate(NULL)
	{}

	// FEdGraphSchemaAction interface
	virtual UEdGraphNode* PerformAction(class UEdGraph* ParentGraph, UEdGraphPin* FromPin, const FVector2D Location, bool bSelectNewNode = true) override;
	virtual UEdGraphNode* PerformAction(class UEdGraph* ParentGraph, TArray<UEdGraphPin*>& FromPins, const FVector2D Location, bool bSelectNewNode = true) override;
	virtual void AddReferencedObjects(FReferenceCollector& Collector) override;
	// End of FEdGraphSchemaAction interface

	template <typename NodeType>
	static NodeType* SpawnNodeFromTemplate(class UEdGraph* ParentGraph, NodeType* InTemplateNode, const FVector2D Location, bool bSelectNewNode = true)
	{
		FDungeonSchemaAction_NewNode Action;
		Action.NodeTemplate = InTemplateNode;

		return Cast<NodeType>(Action.PerformAction(ParentGraph, NULL, Location, bSelectNewNode));
	}
};

UCLASS()
class UEdGraphSchema_DungeonProp : public UEdGraphSchema {
	GENERATED_UCLASS_BODY()

	// Begin EdGraphSchema interface
	virtual void GetGraphContextActions(FGraphContextMenuBuilder& ContextMenuBuilder) const override;
	virtual const FPinConnectionResponse CanCreateConnection(const UEdGraphPin* A, const UEdGraphPin* B) const override;
	virtual class FConnectionDrawingPolicy* CreateConnectionDrawingPolicy(int32 InBackLayerID, int32 InFrontLayerID, float InZoomFactor, const FSlateRect& InClippingRect, class FSlateWindowElementList& InDrawElements, class UEdGraph* InGraphObj) const override;
	virtual FLinearColor GetPinTypeColor(const FEdGraphPinType& PinType) const override;
	virtual bool ShouldHidePinDefaultValue(UEdGraphPin* Pin) const override;
	virtual bool TryCreateConnection(UEdGraphPin* A, UEdGraphPin* B) const override;
	// End EdGraphSchema interface

	void GetActionList(TArray<TSharedPtr<FEdGraphSchemaAction> >& OutActions, const UEdGraph* Graph, UEdGraph* OwnerOfTemporaries, bool bShowNewMesh = true, bool bShowNewMarker = true, bool bShowMarkerEmitters = true) const;

private:
	bool ContainsCycles(const UEdGraphPin* A, const UEdGraphPin* B, TArray<FString>& OutCyclePath) const;

};

class DUNGEONARCHITECTEDITOR_API DungeonSchemaUtils {
public:
	template<typename T>
	static void AddAction(FString Title, FString Tooltip, TArray<TSharedPtr<FEdGraphSchemaAction> >& OutActions, UEdGraph* OwnerOfTemporaries) {
		const FText MenuDesc = FText::FromString(Title);
		const FText Category = FText::FromString(TEXT("Dungeon"));
		TSharedPtr<FDungeonSchemaAction_NewNode> NewActorNodeAction = AddNewNodeAction(OutActions, Category, MenuDesc, Tooltip);
		T* ActorNode = NewObject<T>(OwnerOfTemporaries);
		NewActorNodeAction->NodeTemplate = ActorNode;
	}

	static TSharedPtr<FDungeonSchemaAction_NewNode> AddNewNodeAction(TArray<TSharedPtr<FEdGraphSchemaAction> >& OutActions, const FText& Category, const FText& MenuDesc, const FString& Tooltip)
	{
		TSharedPtr<FDungeonSchemaAction_NewNode> NewAction = TSharedPtr<FDungeonSchemaAction_NewNode>(new FDungeonSchemaAction_NewNode(Category, MenuDesc, Tooltip, 0));
		OutActions.Add(NewAction);
		return NewAction;
	}
};

