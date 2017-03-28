//$ Copyright 2015 Ali Akbar, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//
#include "DungeonArchitectRuntimePrivatePCH.h"
#include "DungeonBuilder.h"
#include "Dungeon.h"
#include "PreviewScene.h"
#include "Volumes/DungeonThemeOverrideVolume.h"
#include "Markers/DungeonMarkerEmitter.h"
#include "Volumes/DungeonMirrorVolume.h"
#include "SceneProviders/InstancedDungeonSceneProvider.h"
#include "SceneProviders/PooledDungeonSceneProvider.h"
#include "Builders/Grid/GridDungeonBuilder.h"


DEFINE_LOG_CATEGORY(DungeonBuilderLog);


void CreatePropLookup(UDungeonThemeAsset* PropAsset, PropBySocketTypeByTheme_t& PropBySocketTypeByTheme) {
	if (!PropAsset || PropBySocketTypeByTheme.Contains(PropAsset)) {
		// Lookup for this theme has already been built
		return;
	}

	PropBySocketTypeByTheme.Add(PropAsset, PropBySocketType_t());
	PropBySocketType_t& PropBySocketType = PropBySocketTypeByTheme[PropAsset];

	for (const FPropTypeData& Prop : PropAsset->Props) {
		if (!PropBySocketType.Contains(Prop.AttachToSocket)) {
			PropBySocketType.Add(Prop.AttachToSocket, TArray<FPropTypeData>());
		}
		PropBySocketType[Prop.AttachToSocket].Add(Prop);
	}
}

// Picks a theme from the list that has a definition for the defined socket
UDungeonThemeAsset* GetBestMatchedTheme(const FRandomStream& random, TArray<UDungeonThemeAsset*>& Themes, const FPropSocket& socket, PropBySocketTypeByTheme_t& PropBySocketTypeByTheme) {
	TArray<UDungeonThemeAsset*> ValidThemes;
	for (UDungeonThemeAsset* Theme : Themes) {
		if (PropBySocketTypeByTheme.Contains(Theme)) {
			PropBySocketType_t& PropBySocketType = PropBySocketTypeByTheme[Theme];
			if (PropBySocketType.Num() > 0) {
				if (PropBySocketType.Contains(socket.SocketType) && PropBySocketType[socket.SocketType].Num() > 0) {
					ValidThemes.Add(Theme);
				}
			}
		}
	}
	if (ValidThemes.Num() == 0) {
		return nullptr;
	}

	int32 index = FMath::FloorToInt(random.FRand() * ValidThemes.Num()) % ValidThemes.Num();
	return ValidThemes[index];
}

struct ThemeOverrideInfo {
	FRectangle Bounds;
	ADungeonThemeOverrideVolume* Volume;
};

void GenerateThemeOverrideList(UWorld* World, ADungeon* Dungeon, PropBySocketTypeByTheme_t& PropBySocketTypeByTheme, TArray<ThemeOverrideInfo>& OutOverrideList) {
	if (!World) return;

	OutOverrideList.Reset();
	if (World) {
		for (TObjectIterator<ADungeonThemeOverrideVolume> Volume; Volume; ++Volume)
		{
			if (Volume->IsPendingKill() || !Volume->IsValidLowLevel()) {
				continue;
			}
			bool valid;
			if (!Dungeon) {
				valid = true;
			}
			else {
				valid = (Volume->Dungeon == Dungeon);
			}
			if (valid && Volume->ThemeOverride) {
				FRectangle VolumeBounds;
				Volume->GetDungeonVolumeBounds(FVector(1, 1, 1), VolumeBounds);
				ThemeOverrideInfo Info;
				Info.Bounds = VolumeBounds;
				Info.Volume = *Volume;
				OutOverrideList.Add(Info);

				// Build a lookup of the theme for faster access later on
				CreatePropLookup(Volume->ThemeOverride, PropBySocketTypeByTheme);
			}
		}
	}
}


void UDungeonBuilder::BuildDungeon(ADungeon* pDungeon, UWorld* World)
{
	this->Dungeon = pDungeon;
	if (!Dungeon) {
		UE_LOG(DungeonBuilderLog, Log, TEXT("Cannot build dungeon due to invalid reference"));
		return;
	}

	BuildDungeon(Dungeon->GetModel(), Dungeon->GetConfig(), World);
}

void UDungeonBuilder::BuildDungeon(UDungeonModel* pModel, UDungeonConfig* pConfig, UWorld* World)
{
	this->model = pModel;
	this->config = pConfig;
	_SocketIdCounter = 0;
	nrandom.Init(config->Seed);
	random.Initialize(config->Seed);

	BuildDungeonImpl(World);
}

