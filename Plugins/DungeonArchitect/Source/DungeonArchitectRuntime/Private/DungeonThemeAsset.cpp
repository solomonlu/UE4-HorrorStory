//$ Copyright 2015 Ali Akbar, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//
#include "DungeonArchitectRuntimePrivatePCH.h"
#include "DungeonThemeAsset.h"

#if WITH_EDITORONLY_DATA
#include "EdGraph/EdGraph.h"
#include "EdGraph/EdGraphNode.h"
#endif // WITH_EDITORONLY_DATA

UDungeonThemeAsset::UDungeonThemeAsset(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer) {
#if WITH_EDITORONLY_DATA
	PreviewViewportProperties = ObjectInitializer.CreateDefaultSubobject<UDungeonEditorViewportProperties>(this, "PreviewProperties");
#endif	// WITH_EDITORONLY_DATA
}

void UDungeonThemeAsset::AddReferencedObjects(UObject* InThis, FReferenceCollector& Collector)
{
	UDungeonThemeAsset* This = CastChecked<UDungeonThemeAsset>(InThis);

#if WITH_EDITORONLY_DATA
	Collector.AddReferencedObject(This->UpdateGraph, This);
#endif	// WITH_EDITORONLY_DATA

	Super::AddReferencedObjects(This, Collector);
}
