//$ Copyright 2015 Ali Akbar, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "DungeonArchitectRuntimePrivatePCH.h"
#include "SnapDungeonBuilder.h"

#include "Core/Dungeon.h"
#include "Core/Volumes/VolumeUtils.h"
#include "Core/Volumes/DungeonThemeOverrideVolume.h"
#include "Core/Volumes/DungeonNegationVolume.h"
#include "Core/DungeonArchitectConstants.h"
#include "Core/Actors/DungeonMesh.h"
#include "Core/Utils/DungeonModelHelper.h"
#include "Core/Utils/Triangulator/Impl/DelauneyTriangleGenerator.h"
#include "Core/Utils/Profiler.h"
#include "Core/Utils/MathUtils.h"
#include "Core/Utils/AssetUtils.h"
#include "SnapDungeonSelectorLogic.h"
#include "SnapDungeonTransformLogic.h"
#include "SnapDungeonToolData.h"
#include "SnapDungeonModel.h"
#include "SnapDungeonConfig.h"
#include "SnapDungeonLayoutNode.h"
#include "SnapModule.h"
#include "SnapDoor.h"

#include <stack>
#include <sstream>
#include <queue>

DEFINE_LOG_CATEGORY(SnapDungeonBuilderLog);

class SnapSceneProviderCommand_UpdateDoorState : public SceneProviderCommand {
public:
	SnapSceneProviderCommand_UpdateDoorState(const FName& InDoorId, ESnapDoorMode InDoorState)
		: SceneProviderCommand(nullptr)
		, DoorId(CreateNodeTagFromId(InDoorId))
		, DoorState(InDoorState)
	{}

	virtual void Execute(UWorld* World) override {
		for (TActorIterator<AActor> ActorIt(World); ActorIt; ++ActorIt) {
			AActor* Actor = *ActorIt;
			if (Actor->Tags.Contains(DoorId)) {
				ASnapDoor* Door = Cast<ASnapDoor>(Actor);
				Door->VisualMode = DoorState;
				Door->BuildDoor();
				break;
			}
		}
	}

	virtual void UpdateExecutionPriority(const FVector& BuildPosition) override {
		ExecutionPriority = INT32_MAX - 1000;
	}

private:
	FName DoorId;
	ESnapDoorMode DoorState;
};

USnapModule* USnapDungeonBuilder::CreateModule(TAssetPtr<UWorld> ModuleWorldAsset)
{
	UWorld* ModuleWorld = ModuleWorldAsset.LoadSynchronous();
	if (!ModuleWorld) return nullptr;

	USnapModule* SnapModule = NewObject<USnapModule>(Dungeon);
	SnapModule->Initialize(ModuleWorld, SnapConfig->IgnoredModuleActorType);
	return SnapModule;
}

bool CanDoorAttachToModuleType(const FString& ModuleType, const TArray<FString>& AllowedModuleTypes) {
	if (AllowedModuleTypes.Num() == 0) return true;
	return AllowedModuleTypes.Contains(ModuleType);
}
bool CanDoorAttachToModuleType(ASnapDoor* Door, const FSnapModuleMetaData& ModuleInfo) {
	return CanDoorAttachToModuleType(ModuleInfo.ModuleType, Door->AllowedModuleTypes);
}

