//$ Copyright 2015 Ali Akbar, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//
#include "DungeonArchitectEditorPrivatePCH.h"
#include "DungeonEdMode.h"
#include "UnrealEd.h"
#include "StaticMeshResources.h"
#include "ObjectTools.h"
#include "ScopedTransaction.h"

#include "ModuleManager.h"
#include "Editor/LevelEditor/Public/LevelEditor.h"
#include "Toolkits/ToolkitManager.h"

#include "Runtime/AssetRegistry/Public/AssetRegistryModule.h"

//Slate dependencies
#include "Editor/LevelEditor/Public/LevelEditor.h"
#include "Editor/LevelEditor/Public/SLevelViewport.h"
#include "ContentBrowserModule.h"
#include "DungeonEdModeToolkit.h"
#include "DungeonEdModeRenderer.h"
#include "IDungeonEdTool.h"
#include "DungeonEdModeHandler.h"
#include "Core/Utils/DungeonEditorUtils.h"
#include "Core/Utils/DungeonBuildingNotification.h"
#include "DungeonEdModeHandlerFactory.h"
#include "Dungeon.h"

#define LOCTEXT_NAMESPACE "DungeonDrawEdMode"
#define DungeonDraw_SNAP_TRACE (10000.f)
DEFINE_LOG_CATEGORY(DungeonDrawMode);

FEditorModeID FEdModeDungeon::EM_Dungeon(TEXT("EM_Dungeon"));


const float ROTATION_SPEED = 10;


/** Constructor */
FEdModeDungeon::FEdModeDungeon()
	: FEdMode()
	, ModeHandler(nullptr)
    , bToolActive(false)
{
}


/** Destructor */
FEdModeDungeon::~FEdModeDungeon()
{
	// Save UI settings to config file
	FEditorDelegates::MapChange.RemoveAll(this);
}


/** FGCObject interface */
void FEdModeDungeon::AddReferencedObjects(FReferenceCollector& Collector)
{
	// Call parent implementation
	FEdMode::AddReferencedObjects(Collector);
	Collector.AddReferencedObject(ModeHandler);
}

/** FEdMode: Called when the mode is entered */
void FEdModeDungeon::Enter()
{
	FEdMode::Enter();

	DungeonBuildingNotification = MakeShareable(new FDungeonBuildingNotification);


	// Clear any selection 
	GEditor->SelectNone(false, true);

	// Bind to editor callbacks
	FEditorDelegates::NewCurrentLevel.AddSP(this, &FEdModeDungeon::NotifyNewCurrentLevel);

	// Force real-time viewports.  We'll back up the current viewport state so we can restore it when the
	// user exits this mode.
	const bool bWantRealTime = true;
	const bool bRememberCurrentState = true;
	ForceRealTimeViewports(bWantRealTime, bRememberCurrentState);
	
	ADungeon* SelectedDungeon = FDungeonEditorUtils::GetDungeonActorFromLevelViewport();
	RecreateHandler(SelectedDungeon);

	RecreateToolkit();

	GEditor->RegisterForUndo(this);
}

void FEdModeDungeon::RecreateToolkit()
{

	if (Toolkit.IsValid())
	{
		FToolkitManager::Get().CloseToolkit(Toolkit.ToSharedRef());
		Toolkit.Reset();
	}

	// Recreate the UI widget
	TSharedPtr<SWidget> ToolkitWidget;
	if (ModeHandler) {
		ToolkitWidget = ModeHandler->CreateToolkitWidget();
	}
	else {
		ToolkitWidget = SNullWidget::NullWidget;
	}
	
	TSharedPtr<FDungeonEdModeToolkit> DungeonToolkit = MakeShareable(new FDungeonEdModeToolkit);
	DungeonToolkit->SetInlineContent(ToolkitWidget);

	Toolkit = DungeonToolkit;
	Toolkit->Init(Owner->GetToolkitHost());
}

/** FEdMode: Called when the mode is exited */
void FEdModeDungeon::Exit()
{
	if (ModeHandler) {
		ModeHandler->Exit();
	}

	if (Toolkit.IsValid()) {
		FToolkitManager::Get().CloseToolkit(Toolkit.ToSharedRef());
		Toolkit.Reset();
	}
	//
	FEditorDelegates::NewCurrentLevel.RemoveAll(this);

	// Restore real-time viewport state if we changed it
	const bool bWantRealTime = false;
	const bool bRememberCurrentState = false;
	ForceRealTimeViewports(bWantRealTime, bRememberCurrentState);

	DungeonBuildingNotification = nullptr;

	// Call base Exit method to ensure proper cleanup
	FEdMode::Exit();

	GEditor->UnregisterForUndo(this);
}


