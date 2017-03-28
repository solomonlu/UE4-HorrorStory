//$ Copyright 2015 Ali Akbar, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//
#include "DungeonArchitectRuntimePrivatePCH.h"
#include "SnapDungeonModel.h"


USnapDungeonModel::USnapDungeonModel(const FObjectInitializer& ObjectInitializer) \
	: Super(ObjectInitializer) 
	, LayoutRootNode(nullptr)
{
}

void USnapDungeonModel::Cleanup()
{
	SnapModules.Reset();
	StartSnapModules.Reset();
	EndSnapModules.Reset();
}

void USnapDungeonModel::Reset()
{
	Cleanup();
	LayoutRootNode = nullptr;
}
