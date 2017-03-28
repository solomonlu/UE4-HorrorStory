//$ Copyright 2015 Ali Akbar, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//
#include "DungeonArchitectRuntimePrivatePCH.h"
#include "SnapDungeonConfig.h"
#include "AI/Navigation/AbstractNavData.h"
#include "InstancedFoliageActor.h"
#include "SnapModuleInfo.h"

DEFINE_LOG_CATEGORY(SnapDungeonConfigLog);

void AddIgnoreClassByPath(TArray<UClass*>& OutIgnoredModuleActorType, const TCHAR* ClassToFind) {
	ConstructorHelpers::FClassFinder<AActor> ClassQuery(ClassToFind);
	if (ClassQuery.Succeeded()) {
		OutIgnoredModuleActorType.Add(ClassQuery.Class);
	}
}

USnapDungeonConfig::USnapDungeonConfig(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, MainBranchSize(30)
	, SideBranchSize(6)
	, SideBranchProbability(0.5f)
	, CollisionTestContraction(100)
	, MaxProcessingPower(1000000)
	, SafetyMaxModuleLimit(200)
{
	IgnoredModuleActorType.Add(APlayerStart::StaticClass());
	IgnoredModuleActorType.Add(AAtmosphericFog::StaticClass());
	IgnoredModuleActorType.Add(AExponentialHeightFog::StaticClass());
	IgnoredModuleActorType.Add(ASkyLight::StaticClass());
	IgnoredModuleActorType.Add(ADirectionalLight::StaticClass());
	IgnoredModuleActorType.Add(AWorldSettings::StaticClass());
	IgnoredModuleActorType.Add(ABrush::StaticClass());
	IgnoredModuleActorType.Add(AInstancedFoliageActor::StaticClass());
	IgnoredModuleActorType.Add(ANavigationData::StaticClass());
	IgnoredModuleActorType.Add(ADefaultPhysicsVolume::StaticClass());
	IgnoredModuleActorType.Add(ASnapModuleInfo::StaticClass());

	AddIgnoreClassByPath(IgnoredModuleActorType, TEXT("/Engine/EngineSky/BP_Sky_Sphere"));
}

