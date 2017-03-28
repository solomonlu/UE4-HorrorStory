//$ Copyright 2015 Ali Akbar, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//
#include "DungeonArchitectRuntimePrivatePCH.h"
#include "SceneProviderCommand.h"
#include "Core/Dungeon.h"
#include "Core/Utils/DungeonModelHelper.h"
#include "IDungeonEditorService.h"
#include "SnapDoor.h"
#include "PhysicsEngine/DestructibleActor.h"



void SceneProviderCommand::AddNodeTag(AActor* Actor, const FName& NodeId) {
	if (Actor) {
		Actor->Tags.Add(CreateNodeTagFromId(NodeId));

		FName DungeonIdTag = UDungeonModelHelper::GetDungeonIdTag(Dungeon);
		Actor->Tags.Add(DungeonIdTag);
	}
}

void SceneProviderCommand::MoveToFolder(AActor* Actor)
{
#if WITH_EDITOR
	if (Actor && Dungeon) {
		Actor->SetFolderPath(Dungeon->ItemFolderPath);
	}
#endif
}

void SceneProviderCommand_CreateMesh::Execute(UWorld* World)
{
    AStaticMeshActor* MeshActor = World->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), transform.GetLocation(), FRotator(transform.GetRotation()));
	
    UStaticMeshComponent* MeshComponent = MeshActor->GetStaticMeshComponent();
    MeshComponent->StaticMesh = Mesh->StaticMesh;
    MeshActor->GetRootComponent()->SetRelativeScale3D(transform.GetScale3D());
    
    for (const FMaterialOverride& MaterialOverride : Mesh->MaterialOverrides) {
        MeshComponent->SetMaterial(MaterialOverride.index, MaterialOverride.Material);
    }
    
    SetMeshComponentAttributes(MeshComponent, Mesh->Template);
    MeshActor->MarkComponentsRenderStateDirty();
    
    AddNodeTag(MeshActor, NodeId);
	MoveToFolder(MeshActor);

    MeshActor->ReregisterAllComponents();
}

void SceneProviderCommand_CreateDestructibleMesh::Execute(UWorld* World)
{
	ADestructibleActor* Destructible = World->SpawnActor<ADestructibleActor>(ADestructibleActor::StaticClass(), transform.GetLocation(), FRotator(transform.GetRotation()));
	UDestructibleComponent* DestructibleComponent = Destructible->GetDestructibleComponent();
	DestructibleComponent->SetDestructibleMesh(DestructibleMesh);

	DestructibleComponent->SetNotifyRigidBodyCollision(true);
	DestructibleComponent->SetCollisionObjectType(ECC_Pawn);

	AddNodeTag(Destructible, NodeId);
	MoveToFolder(Destructible);

	Destructible->ReregisterAllComponents();
}

#define SET_MESH_ATTRIB(Attrib) StaticMeshComponent->Attrib = StaticMeshTemplate->Attrib
void SceneProviderCommand_CreateMesh::SetMeshComponentAttributes(UStaticMeshComponent* StaticMeshComponent, UStaticMeshComponent* StaticMeshTemplate)
{
	if (StaticMeshTemplate) {
		//	SET_MESH_ATTRIB(StaticMesh);	// This is set by the node's main property
		SET_MESH_ATTRIB(MinLOD);
		SET_MESH_ATTRIB(WireframeColorOverride);
		SET_MESH_ATTRIB(bIgnoreInstanceForTextureStreaming);
		SET_MESH_ATTRIB(OverriddenLightMapRes);
		SET_MESH_ATTRIB(StreamingDistanceMultiplier);
		SET_MESH_ATTRIB(MinDrawDistance);
		SET_MESH_ATTRIB(LDMaxDrawDistance);
		SET_MESH_ATTRIB(bAlwaysCreatePhysicsState);
		SET_MESH_ATTRIB(bGenerateOverlapEvents);
		SET_MESH_ATTRIB(bMultiBodyOverlap);
		SET_MESH_ATTRIB(bCheckAsyncSceneOnMove);
		SET_MESH_ATTRIB(bTraceComplexOnMove);
		SET_MESH_ATTRIB(bReturnMaterialOnMove);
		SET_MESH_ATTRIB(bAllowCullDistanceVolume);
		SET_MESH_ATTRIB(bRenderInMainPass);
		SET_MESH_ATTRIB(bReceivesDecals);
		SET_MESH_ATTRIB(bOwnerNoSee);
		SET_MESH_ATTRIB(bOnlyOwnerSee);
		SET_MESH_ATTRIB(bTreatAsBackgroundForOcclusion);
		SET_MESH_ATTRIB(bForceMipStreaming);
		SET_MESH_ATTRIB(CastShadow);
		SET_MESH_ATTRIB(bAffectDynamicIndirectLighting);
		SET_MESH_ATTRIB(bAffectDistanceFieldLighting);
		SET_MESH_ATTRIB(bCastDynamicShadow);
		SET_MESH_ATTRIB(bCastStaticShadow);
		SET_MESH_ATTRIB(bCastVolumetricTranslucentShadow);
		SET_MESH_ATTRIB(bSelfShadowOnly);
		SET_MESH_ATTRIB(bCastFarShadow);
		SET_MESH_ATTRIB(bCastInsetShadow);
		SET_MESH_ATTRIB(bCastCinematicShadow);
		SET_MESH_ATTRIB(bCastHiddenShadow);
		SET_MESH_ATTRIB(bCastShadowAsTwoSided);
		SET_MESH_ATTRIB(bLightAsIfStatic);
		SET_MESH_ATTRIB(bLightAttachmentsAsGroup);
		SET_MESH_ATTRIB(IndirectLightingCacheQuality);
		SET_MESH_ATTRIB(bRenderCustomDepth);
		SET_MESH_ATTRIB(CustomDepthStencilValue);

		SET_MESH_ATTRIB(TranslucencySortPriority);
		SET_MESH_ATTRIB(LpvBiasMultiplier);
		SET_MESH_ATTRIB(BoundsScale);
		SET_MESH_ATTRIB(CanCharacterStepUpOn);

		StaticMeshComponent->SetCanEverAffectNavigation(StaticMeshTemplate->CanEverAffectNavigation());
	}
	else {
		StaticMeshComponent->SetMobility(EComponentMobility::Static);
		StaticMeshComponent->SetCanEverAffectNavigation(true);
		StaticMeshComponent->bCastStaticShadow = true;
	}
}