bool USnapDungeonBuilder::FindAttachmentConfiguration(USnapModule* TargetModule, const FTransform& IncomingModuleTransform, const FSnapModuleMetaData& IncomingModuleInfo,
		ASnapDoor* IncomingDoor, const TArray<FBox>& OccupiedBounds, FSnapAttachmentConfiguration& OutAttachmentConfig)
{
	if (!TargetModule) return false;
	int32 NumDoors = TargetModule->SnapDoors.Num();
	if (NumDoors == 0) return false;

	if (IncomingDoor && !CanDoorAttachToModuleType(IncomingDoor, TargetModule->ModuleInfo)) {
		// The incoming door is not allowed to attach to the requested module
		return false;
	}

	bool bFoundValid = false;
	TArray<int32> ShuffledIndices = FMathUtils::GetShuffledIndices(NumDoors, random);
	for (int si = 0; si < ShuffledIndices.Num(); si++) {
		int Index = ShuffledIndices[si];
		ASnapDoor* AttachmentDoor = TargetModule->SnapDoors[Index];

		// Check if this door is allowed to attach to the original incoming module
		if (IncomingDoor && !CanDoorAttachToModuleType(AttachmentDoor, IncomingModuleInfo)) {
			// Not allowed
			continue;
		}

		// Align the module with a door that fits the incoming door
		FTransform ModuleTransform = FindAttachmentTransform(IncomingModuleTransform, IncomingDoor, AttachmentDoor);

		{
			// Calculate the bounds of the module 
			FBox ModuleWorldBounds = TargetModule->Bounds;
			ModuleWorldBounds = ModuleWorldBounds.TransformBy(ModuleTransform).ExpandBy(FVector(-1 * (SnapConfig->CollisionTestContraction + 1)));
			
			// Check if this module would intersect with any of the existing modules
			bool bIntersects = false;
			for (const FBox& OccupiedBound : OccupiedBounds) {
				if (ModuleWorldBounds.Intersect(OccupiedBound)) {
					// intersects. Do not spawn a module here
					bIntersects = true;
					break;
				}
			}
			if (bIntersects) {
				continue;
			}

			// We found a valid module. Use this
			OutAttachmentConfig.AttachmentDoor = AttachmentDoor;
			OutAttachmentConfig.ModuleWorldBounds = ModuleWorldBounds;
			OutAttachmentConfig.ModuleTransform = ModuleTransform;
			bFoundValid = true;
			break;
		}
	}

	return bFoundValid;
}

