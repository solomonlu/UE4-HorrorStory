//$ Copyright 2015 Ali Akbar, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//
#include "DungeonArchitectEditorPrivatePCH.h"
#include "SnapDoorPreview3DViewportClient.h"

namespace SnapDoorPreview3DConstants {
	static const float GridSize = 2048.0f;
	static const int32 CellSize = 16;
}

FSnapDoorPreview3DViewportClient::FSnapDoorPreview3DViewportClient(TWeakPtr<ISnapDoorEditor> pInLAThemeEditor, TWeakPtr<SSnapDoorPreview3DViewport> pInLAThemeEditorViewport, FPreviewScene& pInPreviewScene, USnapDoorAsset* pInProp)
: FEditorViewportClient(nullptr, &pInPreviewScene),
		InLAThemeEditor(pInLAThemeEditor),
		InLAThemeEditorViewport(pInLAThemeEditorViewport),
		InProp(pInProp)
{

	// Setup defaults for the common draw helper.
	DrawHelper.bDrawPivot = false;
	DrawHelper.bDrawWorldBox = false;
	DrawHelper.bDrawKillZ = false;
	DrawHelper.bDrawGrid = true;
	DrawHelper.GridColorAxis = FColor(160, 160, 160);
	DrawHelper.GridColorMajor = FColor(144, 144, 144);
	DrawHelper.GridColorMinor = FColor(128, 128, 128);
	DrawHelper.PerspectiveGridSize = SnapDoorPreview3DConstants::GridSize;
	DrawHelper.NumCells = DrawHelper.PerspectiveGridSize / (SnapDoorPreview3DConstants::CellSize * 2);

	SetViewMode(VMI_Lit);

	//EngineShowFlags.DisableAdvancedFeatures();
	EngineShowFlags.SetSnap(0);
	EngineShowFlags.CompositeEditorPrimitives = true;
	OverrideNearClipPlane(1.0f);
	bUsingOrbitCamera = true;

	// Set the initial camera position
	FRotator OrbitRotation(-40, 10, 0);
	SetCameraSetup(
		FVector::ZeroVector,
		OrbitRotation,
		FVector(0.0f, 100, 0.0f),
		FVector::ZeroVector,
		FVector(300, 400, 500),
		FRotator(-10, 0, 0)
		);
	SetViewLocation(FVector(500, 300, 500));
	//SetLookAtLocation(FVector(0, 0, 0));
}

void FSnapDoorPreview3DViewportClient::Tick(float DeltaSeconds)
{
	FEditorViewportClient::Tick(DeltaSeconds);

	// Tick the preview scene world.
	if (!GIntraFrameDebuggingGameThread)
	{
		PreviewScene->GetWorld()->Tick(LEVELTICK_All, DeltaSeconds);
	}
}
