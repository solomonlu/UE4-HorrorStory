//$ Copyright 2015 Ali Akbar, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//
#pragma once
#include "Core/EditorMode/IDungeonEdTool.h"
#include "SnapDoor.h"
#include "SnapDoorEditor.h"


class USnapDoorAsset;

class SSnapDoorEdit : public SCompoundWidget, public FGCObject
{
public:
	SLATE_BEGIN_ARGS(SSnapDoorEdit) {}
		SLATE_ARGUMENT(TWeakPtr<FSnapDoorEditor>, SnapDoorEditor)
		SLATE_ARGUMENT(USnapDoorAsset*, ObjectToEdit)
	SLATE_END_ARGS()

public:
	/** SCompoundWidget functions */
	void Construct(const FArguments& InArgs);

	~SSnapDoorEdit();

	// SWidget interface
	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;
	// End of SWidget interface

	// FGCObject interface
	virtual void AddReferencedObjects(FReferenceCollector& Collector) override;
	// End of FGCObject interface

	void OnPropertyChanged(UObject* ObjectBeingModified, FPropertyChangedEvent& PropertyChangedEvent);

	FORCEINLINE ESnapDoorMode GetDoorVisualMode() const { return ActiveMode; }

private:
	/** Clears all the tools selection by setting them to false. */
	void ClearAllToolSelection();

	/** Binds UI commands for the toolbar. */
	void BindCommands();

	void OnModeSelectedDoor();
	void OnModeSelectedClosed();

	bool IsModeActive_Door();
	bool IsModeActive_Closed();

	void SwitchVisualMode(ESnapDoorMode Mode);
	void RefreshDetailsPanel(bool bForceRefresh);

	TSharedRef<SWidget> BuildToolBar();

private:
	/** Command list for binding functions for the toolbar. */
	TSharedPtr<FUICommandList> UICommandList;
	ESnapDoorMode ActiveMode;
	TSharedPtr<class IDetailsView> DetailsPanel;

	TWeakPtr<FSnapDoorEditor> DoorEditorPtr;
	USnapDoorAsset* DoorAsset;
};

