//$ Copyright 2015 Ali Akbar, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//


#pragma once
#include "Core/DungeonModel.h"
#include "Core/Utils/PMRandom.h"
#include "Core/SceneProviders/DungeonSceneProvider.h"
#include "DungeonThemeAsset.h"
#include "Core/DungeonBuilder.h"
#include "SnapDungeonModel.h"
#include "SnapModuleInfo.h"
#include "SnapDungeonBuilder.generated.h"


DECLARE_LOG_CATEGORY_EXTERN(SnapDungeonBuilderLog, Log, All);
class ADungeon;
class ASnapDoor;
class USnapDungeonConfig;
class USnapDungeonModel;
class FDungeonSceneProvider;
struct FSnapModuleMetaData;
class USnapDungeonLayoutNode;

struct FSnapAttachmentConfiguration {
	FBox ModuleWorldBounds;
	ASnapDoor* AttachmentDoor;
	FTransform ModuleTransform;
};


struct FModuleGrowthNode {
	FModuleGrowthNode() : IncomingDoor(nullptr), bStartNode(false) {}
	FTransform ModuleTransform;
	ASnapDoor* IncomingDoor;
	FSnapModuleMetaData IncomingModuleInfo;
	bool bStartNode;
};

struct FModuleBuildNode;
typedef TSharedPtr<FModuleBuildNode> FModuleBuildNodePtr;
struct FModuleBuildNode {
	FModuleBuildNode() : DepthFromLeaf(1), bMainBranch(false) {}
	USnapModule* Module;
	ASnapDoor* IncomingDoor;
	FSnapAttachmentConfiguration AttachmentConfig;
	int32 DepthFromLeaf;
	TArray<FModuleBuildNodePtr> Extensions;
	bool bMainBranch;
};

struct FSnapBuildRecursionGlobalState {
	FSnapBuildRecursionGlobalState() : bSafetyBailOut(false), NumTries(0), bFoundBestBuild(false) {}

	/** 
	  Searching a dense tree can lead to billions of possibilities
	  If this flag is set, the search bails out early to avoid a hang
	*/
	bool bSafetyBailOut;

	int32 NumTries;

	bool bFoundBestBuild;
};

/**
*
*/
UCLASS()
class DUNGEONARCHITECTRUNTIME_API USnapDungeonBuilder : public UDungeonBuilder
{
	GENERATED_BODY()

public:
	virtual void BuildNonThemedDungeonImpl(UWorld* World, TSharedPtr<FDungeonSceneProvider> SceneProvider) override;
	virtual void DrawDebugData(UWorld* InWorld, bool bPersistant = false, float lifeTime = 0) override;
	virtual bool SupportsBackgroundTask() const override { return false; }
	virtual TSubclassOf<UDungeonModel> GetModelClass() override;
	virtual TSubclassOf<UDungeonConfig> GetConfigClass() override;
	virtual TSubclassOf<UDungeonToolData> GetToolDataClass() override;
	virtual bool ProcessSpatialConstraint(UDungeonSpatialConstraint* SpatialConstraint, const FTransform& Transform, FQuat& OutRotationOffset) override;
	virtual bool SupportsProperty(const FName& PropertyName) const override;
	virtual bool SupportsTheming() const { return false; }

protected:
	virtual bool PerformSelectionLogic(const TArray<UDungeonSelectorLogic*>& SelectionLogics, const FPropSocket& socket) override;
	virtual FTransform PerformTransformLogic(const TArray<UDungeonTransformLogic*>& TransformLogics, const FPropSocket& socket) override;
	
private:
	USnapModule* CreateModule(TAssetPtr<UWorld> ModuleWorldAsset);
	USnapModule* GetRandomModule(const FString& ContainingDoorId);
	ASnapDoor* GetRandomDoor(USnapModule* Module, const FString& DesiredDoorId) const;
	FTransform FindAttachmentTransform(const FTransform& ParentModuleTransform, ASnapDoor* IncomingDoor, ASnapDoor* AttachmentDoor);

	bool FindAttachmentConfiguration(USnapModule* TargetModule, const FTransform& IncomingModuleTransform, const FSnapModuleMetaData& IncomingModuleInfo,
			ASnapDoor* IncomingDoor, const TArray<FBox>& OccupiedBounds, FSnapAttachmentConfiguration& OutAttachmentConfig);

	FModuleBuildNodePtr BuildLayoutRecursive(const struct FModuleGrowthNode& GrowthNode, TArray<FBox>& OccupiedBounds, int32 DepthFromStart, int32 DesiredDepth,
		bool bMainBranch, bool bForceIgnoreEndModule, FSnapBuildRecursionGlobalState& RecursiveState);
	void SpawnModuleRecursive(FModuleBuildNodePtr BuildNode, FModuleBuildNodePtr ParentNode, FName IncomingDoorInstanceId, TSharedPtr<FDungeonSceneProvider> SceneProvider);

	// Generates the layout data to be stored in the model for serialization
	void GenerateModelData(FModuleBuildNodePtr RootBuildNode);
	USnapDungeonLayoutNode* GenerateModelDataRecursive(FModuleBuildNodePtr BuildNode, TSet<FModuleBuildNodePtr>& Visited);

	void DrawDebugNodeRecursive(USnapDungeonLayoutNode* LayoutNode, UWorld* InWorld, bool bPersistant, float lifeTime, TSet<USnapDungeonLayoutNode*>& Visited);
		
	template<typename T>
	T* GetRandomObject(TArray<T*> Array, T* Ignore) {
		while (Array.Num() > 0) {
			int32 Len = Array.Num();
			T* Element = Array[random.RandRange(0, Len - 1)];
			if (Element != Ignore) {
				return Element;
			}
			Array.Remove(Element);
		}
		return nullptr;
	}

private:
	USnapDungeonModel* SnapModel;
	USnapDungeonConfig* SnapConfig;
};

