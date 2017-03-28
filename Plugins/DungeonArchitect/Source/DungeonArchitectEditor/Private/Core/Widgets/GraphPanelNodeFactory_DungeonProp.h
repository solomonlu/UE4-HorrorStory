//$ Copyright 2015 Ali Akbar, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//
#pragma once
#include "EdGraphUtilities.h"

class DUNGEONARCHITECTEDITOR_API FGraphPanelNodeFactory_DungeonProp : public FGraphPanelNodeFactory
{
public:
	FGraphPanelNodeFactory_DungeonProp();

private:
	virtual TSharedPtr<class SGraphNode> CreateNode(UEdGraphNode* Node) const override;
};