void UDungeonBuilder::BuildNonThemedDungeon(ADungeon* pDungeon, TSharedPtr<FDungeonSceneProvider>  SceneProvider, UWorld* World)
{
	this->Dungeon = pDungeon;
	if (!Dungeon) {
		UE_LOG(DungeonBuilderLog, Log, TEXT("Cannot build dungeon due to invalid reference"));
		return;
	}

	BuildNonThemedDungeon(Dungeon->GetModel(), Dungeon->GetConfig(), SceneProvider, World);
}

void UDungeonBuilder::BuildNonThemedDungeon(UDungeonModel* pModel, UDungeonConfig* pConfig, TSharedPtr<FDungeonSceneProvider> SceneProvider, UWorld* World)
{
	this->model = pModel;
	this->config = pConfig;
	_SocketIdCounter = 0;
	nrandom.Init(config->Seed);
	random.Initialize(config->Seed);

	BuildNonThemedDungeonImpl(World, SceneProvider);
}

void UDungeonBuilder::ApplyDungeonTheme(TArray<UDungeonThemeAsset*>& Themes, TSharedPtr<FDungeonSceneProvider> SceneProvider, UWorld* World) {
	SceneProvider->OnDungeonBuildStart(config);

	PropBySocketTypeByTheme_t PropBySocketTypeByTheme;
	for (UDungeonThemeAsset* Theme : Themes) {
		CreatePropLookup(Theme, PropBySocketTypeByTheme);
	}
	TArray<ThemeOverrideInfo> ThemeOverrideList;
	GenerateThemeOverrideList(World, Dungeon, PropBySocketTypeByTheme, ThemeOverrideList);

	FCell InvalidCell;
	// Fill up the prop sockets with the defined mesh data 
	for (int32 i = 0; i < PropSockets.Num(); i++) {
		const FPropSocket& socket = PropSockets[i];

		UDungeonThemeAsset* ThemeToUse = GetBestMatchedTheme(random, Themes, socket, PropBySocketTypeByTheme); // PropAsset;

		// Check if this socket resides within a override volume
		for (const ThemeOverrideInfo& OverrideInfo : ThemeOverrideList) {
			FVector Location = socket.Transform.GetLocation();
			ADungeonThemeOverrideVolume* ThemeOverrideVolume = OverrideInfo.Volume;
			FIntVector ILocation(Location.X, Location.Y, Location.Z);
			bool intersects = OverrideInfo.Bounds.Contains(ILocation);
			if (OverrideInfo.Volume->Reversed) {
				intersects = !intersects;
			}

			if (intersects) {
				ThemeToUse = ThemeOverrideVolume->ThemeOverride;
				break;
			}
		}
		if (!ThemeToUse) continue;

		check(PropBySocketTypeByTheme.Contains(ThemeToUse));
		PropBySocketType_t& PropBySocketType = PropBySocketTypeByTheme[ThemeToUse];

		if (PropBySocketType.Contains(socket.SocketType)) {
			const TArray<FPropTypeData>& props = PropBySocketType[socket.SocketType];
			for (const FPropTypeData& prop : props) {
				bool insertMesh = false;
				if (prop.bUseSelectionLogic) {
					insertMesh = PerformSelectionLogic(prop.SelectionLogics, socket);

					if (insertMesh && !prop.bLogicOverridesAffinity) {
						// The logic has selected the mesh and it doesn't override the affinity.
						// Respect the affinity variable and apply probability
						float probability = random.FRand();
						insertMesh = (probability < prop.Probability);
					}
				}
				else {
					// Perform probability based selection logic
					float probability = random.FRand();
					insertMesh = (probability < prop.Probability);
				}

				FQuat spatialRotationOffset = FQuat::Identity;

				// Check if we are using spatial constraints
				if (prop.bUseSpatialConstraint) {
					
					bool bPassesSpatialConstraint = ProcessSpatialConstraint(prop.SpatialConstraint, socket.Transform, spatialRotationOffset);
					if (!bPassesSpatialConstraint) {
						insertMesh = false;
					}
				}

				if (insertMesh) {
					// Attach this prop to the socket
					FTransform transform = socket.Transform;

					// Apply the spatial rotation offset
					if (prop.bUseSpatialConstraint && prop.SpatialConstraint) {
						if (!prop.SpatialConstraint->bApplyBaseRotation) {
							transform.SetRotation(FQuat::Identity);
						}
						FTransform spatialRotationTransform = FTransform::Identity;
						spatialRotationTransform.SetRotation(spatialRotationOffset);
						FTransform::Multiply(&transform, &spatialRotationTransform, &transform);
					}

					FTransform::Multiply(&transform, &prop.Offset, &transform);
					

					// Apply transform logic, if specified
					if (prop.bUseTransformLogic) {
						FTransform logicOffset = PerformTransformLogic(prop.TransformLogics, socket);
						FTransform out;
						FTransform::Multiply(&out, &logicOffset, &transform);
						transform = out;
					}

					if (prop.AssetObject) {
						// Add a mesh instance, if specified
						if (UDungeonMesh* Mesh = Cast<UDungeonMesh>(prop.AssetObject)) {
							SceneProvider->AddStaticMesh(Mesh, transform, prop.NodeId);
						}
						else if (UDestructibleMesh* DestructableMesh = Cast<UDestructibleMesh>(prop.AssetObject)) {
							SceneProvider->AddDestructibleMesh(DestructableMesh, transform, prop.NodeId);
						}
						else if (UPointLightComponent* Light = Cast<UPointLightComponent>(prop.AssetObject)) {
							SceneProvider->AddLight(Light, transform, prop.NodeId);
						}
						else if (UParticleSystem* Particle = Cast<UParticleSystem>(prop.AssetObject)) {
							SceneProvider->AddParticleSystem(Particle, transform, prop.NodeId);
						}
						else if (UClass* ClassTemplate = Cast<UClass>(prop.AssetObject)) {
							SceneProvider->AddActorFromTemplate(ClassTemplate, transform, prop.NodeId);
						}
						else {
							// Not supported.  Give the implementation an opportunity to handle it
							SceneProvider->ProcessUnsupportedObject(prop.AssetObject, transform, prop.NodeId);
						}
					}

					// Add child sockets if any
					for (const FPropChildSocketData& ChildSocket : prop.ChildSockets) {
						FTransform childTransform;
						FTransform::Multiply(&childTransform, &ChildSocket.Offset, &transform);
						AddPropSocket(ChildSocket.SocketType, childTransform);
					}

					if (prop.ConsumeOnAttach) {
						// Attach no more on this socket
						break;
					}
				}
			}
		}
	}

	SceneProvider->OnDungeonBuildStop();
}

