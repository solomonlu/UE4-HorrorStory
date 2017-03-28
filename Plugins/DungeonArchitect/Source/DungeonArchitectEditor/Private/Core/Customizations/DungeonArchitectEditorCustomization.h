//$ Copyright 2015 Ali Akbar, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//


#pragma once

#include "IDetailCustomization.h"
#include "IPropertyChangeListener.h"

DECLARE_LOG_CATEGORY_EXTERN(LogDungeonCustomization, Log, All);

class FDungeonArchitectEditorCustomization : public IDetailCustomization {
public:
	// IDetailCustomization interface
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;
	// End of IDetailCustomization interface

	static TSharedRef<IDetailCustomization> MakeInstance();

	static FReply RebuildDungeon(IDetailLayoutBuilder* DetailBuilder);
	static FReply DestroyDungeon(IDetailLayoutBuilder* DetailBuilder);
	static FReply RandomizeSeed(IDetailLayoutBuilder* DetailBuilder);
};

class FDungeonArchitectMeshNodeCustomization : public IDetailCustomization {
public:
	// IDetailCustomization interface
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;
	// End of IDetailCustomization interface

	static FReply EditAdvancedOptions(IDetailLayoutBuilder* DetailBuilder);
};


class FDungeonEditorViewportPropertiesCustomization : public IDetailCustomization {
public:
	// IDetailCustomization interface
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;
	// End of IDetailCustomization interface

	static TSharedRef<IDetailCustomization> MakeInstance();
};

class FDungeonArchitectVolumeCustomization : public IDetailCustomization {
public:
	// IDetailCustomization interface
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;
	// End of IDetailCustomization interface
	
	static TSharedRef<IDetailCustomization> MakeInstance();

	static FReply RebuildDungeon(IDetailLayoutBuilder* DetailBuilder);
};


class FDungeonPropertyChangeListener : public TSharedFromThis<FDungeonPropertyChangeListener> {
public:
	void Initialize();
	void OnPropertyChanged(UObject* Object, struct FPropertyChangedEvent& Event);

private:
	TSharedPtr<IPropertyChangeListener> PropertyChangeListener;
};


class FDungeonDebugCustomization : public IDetailCustomization {
public:
	// IDetailCustomization interface
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;
	// End of IDetailCustomization interface

	static TSharedRef<IDetailCustomization> MakeInstance();

	static FReply ExecuteCommand(IDetailLayoutBuilder* DetailBuilder, int32 CommandID);
};
