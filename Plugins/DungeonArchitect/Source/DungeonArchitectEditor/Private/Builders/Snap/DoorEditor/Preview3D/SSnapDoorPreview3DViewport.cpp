//$ Copyright 2015 Ali Akbar, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//
#include "DungeonArchitectEditorPrivatePCH.h"
#include "SSnapDoorPreview3DViewport.h"
#include "EditorViewportClient.h"
#include "SnapDoorPreview3DViewportClient.h"
#include "SSnapDoorPreview3DViewportToolbar.h"
#include "SnapDoor.h"

#include "SDockTab.h"

#define LOCTEXT_NAMESPACE "SSnapDoorPreview3DViewport"


void SSnapDoorPreview3DViewport::Construct(const FArguments& InArgs)
{
	LAThemeEditorPtr = InArgs._SnapDoorEditor;
	DoorAsset = InArgs._DoorAsset;

	PreviewScene = MakeShareable(new FPreviewScene);
	SEditorViewport::Construct(SEditorViewport::FArguments());
	//ObjectToEdit->PreviewViewportProperties->PropertyChangeListener = SharedThis(this);

	UWorld* World = PreviewScene->GetWorld();

	Skylight = NewObject<USkyLightComponent>();
	PreviewScene->AddComponent(Skylight, FTransform::Identity);
	
	AtmosphericFog = NewObject<UAtmosphericFogComponent>();
	PreviewScene->AddComponent(AtmosphericFog, FTransform::Identity);

	PreviewScene->DirectionalLight->SetMobility(EComponentMobility::Movable);
	PreviewScene->DirectionalLight->CastShadows = true;
	PreviewScene->DirectionalLight->CastStaticShadows = true;
	PreviewScene->DirectionalLight->CastDynamicShadows = true;

	SpawnDoorActor();
}



SSnapDoorPreview3DViewport::~SSnapDoorPreview3DViewport()
{
	FCoreUObjectDelegates::OnObjectPropertyChanged.RemoveAll(this);
	if (EditorViewportClient.IsValid())
	{
		EditorViewportClient->Viewport = NULL;
	}
}

void SSnapDoorPreview3DViewport::AddReferencedObjects(FReferenceCollector& Collector)
{
	Collector.AddReferencedObject(DoorAsset);
}


void SSnapDoorPreview3DViewport::OnToggleDebugData()
{

}

void SSnapDoorPreview3DViewport::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	SEditorViewport::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);

}

EVisibility SSnapDoorPreview3DViewport::GetToolbarVisibility() const
{
	return EVisibility::Visible;
}

TSharedRef<FEditorViewportClient> SSnapDoorPreview3DViewport::MakeEditorViewportClient()
{
	EditorViewportClient = MakeShareable(new FSnapDoorPreview3DViewportClient(LAThemeEditorPtr, SharedThis(this), *PreviewScene, DoorAsset));

	EditorViewportClient->bSetListenerPosition = false;
	EditorViewportClient->SetRealtime(true);	// TODO: Check if real-time is needed
	EditorViewportClient->VisibilityDelegate.BindSP(this, &SSnapDoorPreview3DViewport::IsVisible);

	return EditorViewportClient.ToSharedRef();
}

EVisibility SSnapDoorPreview3DViewport::OnGetViewportContentVisibility() const
{
	return EVisibility::Visible;
}

void SSnapDoorPreview3DViewport::BindCommands()
{
	SEditorViewport::BindCommands();

	const FSnapDoorEditorViewportCommands& ViewportActions = FSnapDoorEditorViewportCommands::Get();
	CommandList->MapAction(
		ViewportActions.ToggleDebugData,
		FExecuteAction::CreateSP(this, &SSnapDoorPreview3DViewport::OnToggleDebugData));

}

void SSnapDoorPreview3DViewport::OnFocusViewportToSelection()
{
	SEditorViewport::OnFocusViewportToSelection();
}

TSharedPtr<SWidget> SSnapDoorPreview3DViewport::MakeViewportToolbar()
{
	// Build our toolbar level toolbar
	TSharedRef< SSnapDoorPreview3DViewportToolbar > ToolBar =
		SNew(SSnapDoorPreview3DViewportToolbar)
		.Viewport(SharedThis(this))
		.Visibility(this, &SSnapDoorPreview3DViewport::GetToolbarVisibility)
		.IsEnabled(FSlateApplication::Get().GetNormalExecutionAttribute());

	return
		SNew(SVerticalBox)
		.Visibility(EVisibility::SelfHitTestInvisible)
		+ SVerticalBox::Slot()
		.AutoHeight()
		.VAlign(VAlign_Top)
		[
			ToolBar
		];
}

bool SSnapDoorPreview3DViewport::IsVisible() const
{
	return ViewportWidget.IsValid() && (!ParentTab.IsValid() || ParentTab.Pin()->IsForeground());
}

void SSnapDoorPreview3DViewport::SpawnDoorActor()
{
	UWorld* World = PreviewScene->GetWorld();
	Door = World->SpawnActor<ASnapDoor>(FVector::ZeroVector, FQuat::Identity.Rotator());
}


UWorld* SSnapDoorPreview3DViewport::GetWorld() const
{
	return PreviewScene->GetWorld();
}

#undef LOCTEXT_NAMESPACE
