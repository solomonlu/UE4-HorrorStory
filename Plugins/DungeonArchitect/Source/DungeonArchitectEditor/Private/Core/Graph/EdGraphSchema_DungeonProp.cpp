//$ Copyright 2015 Ali Akbar, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//
#include "DungeonArchitectEditorPrivatePCH.h"
#include "EdGraph_DungeonProp.h"
#include "EdGraphSchema_DungeonProp.h"
#include "EdGraphNode_DungeonMesh.h"
#include "EdGraphNode_DungeonMarker.h"
#include "DungeonPropConnectionDrawingPolicy.h"
#include "EdGraphNode_DungeonMarkerEmitter.h"
#include "EdGraph/EdGraphPin.h"
#include "EdGraphNode_DungeonParticleSystem.h"
#include "EdGraphNode_DungeonActorTemplate.h"
#include "EdGraphNode_DungeonPointLight.h"
#include "EdGraphNode_DungeonSpotLight.h"
#include "EdGraphNode_DungeonDestructibleMesh.h"
#include "EdGraph/EdGraph.h"
#include "EdGraphSchema_Extensions.h"

#define LOCTEXT_NAMESPACE "EdGraphSchema_DungeonProp"

#define SNAP_GRID (16) // @todo ensure this is the same as SNodePanel::GetSnapGridSize()

namespace
{
	// Maximum distance a drag can be off a node edge to require 'push off' from node
	const int32 NodeDistance = 60;
}

UEdGraphNode* FDungeonSchemaAction_NewNode::PerformAction(class UEdGraph* ParentGraph, UEdGraphPin* FromPin, const FVector2D Location, bool bSelectNewNode/* = true*/)
{
	UEdGraphNode* ResultNode = NULL;

	// If there is a template, we actually use it
	if (NodeTemplate != NULL)
	{
		NodeTemplate->SetFlags(RF_Transactional);

		// set outer to be the graph so it doesn't go away
		NodeTemplate->Rename(NULL, ParentGraph, REN_NonTransactional);
		ParentGraph->AddNode(NodeTemplate, true, bSelectNewNode);

		NodeTemplate->CreateNewGuid();
		NodeTemplate->PostPlacedNewNode();
		NodeTemplate->AllocateDefaultPins();
		NodeTemplate->AutowireNewNode(FromPin);

		
		// For input pins, new node will generally overlap node being dragged off
		// Work out if we want to visually push away from connected node
		int32 XLocation = Location.X;
		if (FromPin && FromPin->Direction == EGPD_Input)
		{
			UEdGraphNode* PinNode = FromPin->GetOwningNode();
			const float XDelta = FMath::Abs(PinNode->NodePosX - Location.X);

			if (XDelta < NodeDistance)
			{
				// Set location to edge of current node minus the max move distance
				// to force node to push off from connect node enough to give selection handle
				XLocation = PinNode->NodePosX - NodeDistance;
			}
		}

		NodeTemplate->NodePosX = XLocation;
		NodeTemplate->NodePosY = Location.Y;
		NodeTemplate->SnapToGrid(SNAP_GRID);

		ResultNode = NodeTemplate;
	}

	return ResultNode;
}

UEdGraphNode* FDungeonSchemaAction_NewNode::PerformAction(class UEdGraph* ParentGraph, TArray<UEdGraphPin*>& FromPins, const FVector2D Location, bool bSelectNewNode/* = true*/)
{
	UEdGraphNode* ResultNode = NULL;

	if (FromPins.Num() > 0)
	{
		ResultNode = PerformAction(ParentGraph, FromPins[0], Location, bSelectNewNode);

		// Try autowiring the rest of the pins
		for (int32 Index = 1; Index < FromPins.Num(); ++Index)
		{
			ResultNode->AutowireNewNode(FromPins[Index]);
		}
	}
	else
	{
		ResultNode = PerformAction(ParentGraph, NULL, Location, bSelectNewNode);
	}

	return ResultNode;
}

void FDungeonSchemaAction_NewNode::AddReferencedObjects(FReferenceCollector& Collector)
{
	FEdGraphSchemaAction::AddReferencedObjects(Collector);

	// These don't get saved to disk, but we want to make sure the objects don't get GC'd while the action array is around
	Collector.AddReferencedObject(NodeTemplate);
}
/////////////////////////////////////////////////////////

