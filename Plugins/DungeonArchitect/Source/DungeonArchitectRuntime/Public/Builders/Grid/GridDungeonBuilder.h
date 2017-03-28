//$ Copyright 2015 Ali Akbar, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//


#pragma once
#include "Core/DungeonModel.h"
#include "Core/Utils/PMRandom.h"
#include "Core/SceneProviders/DungeonSceneProvider.h"
#include "DungeonThemeAsset.h"
#include "Core/DungeonBuilder.h"
#include "GridDungeonModel.h"
#include "GridDungeonBuilder.generated.h"


DECLARE_LOG_CATEGORY_EXTERN(GridDungeonBuilderLog, Log, All);
class ADungeon;
class UGridDungeonConfig;
class UGridDungeonModel;
class UGridSpatialConstraint3x3;
class UGridSpatialConstraint2x2;
class UGridSpatialConstraintEdge;

/**
*
*/
UCLASS()
class DUNGEONARCHITECTRUNTIME_API UGridDungeonBuilder : public UDungeonBuilder
{
	GENERATED_BODY()

public:
	virtual void BuildDungeonImpl(UWorld* World) override;
	virtual void EmitDungeonMarkers_Implementation() override;
	virtual void DrawDebugData(UWorld* InWorld, bool bPersistant = false, float lifeTime = 0) override;
	virtual bool SupportsBackgroundTask() const override { return false; }
	virtual void MirrorDungeon() override;
	virtual TSubclassOf<UDungeonModel> GetModelClass() override;
	virtual TSubclassOf<UDungeonConfig> GetConfigClass() override;
	virtual TSubclassOf<UDungeonToolData> GetToolDataClass() override;
	virtual bool ProcessSpatialConstraint(UDungeonSpatialConstraint* SpatialConstraint, const FTransform& Transform, FQuat& OutRotationOffset) override;

	virtual void GetDefaultMarkerNames(TArray<FString>& OutMarkerNames) override;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = Dungeon)
	bool ContainsCell(int32 x, int32 y);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = Dungeon)
	void GetRooms(TArray<FCell>& RoomCells);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = Dungeon)
	void GetCorridors(TArray<FCell>& CorridorCells);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = Dungeon)
	void GetCellsOfType(FCellType CellType, TArray<FCell>& Cells);

