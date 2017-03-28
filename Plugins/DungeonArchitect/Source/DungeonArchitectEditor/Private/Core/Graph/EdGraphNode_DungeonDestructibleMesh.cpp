//$ Copyright 2015 Ali Akbar, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//
#include "DungeonArchitectEditorPrivatePCH.h"
#include "EdGraphNode_DungeonDestructibleMesh.h"

UEdGraphNode_DungeonDestructibleMesh::UEdGraphNode_DungeonDestructibleMesh(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{
}


UObject* UEdGraphNode_DungeonDestructibleMesh::GetNodeAssetObject(UObject* Outer)
{	
	return DestructibleMesh;
}

UObject* UEdGraphNode_DungeonDestructibleMesh::GetThumbnailAssetObject()
{
	return DestructibleMesh;
}