void SceneProviderCommand_AddLight::Execute(UWorld* World)
{
    ALight* Light = NULL;
    UPointLightComponent* PointLightComponent = NULL;
    if (USpotLightComponent* SpotLightTemplate = Cast<USpotLightComponent>(LightTemplate)) {
		ASpotLight* SpotLight = World->SpawnActorDeferred<ASpotLight>(ASpotLight::StaticClass(), transform);
        USpotLightComponent* SpotLightComponent = SpotLight->SpotLightComponent;
        SetSpotLightAttributes(SpotLightComponent, SpotLightTemplate);
        
        Light = SpotLight;
        PointLightComponent = SpotLightComponent;
    }
    else {
		APointLight* PointLight = World->SpawnActorDeferred<APointLight>(APointLight::StaticClass(), transform);
        
        Light = PointLight;
        PointLightComponent = PointLight->PointLightComponent;
    }
    
	Light->SetMobility(EComponentMobility::Movable);
	Light->SetActorTransform(transform);
    Light->SetMobility(EComponentMobility::Stationary);
    
	AddNodeTag(Light, NodeId);
	MoveToFolder(Light);
    
    SetPointLightAttributes(PointLightComponent, LightTemplate);
    Light->FinishSpawning(transform, true);
    Light->ReregisterAllComponents();
}

#define SET_SPOT_ATTRIB(Attrib) SpotLightComponent->Attrib = SpotLightTemplate->Attrib
void SceneProviderCommand_AddLight::SetSpotLightAttributes(USpotLightComponent* SpotLightComponent, USpotLightComponent* SpotLightTemplate) {
	SET_SPOT_ATTRIB(InnerConeAngle);
	SET_SPOT_ATTRIB(LightShaftConeAngle);
	SET_SPOT_ATTRIB(OuterConeAngle);
}

#define SET_LIGHT_ATTRIB(Attrib) PointLightComponent->Attrib = LightTemplate->Attrib;
void SceneProviderCommand_AddLight::SetPointLightAttributes(UPointLightComponent* PointLightComponent, UPointLightComponent* LightTemplate) {
	// TODO: Find a better way to clone the properties of the template on to the light actor
	// UPointLightComponent Attributes
	SET_LIGHT_ATTRIB(AttenuationRadius);
	SET_LIGHT_ATTRIB(bUseInverseSquaredFalloff);
	SET_LIGHT_ATTRIB(LightFalloffExponent);
	SET_LIGHT_ATTRIB(LightmassSettings);
	SET_LIGHT_ATTRIB(SourceLength);
	SET_LIGHT_ATTRIB(SourceRadius);

	// ULightComponent Attributes
	SET_LIGHT_ATTRIB(bAffectDynamicIndirectLighting);
	SET_LIGHT_ATTRIB(bEnableLightShaftBloom);
	SET_LIGHT_ATTRIB(BloomScale);
	SET_LIGHT_ATTRIB(BloomThreshold);
	SET_LIGHT_ATTRIB(BloomTint);
	SET_LIGHT_ATTRIB(bUseIESBrightness);
	SET_LIGHT_ATTRIB(bUseRayTracedDistanceFieldShadows);
	SET_LIGHT_ATTRIB(CastTranslucentShadows);
	SET_LIGHT_ATTRIB(DisabledBrightness);
	SET_LIGHT_ATTRIB(IESBrightnessScale);
	SET_LIGHT_ATTRIB(LightFunctionFadeDistance);
	SET_LIGHT_ATTRIB(LightFunctionScale);
	SET_LIGHT_ATTRIB(MinRoughness);
	SET_LIGHT_ATTRIB(PreviewShadowMapChannel);
	SET_LIGHT_ATTRIB(ShadowBias);
	SET_LIGHT_ATTRIB(ShadowSharpen);

	// ULightComponentBase Attributes
	SET_LIGHT_ATTRIB(bAffectsWorld);
	SET_LIGHT_ATTRIB(bAffectTranslucentLighting);
	//SET_LIGHT_ATTRIB(bPrecomputedLightingIsValid);
	SET_LIGHT_ATTRIB(CastDynamicShadows);
	SET_LIGHT_ATTRIB(CastShadows);
	SET_LIGHT_ATTRIB(CastStaticShadows);
	SET_LIGHT_ATTRIB(IndirectLightingIntensity);
	SET_LIGHT_ATTRIB(Intensity);
	SET_LIGHT_ATTRIB(LightColor);
}


