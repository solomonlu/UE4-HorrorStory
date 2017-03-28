//$ Copyright 2015 Ali Akbar, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//
#pragma once
#include "EdGraphNode_DungeonBase.h"
#include "EdGraphNode_DungeonMarker.generated.h"

UCLASS()
class DUNGEONARCHITECTEDITOR_API UEdGraphNode_DungeonMarker : public UEdGraphNode_DungeonBase {
	GENERATED_UCLASS_BODY()

	// Begin UEdGraphNode interface.
	virtual void AllocateDefaultPins() override;
	virtual FLinearColor GetNodeTitleColor() const override;
	virtual FText GetTooltipText() const override;
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual void DestroyNode();
	// End UEdGraphNode interface.

	// UObject interface
	void PostEditChangeProperty(struct FPropertyChangedEvent& e);
	virtual bool CanEditChange(const UProperty* InProperty) const override;
	// End of UObject interface

public:
	virtual UEdGraphPin* GetInputPin() const { return nullptr; }
	virtual UEdGraphPin* GetOutputPin() const { return Pins[0]; }
	virtual void AutowireNewNode(UEdGraphPin* FromPin) override;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Dungeon)
	FString MarkerName;

	UPROPERTY()
	bool bUserDefined;


	UPROPERTY()
	bool bBuilderEmittedMarker;
};