protected:
	void Initialize();

	virtual void MirrorDungeonWithVolume(ADungeonMirrorVolume* MirrorVolume) override;
	virtual bool PerformSelectionLogic(const TArray<UDungeonSelectorLogic*>& SelectionLogics, const FPropSocket& socket) override;
	virtual FTransform PerformTransformLogic(const TArray<UDungeonTransformLogic*>& TransformLogics, const FPropSocket& socket) override;

	FCell BuildCell();

	void Shuffle();

	void BuildCells();
	void Seperate();
	void AddUserDefinedPlatforms(UWorld* World);
	void AddUserDefinedPlatform(class AGridDungeonPlatformVolume* Volume);
	void ApplyNegationVolumes(UWorld* World);
	void TriangulateRooms();
	void BuildMinimumSpanningTree();
	void RemoveRedundantDoors();

	virtual void ConnectCorridors();
	virtual void GenerateAdjacencyLookup();
	virtual void GenerateDungeonHeights();
	virtual void ConnectStairs(int32 WeightThreshold);
	bool ContainsStair(int32 ownerCellId, int32 connectedToCellId);

	/** Iteratively fixes the dungeon adjacent cell heights if they are too high up. Returns true if more iterative fixes are required */
	bool FixDungeonCellHeights(TMap<int32, struct CellHeightNode>& CellHeightNodes);

	/** Applies the specified offset to all the cells in the  model */
	void ApplyCellOffset(const FIntVector& Offset);

	FIntVector GetDungeonOffset();
	DungeonModelBuildState GetBuildState();
	FCell GetCell(int32 Id);
	TArray<FCellDoor> GetDoors();

	void BuildMesh_Floor(const FCell& cell);
	void BuildMesh_Room(const FCell& cell);
	void BuildMesh_RoomDecoration(const FCell& cell);
	void BuildMesh_Corridor(const FCell& cell);
	void BuildMesh_Stairs(const FCell& cell);

	TArray<FCell*> GetCellsOfType(FCellType CellType);

	inline static bool CompareFromCenterPredicate(const FCell& cellA, const FCell& cellB) {
		FIntVector ca = cellA.Bounds.Center();
		FIntVector cb = cellB.Bounds.Center();
		int32 distA = ca.X * ca.X + ca.Y * ca.Y + ca.Z * ca.Z;
		int32 distB = cb.X * cb.X + cb.Y * cb.Y + cb.Z * cb.Z;
		return distA < distB;
	}

	static void ConnectCells(FCell& c1, FCell& c2)
	{
		AddUnique<int32>(c1.ConnectedRooms, c2.Id);
		AddUnique<int32>(c2.ConnectedRooms, c1.Id);
	}

	template<typename T>
	static void AddUnique(TArray<T>& Array, T value) {
		if (!Array.Contains(value)) {
			Array.Add(value);
		}
	}

	bool CheckLoop(FCell* currentNode, FCell* comingFrom, TSet<FCell*> visited);
	bool ContainsLoop(TArray<FCell*> rooms);
	void ConnectCooridorRecursive(int32 incomingRoom, int32 currentRoom, TSet<int32>& visited);
	void ConnectRooms(int32 roomA, int32 roomB);
	void ConnectAdjacentCells(int32 roomA, int32 roomB);
	int32 RegisterCorridorCell(int cellX, int cellY, int cellZ, int32 roomA, int32 roomB, bool canRegisterDoors = false);
	bool GetStair(int32 ownerCell, int32 connectedToCell, FStairInfo& outStair);
	bool CanDrawFence(const FCell& baseCell, int32 x, int32 y, bool& isElevatedFence, bool& drawPillar, int& elevationHeight);
	bool ContainsStair(const FCell& baseCell, int32 x, int32 y);
	int32 GetStairHeight(const FStairInfo& stair);
	void FixDoorTransform(int32 x0, int32 y0, int32 x1, int32 y1, FTransform& OutTransform);
	void AddCorridorPadding(int x, int y, int z);
	void RemoveStairAtDoor(const FCellDoor& Door);

	/* Check if there is a path from the source to destination cell, taking the height information and currently connected stairs into account */
	bool ContainsAdjacencyPath(int32 cellA, int32 cellB, int32 maxDepth, bool bForceRoomConnection);
	//UDungeonPropDataAsset* GetBestMatchedTheme(TArray<UDungeonPropDataAsset*>& Themes, const FPropSocket& socket, PropBySocketTypeByTheme_t& PropBySocketTypeByTheme);

	static int GetForceDirectionMultiplier(float a, float b, float a1, float b1);

	FIntVector GetRandomPointInCircle(double radius);

	bool AreCellsAdjacent(int32 cellAId, int32 cellBId);

	int32 GetRandomRoomSize();

	void CheckAndMarkAdjacent(FCell& cell, int32 otherCellX, int32 otherCellY);


	/**
	* Gets the elevation of the baseCell from the cell defined in (x,y)
	* Also outputs the z value that needs to be applied to the base cell
	*/
	int32 GetElevation(const FCell& baseCell, int32 x, int32 y, int32& OutZOffset);

	int32 GetNextCellId();

	bool ProcessSpatialConstraint3x3(UGridSpatialConstraint3x3* SpatialConstraint, const FTransform& Transform, FQuat& OutRotationOffset);
	bool ProcessSpatialConstraint2x2(UGridSpatialConstraint2x2* SpatialConstraint, const FTransform& Transform, FQuat& OutRotationOffset);
	bool ProcessSpatialConstraintEdge(UGridSpatialConstraintEdge* SpatialConstraint, const FTransform& Transform, FQuat& OutRotationOffset);

	bool ProcessSpatialConstraintEdgeEntry(UGridSpatialConstraintEdge* SpatialConstraint, const FTransform& Transform, 
			const FGridCellInfo& Cell0, const FGridCellInfo& Cell1, float BaseRotation, FQuat& OutRotationOffset);
private:
	int32 _CellIdCounter;
	UGridDungeonModel* gridModel;
	UGridDungeonConfig* gridConfig;
	FVector GridToMeshScale;
};