FModuleBuildNodePtr USnapDungeonBuilder::BuildLayoutRecursive(const FModuleGrowthNode& GrowthNode, TArray<FBox>& OccupiedBounds, 
		int32 DepthFromStart, int32 DesiredDepth, bool bMainBranch, bool bForceIgnoreEndModule, FSnapBuildRecursionGlobalState& RecursiveState) {

	if (RecursiveState.NumTries >= SnapConfig->MaxProcessingPower) {
		return nullptr;
	}
	RecursiveState.NumTries++;

	if (DepthFromStart > DesiredDepth) {
		return nullptr;
	}

	FModuleGrowthNode Top = GrowthNode;
	FString TopDoorPath;
	if (Top.IncomingDoor) {
		TopDoorPath = Top.IncomingDoor->GetDoorAssetPath();
	}

	USnapModule* Module = nullptr;
	USnapModule* PreferedEndModule = nullptr;


	bool bStartModule = (GrowthNode.bStartNode && SnapModel->StartSnapModules.Num() > 0);
	bool bEndModule = false;
	if (bMainBranch) {
		bEndModule = (DepthFromStart >= DesiredDepth && SnapModel->EndSnapModules.Num() > 0);
	}
	else {
		bEndModule = (DepthFromStart >= DesiredDepth && SnapModel->BranchEndSnapModules.Num() > 0);
	}

	// Pick a random module
	TArray<USnapModule*>* SnapModuleListPtr = nullptr;
	if (bStartModule) {
		SnapModuleListPtr = &SnapModel->StartSnapModules;
	}

	if (bEndModule) {
		if (bForceIgnoreEndModule) {
			SnapModuleListPtr = &SnapModel->SnapModules;
		}
		else {
			SnapModuleListPtr = bMainBranch ? &SnapModel->EndSnapModules : &SnapModel->BranchEndSnapModules;
		}
	}

	int32 SnapModuleListLength = 0;
	if (SnapModuleListPtr) {
		SnapModuleListLength = (*SnapModuleListPtr).Num();
	}

	if (SnapModuleListLength == 0) {
		SnapModuleListPtr = &SnapModel->SnapModules;
		SnapModuleListLength = SnapModel->SnapModules.Num();
	}

	if (SnapModuleListLength == 0 || !SnapModuleListPtr) {
		return nullptr;
	}

	// Pick a door from this module to extend
	int32 BestValidMainBranchDifference = MAX_int32;
	FModuleBuildNodePtr BestBuildNode;

	TArray<int32> ShuffledIndices = FMathUtils::GetShuffledIndices(SnapModuleListLength, random);
	for (int si = 0; si < ShuffledIndices.Num(); si++) {
		int Index = ShuffledIndices[si];
		Module = (*SnapModuleListPtr)[Index];
		if (!Module) {
			continue;
		}

		if (Module->SnapDoors.Num() <= 1 && !bStartModule && !bEndModule) {
			continue;
		}

		if (!Module) continue;
		if (GrowthNode.bStartNode && !Module->ModuleInfo.bCanBeEndNode) {
			// Cannot be an start / end node
			continue;
		}

		FSnapAttachmentConfiguration AttachmentConfig;
		if (!FindAttachmentConfiguration(Module, Top.ModuleTransform, Top.IncomingModuleInfo, Top.IncomingDoor, OccupiedBounds, AttachmentConfig)) {
			continue;
		}

		FModuleBuildNodePtr BuildNode = MakeShareable(new FModuleBuildNode);
		BuildNode->AttachmentConfig = AttachmentConfig;
		BuildNode->IncomingDoor = Top.IncomingDoor;
		BuildNode->Module = Module;

		if (DepthFromStart == DesiredDepth && Module->ModuleInfo.bContributesToModuleCount) {
			// This has to be the leaf node
			if (!Module->ModuleInfo.bCanBeEndNode) {
				// This node is flag to not be the last node. Skip this module
				continue;
			}

			return BuildNode;
		}

		if (!BestBuildNode.IsValid()) {
			BestBuildNode = BuildNode;
		}

		// We found a valid module. Use this
		OccupiedBounds.Add(AttachmentConfig.ModuleWorldBounds);

		ASnapDoor* AttachmentDoor = AttachmentConfig.AttachmentDoor;

		// Extend from this door further
		for (ASnapDoor* ExtensionDoor : Module->SnapDoors) {
			if (ExtensionDoor == AttachmentDoor && Top.IncomingDoor) {
				// Don't want to extend from the door we came in through
				continue;
			}

			int32 ModuleCountContribution = Module->ModuleInfo.bContributesToModuleCount ? 1 : 0;

			// Grow this branch further
			FModuleGrowthNode NextNode;
			NextNode.IncomingDoor = ExtensionDoor;
			NextNode.ModuleTransform = AttachmentConfig.ModuleTransform;
			NextNode.IncomingModuleInfo = Module->ModuleInfo;
			FModuleBuildNodePtr ExtensionNode = BuildLayoutRecursive(NextNode, OccupiedBounds, DepthFromStart + ModuleCountContribution, DesiredDepth, 
				bMainBranch, false, RecursiveState);
			
			if (ExtensionNode.IsValid()) {
				int32 BranchLength = DepthFromStart + ExtensionNode->DepthFromLeaf;
				int32 ValidDistanceDifference = FMath::Abs(BranchLength - DesiredDepth);
				if (ValidDistanceDifference < BestValidMainBranchDifference || RecursiveState.bFoundBestBuild) {
					BestValidMainBranchDifference = ValidDistanceDifference;
					BuildNode->Extensions.Reset();
					BuildNode->Extensions.Add(ExtensionNode);
					BuildNode->DepthFromLeaf = FMath::Max(BuildNode->DepthFromLeaf, ExtensionNode->DepthFromLeaf + ModuleCountContribution);

					BestBuildNode = BuildNode;
				}

				if (BranchLength >= DesiredDepth) {
					// We found a branch with the desired length
					RecursiveState.bFoundBestBuild = true;
				}

				if (RecursiveState.bFoundBestBuild) {
					break;
				}
			}
		}

		// Remove it since we move out 
		OccupiedBounds.Remove(AttachmentConfig.ModuleWorldBounds);

		if (RecursiveState.bFoundBestBuild) {
			break;
		}
	}

	if (bEndModule && !bForceIgnoreEndModule) {
		BestBuildNode = BuildLayoutRecursive(GrowthNode, OccupiedBounds, DepthFromStart, DesiredDepth, bMainBranch, true, RecursiveState);
		if (!RecursiveState.bFoundBestBuild) {
			BestBuildNode = nullptr;
		}
	}
	return BestBuildNode;
}