void UDungeonBuilder::MirrorDungeon()
{
	if (Dungeon) {
		for (TObjectIterator<ADungeonMirrorVolume> Volume; Volume; ++Volume)
		{
			if (!Volume || Volume->IsPendingKill() || !Volume->IsValidLowLevel()) {
				continue;
			}
			if (Volume->Dungeon == Dungeon) {
				// Build a lookup of the theme for faster access later on
				MirrorDungeonWithVolume(*Volume);
			}
		}
	}
}

TSharedPtr<class FDungeonSceneProvider> UDungeonBuilder::CreateSceneProvider(UDungeonConfig* pConfig, ADungeon* pDungeon, UWorld* World)
{
	if (!pConfig) {
		UE_LOG(DungeonBuilderLog, Error, TEXT("Invalid config reference"));
		return nullptr;
	}

	if (pConfig->Instanced) {
		return MakeShareable(new FInstancedDungeonSceneProvider(pDungeon, World));
	}
	else {
		return MakeShareable(new FPooledDungeonSceneProvider(pDungeon, World));
	}
}

bool UDungeonBuilder::ProcessSpatialConstraint(UDungeonSpatialConstraint* SpatialConstraint, const FTransform& Transform, FQuat& OutRotationOffset)
{
	return true;
}

void UDungeonBuilder::AddPropSocket(const FString& SocketType, const FTransform& _transform, int count, const FVector& InterOffset)
{
	FTransform transform = _transform;
	FVector Location = transform.GetLocation();
	for (int i = 0; i < count; i++) {
		AddPropSocket(SocketType, transform);
		Location += InterOffset;
		transform.SetLocation(Location);
	}
}

void UDungeonBuilder::AddPropSocket(const FString& SocketType, const FTransform& transform)
{
	FPropSocket socket;
	socket.Id = ++_SocketIdCounter;
	socket.IsConsumed = false;
	socket.SocketType = SocketType;
	socket.Transform = transform;
	PropSockets.Add(socket);
}

void UDungeonBuilder::AddPropSocket(TArray<FPropSocket>& pPropSockets, const FString& SocketType, const FTransform& transform)
{
	FPropSocket socket;
	socket.Id = ++_SocketIdCounter;
	socket.IsConsumed = false;
	socket.SocketType = SocketType;
	socket.Transform = transform;
	pPropSockets.Add(socket);
}

void UDungeonBuilder::EmitDungeonMarkers_Implementation()
{
	random.Initialize(config->Seed);
}

void UDungeonBuilder::EmitMarker(const FString& SocketType, const FTransform& Transform)
{
	AddPropSocket(SocketType, Transform);
}

void UDungeonBuilder::EmitCustomMarkers(TArray<UDungeonMarkerEmitter*> MarkerEmitters)
{
	for (UDungeonMarkerEmitter* MarkerEmitter : MarkerEmitters) {
		if (MarkerEmitter) {
			MarkerEmitter->EmitMarkers(this, GetModel(), config);
		}
	}
}

UClass* UDungeonBuilder::DefaultBuilderClass()
{
	return UGridDungeonBuilder::StaticClass();
}
