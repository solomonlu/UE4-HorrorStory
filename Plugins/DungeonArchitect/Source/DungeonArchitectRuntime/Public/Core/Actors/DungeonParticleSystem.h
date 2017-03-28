//$ Copyright 2015 Ali Akbar, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//


#pragma once
#include "DungeonParticleSystem.generated.h"

/**
*
*/
UCLASS()
class DUNGEONARCHITECTRUNTIME_API ADungeonParticleSystem : public AActor
{
	GENERATED_UCLASS_BODY()

public:
	void SetParticleComponentFromTemplate(UParticleSystemComponent* Template);

private:
	UParticleSystemComponent* ParticleSystem;
	USceneComponent* SceneRoot;
};