void FEdModeDungeon::RecreateHandler(ADungeon* SelectedDungeon)
{

	if (SelectedDungeon) {
		if (ModeHandler) {
			ModeHandler->Exit();
			ModeHandler = nullptr;
		}

		UDungeonBuilder* Builder = SelectedDungeon->GetBuilder();
		if (Builder) {
			ModeHandler = FDungeonEdModeHandlerFactory::Get()->CreateHandler(Builder->GetClass());
			if (ModeHandler) {
				ModeHandler->Enter();
			}
		}
	}
}

void FEdModeDungeon::PostUndo(bool bSuccess)
{
	if (ModeHandler) {
		ModeHandler->Undo();
	}
}

void FEdModeDungeon::PostRedo(bool bSuccess)
{
	if (ModeHandler) {
		ModeHandler->Redo();
	}
}

/** When the user changes the active streaming level with the level browser */
void FEdModeDungeon::NotifyNewCurrentLevel()
{

}

/** When the user changes the current tool in the UI */
void FEdModeDungeon::NotifyToolChanged()
{

}

bool FEdModeDungeon::DisallowMouseDeltaTracking() const
{
	// We never want to use the mouse delta tracker while painting
	return bToolActive;
}

/** FEdMode: Called once per frame */
void FEdModeDungeon::Tick(FEditorViewportClient* ViewportClient, float DeltaTime)
{
	if (bToolActive)
	{
		ApplyBrush(ViewportClient);
	}

	FEdMode::Tick(ViewportClient, DeltaTime);
}

FVector PerformLocationSnap(const FVector& Location) {
	//ULevelEditorViewportSettings* ViewportSettings = GetMutableDefault<ULevelEditorViewportSettings>();
	//int32 SnapWidth = ViewportSettings->GridEnabled;
	int32 SnapWidth = GEditor->GetGridSize();
	if (SnapWidth <= 0) {
		return Location;
	}
	float X = Location.X / SnapWidth;
	float Y = Location.Y / SnapWidth;
	float Z = Location.Z / SnapWidth;

	X = FMath::RoundToInt(X) * SnapWidth;
	Y = FMath::RoundToInt(Y) * SnapWidth;
	Z = FMath::RoundToInt(Z) * SnapWidth;
	return FVector(X, Y, Z);
}

float SnapRotation(float Value, float SnapWidth) {
	return FMath::RoundToInt(Value / SnapWidth) * SnapWidth;
}

/**
 * Called when the mouse is moved over the viewport
 *
 * @param	InViewportClient	Level editor viewport client that captured the mouse input
 * @param	InViewport			Viewport that captured the mouse input
 * @param	InMouseX			New mouse cursor X coordinate
 * @param	InMouseY			New mouse cursor Y coordinate
 *
 * @return	true if input was handled
 */
bool FEdModeDungeon::MouseMove(FEditorViewportClient* ViewportClient, FViewport* Viewport, int32 MouseX, int32 MouseY)
{
	return false;
}

/**
 * Called when the mouse is moved while a window input capture is in effect
 *
 * @param	InViewportClient	Level editor viewport client that captured the mouse input
 * @param	InViewport			Viewport that captured the mouse input
 * @param	InMouseX			New mouse cursor X coordinate
 * @param	InMouseY			New mouse cursor Y coordinate
 *
 * @return	true if input was handled
 */
bool FEdModeDungeon::CapturedMouseMove(FEditorViewportClient* ViewportClient, FViewport* Viewport, int32 MouseX, int32 MouseY)
{
	return false;
}

void FEdModeDungeon::ApplyBrush(FEditorViewportClient* ViewportClient)
{
	if (ModeHandler) {
		ModeHandler->ApplyBrush(ViewportClient);
	}
}

UDungeonEdModeHandler* FEdModeDungeon::GetHandler() const
{
	return ModeHandler;
}

/** FEdMode: Called when a key is pressed */
bool FEdModeDungeon::InputKey(FEditorViewportClient* ViewportClient, FViewport* Viewport, FKey Key, EInputEvent Event)
{
	if (Key == EKeys::LeftMouseButton) {
		bToolActive = (Event == EInputEvent::IE_Pressed);
	}

	bool bKeyProcessed = false;
	if (ModeHandler) {
		bKeyProcessed = ModeHandler->InputKey(ViewportClient, Viewport, Key, Event);
	}
	return bKeyProcessed;
}

/** FEdMode: Render the DungeonDraw edit mode */
void FEdModeDungeon::Render(const FSceneView* View, FViewport* Viewport, FPrimitiveDrawInterface* PDI)
{
	/** Call parent implementation */
	FEdMode::Render(View, Viewport, PDI);

	if (!IsRunningCommandlet() && ModeHandler) {
		ModeHandler->UpdateFrame(View, Viewport, PDI);
	}
}


