//$ Copyright 2015 Ali Akbar, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//
#pragma once
#include "SGraphPalette.h"

class DUNGEONARCHITECTEDITOR_API SGraphPalette_PropActions : public SGraphPalette
{
public:
	SLATE_BEGIN_ARGS(SGraphPalette_PropActions){}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, TWeakPtr<class FDungeonArchitectEditor> InDungeonEditor);
	virtual void CollectAllActions(FGraphActionListBuilderBase& OutAllActions) override;
	virtual TSharedRef<SWidget> OnCreateWidgetForAction(FCreateWidgetForActionData* const InCreateData);
	virtual FReply OnActionDragged(const TArray< TSharedPtr<FEdGraphSchemaAction> >& InActions, const FPointerEvent& MouseEvent);

	void Refresh();

protected:
	TWeakPtr<FDungeonArchitectEditor> DungeonEditor;
};
