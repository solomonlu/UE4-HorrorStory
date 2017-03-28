// Copyright 2015-2016 Code Respawn Technologies. MIT License
#pragma once

#include "Toolkits/BaseToolkit.h"

class SWidget;
class FEdMode;


class SSnapEditor : public SCompoundWidget {
public:
	SLATE_BEGIN_ARGS(SSnapEditor) {}
	SLATE_END_ARGS()


public:
	/** SCompoundWidget functions */
	void Construct(const FArguments& InArgs);
	
	void SetSettingsObject(UObject* Object, bool bForceRefresh = false);

private:
	TSharedPtr<class IDetailsView> DetailsPanel;
};