/** FEdMode: Render HUD elements for this tool */
void FEdModeDungeon::DrawHUD(FEditorViewportClient* ViewportClient, FViewport* Viewport, const FSceneView* View, FCanvas* Canvas)
{
}

/** FEdMode: Check to see if an actor can be selected in this mode - no side effects */
bool FEdModeDungeon::IsSelectionAllowed(AActor* InActor, bool bInSelection) const
{
	if (InActor->IsA(ADungeon::StaticClass())) {
		return true;
	}
	return false;
}

/** FEdMode: Handling SelectActor */
bool FEdModeDungeon::Select(AActor* InActor, bool bInSelected)
{
	return false;
}

/** FEdMode: Called when the currently selected actor has changed */
void FEdModeDungeon::ActorSelectionChangeNotify()
{
	bool bRequiresUpdate = false;
	ADungeon* SelectedDungeon = nullptr;

	for (FSelectionIterator SelectionIt(*Owner->GetSelectedActors()); SelectionIt; ++SelectionIt)
	{
		AActor* CurActor = CastChecked<AActor>(*SelectionIt);

		if (CurActor->bHidden || !CurActor->IsSelected())
		{
			continue;
		}

		if (CurActor->IsA(ADungeon::StaticClass()) && ModeHandler) {
			SelectedDungeon = Cast<ADungeon>(CurActor);
			if (ModeHandler->GetActiveDungeon() != SelectedDungeon) {
				bRequiresUpdate = true;
				break;
			}
		}
	}

	if (bRequiresUpdate) {
		ModeHandler->OnDungeonSelectionChanged(SelectedDungeon);

		RecreateHandler(SelectedDungeon);
		RecreateToolkit();
	}
}


/** Forces real-time perspective viewports */
void FEdModeDungeon::ForceRealTimeViewports(const bool bEnable, const bool bStoreCurrentState)
{
	FLevelEditorModule& LevelEditorModule = FModuleManager::GetModuleChecked<FLevelEditorModule>("LevelEditor");
	TSharedPtr< ILevelViewport > ViewportWindow = LevelEditorModule.GetFirstActiveViewport();
	if (ViewportWindow.IsValid())
	{
		FEditorViewportClient &Viewport = ViewportWindow->GetLevelViewportClient();
		if (Viewport.IsPerspective())
		{
			if (bEnable)
			{
				Viewport.SetRealtime(bEnable, bStoreCurrentState);
			}
			else
			{
				const bool bAllowDisable = true;
				Viewport.RestoreRealtime(bAllowDisable);
			}

		}
	}
}

bool FEdModeDungeon::HandleClick(FEditorViewportClient* InViewportClient, HHitProxy *HitProxy, const FViewportClick &Click)
{
	if (Click.GetKey() == EKeys::LeftMouseButton) {
		ApplyBrush(InViewportClient);
	}

	return FEdMode::HandleClick(InViewportClient, HitProxy, Click);
}

FVector FEdModeDungeon::GetWidgetLocation() const
{
	return FEdMode::GetWidgetLocation();
}

/** FEdMode: Called when a mouse button is pressed */
bool FEdModeDungeon::StartTracking(FEditorViewportClient* InViewportClient, FViewport* InViewport)
{
	return FEdMode::StartTracking(InViewportClient, InViewport);
}

/** FEdMode: Called when the a mouse button is released */
bool FEdModeDungeon::EndTracking(FEditorViewportClient* InViewportClient, FViewport* InViewport)
{
	return FEdMode::EndTracking(InViewportClient, InViewport);
}

/** FEdMode: Called when mouse drag input it applied */
bool FEdModeDungeon::InputDelta(FEditorViewportClient* InViewportClient, FViewport* InViewport, FVector& InDrag, FRotator& InRot, FVector& InScale)
{
	if (bToolActive) {
		return true;
	}

	return FEdMode::InputDelta(InViewportClient, InViewport, InDrag, InRot, InScale);
}

bool FEdModeDungeon::AllowWidgetMove()
{
	return ShouldDrawWidget();
}

bool FEdModeDungeon::UsesTransformWidget() const
{
	return ShouldDrawWidget();
}

bool FEdModeDungeon::ShouldDrawWidget() const
{
	return true;
}

EAxisList::Type FEdModeDungeon::GetWidgetAxisToDraw(FWidget::EWidgetMode InWidgetMode) const
{
	switch (InWidgetMode)
	{
	case FWidget::WM_Translate:
	case FWidget::WM_Rotate:
	case FWidget::WM_Scale:
		return EAxisList::XYZ;
	default:
		return EAxisList::None;
	}
}


#undef LOCTEXT_NAMESPACE