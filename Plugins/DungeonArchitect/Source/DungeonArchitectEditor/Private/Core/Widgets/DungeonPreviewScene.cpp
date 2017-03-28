//$ Copyright 2015 Ali Akbar, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//
#include "DungeonArchitectEditorPrivatePCH.h"
#include "DungeonPreviewScene.h"

FDungeonPreviewScene::FDungeonPreviewScene(ConstructionValues CVS) : FAdvancedPreviewScene(CVS)
{
	bForceAllUsedMipsResident = CVS.bForceMipsResident;
	PreviewWorld = NewObject<UWorld>();
	PreviewWorld->WorldType = EWorldType::Preview;
	if (CVS.bTransactional)
	{
		PreviewWorld->SetFlags(RF_Transactional);
	}

	FWorldContext& WorldContext = GEngine->CreateNewWorldContext(EWorldType::Preview);
	WorldContext.SetCurrentWorld(PreviewWorld);

	PreviewWorld->InitializeNewWorld(UWorld::InitializationValues()
										.AllowAudioPlayback(CVS.bAllowAudioPlayback)
										.CreatePhysicsScene(CVS.bCreatePhysicsScene)
										//.RequiresHitProxies(bCreateHitProxy)
										.CreateNavigation(false)
										.CreateAISystem(false)
										.ShouldSimulatePhysics(CVS.bShouldSimulatePhysics)
										.SetTransactional(CVS.bTransactional));
	PreviewWorld->InitializeActorsForPlay(FURL());

	//GetScene()->UpdateDynamicSkyLight(FLinearColor::White * CVS.SkyBrightness, FLinearColor::Black);
    //SetSkyBrightness(CVS.SkyBrightness);

	DirectionalLight = NewObject<UDirectionalLightComponent>(GetTransientPackage());
	DirectionalLight->Intensity = CVS.LightBrightness;
	DirectionalLight->LightColor = FColor::White;
	AddComponent(DirectionalLight, FTransform(CVS.LightRotation));

	LineBatcher = NewObject<ULineBatchComponent>(GetTransientPackage());
	AddComponent(LineBatcher, FTransform::Identity);
}
