//$ Copyright 2015 Ali Akbar, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//
#pragma once
#include "SnapEdModeUISettings.generated.h"

UCLASS()
class USnapEdModeUISettings : public UObject {
	GENERATED_UCLASS_BODY()

public:

	/** Lets you emit your own markers into the scene */
	UPROPERTY(EditAnywhere, Category = Snap)
	bool bNeonVisuals;
};
