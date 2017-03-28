//$ Copyright 2015 Ali Akbar, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//
#include "DungeonArchitectRuntimePrivatePCH.h"
#include "DungeonSpatialConstraint.h"


UDungeonSpatialConstraint::UDungeonSpatialConstraint(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	bApplyBaseRotation = false;
	bRotateToFitConstraint = true;
}