void USnapDungeonBuilder::SpawnModuleRecursive(FModuleBuildNodePtr BuildNode, FModuleBuildNodePtr ParentNode, FName IncomingDoorInstanceId, TSharedPtr<FDungeonSceneProvider> SceneProvider)
{
	if (!BuildNode.IsValid()) return;

	// Spawn an instance of the module
	DoorTemplateToInstance_t DoorTemplateToInstance;
	USnapModule* Module = BuildNode->Module;
	FSnapAttachmentConfiguration AttachmentConfig = BuildNode->AttachmentConfig;

	FTransform DungeonTransform = Dungeon ? Dungeon->GetActorTransform() : FTransform::Identity;
	
	FTransform TargetTransform;
	FTransform::Multiply(&TargetTransform, &AttachmentConfig.ModuleTransform, &DungeonTransform);
	Module->SpawnModule(TargetTransform, SceneProvider, DoorTemplateToInstance);

	
	// Update the door state by opening or walling them off depending on the module on the other side (or lack of)
	TSet<FName> DoorsToCreate;
	if (BuildNode->IncomingDoor) {
		DoorsToCreate.Add(DoorTemplateToInstance[AttachmentConfig.AttachmentDoor]);
	}
	for (ASnapDoor* TemplateDoor : Module->SnapDoors) {
		FName InstanceId = DoorTemplateToInstance[TemplateDoor];
		ESnapDoorMode DoorMode = DoorsToCreate.Contains(InstanceId) ? ESnapDoorMode::Door : ESnapDoorMode::Closed;
		SceneProvider->ExecuteCustomCommand(MakeShareable(new SnapSceneProviderCommand_UpdateDoorState(InstanceId, DoorMode)));
	}

	if (BuildNode->IncomingDoor && !IncomingDoorInstanceId.IsNone()) {
		// TODO: Handle me
		SceneProvider->ExecuteCustomCommand(MakeShareable(new SceneProviderCommand_DestroyActorWithTag(IncomingDoorInstanceId)));
	}

	for (FModuleBuildNodePtr ChildNode : BuildNode->Extensions) {
		FName ExtensionDoorInstanceId;
		if (DoorTemplateToInstance.Contains(ChildNode->IncomingDoor)) {
			ExtensionDoorInstanceId = DoorTemplateToInstance[ChildNode->IncomingDoor];
		}
		SpawnModuleRecursive(ChildNode, BuildNode, ExtensionDoorInstanceId, SceneProvider);
	}
}

void USnapDungeonBuilder::GenerateModelData(FModuleBuildNodePtr RootBuildNode) {
	TSet<FModuleBuildNodePtr> VisitedList;
	SnapModel->LayoutRootNode = GenerateModelDataRecursive(RootBuildNode, VisitedList);
}

USnapDungeonLayoutNode* USnapDungeonBuilder::GenerateModelDataRecursive(FModuleBuildNodePtr BuildNode, TSet<FModuleBuildNodePtr>& Visited) {
	if (!BuildNode.IsValid()) return nullptr;
	if (Visited.Contains(BuildNode)) {
		return nullptr;
	}
	Visited.Add(BuildNode);

	USnapDungeonLayoutNode* LayoutNode = NewObject<USnapDungeonLayoutNode>(SnapModel);
	//LayoutNode->Module = BuildNode->Module;
	LayoutNode->ModuleTransform = BuildNode->AttachmentConfig.ModuleTransform;
	LayoutNode->Bounds = BuildNode->AttachmentConfig.ModuleWorldBounds;

	for (FModuleBuildNodePtr ChildBuildNode : BuildNode->Extensions) {
		USnapDungeonLayoutNode* ChildLayoutNode = GenerateModelDataRecursive(ChildBuildNode, Visited);
		if (ChildLayoutNode) {
			// Save the child node info
			LayoutNode->LinkedNodes.Add(ChildLayoutNode);

			// Generate the link information to the child node
			FSnapDungeonLayoutLink Link;

			{
				FTransform LocalDoorTransform = ChildBuildNode->IncomingDoor->GetActorTransform();
				FTransform WorldDoorTransform;
				FTransform::Multiply(&WorldDoorTransform, &LocalDoorTransform, &LayoutNode->ModuleTransform);
				Link.DoorTransform = WorldDoorTransform;
			}

			Link.LinkType = (BuildNode->bMainBranch && ChildBuildNode->bMainBranch)
				? ESnapDungeonLayoutLink::MainBranch
				: ESnapDungeonLayoutLink::SideBranch;

			LayoutNode->Links.Add(Link);
		}
	}

	return LayoutNode;
}

