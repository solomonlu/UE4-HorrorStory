// Copyright 2015-2016 Code Respawn Technologies. MIT License
#include "DungeonArchitectEditorPrivatePCH.h"
#include "SSnapEditor.h"
#include "PropertyEditorModule.h"
#include "ModuleManager.h"
#include "IDetailsView.h"

#define LOCTEXT_NAMESPACE "SnapEditMode"

void SSnapEditor::Construct(const FArguments& InArgs)
{
	FPropertyEditorModule& PropertyEditorModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	FDetailsViewArgs DetailsViewArgs(false, false, false, FDetailsViewArgs::HideNameArea);

	DetailsPanel = PropertyEditorModule.CreateDetailView(DetailsViewArgs);

	this->ChildSlot
	[
		SNew(SVerticalBox)

		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			DetailsPanel.ToSharedRef()
		]
	];
}

void SSnapEditor::SetSettingsObject(UObject* Object, bool bForceRefresh /*= false*/)
{
	if (DetailsPanel.IsValid()) {
		DetailsPanel->SetObject(Object, bForceRefresh);
	}
}

#undef LOCTEXT_NAMESPACE