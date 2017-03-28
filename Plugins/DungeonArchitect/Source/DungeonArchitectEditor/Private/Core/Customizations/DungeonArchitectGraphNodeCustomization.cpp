//$ Copyright 2015 Ali Akbar, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//
#include "DungeonArchitectEditorPrivatePCH.h"
#include "DungeonArchitectGraphNodeCustomization.h"
#include "Core/DungeonArchitectEditor.h"
#include "Core/Graph/EdGraphNode_DungeonActorBase.h"
#include "Core/Graph/EdGraphNode_DungeonMesh.h"
#include "Core/Customizations/Details/DATransformDetails.h"

#include "PropertyEditing.h"
#include "SNotificationList.h"
#include "NotificationManager.h"
#include "EditorActorFolders.h"
#include "IDetailsView.h"
#include "AssetSelection.h"


#define LOCTEXT_NAMESPACE "DungeonArchitectEditorModule" 


void FDungeonArchitectVisualGraphNodeCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	CustomizeTransformWidget(DetailBuilder);
	CustomizeSelectorLogic(DetailBuilder);
	CustomizeTransformLogic(DetailBuilder);
}

void FDungeonArchitectVisualGraphNodeCustomization::CustomizeTransformWidget(IDetailLayoutBuilder& DetailBuilder)
{
	const FSelectedActorInfo& SelectedActorInfo = DetailBuilder.GetDetailsView().GetSelectedActorInfo();
	TSharedRef<FDATransformDetails> TransformDetails = MakeShareable(new FDATransformDetails(DetailBuilder.GetDetailsView().GetSelectedObjects(), SelectedActorInfo, DetailBuilder));
	IDetailCategoryBuilder& TransformCategory = DetailBuilder.EditCategory("OffsetCommon", LOCTEXT("OffsetCommonCategory", "Offset"), ECategoryPriority::Transform);
	TransformCategory.AddCustomBuilder(TransformDetails);
}

void FDungeonArchitectVisualGraphNodeCustomization::CustomizeSelectorLogic(IDetailLayoutBuilder& DetailBuilder)
{
	IDetailCategoryBuilder& DetailCategory = DetailBuilder.EditCategory("Selection Logic");
	TSharedRef<IPropertyHandle> MaterialProperty = DetailBuilder.GetProperty("Materials");
	/*
	DetailCategory.AddWidget()
		[
			SNew(SArrayProperty, MaterialProperty)
			// This delegate is called for each array element to generate a widget for it
			.OnGenerateArrayElementWidget(this, &FMeshComponentDetails::OnGenerateElementForMaterials)
		];
		*/
}

void FDungeonArchitectVisualGraphNodeCustomization::CustomizeTransformLogic(IDetailLayoutBuilder& DetailBuilder)
{

}

TSharedRef<IDetailCustomization> FDungeonArchitectVisualGraphNodeCustomization::MakeInstance()
{
	return MakeShareable(new FDungeonArchitectVisualGraphNodeCustomization);
}

TArray<UObject*> FDungeonArchitectVisualGraphNodeCustomization::GetSelectedObjects(IDetailLayoutBuilder& DetailBuilder)
{
	TArray<UObject*> Result;

	for (auto SelectedObject : DetailBuilder.GetDetailsView().GetSelectedObjects()) {
		Result.Add(SelectedObject.Get());
	}

	return Result;
}

#undef LOCTEXT_NAMESPACE