//$ Copyright 2015 Ali Akbar, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//
#include "DungeonArchitectEditorPrivatePCH.h"
#include "EdGraph_DungeonProp.h"
#include "EdGraphSchema_DungeonProp.h"
#include "EdGraphNode_DungeonMarker.h"
#include "EdGraphNode_DungeonMesh.h"
#include "EdGraphNode_DungeonMarkerEmitter.h"
#include "Core/Rules/DungeonSelectorLogic.h"
#include "Core/Rules/DungeonTransformLogic.h"
#include "Core/DungeonBuilder.h"
#include "DungeonThemeAsset.h"
#include "BlueprintEditorUtils.h"

#define SNAP_GRID (16) // @todo ensure this is the same as SNodePanel::GetSnapGridSize()

#define LOCTEXT_NAMESPACE "DungeonPropGraph"

const FString FDungeonDataTypes::PinType_Mesh = "mesh";
const FString FDungeonDataTypes::PinType_Marker = "marker";

UEdGraph_DungeonProp::UEdGraph_DungeonProp(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	Schema = UEdGraphSchema_DungeonProp::StaticClass();
}

template<typename T>
void GetChildNodes(UEdGraphNode_DungeonBase* ParentNode, TArray<T*>& OutChildren) {
	for (UEdGraphPin* ChildPin : ParentNode->GetOutputPin()->LinkedTo) {
		if (ChildPin) {
			if (T* DesiredNode = Cast<T>(ChildPin->GetOwningNode())) {
				OutChildren.Add(DesiredNode);
			}
		}
	}
}

struct ExecutionSortComparer {
	inline bool operator() (const UEdGraphNode_DungeonActorBase& A, const UEdGraphNode_DungeonActorBase& B) const {
		return A.ExecutionOrder < B.ExecutionOrder;
	}
};

template<typename T>
void CloneUObjectArray(UObject* Outer, const TArray<T*>& SourceList, TArray<T*>& DestList) {
	DestList.Reset();
	for (T* Source : SourceList) {
		if (!Source) continue;
		T* Clone = NewObject<T>(Outer, Source->GetClass(), NAME_None, RF_NoFlags, Source);
		DestList.Add(Clone);
	}
}

void UEdGraph_DungeonProp::RebuildGraph(UDungeonThemeAsset* DataAsset, TArray<FPropTypeData>& OutProps, TArray<FDungeonGraphBuildError>& OutErrors)
{
	OutProps.Reset();
	// TODO: Check for cycles

	TArray<UEdGraphNode_DungeonMarker*> MarkerNodes;
	GetNodesOfClass<UEdGraphNode_DungeonMarker>(MarkerNodes);
	for (UEdGraphNode_DungeonMarker* MarkerNode : MarkerNodes) {
		TArray<UEdGraphNode_DungeonActorBase*> ActorNodes;
		GetChildNodes<UEdGraphNode_DungeonActorBase>(MarkerNode, ActorNodes);
		ActorNodes.Sort(ExecutionSortComparer());
		for (UEdGraphNode_DungeonActorBase* ActorNode : ActorNodes) {
            FPropTypeData Prop;
            Prop.NodeId = FName(*ActorNode->NodeGuid.ToString());
			Prop.AttachToSocket = MarkerNode->MarkerName;
			Prop.AssetObject = ActorNode->GetNodeAssetObject(DataAsset);
			Prop.Probability = ActorNode->Probability;
			Prop.Offset = ActorNode->Offset;
			Prop.bUseSpatialConstraint = ActorNode->bUseSpatialConstraint;
			Prop.SpatialConstraint = ActorNode->SpatialConstraint;
			Prop.ConsumeOnAttach = ActorNode->ConsumeOnAttach;

			// Clone the selection logic instances
			CloneUObjectArray<UDungeonSelectorLogic>(DataAsset, ActorNode->SelectionLogics, Prop.SelectionLogics);
			Prop.bUseSelectionLogic = ActorNode->bUseSelectionLogic;
			Prop.bLogicOverridesAffinity = ActorNode->bLogicOverridesAffinity;

			// Clone the transform logic instance
			CloneUObjectArray<UDungeonTransformLogic>(DataAsset, ActorNode->TransformLogics, Prop.TransformLogics);
			Prop.bUseTransformLogic = ActorNode->bUseTransformLogic;


			// Insert Child Marker emitters
			TArray<UEdGraphNode_DungeonMarkerEmitter*> EmitterNodes;
			GetChildNodes<UEdGraphNode_DungeonMarkerEmitter>(ActorNode, EmitterNodes);
			for (UEdGraphNode_DungeonMarkerEmitter* EmitterNode : EmitterNodes) {
				//if (!EmitterNode || !EmitterNode->IsValidLowLevel()) continue;
				FPropChildSocketData ChildSocket;
				ChildSocket.SocketType = EmitterNode->ParentMarker->MarkerName;
				ChildSocket.Offset = EmitterNode->Offset;
				Prop.ChildSockets.Add(ChildSocket);
			}
			OutProps.Add(Prop);
		}
	}
}

