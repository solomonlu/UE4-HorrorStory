//$ Copyright 2015 Ali Akbar, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//
#pragma once
#include "DungeonModel.h"
#include "Core/Utils/PMRandom.h"
#include "SceneProviders/DungeonSceneProvider.h"
#include "DungeonThemeAsset.h"
#include "DungeonBuilder.generated.h"


typedef TMap<FString, TArray<FPropTypeData> > PropBySocketType_t;
typedef TMap<UDungeonThemeAsset*, PropBySocketType_t> PropBySocketTypeByTheme_t;

DECLARE_LOG_CATEGORY_EXTERN(DungeonBuilderLog, Log, All);
class ADungeon;
class UDungeonMarkerEmitter;
class ADungeonMirrorVolume;
class UDungeonModel;
class UDungeonConfig;
class UDungeonToolData;
class UDungeonSpatialConstraint;

/**
*
*/
UCLASS(Blueprintable, abstract)
class DUNGEONARCHITECTRUNTIME_API UDungeonBuilder : public UObject
{
	GENERATED_BODY()

public:
	void BuildDungeon(ADungeon* pDungeon, UWorld* World);
	void BuildDungeon(UDungeonModel* pModel, UDungeonConfig* pConfig, UWorld* World);
	void BuildNonThemedDungeon(ADungeon* pDungeon, TSharedPtr<class FDungeonSceneProvider> SceneProvider, UWorld* World);
	void BuildNonThemedDungeon(UDungeonModel* pModel, UDungeonConfig* pConfig, TSharedPtr<class FDungeonSceneProvider> SceneProvider, UWorld* World);

	virtual void ApplyDungeonTheme(TArray<UDungeonThemeAsset*>& Themes, TSharedPtr<FDungeonSceneProvider> SceneProvider, UWorld* World);

	virtual void MirrorDungeon();

	virtual void DrawDebugData(UWorld* InWorld, bool bPersistant = false, float lifeTime = -1) {}
	virtual bool SupportsBackgroundTask() const { return true; }
	virtual TSubclassOf<UDungeonModel> GetModelClass() { return nullptr; }
	virtual TSubclassOf<UDungeonConfig> GetConfigClass() { return nullptr; }
	virtual TSubclassOf<UDungeonToolData> GetToolDataClass() { return nullptr; }
	virtual bool SupportsProperty(const FName& PropertyName) const { return true; }
	virtual bool SupportsTheming() const { return true; }
	virtual TSharedPtr<class FDungeonSceneProvider> CreateSceneProvider(UDungeonConfig* Config, ADungeon* pDungeon, UWorld* World);
	UDungeonModel* GetModel() const { return model; }

	/** Implementations should process the spatial constraint and return true if it passes */
	virtual bool ProcessSpatialConstraint(UDungeonSpatialConstraint* SpatialConstraint, const FTransform& Transform, FQuat& OutRotationOffset);

	void AddPropSocket(const FString& SocketType, const FTransform& _transform, int count, const FVector& InterOffset);
	void AddPropSocket(const FString& SocketType, const FTransform& transform);
	void AddPropSocket(TArray<FPropSocket>& pPropSockets, const FString& SocketType, const FTransform& transform);

	virtual void GetDefaultMarkerNames(TArray<FString>& OutMarkerNames) {}

	UFUNCTION(BlueprintNativeEvent, Category = "Dungeon")
	void EmitDungeonMarkers();
	virtual void EmitDungeonMarkers_Implementation();

	UFUNCTION(BlueprintCallable, Category = Dungeon)
	void EmitMarker(const FString& SocketType, const FTransform& Transform);


	UFUNCTION(BlueprintCallable, BlueprintPure, Category = Dungeon)
	TArray<FPropSocket> GetMarkers() const { return PropSockets; }

	void EmitCustomMarkers(TArray<UDungeonMarkerEmitter*> MarkerEmitters);

	static UClass* DefaultBuilderClass();

protected:
	virtual void BuildDungeonImpl(UWorld* World) {};
	virtual void BuildNonThemedDungeonImpl(UWorld* World, TSharedPtr<class FDungeonSceneProvider> SceneProvider) {};
	

protected:
	virtual void MirrorDungeonWithVolume(ADungeonMirrorVolume* MirrorVolume) {}
	void ClearSockets() {
		_SocketIdCounter = 0;
		PropSockets.Reset();
	}
	virtual bool PerformSelectionLogic(const TArray<UDungeonSelectorLogic*>& SelectionLogics, const FPropSocket& socket) {
		return false;
	}
	virtual FTransform PerformTransformLogic(const TArray<UDungeonTransformLogic*>& TransformLogics, const FPropSocket& socket) {
		return FTransform::Identity;
	}

public:
	UDungeonConfig*  config;

protected:
	PMRandom nrandom;
	FRandomStream random;
	ADungeon* Dungeon;
	UDungeonModel* model;

	TArray<FPropSocket> PropSockets;
	int32 _SocketIdCounter;
};
