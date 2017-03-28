//$ Copyright 2015 Ali Akbar, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//
#pragma once
#include "SListView.h"
#include "Core/Graph/EdGraphNode_DungeonMarker.h"

/** The list view mode of the asset view */
class DUNGEONARCHITECTEDITOR_API SMarkerListView : public SListView<UEdGraphNode_DungeonMarker*>
{
protected:
	void GetMarkerNodes(TArray<UEdGraphNode_DungeonMarker*>& OutList);
	void GenerateMakerListRow();

};