UEdGraphSchema_DungeonProp::UEdGraphSchema_DungeonProp(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UEdGraphSchema_DungeonProp::GetActionList(TArray<TSharedPtr<FEdGraphSchemaAction> >& OutActions, const UEdGraph* Graph, UEdGraph* OwnerOfTemporaries, bool bShowNewMesh, bool bShowNewMarker, bool bShowMarkerEmitters) const
{
	// Add mesh node
	if (bShowNewMesh)
	{
		DungeonSchemaUtils::AddAction<UEdGraphNode_DungeonMesh>(TEXT("Add Mesh Node"), TEXT("Add a static mesh node to the prop graph"), OutActions, OwnerOfTemporaries);
		DungeonSchemaUtils::AddAction<UEdGraphNode_DungeonDestructibleMesh>(TEXT("Add Destructible Mesh Node"), TEXT("Add a static mesh node to the prop graph"), OutActions, OwnerOfTemporaries);
		DungeonSchemaUtils::AddAction<UEdGraphNode_DungeonPointLight>(TEXT("Add Point Light Node"), TEXT("Add a point light node to the prop graph"), OutActions, OwnerOfTemporaries);
		DungeonSchemaUtils::AddAction<UEdGraphNode_DungeonSpotLight>(TEXT("Add Spot Light Node"), TEXT("Add a spot light node to the prop graph"), OutActions, OwnerOfTemporaries);
		DungeonSchemaUtils::AddAction<UEdGraphNode_DungeonParticleSystem>(TEXT("Add Particle System Node"), TEXT("Add a particle system node to the prop graph"), OutActions, OwnerOfTemporaries);
		DungeonSchemaUtils::AddAction<UEdGraphNode_DungeonActorTemplate>(TEXT("Add Actor Node"), TEXT("Add an actor node to the prop graph"), OutActions, OwnerOfTemporaries);
	}
	// Add marker node
	if (bShowNewMarker)
	{
		const FText MenuDesc = FText::FromString("Add Marker Node");
		TSharedPtr<FDungeonSchemaAction_NewNode> NewMarkerNodeAction = DungeonSchemaUtils::AddNewNodeAction(OutActions, FText::FromString(TEXT("Dungeon")), MenuDesc, TEXT("Add a marker reference"));
		UEdGraphNode_DungeonMarker* MarkerNode = NewObject<UEdGraphNode_DungeonMarker>(OwnerOfTemporaries);
		NewMarkerNodeAction->NodeTemplate = MarkerNode;
	}
	// Add marker emitter node
	if (bShowMarkerEmitters)
	{
		TArray<UEdGraphNode_DungeonMarker*> MarkerNodes;
		Graph->GetNodesOfClass<UEdGraphNode_DungeonMarker>(MarkerNodes);

		for (UEdGraphNode_DungeonMarker* MarkerNode : MarkerNodes) {
			const FText MenuDesc = FText::FromString("Add Marker Emitter Node: " + MarkerNode->MarkerName);
			TSharedPtr<FDungeonSchemaAction_NewNode> NewMarkerEmitterNodeAction = DungeonSchemaUtils::AddNewNodeAction(OutActions, FText::FromString(TEXT("Marker Emitters")), MenuDesc, TEXT("Add a marker emitter"));
			UEdGraphNode_DungeonMarkerEmitter* MarkerEmitterNode = NewObject<UEdGraphNode_DungeonMarkerEmitter>(OwnerOfTemporaries);
			MarkerEmitterNode->ParentMarker = MarkerNode;
			NewMarkerEmitterNodeAction->NodeTemplate = MarkerEmitterNode;
		}
	}

	UEdGraphSchema_Extensions::Get().CreateCustomActions(OutActions, Graph, OwnerOfTemporaries, bShowNewMesh, bShowNewMarker, bShowMarkerEmitters);
}


void UEdGraphSchema_DungeonProp::GetGraphContextActions(FGraphContextMenuBuilder& ContextMenuBuilder) const
{
	FFormatNamedArguments Args;
	const FName AttrName("Attributes");
	Args.Add(TEXT("Attribute"), FText::FromName(AttrName));
	const UEdGraphPin* FromPin = ContextMenuBuilder.FromPin;
	bool bShowNewMesh = true;
	bool bShowNewMarker = true;
	bool bShowMarkerEmitters = true;
	if (FromPin) {

		if (FromPin->Direction == EEdGraphPinDirection::EGPD_Output) {
			if (FromPin->PinType.PinCategory == FDungeonDataTypes::PinType_Mesh) {
				// Show mesh, since this is coming out from a mesh pin
				bShowNewMesh = true;
				bShowNewMarker = false;
				bShowMarkerEmitters = false;
			}
			else if (FromPin->PinType.PinCategory == FDungeonDataTypes::PinType_Marker) {
				// Show markers emitters, since the output type is a marker
				bShowNewMesh = false;
				bShowNewMarker = false;
				bShowMarkerEmitters = true;
			}
		}
		else {	// FromPin->Direction == EEdGraphPinDirection::EGPD_Input
			if (FromPin->PinType.PinCategory == FDungeonDataTypes::PinType_Mesh) {
				// Show marker, since this is coming up from an mesh node
				bShowNewMesh = false;
				bShowNewMarker = true;
				bShowMarkerEmitters = false;
			}
			else if (FromPin->PinType.PinCategory == FDungeonDataTypes::PinType_Marker) {
				// Show only new mesh creation option, since this is coming up from a marker emitter
				bShowNewMesh = true;
				bShowNewMarker = false;
				bShowMarkerEmitters = false;
			}
		}
	}

	const UEdGraph* Graph = ContextMenuBuilder.CurrentGraph;
	TArray<TSharedPtr<FEdGraphSchemaAction> > Actions;
	GetActionList(Actions, Graph, ContextMenuBuilder.OwnerOfTemporaries, bShowNewMesh, bShowNewMarker, bShowMarkerEmitters);

	for (TSharedPtr<FEdGraphSchemaAction> Action : Actions) {
		ContextMenuBuilder.AddAction(Action);
	}
}

/*
const UEdGraphNode_DungeonMarker* GetMarkerNode(const UEdGraphPin* Pin) {
	const UEdGraphNode_DungeonBase* Node = Cast<UEdGraphNode_DungeonBase>(Pin->GetOwningNode());
	if (!Node) return nullptr;

	if (Cast<UEdGraphNode_DungeonMarker>(Node) != NULL) {
		return Cast<UEdGraphNode_DungeonMarker>(Node);
	}

	UEdGraphPin* InputPin = Node->GetInputPin();

	// Go one level up
	for (const UEdGraphPin* OutputPins : InputPin->LinkedTo) {

	}
	return nullptr;
}
*/

FString GetNodeName(const UEdGraphNode* Node) {
	FText emitterText = Node->GetNodeTitle(ENodeTitleType::FullTitle);
	return emitterText.ToString();
}

TArray<UEdGraphNode_DungeonBase*> GetOutgoingNodes(const UEdGraphPin* A, const UEdGraphPin* B, const UEdGraphNode_DungeonBase* Node) {
	TArray<UEdGraphNode_DungeonBase*> Result;
	UEdGraphPin* OutPin = Node->GetOutputPin();
	if (OutPin) {
		for (const UEdGraphPin* InputPin : OutPin->LinkedTo) {
			UEdGraphNode_DungeonBase* NextNode = Cast<UEdGraphNode_DungeonBase>(InputPin->GetOwningNode());
			if (NextNode) {
				Result.Add(NextNode);
			}
		}
	}

	if (OutPin == A) {
		UEdGraphNode_DungeonBase* NextNode = Cast<UEdGraphNode_DungeonBase>(B->GetOwningNode());
		if (NextNode) Result.Add(NextNode);
	}
	if (OutPin == B) {
		UEdGraphNode_DungeonBase* NextNode = Cast<UEdGraphNode_DungeonBase>(A->GetOwningNode());
		if (NextNode) Result.Add(NextNode);
	}
	return Result;
}

TArray<FString> GetEmitters(const UEdGraphPin* A, const UEdGraphPin* B, const UEdGraphNode_DungeonMarker* MarkerNode) {
	TArray<FString> EmitterNames;
	TArray<UEdGraphNode_DungeonBase*> MeshNodes = GetOutgoingNodes(A, B, MarkerNode);

	for (const UEdGraphNode_DungeonBase* MeshNode : MeshNodes) {
		TArray<UEdGraphNode_DungeonBase*> EmitterNodes = GetOutgoingNodes(A, B, MeshNode);
		for (const UEdGraphNode_DungeonBase* EmitterNode : EmitterNodes) {
			EmitterNames.Add(GetNodeName(EmitterNode));
		}
	}
	return EmitterNames;
}

typedef TMap<FString, TArray<FString>> MarkerToEmitterMapping_t;

bool CheckCycleRecursive(const MarkerToEmitterMapping_t& MarkerToEmitterMapping, TArray<FString>& TraversePath) {
	FString TopMarker = TraversePath.Last();
	if (!MarkerToEmitterMapping.Contains(TopMarker)) return false;
	const TArray<FString>& Emitters = MarkerToEmitterMapping[TopMarker];
	for (const FString Emitter : Emitters) {
		// Check if this emitter forms a cycle with the existing traverse path
		if (TraversePath.Contains(Emitter)) {
			TraversePath.Add(Emitter);
			return true;
		}
		TraversePath.Add(Emitter);
		bool bContainsCycle = CheckCycleRecursive(MarkerToEmitterMapping, TraversePath);
		if (bContainsCycle) {
			return true;
		}
		// Remove the last element
		TraversePath.RemoveAt(TraversePath.Num() - 1);
	}
	return false;
}

bool FindCycles(const MarkerToEmitterMapping_t& MarkerToEmitterMapping, TArray<FString>& OutCyclePath) {
	TSet<FString> MarkerVisited;
	TArray<FString> Markers;
	MarkerToEmitterMapping.GenerateKeyArray(Markers);

	for (const FString& MarkerName : Markers) {
		if (MarkerVisited.Contains(MarkerName)) {
			// Already processed
			continue;
		}

		// Start a DFS from here and see if we come back to a visited node
		TArray<FString> TraversePath;
		TraversePath.Add(MarkerName);
		if (CheckCycleRecursive(MarkerToEmitterMapping, TraversePath)) {
			OutCyclePath = TraversePath;
			return true;
		}
	}

	return false;
}

bool UEdGraphSchema_DungeonProp::ContainsCycles(const UEdGraphPin* A, const UEdGraphPin* B, TArray<FString>& OutCyclePath) const
{
	if (!A || !B) return false;
	UEdGraph* Graph = A->GetOwningNode()->GetGraph();

	TArray<UEdGraphNode_DungeonMarker*> MarkerNodes;
	Graph->GetNodesOfClass<UEdGraphNode_DungeonMarker>(MarkerNodes);

	
	MarkerToEmitterMapping_t MarkerToEmitterMapping;
	for (const UEdGraphNode_DungeonMarker* MarkerNode : MarkerNodes) {
		TArray<FString> Emitters = GetEmitters(A, B, MarkerNode);
		FString MarkerName = GetNodeName(MarkerNode);
		if (!MarkerToEmitterMapping.Contains(MarkerName)) {
			MarkerToEmitterMapping.Add(MarkerName, Emitters);
		}
	}

	// Check if we find a cycle

	OutCyclePath = TArray<FString>();
	if (FindCycles(MarkerToEmitterMapping, OutCyclePath)) {
		return true;
	}

	return false;
}

FString Combine(const TArray<FString> Array, FString Separator) {
	// TODO: Use a string builder
	FString Result;
	for (const FString& Item : Array) {
		if (Result.Len() > 0) {
			Result += Separator;
		}
		Result += Item;
	}
	return Result;
}

const FPinConnectionResponse UEdGraphSchema_DungeonProp::CanCreateConnection(const UEdGraphPin* A, const UEdGraphPin* B) const
{
	// Make sure the input is connecting to an output
	if (A->Direction == B->Direction) {
		return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, TEXT("Not allowed"));
	}

	// Make sure the data types match
	if (A->PinType.PinCategory != B->PinType.PinCategory) {
		return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, TEXT("Not allowed"));
	}

	// Make sure we don't have a cycle formed by this link
	TArray<FString> CyclePath;
	if (ContainsCycles(A, B, CyclePath)) {
		FString CycleString = Combine(CyclePath, " > ");
		return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, TEXT("Not allowed. Contains a cycle: " + CycleString));
	}

	return FPinConnectionResponse(CONNECT_RESPONSE_MAKE, TEXT(""));
}

