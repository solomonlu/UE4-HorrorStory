//$ Copyright 2015 Ali Akbar, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//
#include "DungeonArchitectEditorPrivatePCH.h"
#include "GraphPanelNodeFactory_DungeonProp.h"
#include "SGraphNode_DungeonActor.h"
#include "SGraphNode_DungeonMarker.h"
#include "SGraphNode_DungeonMarkerEmitter.h"
#include "Core/Graph/EdGraphNode_DungeonMesh.h"
#include "Core/Graph/EdGraphNode_DungeonMarker.h"
#include "Core/Graph/EdGraphNode_DungeonMarkerEmitter.h"

FGraphPanelNodeFactory_DungeonProp::FGraphPanelNodeFactory_DungeonProp()
{
}

TSharedPtr<class SGraphNode> FGraphPanelNodeFactory_DungeonProp::CreateNode(UEdGraphNode* Node) const
{
	if (UEdGraphNode_DungeonActorBase* ActorNode = Cast<UEdGraphNode_DungeonActorBase>(Node))
	{
		TSharedPtr<SGraphNode_DungeonActor> SNode = SNew(SGraphNode_DungeonActor, ActorNode);
		ActorNode->PropertyObserver = SNode;
		return SNode;
	}
	else if (UEdGraphNode_DungeonMarker* MarkerNode = Cast<UEdGraphNode_DungeonMarker>(Node))
	{
		TSharedPtr<SGraphNode_DungeonMarker> SNode = SNew(SGraphNode_DungeonMarker, MarkerNode);
		MarkerNode->PropertyObserver = SNode;
		return SNode;
	}
	else if (UEdGraphNode_DungeonMarkerEmitter* MarkerEmitterNode = Cast<UEdGraphNode_DungeonMarkerEmitter>(Node))
	{
		TSharedPtr<SGraphNode_DungeonMarkerEmitter> SNode = SNew(SGraphNode_DungeonMarkerEmitter, MarkerEmitterNode);
		MarkerEmitterNode->PropertyObserver = SNode;
		return SNode;
	}

	return NULL;
}