void CalculateOccupiedBounds(FModuleBuildNodePtr Node, TArray<FBox>& OccupiedBounds) {
	if (!Node.IsValid()) return;
	OccupiedBounds.Add(Node->AttachmentConfig.ModuleWorldBounds);

	for (FModuleBuildNodePtr ChildNode : Node->Extensions) {
		CalculateOccupiedBounds(ChildNode, OccupiedBounds);
	}
}

void USnapDungeonBuilder::BuildNonThemedDungeonImpl(UWorld* World, TSharedPtr<FDungeonSceneProvider> SceneProvider) {
	SnapModel = Cast<USnapDungeonModel>(model);
	SnapConfig = Cast<USnapDungeonConfig>(config);

	if (!SnapModel) {
		UE_LOG(SnapDungeonBuilderLog, Error, TEXT("Invalid dungeon model provided to the snap builder"));
		return;
	}

	if (!SnapConfig) {
		UE_LOG(SnapDungeonBuilderLog, Error, TEXT("Invalid dungeon gridConfig provided to the snap builder"));
		return;
	}
	PropSockets.Reset();

	if (!World) {
		UE_LOG(SnapDungeonBuilderLog, Error, TEXT("Invalid World reference passed to the snap builder"));
		return;
	}

	// Initialize the snap modules
	SnapModel->SnapModules.Reset();
	for (TAssetPtr<UWorld> ModuleWorldAsset : SnapConfig->Modules) {
		USnapModule* SnapModule = CreateModule(ModuleWorldAsset);
		SnapModel->SnapModules.Add(SnapModule);
	}

	// Create the start module, if specified
	SnapModel->StartSnapModules.Reset();
	for (TAssetPtr<UWorld> ModuleWorldAsset : SnapConfig->StartModules) {
		USnapModule* SnapModule = CreateModule(ModuleWorldAsset);
		SnapModel->StartSnapModules.Add(SnapModule);
	}

	// Create the end module, if specified
	SnapModel->EndSnapModules.Reset();
	for (TAssetPtr<UWorld> ModuleWorldAsset : SnapConfig->EndModules) {
		USnapModule* SnapModule = CreateModule(ModuleWorldAsset);
		SnapModel->EndSnapModules.Add(SnapModule);
	}

	// Create the branch end module, if specified
	SnapModel->BranchEndSnapModules.Reset();
	for (TAssetPtr<UWorld> ModuleWorldAsset : SnapConfig->BranchEndModules) {
		USnapModule* SnapModule = CreateModule(ModuleWorldAsset);
		SnapModel->BranchEndSnapModules.Add(SnapModule);
	}

	SceneProvider->OnDungeonBuildStart(config);

	FModuleGrowthNode StartNode;
	StartNode.IncomingDoor = nullptr;
	StartNode.bStartNode = true;
	StartNode.ModuleTransform = FTransform::Identity;

	TArray<FBox> OccupiedBounds;

	FSnapBuildRecursionGlobalState GlobalState;

	// Build the main branch
	FModuleBuildNodePtr BuildNode = BuildLayoutRecursive(StartNode, OccupiedBounds, 1, SnapConfig->MainBranchSize, true, false, GlobalState);

	// Build the side branch
	{
		TArray<FModuleBuildNodePtr> MainBranchNodes;

		// Grab the nodes in the main branch
		{
			FModuleBuildNodePtr BranchNode = BuildNode;
			while (BranchNode.IsValid()) {
				BranchNode->bMainBranch = true;
				MainBranchNodes.Add(BranchNode);

				// Move forward
				if (BranchNode->Extensions.Num() == 0) {
					break;
				}

				BranchNode = BranchNode->Extensions[0];
			}
		}

		// Iterate through the nodes in the main branch and start branching out
		for (int i = 0; i < MainBranchNodes.Num(); i++) {
			FModuleBuildNodePtr BranchStartNode = MainBranchNodes[i];
			FModuleBuildNodePtr BranchNextNode = i + 1 < MainBranchNodes.Num() ? MainBranchNodes[i + 1] : nullptr;

			USnapModule* BranchModule = BranchStartNode->Module;
			ASnapDoor* IncomingDoor = BranchStartNode->IncomingDoor;
			ASnapDoor* OutgoingDoor = BranchNextNode.IsValid() ? BranchNextNode->IncomingDoor : nullptr;
			for (ASnapDoor* Door : BranchModule->SnapDoors) {
				if (Door == IncomingDoor || Door == OutgoingDoor) {
					// These doors are already extended
					continue;
				}

				bool bGrowFromHere = (random.FRand() < SnapConfig->SideBranchProbability);
				if (!bGrowFromHere) {
					continue;
				}

				OccupiedBounds.Reset();
				CalculateOccupiedBounds(BuildNode, OccupiedBounds);

				FModuleGrowthNode BranchGrowNode;
				BranchGrowNode.IncomingDoor = Door;
				BranchGrowNode.IncomingModuleInfo = BranchStartNode->Module->ModuleInfo;
				BranchGrowNode.ModuleTransform = BranchStartNode->AttachmentConfig.ModuleTransform;

				GlobalState = FSnapBuildRecursionGlobalState();
				FModuleBuildNodePtr BranchBuildNode = BuildLayoutRecursive(BranchGrowNode, OccupiedBounds, 1, SnapConfig->SideBranchSize, false, false, GlobalState);
				if (BranchBuildNode.IsValid()) {
					// Make sure we don't end up with an undesirable leaf node
					if (BranchBuildNode->Extensions.Num() == 0 && BranchBuildNode->Module && !BranchBuildNode->Module->ModuleInfo.bCanBeEndNode) {
						continue;
					}

					BranchStartNode->Extensions.Add(BranchBuildNode);
				}

			}

			// Move to the next branch root node from the main branch
			if (BranchStartNode->Extensions.Num() > 0) {
				BranchStartNode = BranchStartNode->Extensions[0];
			}
			else {
				BranchStartNode = nullptr;
			}
		}

	}

	SpawnModuleRecursive(BuildNode, nullptr, FName(), SceneProvider);

	SceneProvider->OnDungeonBuildStop();

	// Save the layout in the model object for serialization
	GenerateModelData(BuildNode);
}