class FConnectionDrawingPolicy* UEdGraphSchema_DungeonProp::CreateConnectionDrawingPolicy(int32 InBackLayerID, int32 InFrontLayerID, float InZoomFactor, const FSlateRect& InClippingRect, class FSlateWindowElementList& InDrawElements, class UEdGraph* InGraphObj) const
{
	return new FDungeonPropConnectionDrawingPolicy(InBackLayerID, InFrontLayerID, InZoomFactor, InClippingRect, InDrawElements, InGraphObj);
}

FLinearColor UEdGraphSchema_DungeonProp::GetPinTypeColor(const FEdGraphPinType& PinType) const
{
	return FColor::Yellow;
}

bool UEdGraphSchema_DungeonProp::ShouldHidePinDefaultValue(UEdGraphPin* Pin) const
{
	return false;
}


bool UEdGraphSchema_DungeonProp::TryCreateConnection(UEdGraphPin* A, UEdGraphPin* B) const
{
	bool ConnectionMade = UEdGraphSchema::TryCreateConnection(A, B);
	if (ConnectionMade) {
		UEdGraphPin* OutputPin = (A->Direction == EEdGraphPinDirection::EGPD_Output) ? A : B;
		UEdGraphNode_DungeonBase* OutputNode = Cast<UEdGraphNode_DungeonBase>(OutputPin->GetOwningNode());
		if (OutputNode) {
			OutputNode->UpdateChildExecutionOrder();
			OutputNode->GetGraph()->NotifyGraphChanged();
		}

	}

	return ConnectionMade;
}

#undef LOCTEXT_NAMESPACE
