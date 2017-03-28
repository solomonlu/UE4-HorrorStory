//$ Copyright 2015 Ali Akbar, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "DungeonArchitectRuntimePrivatePCH.h"
#include "DungeonParticleSystem.h"

ADungeonParticleSystem::ADungeonParticleSystem(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer), ParticleSystem(NULL)
{
	SceneRoot = ObjectInitializer.CreateDefaultSubobject<USceneComponent>(this, "SceneRoot");
	SceneRoot->SetMobility(EComponentMobility::Static);
	RootComponent = SceneRoot;
}

void ADungeonParticleSystem::SetParticleComponentFromTemplate(UParticleSystemComponent* Template)
{
	if (ParticleSystem) {
		ParticleSystem->DetachFromComponent(FDetachmentTransformRules(EDetachmentRule::KeepRelative, false));
		ParticleSystem->DestroyComponent();
	}
	ParticleSystem = NewObject<UParticleSystemComponent>(this, TEXT("ParticleSystem"), RF_NoFlags, Template);
	ParticleSystem->SetMobility(EComponentMobility::Stationary);
	ParticleSystem->SetupAttachment(SceneRoot);

	ReregisterAllComponents();
}