void SceneProviderCommand_AddParticleSystem::Execute(UWorld* World)
{
    AEmitter* ParticleEmitter = AddActor<AEmitter>(World, transform, NodeId);
    ParticleEmitter->GetParticleSystemComponent()->SetTemplate(ParticleTemplate);
}

void SetActorTransform(AActor* Actor, const FTransform& transform) {
	USceneComponent* SceneComponent = Actor->GetRootComponent();
	EComponentMobility::Type OriginialMobility = SceneComponent->Mobility;
	SceneComponent->SetMobility(EComponentMobility::Movable);
	SceneComponent->SetWorldTransform(transform);
	SceneComponent->UpdateChildTransforms();
	SceneComponent->SetMobility(OriginialMobility);
	SceneComponent->ReregisterComponent();
}

void SceneProviderCommand_AddActor::Execute(UWorld* World)
{
	AActor* Actor = World->SpawnActor(ClassTemplate, &transform);
	if (Actor) {
		SetActorTransform(Actor, transform);
		Actor->RerunConstructionScripts();
		AddNodeTag(Actor, NodeId);
		MoveToFolder(Actor);
	}
}

void SceneProviderCommand_CloneActor::Execute(UWorld* World)
{
	FActorSpawnParameters SpawnParams;
	SpawnParams.Template = ActorTemplate;
	SpawnParams.bAllowDuringConstructionScript = true;
	SpawnParams.OverrideLevel = World->GetLevel(0);
	AActor* Actor = World->SpawnActor<AActor>(ActorTemplate->GetClass(), SpawnParams);
	if (Actor) {
		if (Actor->IsA<ASnapDoor>()) {
			ASnapDoor* Door = Cast<ASnapDoor>(Actor);
			Door->BuildDoor();
		}

		SetActorTransform(Actor, transform);
		Actor->RerunConstructionScripts();
		AddNodeTag(Actor, NodeId);
		MoveToFolder(Actor);
	}
}

void SceneProviderCommand_SetActorTransform::Execute(UWorld* World)
{
	AStaticMeshActor* StaticMesh = Cast<AStaticMeshActor>(Actor);
	SetActorTransform(Actor, transform);
}

void SceneProviderCommand_SetParticleTemplate::Execute(UWorld* World)
{
	if (EmitterActor) {
		EmitterActor->SetTemplate(ParticleTemplate);
	}
}

void SceneProviderCommand_CreateGroupActor::Execute(UWorld* World)
{
	TSharedPtr<IDungeonEditorService> EditorService = IDungeonEditorService::Get();
	if (!EditorService.IsValid()) {
		// Editor service not specified. Cannot create editor specific functionality (e.g. when running as standalone game)
		return;
	}

	TArray<FName> GroupActorNodeTags;
	for (const FName& ActorNodeId : ActorNodeIds) {
		GroupActorNodeTags.Add(CreateNodeTagFromId(ActorNodeId));
	}

	TArray<AActor*> GroupedActors;
	for (TActorIterator<AActor> ActorIt(World); ActorIt; ++ActorIt)
	{
		AActor* Actor = *ActorIt;
		for (const FName& Tag : Actor->Tags) {
			if (GroupActorNodeTags.Contains(Tag)) {
				GroupedActors.Add(Actor);
				break;
			}
		}
	}
	
	AActor* GroupActor = EditorService->CreateGroupActor(World, GroupedActors, transform);
	if (GroupActor) {
		//SetActorTransform(Actor, transform);
		AddNodeTag(GroupActor, NodeId);
		MoveToFolder(GroupActor);
	}
}

void SceneProviderCommand_DestroyActorWithTag::Execute(UWorld* World)
{
	FName NodeTag = CreateNodeTagFromId(Tag);
	for (TActorIterator<AActor> ActorIt(World); ActorIt; ++ActorIt)
	{
		AActor* Actor = *ActorIt;
		if (Actor->Tags.Contains(NodeTag)) {
			Actor->Destroy();
			break;
		}
	}
}
