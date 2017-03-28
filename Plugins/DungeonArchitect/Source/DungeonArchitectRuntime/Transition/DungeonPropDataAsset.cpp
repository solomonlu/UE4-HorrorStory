//$ Copyright 2015 Ali Akbar, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//
#include "DungeonArchitectRuntimePrivatePCH.h"
#include "DungeonPropDataAsset.h"

#if WITH_EDITORONLY_DATA
#include "EdGraph/EdGraph.h"
#include "EdGraph/EdGraphNode.h"
#endif // WITH_EDITORONLY_DATA

UDungeonPropDataAsset::UDungeonPropDataAsset(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer) {
#if WITH_EDITORONLY_DATA
	PreviewViewportProperties = ObjectInitializer.CreateDefaultSubobject<UDungeonEditorViewportProperties>(this, "PreviewProperties");
#endif	// WITH_EDITORONLY_DATA
}

void UDungeonPropDataAsset::AddReferencedObjects(UObject* InThis, FReferenceCollector& Collector)
{
	UDungeonPropDataAsset* This = CastChecked<UDungeonPropDataAsset>(InThis);

#if WITH_EDITORONLY_DATA
	Collector.AddReferencedObject(This->UpdateGraph, This);
#endif	// WITH_EDITORONLY_DATA

	Super::AddReferencedObjects(This, Collector);
}
