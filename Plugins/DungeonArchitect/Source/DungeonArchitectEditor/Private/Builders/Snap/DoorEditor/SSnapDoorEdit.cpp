//$ Copyright 2015 Ali Akbar, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//
#include "DungeonArchitectEditorPrivatePCH.h"
#include "SSnapDoorEdit.h"
#include "PropertyEditorModule.h"
#include "SnapDoorEditorCommands.h"
#include "IDetailsView.h"
#include "SnapDoorMeshInfo.h"
#include "SnapDoorAsset.h"

void SSnapDoorEdit::Construct(const FArguments& InArgs)
{
	DoorEditorPtr = InArgs._SnapDoorEditor;
	DoorAsset = InArgs._ObjectToEdit;

	// Everything (or almost) uses this padding, change it to expand the padding.
	FMargin StandardPadding(0.0f, 4.0f, 0.0f, 4.0f);

	UICommandList = MakeShareable(new FUICommandList);

	BindCommands();

	FPropertyEditorModule& PropertyEditorModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	FDetailsViewArgs DetailsViewArgs(false, false, true, FDetailsViewArgs::HideNameArea);

	DetailsPanel = PropertyEditorModule.CreateDetailView(DetailsViewArgs);

	TSharedRef<SWidget> DetailsPanelRef = DetailsPanel.ToSharedRef();

	this->ChildSlot
	[
		SNew(SOverlay)
		+ SOverlay::Slot()
		[
			SNew(SScrollBox)
			+ SScrollBox::Slot()
			.Padding(2.0f, 0)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(StandardPadding)
				[
					BuildToolBar()
				]

				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					DetailsPanelRef
				]
			]
		]
	];

	SwitchVisualMode(ESnapDoorMode::Door);

	// Listen for property changes
	FCoreUObjectDelegates::OnObjectPropertyChanged.AddRaw(this, &SSnapDoorEdit::OnPropertyChanged);
}

SSnapDoorEdit::~SSnapDoorEdit()
{
	FCoreUObjectDelegates::OnObjectPropertyChanged.RemoveAll(this);
}

TSharedRef<SWidget> SSnapDoorEdit::BuildToolBar()
{

	FToolBarBuilder Toolbar(UICommandList, FMultiBoxCustomization::None);
	{
		Toolbar.AddToolBarButton(FSnapDoorEditorCommands::Get().ModeDoor);
		Toolbar.AddToolBarButton(FSnapDoorEditorCommands::Get().ModeClosed);
	}

	return
		SNew(SHorizontalBox)

		+ SHorizontalBox::Slot()
		.Padding(4, 0)
		[
			SNew(SOverlay)
			+ SOverlay::Slot()
		[
			SNew(SBorder)
			.HAlign(HAlign_Center)
		.Padding(0)
		.BorderImage(FEditorStyle::GetBrush("NoBorder"))
		.IsEnabled(FSlateApplication::Get().GetNormalExecutionAttribute())
		[
			Toolbar.MakeWidget()
		]
		]
		];
}

void SSnapDoorEdit::ClearAllToolSelection()
{

}

void SSnapDoorEdit::BindCommands()
{
	const FSnapDoorEditorCommands& Commands = FSnapDoorEditorCommands::Get();

	UICommandList->MapAction(
		Commands.ModeDoor,
		FExecuteAction::CreateSP(this, &SSnapDoorEdit::OnModeSelectedDoor),
		FCanExecuteAction(),
		FIsActionChecked::CreateSP(this, &SSnapDoorEdit::IsModeActive_Door));

	UICommandList->MapAction(
		Commands.ModeClosed,
		FExecuteAction::CreateSP(this, &SSnapDoorEdit::OnModeSelectedClosed),
		FCanExecuteAction(),
		FIsActionChecked::CreateSP(this, &SSnapDoorEdit::IsModeActive_Closed));

}

void SSnapDoorEdit::OnModeSelectedDoor()
{
	SwitchVisualMode(ESnapDoorMode::Door);
}

void SSnapDoorEdit::OnModeSelectedClosed()
{
	SwitchVisualMode(ESnapDoorMode::Closed);
}

bool SSnapDoorEdit::IsModeActive_Door()
{
	return ActiveMode == ESnapDoorMode::Door;
}

bool SSnapDoorEdit::IsModeActive_Closed()
{
	return ActiveMode == ESnapDoorMode::Closed;
}


void SSnapDoorEdit::SwitchVisualMode(ESnapDoorMode Mode)
{
	ActiveMode = Mode;
	RefreshDetailsPanel(false);

	// Rebuild the door
	DoorEditorPtr.Pin()->RebuildDoor();
}

void SSnapDoorEdit::RefreshDetailsPanel(bool bForceRefresh)
{
	// Set the mesh info properties in the details panel
	if (DoorAsset) {
		UObject* Properties = (ActiveMode == ESnapDoorMode::Door) ? DoorAsset->DoorMesh : DoorAsset->ClosedMesh;
		DetailsPanel->SetObject(Properties, bForceRefresh);
	}
}

void SSnapDoorEdit::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
}

void SSnapDoorEdit::AddReferencedObjects(FReferenceCollector& Collector)
{
	Collector.AddReferencedObject(DoorAsset);
}

void SSnapDoorEdit::OnPropertyChanged(UObject* ObjectBeingModified, FPropertyChangedEvent& PropertyChangedEvent)
{
	if (!ensure(ObjectBeingModified))
	{
		return;
	}

	if (ObjectBeingModified->IsA<USnapDoorMeshInfo>() && PropertyChangedEvent.Property && PropertyChangedEvent.Property->GetName() == "MeshType") {
		RefreshDetailsPanel(true);
	}

	// Notify the host editor
	if (DoorEditorPtr.IsValid()) {
		DoorEditorPtr.Pin()->OnPropertyChanged(ObjectBeingModified, PropertyChangedEvent);
	}
}