void UEdGraph_DungeonProp::RecreateDefaultMarkerNodes(TSubclassOf<UDungeonBuilder> BuilderClass)
{
	// Remove all the unused builder marker nodes 
	{
		TSet<UEdGraphNode_DungeonMarker*> ReferencedNodes;

		// First cache all the marker emitter nodes' references
		TArray<UEdGraphNode_DungeonMarkerEmitter*> EmitterNodes;
		GetNodesOfClass<UEdGraphNode_DungeonMarkerEmitter>(EmitterNodes);
		for (UEdGraphNode_DungeonMarkerEmitter* EmitterNode : EmitterNodes) {
			if (EmitterNode && EmitterNode->ParentMarker) {
				ReferencedNodes.Add(EmitterNode->ParentMarker);
			}
		}

		// Now grab all the marker nodes, and remove the builder created nodes that are not referenced by emitters
		TArray<UEdGraphNode_DungeonMarker*> MarkerNodes;
		GetNodesOfClass<UEdGraphNode_DungeonMarker>(MarkerNodes);
		for (UEdGraphNode_DungeonMarker* MarkerNode : MarkerNodes) {
			if (MarkerNode->bBuilderEmittedMarker && !ReferencedNodes.Contains(MarkerNode)) {
				if (MarkerNode->GetOutputPin()->LinkedTo.Num() > 0) {
					// This node has children
					continue;
				}

				// Remove this node from the graph
				// Break all node links first so that we don't update the material before deleting
				MarkerNode->BreakAllNodeLinks();
				FBlueprintEditorUtils::RemoveNode(NULL, MarkerNode, true);
			}
		}
	}

	TSet<FString> ExistingMarkerNames;
	{
		TArray<UEdGraphNode_DungeonMarker*> MarkerNodes;
		GetNodesOfClass<UEdGraphNode_DungeonMarker>(MarkerNodes);
		for (UEdGraphNode_DungeonMarker* MarkerNode : MarkerNodes) {
			ExistingMarkerNames.Add(MarkerNode->MarkerName);
		}
	}



	TArray<FString> DefaultMarkerNames;
	if (BuilderClass) {
		UDungeonBuilder* Builder = NewObject<UDungeonBuilder>((UObject*)GetTransientPackage(), BuilderClass);
		if (Builder) {
			Builder->GetDefaultMarkerNames(DefaultMarkerNames);
		}
	}

	int32 NumItemsPerRow = 5;
	int32 XDelta = 200;
	int32 YDelta = 400;

	int PositionIndex = 0;

	for (int i = 0; i < DefaultMarkerNames.Num(); i++) {
		FString MarkerName = DefaultMarkerNames[i];
		if (ExistingMarkerNames.Contains(MarkerName)) {
			continue;
		}
		ExistingMarkerNames.Add(MarkerName);

		UEdGraphNode_DungeonMarker* MarkerNode = NewObject<UEdGraphNode_DungeonMarker>(this);
		MarkerNode->bUserDefined = false;
		MarkerNode->bBuilderEmittedMarker = true;
		MarkerNode->Rename(NULL, this, REN_NonTransactional);
		MarkerNode->MarkerName = MarkerName;
		this->AddNode(MarkerNode, true, false);

		MarkerNode->CreateNewGuid();
		MarkerNode->PostPlacedNewNode();
		MarkerNode->AllocateDefaultPins();
		MarkerNode->NodePosY = -1000;

		bool bOccupied = false;
		int32 X, Y;
		do {
			X = (PositionIndex % NumItemsPerRow) * XDelta;
			Y = (PositionIndex / NumItemsPerRow) * YDelta;
			PositionIndex++;
		} while (!ContainsFreeSpace(X, Y, 100));

		MarkerNode->NodePosX = X;
		MarkerNode->NodePosY = Y;
		MarkerNode->SnapToGrid(SNAP_GRID);
	}

}

float GetDistanceSq(int32 X0, int32 Y0, int32 X1, int32 Y1) {
	return (X0 - X1) * (X0 - X1) + (Y0 - Y1) * (Y0 - Y1);
}

bool UEdGraph_DungeonProp::ContainsFreeSpace(int32 X, int32 Y, float Distance)
{
	TArray<UEdGraphNode_DungeonBase*> DungeonNodes;
	GetNodesOfClass<UEdGraphNode_DungeonBase>(DungeonNodes);
	
	for (UEdGraphNode_DungeonBase* Node : DungeonNodes) {
		float DistSq = GetDistanceSq(X, Y, Node->NodePosX, Node->NodePosY);
		if (DistSq < Distance * Distance) {
			return false;
		}
	}
	return true;
}

FDelegateHandle UEdGraph_DungeonProp::AddOnNodePropertyChangedHandler(const FOnGraphChanged::FDelegate& InHandler)
{
	return OnNodePropertyChanged.Add(InHandler);
}

void UEdGraph_DungeonProp::RemoveOnNodePropertyChangedHandler(FDelegateHandle Handle)
{
	OnNodePropertyChanged.Remove(Handle);
}

void UEdGraph_DungeonProp::NotifyNodePropertyChanged(const FEdGraphEditAction& InAction)
{
	OnNodePropertyChanged.Broadcast(InAction);
}

#undef LOCTEXT_NAMESPACE
