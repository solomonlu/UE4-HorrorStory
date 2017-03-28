//$ Copyright 2015 Ali Akbar, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//
#pragma once
#include "EdGraph/EdGraphNode.h"
#include "EdGraphNode_DungeonBase.generated.h"


class DUNGEONARCHITECTEDITOR_API FNodePropertyObserver {
public:
	virtual void OnPropertyChanged(class UEdGraphNode_DungeonBase* Sender, const FName& PropertyName) = 0;
};

UCLASS()
class DUNGEONARCHITECTEDITOR_API UEdGraphNode_DungeonBase : public UEdGraphNode {
	GENERATED_UCLASS_BODY()

public:
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& e);
	virtual UEdGraphPin* GetInputPin() const { return nullptr;  }
	virtual UEdGraphPin* GetOutputPin() const { return nullptr; }
	virtual void NodeConnectionListChanged() override;
	
	void UpdateChildExecutionOrder();

public:
	TSharedPtr<FNodePropertyObserver> PropertyObserver;
	bool bHighlightChildNodeIndices;
};