USnapModule* USnapDungeonBuilder::GetRandomModule(const FString& ContainingDoorId)
{
	const int32 Len = SnapModel->SnapModules.Num();
	if (Len == 0) return nullptr;
	TArray<int32> ShuffledIndices = FMathUtils::GetShuffledIndices(Len, random);
	for (int si = 0; si < Len; si++) {
		int Index = ShuffledIndices[si];
		USnapModule* Module = SnapModel->SnapModules[Index];
		if (Module->ContainsDoorType(ContainingDoorId)) {
			return Module;
		}
	}
	return nullptr;
}

ASnapDoor* USnapDungeonBuilder::GetRandomDoor(USnapModule* Module, const FString& DesiredDoorId) const
{
	if (Module->SnapDoors.Num() == 0) return nullptr;
	int32 Len = Module->SnapDoors.Num();
	return Module->SnapDoors[random.RandRange(0, Len - 1)];
}

FTransform USnapDungeonBuilder::FindAttachmentTransform(const FTransform& ParentModuleTransform, ASnapDoor* IncomingDoor, ASnapDoor* AttachmentDoor)
{
	if (!IncomingDoor || !AttachmentDoor) {
		return FTransform::Identity;
	}

	FTransform DesiredDoorTransform = FTransform(FQuat::MakeFromEuler(FVector(0, 180, 0))) * IncomingDoor->GetActorTransform() * ParentModuleTransform;
	// Calculate the rotation
	FQuat DesiredRotation;
	{
		FVector TargetVector = DesiredDoorTransform.GetRotation().Vector();
		FVector SourceVector = AttachmentDoor->GetActorRotation().Vector();
		DesiredRotation = FQuat::FindBetween(SourceVector, TargetVector);
	}

	// Calculate the translation
	FVector DesiredOffset;
	{
		FVector TargetOffset = DesiredDoorTransform.GetLocation();
		FVector SourceOffset = AttachmentDoor->GetActorLocation();
		SourceOffset = DesiredRotation * SourceOffset;
		DesiredOffset = TargetOffset - SourceOffset;
	}

	FTransform ModuleTransform = FTransform(DesiredRotation, DesiredOffset);
	return ModuleTransform;
}

FVector TransformPoint(const FTransform& Transform, const FVector& Location) {
	FTransform TargetTransform;
	FTransform SourceTransform = FTransform::Identity;
	SourceTransform.SetLocation(Location);
	FTransform::Multiply(&TargetTransform, &SourceTransform, &Transform);
	return TargetTransform.GetLocation();
}

