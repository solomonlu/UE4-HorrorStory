//$ Copyright 2015 Ali Akbar, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//
#include "DungeonArchitectEditorPrivatePCH.h"
#include "SnapEditorCustomization.h"
#include "Core/Utils/DungeonEditorUtils.h"
#include "SnapDoorMeshInfo.h"
#include "DetailLayoutBuilder.h"
#include "PropertyHandle.h"

void FSnapDoorMeshInfoCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	USnapDoorMeshInfo* MeshInfo = FDungeonEditorUtils::GetBuilderObject<USnapDoorMeshInfo>(&DetailBuilder);
	if (!MeshInfo) {
		return;
	}
	
	// Show only the selected mesh type property
	FName PropertyToHide = (MeshInfo->MeshType == EUSnapDoorMeshType::StaticMesh) ? "BlueprintClass" : "MeshComponent";
	TSharedRef<IPropertyHandle> PropertyHandle = DetailBuilder.GetProperty(PropertyToHide);
	PropertyHandle->MarkHiddenByCustomization();
}

TSharedRef<IDetailCustomization> FSnapDoorMeshInfoCustomization::MakeInstance()
{
	return MakeShareable(new FSnapDoorMeshInfoCustomization);
}