void USnapDungeonBuilder::DrawDebugNodeRecursive(USnapDungeonLayoutNode* LayoutNode, UWorld* InWorld, bool bPersistant, float lifeTime, TSet<USnapDungeonLayoutNode*>& Visited) {
	if (!LayoutNode || Visited.Contains(LayoutNode)) {
		return;
	}
	Visited.Add(LayoutNode);

	const FTransform DungeonTransform = Dungeon ? Dungeon->GetActorTransform() : FTransform::Identity;

	USnapDungeonLayoutNode* StartNode = LayoutNode;
	FVector StartPoint = StartNode->Bounds.GetCenter();
	FVector StartPointWorld = TransformPoint(DungeonTransform, StartPoint);

	DrawDebugPoint(InWorld, StartPointWorld, 2.5, FColor::Red, bPersistant, lifeTime);

	// Draw all the links coming out of this point
	int32 NumLinks = LayoutNode->Links.Num();
	if (LayoutNode->LinkedNodes.Num() != NumLinks) {
		return;
	}

	
	for (int i = 0; i < NumLinks; i++) {
		USnapDungeonLayoutNode* EndNode = LayoutNode->LinkedNodes[i];
		if (!EndNode) continue;
		bool bMainBranch = LayoutNode->Links[i].LinkType == ESnapDungeonLayoutLink::MainBranch;

		FColor LineColor = bMainBranch ? FColor::Red : FColor::Yellow;
		float LineThickness = bMainBranch ? 50 : 30;
		FVector EndPoint = EndNode->Bounds.GetCenter();

		// Find the door position
		FVector DoorPosition = LayoutNode->Links[i].DoorTransform.GetLocation();
		DoorPosition.Z = (StartPoint.Z + EndPoint.Z) / 2.0f;

		FVector MidPointWorld = TransformPoint(DungeonTransform, DoorPosition);
		FVector EndPointWorld = TransformPoint(DungeonTransform, EndPoint);

		DrawDebugLine(InWorld, StartPointWorld, MidPointWorld, LineColor, bPersistant, lifeTime, 0, LineThickness);
		DrawDebugLine(InWorld, MidPointWorld, EndPointWorld, LineColor, bPersistant, lifeTime, 0, LineThickness);

		DrawDebugNodeRecursive(EndNode, InWorld, bPersistant, lifeTime, Visited);
	}
}


void USnapDungeonBuilder::DrawDebugData(UWorld* InWorld, bool bPersistant /*= false*/, float lifeTime /*= 0*/) {
	if (!SnapModel || !SnapModel->LayoutRootNode) {
		return;
	}

	TSet<USnapDungeonLayoutNode*> Visited;
	DrawDebugNodeRecursive(SnapModel->LayoutRootNode, InWorld, bPersistant, lifeTime, Visited);
}

TSubclassOf<UDungeonModel> USnapDungeonBuilder::GetModelClass()
{
	return USnapDungeonModel::StaticClass();
}

TSubclassOf<UDungeonConfig> USnapDungeonBuilder::GetConfigClass()
{
	return USnapDungeonConfig::StaticClass();
}

TSubclassOf<UDungeonToolData> USnapDungeonBuilder::GetToolDataClass()
{
	return USnapDungeonToolData::StaticClass();
}

bool USnapDungeonBuilder::ProcessSpatialConstraint(UDungeonSpatialConstraint* SpatialConstraint, const FTransform& Transform, FQuat& OutRotationOffset)
{
	return false;
}

bool USnapDungeonBuilder::SupportsProperty(const FName& PropertyName) const
{
	if (PropertyName == "Themes") return false;
	if (PropertyName == "MarkerEmitters") return false;
	if (PropertyName == "EventListeners") return false;

	return true;
}

bool USnapDungeonBuilder::PerformSelectionLogic(const TArray<UDungeonSelectorLogic*>& SelectionLogics, const FPropSocket& socket)
{
	return false;
}

FTransform USnapDungeonBuilder::PerformTransformLogic(const TArray<UDungeonTransformLogic*>& TransformLogics, const FPropSocket& socket)
{
	return FTransform::Identity;
}
