//$ Copyright 2015 Ali Akbar, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "DungeonArchitectRuntimePrivatePCH.h"
#include "GridDungeonBuilder.h"

#include "Core/Dungeon.h"
#include "Core/Volumes/VolumeUtils.h"
#include "Core/Volumes/DungeonThemeOverrideVolume.h"
#include "Core/Volumes/DungeonNegationVolume.h"
#include "Core/Volumes/DungeonMirrorVolume.h"
#include "Core/DungeonArchitectConstants.h"
#include "Core/Actors/DungeonMesh.h"
#include "Core/Utils/DungeonModelHelper.h"
#include "Core/Utils/Triangulator/Impl/DelauneyTriangleGenerator.h"
#include "Core/Utils/Profiler.h"
#include "Volumes/GridDungeonPlatformVolume.h"
#include "GridDungeonSelectorLogic.h"
#include "GridDungeonTransformLogic.h"
#include "GridDungeonModel.h"
#include "GridDungeonModelHelper.h"
#include "GridDungeonToolData.h"
#include "GridDungeonConfig.h"

#include <stack>
#include <sstream>
#include <queue>
#include "SpatialConstraints/GridSpatialConstraint3x3.h"
#include "SpatialConstraints/GridSpatialConstraint2x2.h"
#include "SpatialConstraints/GridSpatialConstraintEdge.h"

DEFINE_LOG_CATEGORY(GridDungeonBuilderLog);


void UGridDungeonBuilder::BuildDungeonImpl(UWorld* World) {
	gridModel = Cast<UGridDungeonModel>(model);
	gridConfig = Cast<UGridDungeonConfig>(config);
	
	if (!gridModel) {
		UE_LOG(GridDungeonBuilderLog, Error, TEXT("Invalid dungeon model provided to the grid builder"));
		return;
	}

	if (!gridConfig) {
		UE_LOG(GridDungeonBuilderLog, Error, TEXT("Invalid dungeon gridConfig provided to the grid builder"));
		return;
	}

	Initialize();
	GridToMeshScale = gridConfig->GridCellSize;

	BuildCells();
	while (gridModel->BuildState == DungeonModelBuildState::Separation) {
		Seperate();
	}

	FIntVector Offset = GetDungeonOffset();
	ApplyCellOffset(Offset);

	// Apply negation volumes by removing procedural geometry that lie within it
	ApplyNegationVolumes(World);

	// Add cells defined by platform volumes in the world
	AddUserDefinedPlatforms(World);

	// Connect the rooms with delaunay triangulation to have nice evenly spaced triangles
	TriangulateRooms();

	// Build a minimum spanning tree of the above triangulation, to avoid having lots of loops
	BuildMinimumSpanningTree();

	// Connect the rooms by converting cells between the rooms into corridors.  Also adds new corridor cells if needed for the connection
	ConnectCorridors();

	// Apply negation volumes by removing procedural geometry that lie within it
	ApplyNegationVolumes(World);

	// Build a lookup of adjacent tiles for later use with height and stair creation
	GenerateAdjacencyLookup();
	GenerateDungeonHeights();

	int IterationWeights[] = { 100, 50, 0, -100 };
	for (int WeightIndex = 0; WeightIndex < 4; WeightIndex++) {
		ConnectStairs(IterationWeights[WeightIndex]);
	}
	
	RemoveRedundantDoors();

	gridModel->BuildState = DungeonModelBuildState::Complete;
}

void UGridDungeonBuilder::Initialize()
{
	if (gridModel) {
		gridModel->GridCellInfoLookup.Reset();
		gridModel->CellStairs.Reset();
	}
	PropSockets.Reset();
}

void UGridDungeonBuilder::MirrorDungeonWithVolume(ADungeonMirrorVolume* MirrorVolume)
{
	FVector MirrorAxis(1, 0, 0);
	FVector MirrorNormal(0, 1, 0);
	MirrorAxis = MirrorVolume->GetTransform().GetRotation() * MirrorAxis;
	MirrorNormal = MirrorVolume->GetTransform().GetRotation() * MirrorNormal;
	FVector MirrorLocation = MirrorVolume->GetTransform().GetTranslation();

	TArray<FCell> CellsToRemove;
	TArray<FCell> CellsToAdd;

	for (FCell& Cell : gridModel->Cells) {
		float Width = Cell.Bounds.Size.X;
		float Length = Cell.Bounds.Size.Y;
		FVector CellLocation(Cell.Bounds.Location.X, Cell.Bounds.Location.Y, 0);
		const FVector Tips[4] = {
			CellLocation,
			CellLocation + FVector(Width, 0, 0),
			CellLocation + FVector(Width, Length, 0),
			CellLocation + FVector(0, Length, 0)
		}; 

		int32 TipsReflected = 0;
		FVector GridSize = gridConfig->GridCellSize;
		TArray<FVector> ReflectedTips;
		for (int i = 0; i < 4; i++) {
			const FVector Tip = Tips[i] * GridSize;
			
			// Check if this point lies in the reflective side
			FVector DirectionToTip = Tip - MirrorLocation;
			FVector TipUp = FVector::CrossProduct(DirectionToTip, MirrorAxis);
			if (TipUp.Z > 0) {
				TipsReflected++;
				ReflectedTips.Add(Tip);
				FVector ReflectedTip = FMath::GetReflectionVector(DirectionToTip, MirrorNormal);
				ReflectedTips.Add(ReflectedTip);
			}
		}

		if (TipsReflected == 0) {
			// This cell lies totally outside the reflective volume
			CellsToRemove.Add(Cell);
		}
		else if (TipsReflected == 4) {
			CellLocation = FVector(Cell.Bounds.Location.X, Cell.Bounds.Location.Y, Cell.Bounds.Location.Z);
			// This cell lies totally inside the reflective volume
			const FVector Center = (CellLocation + FVector(Width, Length, 0) / 2.0f) * GridSize;
			float DistanceToMirror = FVector::DotProduct(Center - MirrorLocation, MirrorNormal);
			FVector MirroredCenter = Center - MirrorNormal * (DistanceToMirror * 2.0f);
			
			FCell MirroredCell;
			MirroredCell.Id = GetNextCellId();
			MirroredCell.UserDefined = false;
			FRectangle bounds;
			bounds.Size = Cell.Bounds.Size;
			FVector MirroredCenterGrid = MirroredCenter / GridSize;
			bounds.Location.X = FMath::RoundToInt(MirroredCenterGrid.X - bounds.Size.X / 2.0f);
			bounds.Location.Y = FMath::RoundToInt(MirroredCenterGrid.Y - bounds.Size.Y / 2.0f);
			bounds.Location.Z = FMath::RoundToInt(MirroredCenterGrid.Z);
			
			MirroredCell.Bounds = bounds;
			MirroredCell.CellType = Cell.CellType;
			CellsToAdd.Add(MirroredCell);
		}
		else {
			if (ReflectedTips.Num() > 0) {
				// This cell partially lies within the reflective volume
				// Extend the bounds
				float MinX = ReflectedTips[0].X;
				float MaxX = ReflectedTips[0].X;
				float MinY = ReflectedTips[0].Y;
				float MaxY = ReflectedTips[0].Y;

				for (const FVector& ReflectedTip : ReflectedTips) {
					MinX = FMath::Min(MinX, ReflectedTip.X);
					MinY = FMath::Min(MinY, ReflectedTip.Y);

					MaxX = FMath::Max(MaxX, ReflectedTip.X);
					MaxY = FMath::Max(MaxY, ReflectedTip.Y);
				}

				FRectangle bounds;
				bounds.Size.X = FMath::RoundToInt((MaxX - MinX) / GridSize.X);
				bounds.Size.Y = FMath::RoundToInt((MaxY - MinY) / GridSize.Y);
				bounds.Location.X = FMath::RoundToInt(MinX / GridSize.X);
				bounds.Location.Y = FMath::RoundToInt(MinY / GridSize.Y);
				Cell.Bounds = bounds;
			}

		}

	}
	for (const FCell& CellToRemove : CellsToRemove) {
		gridModel->Cells.Remove(CellToRemove);
	}
	for (const FCell& CellToAdd : CellsToAdd) {
		gridModel->Cells.Add(CellToAdd);
	}

	// TODO: reflect doors and stair gridModel

	// Reflect the low level markers
	TArray<FPropSocket> MarkersToAdd;
	TArray<FPropSocket> MarkersToRemove;

	for (const FPropSocket& Marker : PropSockets) {
		// Check if this point lies in the reflective side
		FVector DirectionToMarker = Marker.Transform.GetLocation() - MirrorLocation;
		FVector MarkerUp = FVector::CrossProduct(DirectionToMarker, MirrorAxis);
		if (MarkerUp.Z > 0) {
			// Reflect this tip
			FVector MarkerLocation = Marker.Transform.GetLocation();
			float DistanceToMirror = FVector::DotProduct(MarkerLocation - MirrorLocation, MirrorNormal);
			FVector MirroredLocation = MarkerLocation - MirrorNormal * (DistanceToMirror * 2.0f);
			FQuat MirroredRotation = Marker.Transform.GetRotation();	// TODO: Rotate by 180
			{
				FRotator Rotator(MirroredRotation);
				FVector ReflectedVector = FMath::GetReflectionVector(Rotator.Vector(), MirrorNormal);
				MirroredRotation = FQuat(ReflectedVector.Rotation());
			}

			FPropSocket ReflectedMarker = Marker;
			ReflectedMarker.Id = ++_SocketIdCounter;
			ReflectedMarker.Transform.SetLocation(MirroredLocation);
			ReflectedMarker.Transform.SetRotation(MirroredRotation);

			MarkersToAdd.Add(ReflectedMarker);
		}
		else {
			// Lies outside. Discard
			MarkersToRemove.Add(Marker);
		}
	}

	// Remove discarded markers
	for (const FPropSocket& Marker : MarkersToRemove) {
		PropSockets.Remove(Marker);
	}

	// Add reflected markers
	for (const FPropSocket& Marker : MarkersToAdd) {
		PropSockets.Add(Marker);
	}
}

bool UGridDungeonBuilder::PerformSelectionLogic(const TArray<UDungeonSelectorLogic*>& SelectionLogics, const FPropSocket& socket)
{
	for (UDungeonSelectorLogic* SelectionLogic : SelectionLogics) {
		UGridDungeonSelectorLogic* GridSelectionLogic = Cast<UGridDungeonSelectorLogic>(SelectionLogic);
		if (!GridSelectionLogic) {
			UE_LOG(GridDungeonBuilderLog, Warning, TEXT("Invalid selection logic specified.  GridDungeonSelectorLogic expected"));
			return false;
		}

		FCell InvalidCell;
		// Perform blueprint based selection logic
		FVector location = socket.Transform.GetLocation();

		int32 gridX = FMath::FloorToInt(location.X / GridToMeshScale.X);
		int32 gridY = FMath::FloorToInt(location.Y / GridToMeshScale.Y);
		FGridCellInfo cellInfo = gridModel->GetGridCellLookup(gridX, gridY);
		FCell* cell = gridModel->GetCell(cellInfo.CellId);
		FCell& cellRef = cell ? *cell : InvalidCell;
		bool selected = GridSelectionLogic->SelectNode(gridModel, gridConfig, cellRef, random, gridX, gridY);
		if (!selected) {
			return false;
		}
	}
	return true;
}

FTransform UGridDungeonBuilder::PerformTransformLogic(const TArray<UDungeonTransformLogic*>& TransformLogics, const FPropSocket& socket)
{
	FTransform result = FTransform::Identity;

	for (UDungeonTransformLogic* TransformLogic : TransformLogics) {
		UGridDungeonTransformLogic* GridTransformLogic = Cast<UGridDungeonTransformLogic>(TransformLogic);
		if (!GridTransformLogic) {
			UE_LOG(GridDungeonBuilderLog, Warning, TEXT("Invalid transform logic specified.  GridDungeonTransformLogic expected"));
			continue;
		}

		FCell InvalidCell;
		FVector location = socket.Transform.GetLocation();
		int32 gridX = FMath::FloorToInt(location.X / GridToMeshScale.X);
		int32 gridY = FMath::FloorToInt(location.Y / GridToMeshScale.Y);
		FGridCellInfo cellInfo = gridModel->GetGridCellLookup(gridX, gridY);
		FCell* cell = gridModel->GetCell(cellInfo.CellId);
		FCell& cellRef = cell ? *cell : InvalidCell;
		FTransform logicOffset;
		if (TransformLogic) {
			GridTransformLogic->GetNodeOffset(gridModel, gridConfig, cellRef, random, gridX, gridY, logicOffset);
		}
		else {
			logicOffset = FTransform::Identity;
		}

		FTransform out;
		FTransform::Multiply(&out, &logicOffset, &result);
		result = out;
	}
	return result;
}

FCell UGridDungeonBuilder::BuildCell()
{
	FCell cell;
	cell.Id = GetNextCellId();
	cell.UserDefined = false;
	FRectangle bounds;
	bounds.Location = GetRandomPointInCircle(gridConfig->InitialRoomRadius);
	int32 baseSize = GetRandomRoomSize();
	int32 width = baseSize;
	float aspectRatio = 1 + (random.FRand() * 2 - 1) * gridConfig->RoomAspectDelta;
	int32 length = FMath::RoundToInt(width * aspectRatio);
	bounds.Size = FIntVector(width, length, 0);
	cell.Bounds = bounds;

	int32 area = width * length;
	if (area >= gridConfig->RoomAreaThreshold)
	{
		cell.CellType = FCellType::Room;
	}

	return cell;
}

void UGridDungeonBuilder::Shuffle()
{
	TArray<FCell>& cells = gridModel->Cells;
	int n = cells.Num();
	for (int i = 0; i < n; i++)
	{
		int r = i + (int)(random.FRand() * (n - i));
		FCell t = cells[r];
		cells[r] = cells[i];
		cells[i] = t;
	}

	gridModel->BuildCellLookup();
}

void UGridDungeonBuilder::BuildCells() {
	TArray<FCell> cells;
	_CellIdCounter = 0;
	for (int i = 0; i < gridConfig->NumCells; i++)
	{
		FCell cell = BuildCell();
		cells.Add(cell);
	}

	gridModel->Cells = cells;
	gridModel->BuildCellLookup();
	gridModel->BuildState = DungeonModelBuildState::Separation;
}

void UGridDungeonBuilder::ApplyCellOffset(const FIntVector& Offset)
{
	for (FCell& Cell : gridModel->Cells) {
		Cell.Bounds.Location += Offset;
	}
}

int32 UGridDungeonBuilder::GetNextCellId()
{
	++_CellIdCounter;
	return _CellIdCounter;
}

bool IsRoomCorridor(FCellType type0, FCellType type1) {
	int32 rooms = 0, corridors = 0;
	rooms += (type0 == FCellType::Room) ? 1 : 0;
	rooms += (type1 == FCellType::Room) ? 1 : 0;

	corridors += (type0 == FCellType::Corridor || type0 == FCellType::CorridorPadding) ? 1 : 0;
	corridors += (type1 == FCellType::Corridor || type1 == FCellType::CorridorPadding) ? 1 : 0;
	return (rooms == 1 && corridors == 1);
}

TArray<FGridSpatialConstraintCellData> RotateNeighborConfig3x3(const TArray<FGridSpatialConstraintCellData>& Neighbors) {
	const int SrcIndex[] = {
		0, 1, 2,
		3, 4, 5,
		6, 7, 8
	};
	const int DstIndex[] = {
		6, 3, 0,
		7, 4, 1,
		8, 5, 2
	};

	TArray<FGridSpatialConstraintCellData> Result;
	Result.SetNumUninitialized(9);
	for (int i = 0; i < 9; i++) {
		Result[DstIndex[i]] = Neighbors[SrcIndex[i]];
	}
	return Result;
}

TArray<FGridSpatialConstraintCellData> RotateNeighborConfig2x2(const TArray<FGridSpatialConstraintCellData>& Neighbors) {
	const int SrcIndex[] = {
		0, 1,
		2, 3
	};
	const int DstIndex[] = {
		2, 0,
		3, 1
	};

	TArray<FGridSpatialConstraintCellData> Result;
	Result.SetNumUninitialized(4);
	for (int i = 0; i < 4; i++) {
		Result[DstIndex[i]] = Neighbors[SrcIndex[i]];
	}
	return Result;
}

int ClampToInt(float value) {
	return FMath::FloorToInt(value);
}

bool UGridDungeonBuilder::ProcessSpatialConstraint3x3(UGridSpatialConstraint3x3* SpatialConstraint, const FTransform& Transform, FQuat& OutRotationOffset)
{
	if (!SpatialConstraint) return false;
	FVector WorldLoc = Transform.GetLocation();
	FVector GridLocF = WorldLoc / GridToMeshScale;
	FIntVector GridLoc(
		ClampToInt(GridLocF.X),
		ClampToInt(GridLocF.Y),
		ClampToInt(GridLocF.Z));

	auto CenterCellInfo = gridModel->GetGridCellLookup(GridLoc.X, GridLoc.Y);
	auto Neighbors = SpatialConstraint->Configuration.Cells;
	int RotationsRequired = SpatialConstraint->bRotateToFitConstraint ? 3 : 0;
	
	for (int RotationStep = 0; RotationStep <= RotationsRequired; RotationStep++) {
		bool ConfigMatches = true;
		for (int i = 0; i < Neighbors.Num(); i++) {
			auto code = Neighbors[i];
			if (code.OccupationConstraint == EGridSpatialCellOccupation::DontCare) {
				// Don't care about this cell
				continue;
			}
			int32 dx = i % 3;
			int32 dy = i / 3;
			dx--; dy--;	 // bring to -1..1 range (from previous 0..2)
						 //dy *= -1;
			int32 x = GridLoc.X + dx;
			int32 y = GridLoc.Y + dy;

			auto cellInfo = gridModel->GetGridCellLookup(x, y);
			bool empty = cellInfo.CellType == FCellType::Unknown;
			if (IsRoomCorridor(CenterCellInfo.CellType, cellInfo.CellType)) {
				// Make sure we aren't within a door
				if (CenterCellInfo.ContainsDoor || cellInfo.ContainsDoor) {
					empty = false;
				}
				else {
					empty = true;
				}
			}
			
			if (code.OccupationConstraint == EGridSpatialCellOccupation::Occupied && empty) {
				// We were expecting a non-empty space here, but it is empty
				ConfigMatches = false;
				break;
			}
			else if (code.OccupationConstraint == EGridSpatialCellOccupation::Empty && !empty) {
				// We were expecting a empty space here, but it is not empty
				ConfigMatches = false;
				break;
			}
		}

		if (ConfigMatches) {
			float RotationAngle = -90 * RotationStep;
			OutRotationOffset = FQuat::MakeFromEuler(FVector(0, 0, RotationAngle));
			return true;
		}

		if (RotationStep < RotationsRequired) {
			Neighbors = RotateNeighborConfig3x3(Neighbors);
		}
	}

	// No configurations matched
	OutRotationOffset = FQuat::Identity;
	return false;
}

bool UGridDungeonBuilder::ProcessSpatialConstraint2x2(UGridSpatialConstraint2x2* SpatialConstraint, const FTransform& Transform, FQuat& OutRotationOffset)
{
	FVector WorldLoc = Transform.GetLocation();
	FVector GridLocF = WorldLoc / GridToMeshScale;
	FIntVector GridLoc(
		ClampToInt(GridLocF.X + 0.01f),
		ClampToInt(GridLocF.Y + 0.01f),
		ClampToInt(GridLocF.Z + 0.01f));

	auto CenterCellInfo = gridModel->GetGridCellLookup(GridLoc.X, GridLoc.Y);
	auto Neighbors = SpatialConstraint->Configuration.Cells;
	int RotationsRequired = SpatialConstraint->bRotateToFitConstraint ? 3 : 0;

	for (int RotationStep = 0; RotationStep <= RotationsRequired; RotationStep++) {
		bool ConfigMatches = true;
		for (int i = 0; i < Neighbors.Num(); i++) {
			auto code = Neighbors[i];
			if (code.OccupationConstraint == EGridSpatialCellOccupation::DontCare) {
				// Don't care about this cell
				continue;
			}
			int32 dx = i % 2;
			int32 dy = i / 2;
			dx--; 
			dy--;	 // bring to -1..0 range (from previous 0..1)
						 
			int32 x = GridLoc.X + dx;
			int32 y = GridLoc.Y + dy;

			auto cellInfo = gridModel->GetGridCellLookup(x, y);
			bool empty = cellInfo.CellType == FCellType::Unknown;
			auto cell0 = gridModel->GetCell(cellInfo.CellId);
			auto cell1 = gridModel->GetCell(CenterCellInfo.CellId);
			if (!empty) {
				if (cell0 && cell1 && cell0->Bounds.Location.Z != cell1->Bounds.Location.Z) {
					empty = true;
				}
			}
			/*
			if (IsRoomCorridor(CenterCellInfo.CellType, cellInfo.CellType)) {
				// Make sure we aren't within a door
				if (CenterCellInfo.ContainsDoor && cellInfo.ContainsDoor) {
					empty = false;
				}
				else {
					empty = true;
				}
			}
			*/

			if (code.OccupationConstraint == EGridSpatialCellOccupation::Occupied && empty) {
				// We were expecting a non-empty space here, but it is empty
				ConfigMatches = false;
				break;
			}
			else if (code.OccupationConstraint == EGridSpatialCellOccupation::Empty && !empty) {
				// We were expecting a empty space here, but it is not empty
				ConfigMatches = false;
				break;
			}
		}

		if (ConfigMatches) {
			float RotationAngle = -90 * RotationStep;
			OutRotationOffset = FQuat::MakeFromEuler(FVector(0, 0, RotationAngle));
			return true;
		}

		if (RotationStep < RotationsRequired) {
			Neighbors = RotateNeighborConfig2x2(Neighbors);
		}
	}

	// No configurations matched
	OutRotationOffset = FQuat::Identity;
	return false;
}

bool UGridDungeonBuilder::ProcessSpatialConstraintEdge(UGridSpatialConstraintEdge* SpatialConstraint, const FTransform& Transform, FQuat& OutRotationOffset)
{
	FVector WorldLoc = Transform.GetLocation();
	FVector GridLocF = WorldLoc / GridToMeshScale;
	float XFrac = FMath::Frac(GridLocF.X);
	float YFrac = FMath::Frac(GridLocF.Y);

	FIntVector GridLoc(
		ClampToInt(GridLocF.X),
		ClampToInt(GridLocF.Y),
		ClampToInt(GridLocF.Z));

	auto CenterCellInfo = gridModel->GetGridCellLookup(GridLoc.X, GridLoc.Y);

	auto LeftInfo = gridModel->GetGridCellLookup(GridLoc.X - 1, GridLoc.Y);
	auto RightInfo = CenterCellInfo;
	auto TopInfo = gridModel->GetGridCellLookup(GridLoc.X, GridLoc.Y - 1);
	auto BottomInfo = CenterCellInfo;

	bool passed;
	if (XFrac > YFrac) {
		// Vertical comparison since the point in in the middle of a horizontal line
		passed = ProcessSpatialConstraintEdgeEntry(SpatialConstraint, Transform, TopInfo, BottomInfo, 90, OutRotationOffset);
		if (!passed) {
			passed = ProcessSpatialConstraintEdgeEntry(SpatialConstraint, Transform, BottomInfo, TopInfo, 270, OutRotationOffset);
		}
	}
	else {
		// Horizontal comparison since the point in in the middle of a vertical line
		passed = ProcessSpatialConstraintEdgeEntry(SpatialConstraint, Transform, LeftInfo, RightInfo, 0, OutRotationOffset);
		if (!passed) {
			passed = ProcessSpatialConstraintEdgeEntry(SpatialConstraint, Transform, RightInfo, LeftInfo, 180, OutRotationOffset);
		}
	}
	return passed;
}

bool UGridDungeonBuilder::ProcessSpatialConstraintEdgeEntry(UGridSpatialConstraintEdge* SpatialConstraint, const FTransform& Transform, const FGridCellInfo& Cell0, const FGridCellInfo& Cell1, float BaseRotation, FQuat& OutRotationOffset)
{
	if (!SpatialConstraint) return false;
	FGridCellInfo CellInfos[] = { Cell0, Cell1 };
	for (int i = 0; i < 2; i++) {
		bool empty = (CellInfos[i].CellType == FCellType::Unknown);
		auto data = SpatialConstraint->Configuration.Cells[i];
		if (data.OccupationConstraint == EGridSpatialCellOccupation::Occupied && empty) {
			// We were expecting a non-empty space here, but it is empty
			return false;
		}
		else if (data.OccupationConstraint == EGridSpatialCellOccupation::Empty && !empty) {
			// We were expecting a empty space here, but it is not empty
			return false;
		}
	}
	
	OutRotationOffset = FQuat::MakeFromEuler(FVector(0, 0, BaseRotation));
	return true;
}

void UGridDungeonBuilder::AddUserDefinedPlatforms(UWorld* World)
{
	if (!Dungeon) return;

	// Add geometry defined by the editor paint tool
	UGridDungeonToolData* ToolData = Cast<UGridDungeonToolData>(Dungeon->GetToolData());
	if (ToolData) {
		for (const FIntVector& CellPoint : ToolData->PaintedCells) {
			FCell cell;
			cell.Id = GetNextCellId();
			cell.UserDefined = true;
			FRectangle Bounds;
			Bounds.Location = CellPoint;
			Bounds.Size = FIntVector(1, 1, 1);
			cell.Bounds = Bounds;
			cell.CellType = FCellType::Corridor;
			gridModel->Cells.Add(cell);		// TODO: Check for duplicates
		}
	}

	// Add platform volumes defined in the world
	if (World) {
		for (TObjectIterator<AGridDungeonPlatformVolume> Volume; Volume; ++Volume)
		{
			if (Volume->Dungeon == Dungeon) {
				AddUserDefinedPlatform(*Volume);
			}
		}
	}

	gridModel->BuildCellLookup();
}

void UGridDungeonBuilder::AddUserDefinedPlatform(AGridDungeonPlatformVolume* Volume)
{
	if (Volume->IsPendingKill()) {
		return;
	}
	FCell cell;
	cell.Id = GetNextCellId();
	cell.UserDefined = true;

	Volume->GetDungeonVolumeBounds(GridToMeshScale, cell.Bounds);
	cell.CellType = Volume->CellType;

	// Remove any cell that intersects with this user defined platform
	for (int i = 0; i < gridModel->Cells.Num();) {
		if (cell.Bounds.IntersectsWith(gridModel->Cells[i].Bounds)) {
			gridModel->Cells.RemoveAt(i);
		}
		else {
			i++;
		}
	}

	gridModel->Cells.Add(cell);
}

struct NegationVolumeInfo {
	FRectangle Bounds;
	ADungeonNegationVolume* Volume;
};

void UGridDungeonBuilder::ApplyNegationVolumes(UWorld* World)
{
	if (World) {
		// Grab the bounds of all the negation volumes
		TArray<NegationVolumeInfo> NegationList;
		for (TObjectIterator<ADungeonNegationVolume> NegationVolume; NegationVolume; ++NegationVolume)
		{
			if (!NegationVolume->IsValidLowLevel() || NegationVolume->IsPendingKill()) {
				continue;
			}
			if (NegationVolume->Dungeon != Dungeon) {
				continue;
			}

			FRectangle VolumeBounds;
			NegationVolume->GetDungeonVolumeBounds(GridToMeshScale, VolumeBounds);

			NegationVolumeInfo Info;
			Info.Volume = *NegationVolume;
			Info.Bounds = VolumeBounds;
			NegationList.Add(Info);
		}

		// Remove any cells that fall within any of the negation bounds
		TSet<int32> CellsToRemove;
		for (const FCell& cell : gridModel->Cells) {
			for (const NegationVolumeInfo& VolumeInfo : NegationList) {
				if (cell.UserDefined && !VolumeInfo.Volume->AffectsUserDefinedCells) {
					// The volume does not affect user defined cells. Ignore it
					continue;
				}
				bool intersects = cell.Bounds.IntersectsWith(VolumeInfo.Bounds);
				bool remove = intersects;
				if (VolumeInfo.Volume->Reversed) {
					remove = !remove;

					if (!remove) {
						// Do an exhaustive search, and make sure it is fully inside
						FRectangle intersection = FRectangle::Intersect(cell.Bounds, VolumeInfo.Bounds);
						if (intersection.Size != cell.Bounds.Size) {
							remove = true;
						}
					}
				}

				if (remove) {
					CellsToRemove.Add(cell.Id);
					break;
				}
			}
		}

		gridModel->CellLookup.Reset();

		for (int i = 0; i < gridModel->Cells.Num();) {
			int32 cellId = gridModel->Cells[i].Id;
			if (CellsToRemove.Contains(cellId)) {
				gridModel->Cells.RemoveAt(i);
			}
			else {
				i++;
			}
		}
	}

	gridModel->BuildCellLookup();
}

FIntVector UGridDungeonBuilder::GetDungeonOffset() {
	FIntVector Offset = FIntVector::ZeroValue;
	if (Dungeon) {
		FVector OffsetF = Dungeon->GetActorLocation() / gridConfig->GridCellSize;
		Offset = FIntVector(FMath::RoundToInt(OffsetF.X),
			FMath::RoundToInt(OffsetF.Y),
			FMath::RoundToInt(OffsetF.Z));

	}
	return Offset;
}

void UGridDungeonBuilder::Seperate()
{
	if (gridModel->BuildState != DungeonModelBuildState::Separation) return;

	Shuffle();
	int32 count = gridModel->Cells.Num();
	TArray<FIntVector> forces;
	forces.AddZeroed(count);

	gridModel->Cells.Sort(CompareFromCenterPredicate);

	bool separated = false;
	for (int a = 0; a < count; a++)
	{
		for (int b = a + 1; b < count; b++)
		{
			if (a == b) continue;
			FRectangle& c0 = gridModel->Cells[a].Bounds;
			FRectangle& c1 = gridModel->Cells[b].Bounds;

			if (c0.IntersectsWith(c1))
			{
				FIntVector force(0, 0, 0);
				FRectangle intersection = FRectangle::Intersect(c0, c1);
				bool applyOnX = (intersection.Width() < intersection.Height());
				if (intersection.Width() == intersection.Height())
				{
					applyOnX = random.FRand() > 0.5f;
				}
				if (applyOnX)
				{
					force.X = intersection.Width();
					force.X *= GetForceDirectionMultiplier(c0.X(), c1.X(), c0.Y(), c1.Y());
				}
				else
				{
					force.Y = intersection.Height();
					force.Y *= GetForceDirectionMultiplier(c0.Y(), c1.Y(), c0.X(), c1.X());
				}
				forces[a].X += force.X;
				forces[a].Y += force.Y;

				forces[b].X -= force.X;
				forces[b].Y -= force.Y;

				separated = true;
			}
		}
	}

	for (int a = 0; a < count; a++)
	{
		FIntVector force(0, 0, 0);
		FIntVector& f = forces[a];
		if (FMath::Abs(f.X) > 0 || FMath::Abs(f.Y) > 0)
		{
			if (FMath::Abs(f.X) > FMath::Abs(f.Y))
			{
				force.X = FMath::Sign(f.X);
			}
			else
			{
				force.Y = FMath::Sign(f.Y);
			}
		}
		FCell& cell = gridModel->Cells[a];
		FIntVector& location = cell.Bounds.Location;
		location.X += force.X;
		location.Y += force.Y;
	}

	if (!separated)
	{
		gridModel->BuildState = DungeonModelBuildState::Triangulation;
	}

	return;
}

TArray<FCell*> UGridDungeonBuilder::GetCellsOfType(FCellType CellType) {
	FCell* CellArray = gridModel->Cells.GetData();
	TArray<FCell*> filtered;
	for (int i = 0; i < gridModel->Cells.Num(); i++) {
		FCell* cell = CellArray + i;
		if (cell->CellType == CellType) {
			filtered.Add(cell);
		}
	}
	return filtered;
}

void UGridDungeonBuilder::TriangulateRooms()
{
	FRandomStream RoomCenterOffsetRandom;
	RoomCenterOffsetRandom.Initialize(gridConfig->Seed);

	TArray<FCell*> rooms = GetCellsOfType(FCellType::Room);
	TArray<float> positions;
	DelauneyTriangleGenerator generator;

	const float RandomOffsetLength = 0.01f;
	for (const FCell* room : rooms) {
		FVector2D RoomCenterOffset = FMathUtils::GetRandomDirection2D(RoomCenterOffsetRandom) * RandomOffsetLength;
		FIntVector RoomCenter = room->Center();
		generator.AddPoint(FVector2D(RoomCenter.X, RoomCenter.Y) + RoomCenterOffset);
	}

	if (rooms.Num() >= 3) {
		generator.Triangulate();
		const TArray<FDelauneyTriangle>& Triangles = generator.GetTriangles();
		for (const FDelauneyTriangle& Triangle : Triangles) {
			FCell& cell0 = *rooms[Triangle.v0];
			FCell& cell1 = *rooms[Triangle.v1];
			FCell& cell2 = *rooms[Triangle.v2];

			ConnectCells(cell0, cell1);
			ConnectCells(cell1, cell2);
			ConnectCells(cell2, cell0);
		}
	}
	else if (rooms.Num() == 2) {
		ConnectCells(*rooms[0], *rooms[1]);
	}

	gridModel->BuildState = DungeonModelBuildState::SpanningTree;
}

struct Edge {
	Edge(int32 pCellA, int32 pCellB, float pWeight) : cellA(pCellA), cellB(pCellB), weight(pWeight) {}
	int32 cellA;
	int32 cellB;
	float weight;

	bool operator<(const Edge& other) const {
		return weight < other.weight;
	}
};

float GetDistance(const FCell& A, const FCell& B) {
	FVector pointA = UDungeonModelHelper::MakeVector(A.Center());
	FVector pointB = UDungeonModelHelper::MakeVector(B.Center());
	return (pointA - pointB).Size();
}


bool UGridDungeonBuilder::CheckLoop(FCell* currentNode, FCell* comingFrom, TSet<FCell*> visited)
{
	visited.Add(currentNode);
	// check if any of the children have already been visited
	for (int32 childId : currentNode->FixedRoomConnections)
	{
		FCell* child = gridModel->GetCell(childId);
		if (!child) continue;

		if (child == comingFrom) continue;
		if (visited.Contains(child))
		{
			return true;
		}
		bool branchHasLoop = CheckLoop(child, currentNode, visited);
		if (branchHasLoop) return true;
	}
	return false;
}

bool UGridDungeonBuilder::ContainsLoop(TArray<FCell*> rooms)
{
	for (FCell* room : rooms)
	{
		if (room) {
			TSet<FCell*> visited;
			bool hasLoop = CheckLoop(room, 0, visited);
			if (hasLoop) return true;
		}
	}

	return false;
}


void UGridDungeonBuilder::BuildMinimumSpanningTree() {
	TArray<FCell*> rooms = GetCellsOfType(FCellType::Room);
	TMap<int32, TSet<int32> > edgesMapped;

	// Generate unique edge list
	TArray<Edge> edges;
	for (const FCell* room : rooms) {
		for (int32 connectedRoomId : room->ConnectedRooms) {
			FCell* other = gridModel->GetCell(connectedRoomId);
			float distance = GetDistance(*room, *other);
			int32 id0 = room->Id;
			int32 id1 = other->Id;
			if (!edgesMapped.Contains(id0) || !edgesMapped[id0].Contains(id1)) {
				Edge edge(id0, id1, distance);
				edges.Add(edge);

				if (!edgesMapped.Contains(id0)) edgesMapped.Add(id0, TSet<int32>());
				if (!edgesMapped.Contains(id1)) edgesMapped.Add(id1, TSet<int32>());
				edgesMapped[id0].Add(id1);
				edgesMapped[id1].Add(id0);
			}
		}
	}

	edges.Sort();

	for (const Edge& edge : edges) {
		FCell* cell0 = gridModel->GetCell(edge.cellA);
		FCell* cell1 = gridModel->GetCell(edge.cellB);
		if (cell0 && cell1) {
			AddUnique<int32>(cell0->FixedRoomConnections, cell1->Id);
			AddUnique<int32>(cell1->FixedRoomConnections, cell0->Id);

			// Check if this new edge insertion caused a loop in the MST
			bool loop = ContainsLoop(rooms);
			if (loop) {
				cell0->FixedRoomConnections.Remove(cell1->Id);
				cell1->FixedRoomConnections.Remove(cell0->Id);
			}
		}
	}

	// Add some edges from the Delauney triangulation based on a probability
	for (FCell* room : rooms) {
		for (int32 otherDelauney : room->ConnectedRooms) {
			if (!room->FixedRoomConnections.Contains(otherDelauney)) {
				float probability = random.FRand();
				if (probability < gridConfig->SpanningTreeLoopProbability) {
					FCell* other = gridModel->GetCell(otherDelauney);
					if (other) {
						room->FixedRoomConnections.Add(otherDelauney);
						other->FixedRoomConnections.Add(room->Id);
					}
				}
			}
		}
	}


	gridModel->BuildState = DungeonModelBuildState::Corridors;
}

void UGridDungeonBuilder::RemoveRedundantDoors()
{
	if (!gridConfig || !gridModel || gridConfig->DoorProximitySteps <= 0) {
		return;
	}

	TArray<FCellDoor>& Doors = gridModel->DoorManager.GetDoors();
	TArray<FCellDoor> DoorsToRemove;
	for (FCellDoor& Door : Doors) {
		int32 CellIdA = Door.AdjacentCells[0];
		int32 CellIdB = Door.AdjacentCells[1];

		// Temporarily disable the door to see if the two adjacent cells can still reach (possbily through a nearby door)
		Door.bEnabled = false;

		bool pathExists = ContainsAdjacencyPath(CellIdA, CellIdB, gridConfig->DoorProximitySteps, false);
		if (!pathExists)
		{
			// No path exists. We need a door here
			Door.bEnabled = true;
		}
		else {
			// Remove the door
			DoorsToRemove.Add(Door);
		}
	}

	for (const FCellDoor& Door : DoorsToRemove) {
		gridModel->DoorManager.RemoveDoor(Door);
		RemoveStairAtDoor(Door);
	}

	gridModel->BuildCellLookup();
	GenerateAdjacencyLookup();
}

#define HASH(a, b) (((a) << 16) + (b))

void UGridDungeonBuilder::ConnectCorridors() {
	TArray<FCell*> rooms = GetCellsOfType(FCellType::Room);
	if (rooms.Num() < 2) return;
	TSet<int32> visited;
	FCell* startingRoom = rooms[0];
	ConnectCooridorRecursive(-1, startingRoom->Id, visited);

	// Remove unused cells
	for (int i = 0; i < gridModel->Cells.Num();) {
		if (gridModel->Cells[i].CellType == FCellType::Unknown) {
			gridModel->Cells.RemoveAt(i);
		}
		else {
			i++;
		}
	}
	// Rebuild the cell cache list, since it has been modified
	gridModel->BuildCellLookup();

	gridModel->BuildState = DungeonModelBuildState::Complete;
}

void UGridDungeonBuilder::ConnectCooridorRecursive(int32 incomingRoomId, int32 currentRoomId, TSet<int32>& visited) {
	if (incomingRoomId >= 0)
	{
		int32 c0 = incomingRoomId;
		int32 c1 = currentRoomId;
		if (visited.Contains(HASH(c0, c1))) return;
		visited.Add(HASH(c0, c1));
		visited.Add(HASH(c1, c0));

		ConnectRooms(incomingRoomId, currentRoomId);
	}

	FCell* currentRoom = gridModel->GetCell(currentRoomId);
	check(currentRoom);
	TArray<int32> children = currentRoom->FixedRoomConnections;
	for (int32 otherRoomId : children)
	{
		FCell* otherRoom = gridModel->GetCell(otherRoomId);
		check(otherRoom);
		int32 i0 = currentRoom->Id;
		int32 i1 = otherRoom->Id;
		if (!visited.Contains(HASH(i0, i1))) {
			ConnectCooridorRecursive(currentRoomId, otherRoomId, visited);
		}
	}
}
void UGridDungeonBuilder::ConnectAdjacentCells(int32 roomA, int32 roomB) {
	FCell* cellA = gridModel->GetCell(roomA);
	FCell* cellB = gridModel->GetCell(roomB);
	check(cellA && cellB);

	FRectangle intersection = FRectangle::Intersect(cellA->Bounds, cellB->Bounds);
	bool adjacent = (intersection.Width() > 0 || intersection.Height() > 0);
	if (adjacent)
	{
		FIntVector doorPointA;
		FIntVector doorPointB;
		doorPointA.Z = cellA->Bounds.Location.Z;
		doorPointB.Z = cellB->Bounds.Location.Z;
		if (intersection.Width() > 0)
		{
			// shares a horizontal edge
			doorPointA.X = intersection.X() + intersection.Width() / 2;
			doorPointA.Y = intersection.Y() - 1;

			doorPointB.X = doorPointA.X;
			doorPointB.Y = doorPointA.Y + 1;
		}
		else
		{
			// shares a vertical edge
			doorPointA.X = intersection.X() - 1;
			doorPointA.Y = intersection.Y() + intersection.Height() / 2;

			doorPointB.X = doorPointA.X + 1;
			doorPointB.Y = doorPointA.Y;
		}

		// Add a door and return (no corridors needed for adjacent rooms)
		gridModel->DoorManager.CreateDoor(doorPointA, doorPointB, roomA, roomB);
	}
}

TArray<FCellDoor> UGridDungeonBuilder::GetDoors() {
	return gridModel->DoorManager.GetDoors();
}

int32 UGridDungeonBuilder::RegisterCorridorCell(int cellX, int cellY, int cellZ, int32 roomA, int32 roomB, bool canRegisterDoors) {
	FCell* cellA = gridModel->GetCell(roomA);
	FCell* cellB = gridModel->GetCell(roomB);
	check(roomA && roomB);

	FRectangle PaddingBounds(cellX, cellY, 1, 1);
	PaddingBounds.Location.Z = cellZ;

	if (cellA->Bounds.Contains(PaddingBounds.Location) || cellB->Bounds.Contains(PaddingBounds.Location)) {
		// ignore
		return -1;
	}

	bool bRequiresPadding = true;
	int32 CurrentCellId = -1;
	for (FCell& cell : gridModel->Cells)
	{
		if (cell.Bounds.Contains(PaddingBounds.Location))
		{
			if (cell.Id == roomA || cell.Id == roomB)
			{
				// collides with inside of the room.
				return -1;
			}

			if (cell.CellType == FCellType::Unknown)
			{
				// Convert this cell into a corridor 
				cell.CellType = FCellType::Corridor;
			}

			// Intersects with an existing cell. do not add corridor padding
			bRequiresPadding = false;
			CurrentCellId = cell.Id;
			break;
		}
	}

	if (bRequiresPadding) {
		FCell corridorCell;
		corridorCell.Id = GetNextCellId();
		corridorCell.UserDefined = false;
		corridorCell.Bounds = PaddingBounds;
		corridorCell.CellType = FCellType::CorridorPadding;
		gridModel->Cells.Add(corridorCell);
		gridModel->BuildCellLookup();

		CurrentCellId = corridorCell.Id;
	}


	if (canRegisterDoors)
	{
		// Check if we are adjacent to to any of the room nodes
		if (AreCellsAdjacent(CurrentCellId, roomA))
		{
			ConnectAdjacentCells(CurrentCellId, roomA);
		}
		if (AreCellsAdjacent(CurrentCellId, roomB))
		{
			ConnectAdjacentCells(CurrentCellId, roomB);
		}
	}

	return CurrentCellId;    // Return the cell id of the registered cell
}

void UGridDungeonBuilder::ConnectRooms(int32 roomAId, int32 roomBId) {
	FCell* roomA = gridModel->GetCell(roomAId);
	FCell* roomB = gridModel->GetCell(roomBId);
	check(roomA && roomB);

	FRectangle intersection = FRectangle::Intersect(roomA->Bounds, roomB->Bounds);
	bool adjacent = (intersection.Width() > 0 || intersection.Height() > 0);
	if (adjacent)
	{
		ConnectAdjacentCells(roomAId, roomBId);
	}
	else
	{
		// Create a corridor segment as the rooms are not touching each other
		FIntVector centerA = roomA->Center();
		FIntVector centerB = roomB->Center();

		int32 LaneThicknessLeft = 0;
		int32 LaneThicknessRight = 0;
		// Calculate the lane thickness
		{
			int32 width = FMath::Max(gridConfig->LaneWidth, 1);	// We need a with of at least one 
			width = gridConfig->LaneWidth - 1;	// Since we draw a line in the center regardless of the thickness value
			LaneThicknessRight = width / 2;	// Will be an integer division (truncated)
			LaneThicknessLeft = width - LaneThicknessRight;
		}

		// Sweep X axis
		{
			int32 sourceX = centerA.X;
			int32 targetX = centerB.X;
			int32 sourceY = centerA.Y;
			int32 direction = FMath::Sign(targetX - sourceX);
			int32 PreviousCellId = -1;
			for (int x = sourceX; x != targetX; x += direction)
			{
				int y = sourceY;
				int z = 0;
				// add a corridor cell
				int32 CurrentCellId = RegisterCorridorCell(x, y, z, roomAId, roomBId, true);

				// Check if we need to create a door between the two.  
				// This is needed in case we have an extra room through the corridor.  
				// This room needs to have doors created
				{
					if (PreviousCellId != -1 && CurrentCellId != PreviousCellId) {
						FCell* PreviousCell = gridModel->GetCell(PreviousCellId);
						FCell* CurrentCell = gridModel->GetCell(CurrentCellId);
						if (PreviousCell && CurrentCell && PreviousCell != CurrentCell) {
							if (PreviousCell->CellType == FCellType::Room || PreviousCell->CellType == FCellType::Room) {
								ConnectAdjacentCells(PreviousCellId, CurrentCellId);
							}
						}
					}
					PreviousCellId = CurrentCellId;
				}

				for (int i = 1; i <= LaneThicknessLeft; i++) {
					RegisterCorridorCell(x, y - i, z, roomAId, roomBId);
				}
				for (int i = 1; i <= LaneThicknessRight; i++) {
					RegisterCorridorCell(x, y + i, z, roomAId, roomBId);
				}
			}
		}

		// Sweep Y axis
		{
			int32 sourceY = centerA.Y;
			int32 targetY = centerB.Y;
			int32 sourceX = centerB.X;
			int32 direction = FMath::Sign(targetY - sourceY);
			int32 PreviousCellId = -1;

			for (int y = sourceY; y != targetY; y += direction)
			{
				int x = sourceX;
				int z = 0;
				// add a corridor cell
				int32 CurrentCellId = RegisterCorridorCell(x, y, z, roomAId, roomBId, true);

				// Check if we need to create a door between the two.  
				// This is needed in case we have an extra room through the corridor.  
				// This room needs to have doors created
				{
					if (PreviousCellId != -1 && CurrentCellId != PreviousCellId) {
						FCell* PreviousCell = gridModel->GetCell(PreviousCellId);
						FCell* CurrentCell = gridModel->GetCell(CurrentCellId);
						if (PreviousCell && CurrentCell && PreviousCell != CurrentCell) {
							if (PreviousCell->CellType == FCellType::Room || PreviousCell->CellType == FCellType::Room) {
								ConnectAdjacentCells(PreviousCellId, CurrentCellId);
							}
						}
					}
					PreviousCellId = CurrentCellId;
				}

				for (int i = 1; i <= LaneThicknessLeft; i++) {
					RegisterCorridorCell(x - i, y, z, roomAId, roomBId);
				}
				for (int i = 1; i <= LaneThicknessRight; i++) {
					RegisterCorridorCell(x + i, y, z, roomAId, roomBId);
				}
			}
		}
	}

}

DungeonModelBuildState UGridDungeonBuilder::GetBuildState() {
	return gridModel->BuildState;
}

FCell UGridDungeonBuilder::GetCell(int32 Id) {
	FCell* cell = gridModel->GetCell(Id);
	return cell ? *cell : FCell();
}

struct CellHeightNode {
	int32 CellId;
	int32 Height;
	bool MarkForIncrease;
	bool MarkForDecrease;
};

struct CellHeightFrameInfo {
	CellHeightFrameInfo(int32 pCellId, int32 pCurrentHeight) : CellId(pCellId), CurrentHeight(pCurrentHeight) {}
	int32 CellId;
	int32 CurrentHeight;
};

struct StairEdgeInfo {
	StairEdgeInfo(int32 pCellIdA, int32 pCellIdB) : CellIdA(pCellIdA), CellIdB(pCellIdB) {}
	int32 CellIdA;
	int32 CellIdB;
};

bool UGridDungeonBuilder::GetStair(int32 ownerCell, int32 connectedToCell, FStairInfo& outStair) {
	if (gridModel->CellStairs.Contains(ownerCell)) {
		for (const FStairInfo& stair : gridModel->CellStairs[ownerCell]) {
			if (stair.ConnectedToCell == connectedToCell) {
				outStair = stair;
				return true;
			}
		}
	}
	return false;
}

struct StairAdjacencyQueueNode {
	StairAdjacencyQueueNode(int32 pCellId, int32 pDepth) : cellId(pCellId), depth(pDepth) {}
	int32 cellId;
	int32 depth;
};

bool UGridDungeonBuilder::ContainsAdjacencyPath(int32 cellIdA, int32 cellIdB, int32 maxDepth, bool bForceRoomConnection) {
	FCell* cellA = gridModel->GetCell(cellIdA);
	FCell* cellB = gridModel->GetCell(cellIdB);
	if (!cellA || !cellB) return false;

	// Force a connection if any one is a room
	if (bForceRoomConnection && (cellA->CellType == FCellType::Room || cellB->CellType == FCellType::Room)) {
		return false;
	}

	std::queue<StairAdjacencyQueueNode> queue;
	TSet<uint32> visited;
	queue.push(StairAdjacencyQueueNode(cellIdA, 0));

	while (!queue.empty()) {
		StairAdjacencyQueueNode topNode = queue.front(); queue.pop();
		if (topNode.depth > maxDepth) continue;

		int32 topId = topNode.cellId;
		if (visited.Contains(topId)) continue;
		visited.Add(topId);
		if (topId == cellIdB) {
			// Reached the target cell
			return true;
		}
		FCell* top = gridModel->GetCell(topId);
		if (!top) continue;
		for (int32 adjacentCellId : top->AdjacentCells) {
			if (visited.Contains(adjacentCellId)) continue;

			// Check if we have a valid path between these two adjacent cells 
			// (either through same height or by a already registered stair)
			FCell* adjacentCell = gridModel->GetCell(adjacentCellId);

			bool pathExists = (adjacentCell->Bounds.Location.Z == top->Bounds.Location.Z);
			if (!pathExists) {
				// Cells are on different heights.  Check if we have a stair connecting these cells
				FStairInfo stair;
				if (GetStair(topId, adjacentCellId, stair)) {
					pathExists = true;
				}
				if (!pathExists) {
					if (GetStair(adjacentCellId, topId, stair)) {
						pathExists = true;
					}
				}
			}

			if (pathExists) {
				// We sure we are not going through a wall
				if (top->CellType == FCellType::Room || adjacentCell->CellType == FCellType::Room) {
					const bool containsDoor = gridModel->DoorManager.ContainsDoorBetweenCells(top->Id, adjacentCellId);
					pathExists = containsDoor;
				}
			}

			if (pathExists) {
				queue.push(StairAdjacencyQueueNode(adjacentCellId, topNode.depth + 1));
			}
		}
	}
	return false;
}

int UGridDungeonBuilder::GetForceDirectionMultiplier(float a, float b, float a1, float b1)
{
	if (a == b)
	{
		return (a1 < b1) ? -1 : 1;
	}
	return (a < b) ? -1 : 1;
}

FIntVector UGridDungeonBuilder::GetRandomPointInCircle(double radius)
{
	float angle = random.FRand() * PI * 2;
	float u = random.FRand() + random.FRand();
	float r = (u > 1) ? 2 - u : u;
	r *= radius;
	int32 x = FMath::RoundToInt(FMath::Cos(angle) * r);
	int32 y = FMath::RoundToInt(FMath::Sin(angle) * r);
	return FIntVector(x, y, 0);
}

bool UGridDungeonBuilder::AreCellsAdjacent(int32 cellAId, int32 cellBId)
{
	FCell* cellA = gridModel->GetCell(cellAId);
	FCell* cellB = gridModel->GetCell(cellBId);
	check(cellA && cellB);
	FRectangle intersection = FRectangle::Intersect(cellA->Bounds, cellB->Bounds);
	bool adjacent = (intersection.Width() > 0 || intersection.Height() > 0);
	return adjacent;
}

int32 UGridDungeonBuilder::GetRandomRoomSize()
{
	float r = 0;
	while (r <= 0) r = nrandom.NextGaussianFloat(gridConfig->NormalMean, gridConfig->NormalStd);
	float roomSize = gridConfig->MinCellSize + r * (gridConfig->MaxCellSize - gridConfig->MinCellSize);
	return FMath::RoundToInt(roomSize);
}

struct StairConnectionWeight {
	StairConnectionWeight(int32 InPosition, int32 InWeight) : position(InPosition), weight(InWeight) {}
	int32 position;
	int32 weight;

	bool operator<(const StairConnectionWeight& other) const {
		return weight > other.weight;
	}
};

void UGridDungeonBuilder::ConnectStairs(int32 WeightThreshold) {
	if (gridModel->Cells.Num() == 0) return;
	TSet<uint32> visited;
	TSet<int32> islandVisited;  // The node visited, to track multiple isolated islands
	// Loop through all the nodes, pick only one node from an island.  The entire island is then processed within the inner while loop with BFS
	for (int i = 0; i < gridModel->Cells.Num(); i++) {
		const FCell& start = gridModel->Cells[i];
		if (islandVisited.Contains(start.Id)) {
			// This island has already been processed
			continue;
		}
		std::stack<StairEdgeInfo> stack;
		stack.push(StairEdgeInfo(-1, start.Id));
		while (!stack.empty()) {
			StairEdgeInfo top = stack.top(); stack.pop();
			if (top.CellIdA >= 0) {
				int32 hash1 = HASH(top.CellIdA, top.CellIdB);
				int32 hash2 = HASH(top.CellIdB, top.CellIdA);
				if (visited.Contains(hash1) || visited.Contains(hash2)) {
					// Already processed
					continue;
				}
				// Mark as processed
				visited.Add(hash1);
				visited.Add(hash2);

				islandVisited.Add(top.CellIdA);
				islandVisited.Add(top.CellIdB);

				// Check if it is really required to place a stair here.  There might be other paths nearby to this cell
				bool pathExists = ContainsAdjacencyPath(top.CellIdA, top.CellIdB, gridConfig->StairConnectionTollerance, true);
				if (!pathExists) {
					// Process the edge
					FCell* cellA = gridModel->GetCell(top.CellIdA);
					FCell* cellB = gridModel->GetCell(top.CellIdB);
					if (!cellA || !cellB) continue;
					if (cellA->Bounds.Location.Z != cellB->Bounds.Location.Z) {
						bool bothRooms = false;
						if (cellA->CellType == FCellType::Room && cellB->CellType == FCellType::Room) {
							bothRooms = true;

						}
						// Find the intersecting line
						FRectangle intersection = FRectangle::Intersect(cellA->Bounds, cellB->Bounds);
						if (intersection.Size.X > 0) {
							bool cellAAbove = (cellA->Bounds.Location.Z > cellB->Bounds.Location.Z);
							FCell* stairOwner = (cellAAbove ? cellB : cellA);
							FCell* stairConnectedTo = (!cellAAbove ? cellB : cellA);

							if (ContainsStair(stairOwner->Id, stairConnectedTo->Id)) {
								// Stair already exists here. Move to the next one
								continue;
							}

							bool cellOwnerOnLeft = (stairOwner->Bounds.Center().Y < intersection.Location.Y);

							bool foundValidStairLocation = false;
							int32 validX = intersection.Location.X;

							int32 validY = intersection.Location.Y;
							if (cellOwnerOnLeft) validY--;

							TArray<StairConnectionWeight> StairConnectionCandidates;
							for (validX = intersection.Location.X; validX < intersection.Location.X + intersection.Size.X; validX++) {
								auto currentPointInfo = gridModel->GetGridCellLookup(validX, validY);
								if (stairOwner->CellType == FCellType::Room || stairConnectedTo->CellType == FCellType::Room) {
									// Make sure the stair is on a door cell
									FGridCellInfo stairCellInfo = gridModel->GetGridCellLookup(validX, validY);
									if (!stairCellInfo.ContainsDoor) {
										// Stair not connected to a door. Probably trying to attach itself to a room wall. ignore
										continue;
									}

									// We have a door here.  A stair case is a must, but first make sure we have a door between these two cells 
									bool hasDoor = gridModel->DoorManager.ContainsDoorBetweenCells(stairOwner->Id, stairConnectedTo->Id);
									if (!hasDoor) continue;

									// Check again in more detail
									auto tz1 = validY;
									auto tz2 = validY - 1;
									if (cellOwnerOnLeft) {
										tz2 = validY + 1;
									}

									hasDoor = gridModel->DoorManager.ContainsDoor(validX, tz1, validX, tz2);
									if (hasDoor) {
										StairConnectionCandidates.Add(StairConnectionWeight(validX, 100));
										foundValidStairLocation = true;
										break;
									}
								}
								else {	// Both the cells are non-rooms (corridors)
									int32 weight = 0;

									FGridCellInfo cellInfo0 = gridModel->GetGridCellLookup(validX, validY - 1);
									FGridCellInfo cellInfo1 = gridModel->GetGridCellLookup(validX, validY + 1);
									weight += (cellInfo0.CellType != FCellType::Unknown) ? 10 : 0;
									weight += (cellInfo1.CellType != FCellType::Unknown) ? 10 : 0;

									if (currentPointInfo.ContainsDoor) {
										// Increase the weight if we connect into a door
										int adjacentY = cellOwnerOnLeft ? (validY - 1) : (validY + 1);
										bool ownerOnDoor = gridModel->DoorManager.ContainsDoor(validX, validY, validX, adjacentY);
										if (ownerOnDoor) {
											// Connect to this
											weight += 100;
										}
										else {
											// Add a penalty if we are creating a stair blocking a door entry/exit
											weight -= 100;
										}
									}
									else {
										// Make sure we don't connect to a wall
										int adjacentY = cellOwnerOnLeft ? (validY - 1) : (validY + 1);
										FGridCellInfo adjacentOwnerCellInfo = gridModel->GetGridCellLookup(validX, adjacentY);
										if (adjacentOwnerCellInfo.CellType == FCellType::Room) {
											// We are connecting to a wall. Add a penalty
											weight -= 100;
										}
									}

									// Check the side of the stairs to see if we are not blocking a stair entry / exit
									if (gridModel->ContainsStairAtLocation(validX - 1, validY)) {
										weight -= 60;
									}
									if (gridModel->ContainsStairAtLocation(validX + 1, validY)) {
										weight -= 60;
									}

									StairConnectionCandidates.Add(StairConnectionWeight(validX, weight));
								}
							}

							// Create a stair if necessary
							if (StairConnectionCandidates.Num() > 0) {
								StairConnectionCandidates.Sort();
								StairConnectionWeight candidate = StairConnectionCandidates[0];
								if (candidate.weight < WeightThreshold) {
									continue;
								}

								validX = candidate.position;
								int stairZ = stairOwner->Bounds.Location.Z;
								int paddingOffset = (stairOwner->Bounds.Y() > stairConnectedTo->Bounds.Y()) ? 1 : -1;
								// Add a corridor padding here
								for (int dx = -1; dx <= 1; dx++) {
									bool requiresPadding = false;
									if (dx == 0) {
										requiresPadding = true;
									}
									else {
										auto cellInfo = gridModel->GetGridCellLookup(validX + dx, validY);
										if (cellInfo.CellType != FCellType::Unknown) {
											requiresPadding = true;
										}
									}

									if (requiresPadding) {
										auto paddingInfo = gridModel->GetGridCellLookup(validX + dx, validY + paddingOffset);
										if (paddingInfo.CellType == FCellType::Unknown) {
											AddCorridorPadding(validX + dx, validY + paddingOffset, stairZ);
										}
									}
								}
								gridModel->BuildCellLookup();
								GenerateAdjacencyLookup();
							}
							else {
								continue;
							}

							float validZ = stairOwner->Bounds.Location.Z;
							FVector StairLocation(validX, validY, validZ);
							StairLocation += FVector(0.5f, 0.5f, 0);
							StairLocation *= GridToMeshScale;

							FQuat StairRotation = FQuat::Identity;
							StairRotation = FQuat(FVector(0, 0, 1), cellOwnerOnLeft ? -PI / 2 : PI / 2);

							if (!gridModel->CellStairs.Contains(stairOwner->Id)) {
								gridModel->CellStairs.Add(stairOwner->Id, TArray<FStairInfo>());
							}
							FStairInfo Stair;
							Stair.OwnerCell = stairOwner->Id;
							Stair.ConnectedToCell = stairConnectedTo->Id;
							Stair.Position = StairLocation;
							Stair.Rotation = StairRotation;
							Stair.IPosition = FIntVector(validX, validY, validZ);
							gridModel->CellStairs[stairOwner->Id].Add(Stair);
						}
						else if (intersection.Size.Y > 0) {
							bool cellAAbove = (cellA->Bounds.Location.Z > cellB->Bounds.Location.Z);

							FCell* stairOwner = (cellAAbove ? cellB : cellA);
							FCell* stairConnectedTo = (!cellAAbove ? cellB : cellA);

							if (ContainsStair(stairOwner->Id, stairConnectedTo->Id)) {
								// Stair already exists here. Move to the next one
								continue;
							}

							bool cellOwnerOnLeft = (stairOwner->Bounds.Center().X < intersection.Location.X);

							float validX = intersection.Location.X;
							if (cellOwnerOnLeft) validX--;

							float validY = intersection.Location.Y;
							bool foundValidStairLocation = false;

							TArray<StairConnectionWeight> StairConnectionCandidates;
							for (validY = intersection.Location.Y; validY < intersection.Location.Y + intersection.Size.Y; validY++) {
								auto currentPointInfo = gridModel->GetGridCellLookup(validX, validY);
								if (stairOwner->CellType == FCellType::Room || stairConnectedTo->CellType == FCellType::Room) {
									// Make sure the stair is on a door cell
									FGridCellInfo stairCellInfo = gridModel->GetGridCellLookup(validX, validY);
									if (!stairCellInfo.ContainsDoor) {
										// Stair not connected to a door. Probably trying to attach itself to a room wall. ignore
										continue;
									}

									// We have a door here.  A stair case is a must, but first make sure we have a door between these two cells 
									bool hasDoor = gridModel->DoorManager.ContainsDoorBetweenCells(stairOwner->Id, stairConnectedTo->Id);
									if (!hasDoor) continue;

									// Check again in more detail
									auto tx1 = validX;
									auto tx2 = validX - 1;
									if (cellOwnerOnLeft) {
										tx2 = validX + 1;
									}

									hasDoor = gridModel->DoorManager.ContainsDoor(tx1, validY, tx2, validY);
									if (hasDoor) {
										StairConnectionCandidates.Add(StairConnectionWeight(validY, 100));
										foundValidStairLocation = true;
										break;
									}
								}
								else {	// Both the cells are non-rooms (corridors)
									int32 weight = 0;

									FGridCellInfo cellInfo0 = gridModel->GetGridCellLookup(validX - 1, validY);
									FGridCellInfo cellInfo1 = gridModel->GetGridCellLookup(validX + 1, validY);
									weight += (cellInfo0.CellType != FCellType::Unknown) ? 10 : 0;
									weight += (cellInfo1.CellType != FCellType::Unknown) ? 10 : 0;

									if (currentPointInfo.ContainsDoor) {
										// Increase the weight if we connect into a door
										int adjacentX = cellOwnerOnLeft ? (validX - 1) : (validX + 1);
										bool ownerOnDoor = gridModel->DoorManager.ContainsDoor(validX, validY, adjacentX, validY);
										if (ownerOnDoor) {
											// Connect to this
											weight += 100;
										}
										else {
											// Add a penalty if we are creating a stair blocking a door entry/exit
											weight -= 100;
										}
									}
									else {
										// Make sure we don't connect to a wall
										int adjacentX = cellOwnerOnLeft ? (validX - 1) : (validX + 1);
										FGridCellInfo adjacentOwnerCellInfo = gridModel->GetGridCellLookup(adjacentX, validY);
										if (adjacentOwnerCellInfo.CellType == FCellType::Room) {
											// We are connecting to a wall. Add a penalty
											weight -= 100;
										}
									}

									// Check the side of the stairs to see if we are not blocking a stair entry / exit
									if (gridModel->ContainsStairAtLocation(validX, validY - 1)) {
										weight -= 60;
									}
									if (gridModel->ContainsStairAtLocation(validX, validY + 1)) {
										weight -= 60;
									}

									StairConnectionCandidates.Add(StairConnectionWeight(validY, weight));
								}
							}

							// Connect the stairs if necessary
							if (StairConnectionCandidates.Num() > 0) {
								StairConnectionCandidates.Sort();
								StairConnectionWeight candidate = StairConnectionCandidates[0];
								if (candidate.weight < WeightThreshold) {
									continue;
								}
								validY = candidate.position;
								int stairZ = stairOwner->Bounds.Location.Z;
								int paddingOffset = (stairOwner->Bounds.X() > stairConnectedTo->Bounds.X()) ? 1 : -1;
								// Add a corridor padding here
								for (int dy = -1; dy <= 1; dy++) {
									bool requiresPadding = false;
									if (dy == 0) {
										requiresPadding = true;
									}
									else {
										auto cellInfo = gridModel->GetGridCellLookup(validX, validY + dy);
										if (cellInfo.CellType != FCellType::Unknown) {
											requiresPadding = true;
										}
									}

									if (requiresPadding) {
										auto paddingInfo = gridModel->GetGridCellLookup(validX + paddingOffset, validY + dy);
										if (paddingInfo.CellType == FCellType::Unknown) {
											AddCorridorPadding(validX + paddingOffset, validY + dy, stairZ);
										}
									}
								}
								gridModel->BuildCellLookup();
								//gridModel.BuildSpatialCellLookup();
								GenerateAdjacencyLookup();
							}
							else {
								continue;
							}


							float validZ = stairOwner->Bounds.Location.Z;
							FVector StairLocation(validX, validY, validZ);
							StairLocation += FVector(0.5f, 0.5f, 0);
							StairLocation *= GridToMeshScale;

							FQuat StairRotation = FQuat::Identity;
							StairRotation = FQuat(FVector(0, 0, 1), cellOwnerOnLeft ? PI : 0);

							if (!gridModel->CellStairs.Contains(stairOwner->Id)) {
								gridModel->CellStairs.Add(stairOwner->Id, TArray<FStairInfo>());
							}
							FStairInfo Stair;
							Stair.OwnerCell = stairOwner->Id;
							Stair.ConnectedToCell = stairConnectedTo->Id;
							Stair.Position = StairLocation;
							Stair.Rotation = StairRotation;
							Stair.IPosition = FIntVector(validX, validY, validZ);
							gridModel->CellStairs[stairOwner->Id].Add(Stair);
						}
					}
				}
			}

			// Move to the next adjacent nodes
			FCell* cellB = gridModel->GetCell(top.CellIdB);
			if (!cellB) continue;
			for (int32 adjacentCell : cellB->AdjacentCells) {
				int32 hash1 = HASH(cellB->Id, adjacentCell);
				int32 hash2 = HASH(adjacentCell, cellB->Id);
				if (visited.Contains(hash1) || visited.Contains(hash2)) continue;
				StairEdgeInfo edge(top.CellIdB, adjacentCell);
				stack.push(edge);
			}
		}
	}
}

void UGridDungeonBuilder::GenerateDungeonHeights() {
	// build the adjacency graph in memory
	if (gridModel->Cells.Num() == 0) return;
	TMap<int32, CellHeightNode> CellHeightNodes;

	TSet<int32> visited;
	for (const FCell& startCell : gridModel->Cells) {
		std::stack<CellHeightFrameInfo> stack;
		if (visited.Contains(startCell.Id)) {
			continue;
		}
		stack.push(CellHeightFrameInfo(startCell.Id, startCell.Bounds.Location.Z));

		while (!stack.empty()) {
			CellHeightFrameInfo top = stack.top(); stack.pop();
			if (visited.Contains(top.CellId)) continue;
			visited.Add(top.CellId);

			FCell* cell = gridModel->GetCell(top.CellId);
			if (!cell) continue;

			bool applyHeightVariation = (cell->Bounds.Size.X > 1 && cell->Bounds.Size.Y > 1);
			applyHeightVariation &= (cell->CellType != FCellType::Room && cell->CellType != FCellType::CorridorPadding);
			applyHeightVariation &= !cell->UserDefined;

			if (applyHeightVariation) {
				float rand = random.FRand();
				if (rand < gridConfig->HeightVariationProbability / 2.0f) {
					top.CurrentHeight--;
				}
				else if (rand < gridConfig->HeightVariationProbability) {
					top.CurrentHeight++;
				}
			}
			if (cell->UserDefined) {
				top.CurrentHeight = cell->Bounds.Location.Z;
			}

			CellHeightNode node;
			node.CellId = cell->Id;
			node.Height = top.CurrentHeight;
			node.MarkForIncrease = false;
			node.MarkForDecrease = false;
			CellHeightNodes.Add(node.CellId, node);

			// Add the child nodes
			for (int32 childId : cell->AdjacentCells) {
				if (visited.Contains(childId)) continue;
				stack.push(CellHeightFrameInfo(childId, top.CurrentHeight));
			}
		}
	}

	// Fix the dungeon heights
	const int32 FIX_MAX_TRIES = 50;	// TODO: Move to gridConfig
	int32 fixIterations = 0;
	while (fixIterations < FIX_MAX_TRIES && FixDungeonCellHeights(CellHeightNodes)) {
		fixIterations++;
	}

	// Assign the calculated heights
	for (FCell& cell : gridModel->Cells) {
		if (CellHeightNodes.Contains(cell.Id)) {
			const CellHeightNode& node = CellHeightNodes[cell.Id];
			cell.Bounds.Location.Z = node.Height;
		}
	}
}

bool UGridDungeonBuilder::FixDungeonCellHeights(TMap<int32, CellHeightNode>& CellHeightNodes)
{
	bool bContinueIteration = false;
	if (gridModel->Cells.Num() == 0) return bContinueIteration;

	TSet<int32> visited;
	std::stack<int32> stack;
	const FCell& rootCell = gridModel->Cells[0];
	stack.push(rootCell.Id);
	while (!stack.empty()) {
		int32 cellId = stack.top(); stack.pop();
		if (visited.Contains(cellId)) continue;
		visited.Add(cellId);

		FCell* cell = gridModel->GetCell(cellId);
		if (!cell) continue;

		if (!CellHeightNodes.Contains(cellId)) continue;
		CellHeightNode& heightNode = CellHeightNodes[cellId];

		heightNode.MarkForIncrease = false;
		heightNode.MarkForDecrease = false;

		// Check if the adjacent cells have unreachable heights
		for (int32 childId : cell->AdjacentCells) {
			FCell* childCell = gridModel->GetCell(childId);
			if (!childCell || !CellHeightNodes.Contains(childId)) continue;
			CellHeightNode& childHeightNode = CellHeightNodes[childId];
			int32 heightDifference = FMath::Abs(childHeightNode.Height - heightNode.Height);
			if (heightDifference > gridConfig->MaxAllowedStairHeight) {
				if (heightNode.Height > childHeightNode.Height) {
					heightNode.MarkForDecrease = true;
				}
				else {
					heightNode.MarkForIncrease = true;
				}
				break;
			}
		}

		// Add the child nodes
		for (int32 childId : cell->AdjacentCells) {
			if (visited.Contains(childId)) continue;
			stack.push(childId);
		}
	}


	TArray<int32> HeightCellIds;
	CellHeightNodes.GenerateKeyArray(HeightCellIds);
	bool bHeightChanged = false;
	for (int32 cellId : HeightCellIds) {
		CellHeightNode& heightNode = CellHeightNodes[cellId];
		if (heightNode.MarkForDecrease) {
			heightNode.Height--;
			bHeightChanged = true;
		}
		else if (heightNode.MarkForIncrease) {
			heightNode.Height++;
			bHeightChanged = true;
		}
	}

	// Iterate this function again if the height was changed in this step
	bContinueIteration = bHeightChanged;
	return bContinueIteration;
}

void UGridDungeonBuilder::GenerateAdjacencyLookup() {
	// Cache the cell types based on their positions
	TMap<int32, TMap<int32, FGridCellInfo> >& GridCellInfoLookup = gridModel->GridCellInfoLookup;
	GridCellInfoLookup.Reset();
	for (const FCell& cell : gridModel->Cells) {
		if (cell.CellType == FCellType::Unknown) continue;
		FIntVector basePosition = cell.Bounds.Location;
		FTransform transform = FTransform::Identity;
		for (int dx = 0; dx < cell.Bounds.Size.X; dx++) {
			for (int dy = 0; dy < cell.Bounds.Size.Y; dy++) {
				int32 x = basePosition.X + dx;
				int32 y = basePosition.Y + dy;

				// register the cell type in the lookup
				if (!GridCellInfoLookup.Contains(x)) GridCellInfoLookup.Add(x, TMap<int32, FGridCellInfo>());
				GridCellInfoLookup[x].Add(y, FGridCellInfo(cell.Id, cell.CellType));
			}
		}
	}

	// Create cell adjacency list
	for (FCell& cell : gridModel->Cells) {
		if (cell.CellType == FCellType::Unknown) continue;
		FIntVector basePosition = cell.Bounds.Location;
		FTransform transform = FTransform::Identity;
		int32 SizeX = cell.Bounds.Size.X;
		int32 SizeY = cell.Bounds.Size.Y;
		for (int dx = 0; dx < SizeX; dx++) {
			for (int dy = 0; dy < SizeY; dy++) {
				if (dx >= 0 && dx < SizeX - 1 && dy >= 0 && dy < SizeY - 1) {
					// Ignore the cells in the middle
					continue;
				}

				int32 x = cell.Bounds.Location.X + dx;
				int32 y = cell.Bounds.Location.Y + dy;
				CheckAndMarkAdjacent(cell, x + 1, y);
				CheckAndMarkAdjacent(cell, x, y + 1);
			}
		}
	}

	// Cache the positions of the doors in the grid
	for (const FCellDoor& Door : GetDoors()) {
		int32 x0 = Door.AdjacentTiles[0].X;
		int32 y0 = Door.AdjacentTiles[0].Y;
		int32 x1 = Door.AdjacentTiles[1].X;
		int32 y1 = Door.AdjacentTiles[1].Y;
		if (GridCellInfoLookup.Contains(x0) && GridCellInfoLookup[x0].Contains(y0)) GridCellInfoLookup[x0][y0].ContainsDoor = true;
		if (GridCellInfoLookup.Contains(x1) && GridCellInfoLookup[x1].Contains(y1)) GridCellInfoLookup[x1][y1].ContainsDoor = true;
	}


}

void UGridDungeonBuilder::CheckAndMarkAdjacent(FCell& cell, int32 otherCellX, int32 otherCellY) {
	FGridCellInfo info = gridModel->GetGridCellLookup(otherCellX, otherCellY);
	if (info.CellId == cell.Id) return;
	FCell* otherCell = gridModel->GetCell(info.CellId);
	if (!otherCell) return;
	if (otherCell->CellType == FCellType::Unknown || cell.CellType == FCellType::Unknown) return;

	// Mark the two cells as adjacent
	cell.AdjacentCells.Add(otherCell->Id);
	otherCell->AdjacentCells.Add(cell.Id);
}

void UGridDungeonBuilder::DrawDebugData(UWorld* InWorld, bool bPersistant, float lifeTime) {
	if (!gridModel || !gridConfig) return;
	for (const FCell& cell : gridModel->Cells) {
		if (cell.CellType == FCellType::Unknown) continue;
		FVector Location = UDungeonModelHelper::MakeVector(cell.Bounds.Location);
		FVector Size = UDungeonModelHelper::MakeVector(cell.Bounds.Size);
		Size /= 2;
		Location += Size;
		Location *= GridToMeshScale;
		Size *= GridToMeshScale;
		Size += FVector(0, 0, 5);
		FColor Color = FColor::White;
		switch (cell.CellType) {
		case FCellType::Room:
			Color = FColor::Red;
			break;

		case FCellType::Corridor:
			Color = FColor(0, 255, 255);
			break;

		case FCellType::CorridorPadding:
			Color = FColor(0, 0, 255);
			break;
		}

		FColor SolidColor = Color;
		SolidColor.A = 32;

		DrawDebugSolidBox(InWorld, Location, Size, SolidColor, bPersistant, lifeTime);
		DrawDebugBox(InWorld, Location, Size, Color, bPersistant, lifeTime);

		std::stringstream message;
		message << Location.Z << " [" << cell.Id << "]";
		//FString HeightText = FString::FormatAsNumber(Location.Z);
		FString HeightText = message.str().c_str();
		DrawDebugString(InWorld, Location, HeightText, NULL, Color.White, lifeTime);

		// Debug draw the adjacency list
		for (int32 adjacentCellId : cell.AdjacentCells) {
			FCell* adjacentCell = gridModel->GetCell(adjacentCellId);
			if (!adjacentCell) continue;
			FVector CellCenter0, CellCenter1;
			UGridDungeonModelHelper::GetCellCenter(cell, CellCenter0);
			UGridDungeonModelHelper::GetCellCenter(*adjacentCell, CellCenter1);
			CellCenter0 *= GridToMeshScale;
			CellCenter1 *= GridToMeshScale;
			DrawDebugLine(InWorld, CellCenter0, CellCenter1, FColor(255, 128, 0), bPersistant, lifeTime, 0, 20);
		}
	}

	// Draw room connections
	TArray<FCell*> rooms = GetCellsOfType(FCellType::Room);
	for (FCell* room : rooms) {
		for (int32 otherDelauney : room->ConnectedRooms) {
			FCell* other = gridModel->GetCell(otherDelauney);
			if (other) {
				bool bPreferedEdge = room->FixedRoomConnections.Contains(otherDelauney);
				FColor color = bPreferedEdge ? FColor::Red : FColor::Green;
				FVector CellCenter0, CellCenter1;
				UGridDungeonModelHelper::GetCellCenter(*room, CellCenter0);
				UGridDungeonModelHelper::GetCellCenter(*other, CellCenter1);
				CellCenter0 *= GridToMeshScale;
				CellCenter1 *= GridToMeshScale;
				DrawDebugLine(InWorld, CellCenter0, CellCenter1, color, bPersistant, lifeTime, 0, 40);
			}
		}
	}

	for (const FCellDoor& door : GetDoors()) {
		FVector Start = UDungeonModelHelper::MakeVector(door.AdjacentTiles[0]);
		FVector End = UDungeonModelHelper::MakeVector(door.AdjacentTiles[1]);

		Start += FVector(0.5f, 0.5f, 0);
		End += FVector(0.5f, 0.5f, 0);
		Start *= GridToMeshScale;
		End *= GridToMeshScale;
		DrawDebugLine(InWorld, Start, End, FColor::Green, bPersistant, lifeTime, 0, 20);
	}
}

void UGridDungeonBuilder::MirrorDungeon()
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
				
				// Cache the cell types based on their positions
				TMap<int32, TMap<int32, FGridCellInfo> >& GridCellInfoLookup = gridModel->GridCellInfoLookup;
				GridCellInfoLookup.Reset();
				for (const FCell& cell : gridModel->Cells) {
					if (cell.CellType == FCellType::Unknown) continue;
					FIntVector basePosition = cell.Bounds.Location;
					FTransform transform = FTransform::Identity;
					for (int dx = 0; dx < cell.Bounds.Size.X; dx++) {
						for (int dy = 0; dy < cell.Bounds.Size.Y; dy++) {
							int32 x = basePosition.X + dx;
							int32 y = basePosition.Y + dy;

							// register the cell type in the lookup
							if (!GridCellInfoLookup.Contains(x)) GridCellInfoLookup.Add(x, TMap<int32, FGridCellInfo>());
							GridCellInfoLookup[x].Add(y, FGridCellInfo(cell.Id, cell.CellType));
						}
					}
				}

				gridModel->BuildCellLookup();
			}
		}
	}
}

TSubclassOf<UDungeonModel> UGridDungeonBuilder::GetModelClass()
{
	return UGridDungeonModel::StaticClass();
}

TSubclassOf<UDungeonConfig> UGridDungeonBuilder::GetConfigClass()
{
	return UGridDungeonConfig::StaticClass();
}

TSubclassOf<UDungeonToolData> UGridDungeonBuilder::GetToolDataClass()
{
	return UGridDungeonToolData::StaticClass();
}

bool UGridDungeonBuilder::ProcessSpatialConstraint(UDungeonSpatialConstraint* SpatialConstraint, const FTransform& Transform, FQuat& OutRotationOffset)
{
	if (UGridSpatialConstraint3x3* Constraint = Cast<UGridSpatialConstraint3x3>(SpatialConstraint)) {
		return ProcessSpatialConstraint3x3(Constraint, Transform, OutRotationOffset);
	}
	if (UGridSpatialConstraint2x2* Constraint = Cast<UGridSpatialConstraint2x2>(SpatialConstraint)) {
		return ProcessSpatialConstraint2x2(Constraint, Transform, OutRotationOffset);
	}
	if (UGridSpatialConstraintEdge* Constraint = Cast<UGridSpatialConstraintEdge>(SpatialConstraint)) {
		return ProcessSpatialConstraintEdge(Constraint, Transform, OutRotationOffset);
	}
	return false;
}

void UGridDungeonBuilder::GetDefaultMarkerNames(TArray<FString>& OutMarkerNames)
{
	OutMarkerNames.Reset();
	OutMarkerNames.Add("Ground");
	OutMarkerNames.Add("Wall");
	OutMarkerNames.Add("WallSeparator");
	OutMarkerNames.Add("Fence");
	OutMarkerNames.Add("FenceSeparator");
	OutMarkerNames.Add("Door");
	OutMarkerNames.Add("Stair");
	OutMarkerNames.Add("Stair2X");
	OutMarkerNames.Add("WallHalf");
	OutMarkerNames.Add("WallHalfSeparator");
}

bool UGridDungeonBuilder::ContainsCell(int32 x, int32 y)
{
	return gridModel->GridCellInfoLookup.Contains(x) && gridModel->GridCellInfoLookup[x].Contains(y);
}

void UGridDungeonBuilder::GetRooms(TArray<FCell>& RoomCells)
{
	GetCellsOfType(FCellType::Room, RoomCells);
}

void UGridDungeonBuilder::GetCorridors(TArray<FCell>& CorridorCells)
{
	GetCellsOfType(FCellType::Corridor, CorridorCells);
	GetCellsOfType(FCellType::CorridorPadding, CorridorCells);
}

void UGridDungeonBuilder::GetCellsOfType(FCellType CellType, TArray<FCell>& Cells)
{
	for (const FCell& cell : gridModel->Cells) {
		if (cell.CellType == CellType) {
			Cells.Add(cell);
		}
	}
}

void UGridDungeonBuilder::BuildMesh_Floor(const FCell& cell) {
	FIntVector basePosition = cell.Bounds.Location;
	FTransform transform = FTransform::Identity;
	for (int dx = 0; dx < cell.Bounds.Size.X; dx++) {
		for (int dy = 0; dy < cell.Bounds.Size.Y; dy++) {
			int32 x = basePosition.X + dx;
			int32 y = basePosition.Y + dy;
			int32 z = basePosition.Z;

			FVector position(x, y, z);
			position += FVector(0.5f, 0.5f, 0);
			position *= GridToMeshScale;
			transform.SetLocation(position);

			AddPropSocket(ST_GROUND, transform);
		}
	}
}

/*
void UGridDungeonBuilder::DrawDebugSockets(const TArray<FNamedPropSocket>& sockets) {
for (const FNamedPropSocket& socket : sockets) {
FVector Location = socket.Transform.GetLocation();
DrawDebugSphere(GetWorld(), Location, 50, 8, FColor::Green, true, 1000);
DrawDebugString(GetWorld(), Location, socket.SocketType, NULL, FColor::White, 1000);
}
}
*/

void UGridDungeonBuilder::BuildMesh_RoomDecoration(const FCell& cell) {
	FIntVector ILocation = cell.Bounds.Location;
	FIntVector ISize = cell.Bounds.Size;
	FVector Position = UDungeonModelHelper::MakeVector(ILocation) * GridToMeshScale;
	FVector Size = UDungeonModelHelper::MakeVector(ISize) * GridToMeshScale;
	FVector Center = Position + Size / 2.0f;

	FTransform transform = FTransform::Identity;
	int32 x = ILocation.X;
	int32 y = ILocation.Y;
	int32 z = ILocation.Z;
	for (int32 dx = 0; dx < ISize.X; dx++) {
		FVector Location = FVector(x + dx, y, z);
		transform.SetRotation(FQuat(FVector(0, 0, 1), 0));
		transform.SetLocation(Location * GridToMeshScale);
		if (dx > 0) {
			AddPropSocket(ST_ROOMWALLSEPARATOR, transform);
		}
		Location += FVector(0.5f, 0, 0);
		transform.SetLocation(Location * GridToMeshScale);
		AddPropSocket(ST_ROOMWALL, transform);

		Location = FVector(x + dx, y + ISize.Y, z);
		transform.SetRotation(FQuat(FVector(0, 0, 1), PI));
		transform.SetLocation(Location * GridToMeshScale);
		if (dx > 0) {
			AddPropSocket(ST_ROOMWALLSEPARATOR, transform);
		}
		Location += FVector(0.5f, 0, 0);
		transform.SetLocation(Location * GridToMeshScale);
		AddPropSocket(ST_ROOMWALL, transform);
	}

	for (int32 dy = 0; dy < ISize.Y; dy++) {
		FVector Location = FVector(x, y + dy, z);
		transform.SetLocation(Location * GridToMeshScale);
		transform.SetRotation(FQuat(FVector(0, 0, 1), -PI / 2));
		if (dy > 0) {
			AddPropSocket(ST_ROOMWALLSEPARATOR, transform);
		}
		Location += FVector(0, 0.5f, 0);
		transform.SetLocation(Location * GridToMeshScale);
		AddPropSocket(ST_ROOMWALL, transform);

		Location = FVector(x + ISize.X, y + dy, z);
		transform.SetRotation(FQuat(FVector(0, 0, 1), PI / 2));
		transform.SetLocation(Location * GridToMeshScale);
		if (dy > 0) {
			AddPropSocket(ST_ROOMWALLSEPARATOR, transform);
		}
		Location += FVector(0, 0.5f, 0);
		transform.SetLocation(Location * GridToMeshScale);
		AddPropSocket(ST_ROOMWALL, transform);
	}

	{
		// Add open space markers
		transform = FTransform::Identity;
		transform.SetLocation(Center);
		AddPropSocket(ST_ROOMOPENSPACE, transform);
	}
	//DrawDebugSockets(RoomSockets);
}

void UGridDungeonBuilder::FixDoorTransform(int32 x0, int32 y0, int32 x1, int32 y1, FTransform& OutTransform)
{
	FCell* cell0 = gridModel->GetCell(gridModel->GetGridCellLookup(x0, y0).CellId);
	FCell* cell1 = gridModel->GetCell(gridModel->GetGridCellLookup(x1, y1).CellId);
	if (!cell0 || !cell1) return;
	float z = FMath::Max(cell0->Bounds.Location.Z, cell1->Bounds.Location.Z);
	FVector Location = OutTransform.GetLocation();
	Location.Z = z * GridToMeshScale.Z;
	//OutTransform.SetLocation(Location);
}

void UGridDungeonBuilder::AddCorridorPadding(int x, int y, int z)
{
	FCell padding;
	padding.Id = GetNextCellId();
	padding.UserDefined = false;

	FRectangle bounds(x, y, 1, 1);
	bounds.Location.Z = z;
	padding.Bounds = bounds;
	padding.CellType = FCellType::CorridorPadding;

	gridModel->Cells.Add(padding);
}

void UGridDungeonBuilder::RemoveStairAtDoor(const FCellDoor& Door)
{
	int32 CellA = Door.AdjacentCells[0];
	int32 CellB = Door.AdjacentCells[1];

	if (gridModel->CellStairs.Contains(CellA)) {
		TArray<FStairInfo>& CellAStairs = gridModel->CellStairs[CellA];
		for (int i = 0; i < CellAStairs.Num(); i++) {
			if (CellAStairs[i].ConnectedToCell == CellB) {
				// Found our cell. Remove it
				CellAStairs.RemoveAt(i);
				break;
			}
		}
	}

	// Remove from the other direction
	if (gridModel->CellStairs.Contains(CellB)) {
		TArray<FStairInfo>& CellBStairs = gridModel->CellStairs[CellB];
		for (int i = 0; i < CellBStairs.Num(); i++) {
			if (CellBStairs[i].ConnectedToCell == CellA) {
				// Found our cell. Remove it
				CellBStairs.RemoveAt(i);
				break;
			}
		}
	}
}

void OffsetTransformZ(const float Z, FTransform& OutTransform) {
	FVector Location = OutTransform.GetLocation();
	Location.Z += Z;
	OutTransform.SetLocation(Location);
}

void UGridDungeonBuilder::BuildMesh_Room(const FCell& cell) {
	BuildMesh_Floor(cell);

	FVector HalfWallOffset = GridToMeshScale * FVector(0, 0, -1);
	// Build the room walls
	FIntVector basePosition = cell.Bounds.Location;
	int32 elevation;

	// build walls along the width
	for (int dx = 0; dx < cell.Bounds.Size.X; dx++) {
		int32 x = basePosition.X + dx;
		int32 y = basePosition.Y;
		int32 z = basePosition.Z;

		FTransform transform = FTransform::Identity;
		FVector position(x, y, z);
		position += FVector(0.5f, 0, 0);
		position *= GridToMeshScale;
		transform.SetLocation(position);
		transform.SetRotation(FQuat(FVector(0, 0, 1), PI));
		int32 OffsetZ = 0;

		bool makeDoor = (gridModel->GetGridCellLookup(x, y).ContainsDoor && gridModel->GetGridCellLookup(x, y - 1).ContainsDoor);
		elevation = GetElevation(cell, x, y - 1, OffsetZ);
		OffsetTransformZ(OffsetZ * GridToMeshScale.Z, transform);

		FString SocketType = makeDoor ? ST_DOOR : ST_WALL;
		AddPropSocket(SocketType, transform);
		AddPropSocket(ST_WALLHALF, transform, elevation, HalfWallOffset);

		// Add the pillar
		transform.SetLocation(FVector(x, y, z) * GridToMeshScale);
		OffsetTransformZ(OffsetZ * GridToMeshScale.Z, transform);
		AddPropSocket(ST_WALLSEPARATOR, transform);
		AddPropSocket(ST_WALLHALFSEPARATOR, transform, elevation, HalfWallOffset);

		y += cell.Bounds.Size.Y;
		elevation = GetElevation(cell, x, y, OffsetZ);
		FGridCellInfo AdjacentCellInfo = gridModel->GetGridCellLookup(x, y);
		if (AdjacentCellInfo.CellType != FCellType::Room) {
			position.Y = y * GridToMeshScale.Y;
			transform.SetLocation(position);
			transform.SetRotation(FQuat(FVector(0, 0, 1), 0));

			makeDoor = (gridModel->GetGridCellLookup(x, y).ContainsDoor && gridModel->GetGridCellLookup(x, y - 1).ContainsDoor);

			SocketType = makeDoor ? ST_DOOR : ST_WALL;
			OffsetTransformZ(OffsetZ * GridToMeshScale.Z, transform);
			AddPropSocket(SocketType, transform);
			AddPropSocket(ST_WALLHALF, transform, elevation, HalfWallOffset);

			// Add the pillar
			transform.SetLocation(FVector(x + 1, y, z) * GridToMeshScale);
			OffsetTransformZ(OffsetZ * GridToMeshScale.Z, transform);
			AddPropSocket(ST_WALLSEPARATOR, transform);
			AddPropSocket(ST_WALLHALFSEPARATOR, transform, elevation, HalfWallOffset);
		}
	}

	// build walls along the length
	for (int dy = 0; dy < cell.Bounds.Size.Y; dy++) {
		int32 x = basePosition.X;
		int32 y = basePosition.Y + dy;
		int32 z = basePosition.Z;

		FTransform transform = FTransform::Identity;
		FVector position(x, y, z);
		position += FVector(0, 0.5f, 0);
		position *= GridToMeshScale;
		transform.SetLocation(position);
		transform.SetRotation(FQuat(FVector(0, 0, 1), PI / 2));
		int32 OffsetZ = 0;

		bool makeDoor = (gridModel->GetGridCellLookup(x, y).ContainsDoor && gridModel->GetGridCellLookup(x - 1, y).ContainsDoor);
		elevation = GetElevation(cell, x - 1, y, OffsetZ);

		FString SocketType = makeDoor ? ST_DOOR : ST_WALL;
		OffsetTransformZ(OffsetZ * GridToMeshScale.Z, transform);
		AddPropSocket(SocketType, transform);
		AddPropSocket(ST_WALLHALF, transform, elevation, HalfWallOffset);

		// Add the pillar
		transform.SetLocation(FVector(x, y + 1, z) * GridToMeshScale);
		OffsetTransformZ(OffsetZ * GridToMeshScale.Z, transform);
		AddPropSocket(ST_WALLSEPARATOR, transform);
		AddPropSocket(ST_WALLHALFSEPARATOR, transform, elevation, HalfWallOffset);

		x += cell.Bounds.Size.X;
		elevation = GetElevation(cell, x, y, OffsetZ);
		FGridCellInfo AdjacentCellInfo = gridModel->GetGridCellLookup(x, y);
		if (AdjacentCellInfo.CellType != FCellType::Room) {
			position.X = x * GridToMeshScale.X;
			transform.SetLocation(position);
			transform.SetRotation(FQuat(FVector(0, 0, 1), -PI / 2));

			makeDoor = (gridModel->GetGridCellLookup(x, y).ContainsDoor && gridModel->GetGridCellLookup(x - 1, y).ContainsDoor);

			SocketType = makeDoor ? ST_DOOR : ST_WALL;
			OffsetTransformZ(OffsetZ * GridToMeshScale.Z, transform);
			AddPropSocket(SocketType, transform);
			AddPropSocket(ST_WALLHALF, transform, elevation, HalfWallOffset);

			// Add the pillar
			transform.SetLocation(FVector(x, y, z) * GridToMeshScale);
			OffsetTransformZ(OffsetZ * GridToMeshScale.Z, transform);
			AddPropSocket(ST_WALLSEPARATOR, transform);
			AddPropSocket(ST_WALLHALFSEPARATOR, transform, elevation, HalfWallOffset);
		}
	}
}

int32 UGridDungeonBuilder::GetElevation(const FCell& baseCell, int32 x, int32 y, int32& OutZOffset) {
	OutZOffset = 0;
	FGridCellInfo info = gridModel->GetGridCellLookup(x, y);
	int32 elevation = gridConfig->FloorHeight;
	if (info.CellType == FCellType::Unknown) return elevation;
	FCell* otherCell = gridModel->GetCell(info.CellId);
	if (!otherCell) {
		return elevation;
	}
	OutZOffset = otherCell->Bounds.Location.Z - baseCell.Bounds.Location.Z;
	elevation = FMath::Max(elevation, FMath::Abs(OutZOffset));
	OutZOffset = FMath::Max(0, OutZOffset);

	//return FMath::Max(elevation, baseCell.Bounds.Location.Z - otherCell->Bounds.Location.Z);
	return elevation;
}

int32 UGridDungeonBuilder::GetStairHeight(const FStairInfo& stair) {
	FCell* owner = gridModel->GetCell(stair.OwnerCell);
	FCell* target = gridModel->GetCell(stair.ConnectedToCell);
	if (!owner || !target) return 1;
	return FMath::Abs(owner->Bounds.Location.Z - target->Bounds.Location.Z);
}

void UGridDungeonBuilder::BuildMesh_Stairs(const FCell& cell) {
	// Draw all the stairs registered with this cell
	if (!gridModel->CellStairs.Contains(cell.Id)) {
		// No stairs registered here
		return;
	}

	for (const FStairInfo stair : gridModel->CellStairs[cell.Id]) {
		FTransform transform = FTransform::Identity;
		transform.SetLocation(stair.Position);
		transform.SetRotation(stair.Rotation);
		int stairHeight = GetStairHeight(stair);
		FString StairType = (stairHeight > 1) ? ST_STAIR2X : ST_STAIR;
		AddPropSocket(StairType, transform);
	}
}

bool UGridDungeonBuilder::ContainsStair(const FCell& baseCell, int32 x, int32 y) {
	FGridCellInfo info = gridModel->GetGridCellLookup(x, y);
	if (info.CellType == FCellType::Unknown) return false;

	FCell* cell = gridModel->GetCell(info.CellId);
	if (!cell) return false;

	FStairInfo stair;
	if (GetStair(cell->Id, baseCell.Id, stair)) {
		FVector IPosition = (stair.Position / GridToMeshScale);
		int32 ix = FMath::FloorToInt(IPosition.X);
		int32 iy = FMath::FloorToInt(IPosition.Y);
		if (ix == x && iy == y) {
			return true;
		}
	}
	return false;
}

bool UGridDungeonBuilder::ContainsStair(int32 ownerCellId, int32 connectedToCellId)
{
	if (!gridModel->CellStairs.Contains(ownerCellId)) {
		return false;
	}

	for (const FStairInfo& stairInfo : gridModel->CellStairs[ownerCellId]) {
		if (stairInfo.ConnectedToCell == connectedToCellId) {
			return true;
		}
	}
	return false;
}

bool UGridDungeonBuilder::CanDrawFence(const FCell& baseCell, int32 x, int32 y, bool& isElevatedFence, bool& drawPillar, int& elevationHeight) {
	FGridCellInfo info = gridModel->GetGridCellLookup(x, y);
	isElevatedFence = false;
	drawPillar = false;
	elevationHeight = 0;
	if (info.CellType == FCellType::Unknown) {
		isElevatedFence = false;
		drawPillar = true;
		return true;
	}
	bool bCanDrawFence = false;
	FCell* otherCell = gridModel->GetCell(info.CellId);
	if (otherCell->Bounds.Location.Z < baseCell.Bounds.Location.Z) {
		isElevatedFence = true;
		elevationHeight = baseCell.Bounds.Location.Z - otherCell->Bounds.Location.Z;
		drawPillar = true;
		// Make sure we don't have a stair between the two cells
		if (!ContainsStair(baseCell, x, y)) {
			bCanDrawFence = true;
		}
	}

	// If the two cells are a room / corridor combo, we don't want a fence here
	if (IsRoomCorridor(baseCell.CellType, info.CellType)) {
		isElevatedFence = false;
		elevationHeight = 0;
		bCanDrawFence = false;
	}
	return bCanDrawFence;
}

void UGridDungeonBuilder::BuildMesh_Corridor(const FCell& cell) {
	BuildMesh_Floor(cell);

	FVector HalfWallOffset = GridToMeshScale * FVector(0, 0, -1);
	FIntVector basePosition = cell.Bounds.Location;
	// build fence along the width
	for (int dx = 0; dx < cell.Bounds.Size.X; dx++) {
		int32 x = basePosition.X + dx;
		int32 y = basePosition.Y;
		int32 z = basePosition.Z;

		int elevationHeight;
		bool isElevatedFence, drawPillar;
		bool drawFence = CanDrawFence(cell, x, y - 1, isElevatedFence, drawPillar, elevationHeight);
		FTransform transform = FTransform::Identity;
		if (drawFence || isElevatedFence) {
			FVector position(x, y, z);
			position += FVector(0.5f, 0, 0);
			position *= GridToMeshScale;
			transform.SetLocation(position);
			transform.SetRotation(FQuat(FVector(0, 0, 1), 0));
			if (drawFence) {
				AddPropSocket(ST_FENCE, transform);
			}
			if (isElevatedFence) {
				AddPropSocket(ST_WALLHALF, transform, elevationHeight, HalfWallOffset);
			}
		}
		if (drawFence || drawPillar) {
			transform.SetLocation(FVector(x, y, z) * GridToMeshScale);
			AddPropSocket(ST_FENCESEPARATOR, transform);
			if (isElevatedFence) {
				AddPropSocket(ST_WALLHALFSEPARATOR, transform, elevationHeight, HalfWallOffset);
			}
		}


		y += cell.Bounds.Size.Y;
		drawFence = CanDrawFence(cell, x, y, isElevatedFence, drawPillar, elevationHeight);
		transform = FTransform::Identity;
		if (drawFence || isElevatedFence) {
			FVector position(x, y, z);
			position += FVector(0.5f, 0, 0);
			position *= GridToMeshScale;
			transform.SetLocation(position);
			transform.SetRotation(FQuat(FVector(0, 0, 1), PI));
			if (drawFence) {
				AddPropSocket(ST_FENCE, transform);
			}
			if (isElevatedFence) {
				AddPropSocket(ST_WALLHALF, transform, elevationHeight, HalfWallOffset);
			}
		}
		if (drawFence || drawPillar) {
			transform.SetLocation(FVector(x + 1, y, z) * GridToMeshScale);
			AddPropSocket(ST_FENCESEPARATOR, transform);
			if (isElevatedFence) {
				AddPropSocket(ST_WALLHALFSEPARATOR, transform, elevationHeight, HalfWallOffset);
			}
		}

	}

	// build fence along the length
	for (int dy = 0; dy < cell.Bounds.Size.Y; dy++) {
		int32 x = basePosition.X;
		int32 y = basePosition.Y + dy;
		int32 z = basePosition.Z;

		int elevationHeight;
		bool isElevatedFence, drawPillar;
		bool drawFence = CanDrawFence(cell, x - 1, y, isElevatedFence, drawPillar, elevationHeight);
		FTransform transform = FTransform::Identity;
		if (drawFence || isElevatedFence) {
			FVector position(x, y, z);
			position += FVector(0, 0.5f, 0);
			position *= GridToMeshScale;
			transform.SetLocation(position);
			transform.SetRotation(FQuat(FVector(0, 0, 1), -PI / 2));
			if (drawFence) {
				AddPropSocket(ST_FENCE, transform);
			}
			if (isElevatedFence) {
				AddPropSocket(ST_WALLHALF, transform, elevationHeight, HalfWallOffset);
			}
		}
		if (drawFence || drawPillar) {
			transform.SetLocation(FVector(x, y + 1, z) * GridToMeshScale);
			AddPropSocket(ST_FENCESEPARATOR, transform);
			if (isElevatedFence) {
				AddPropSocket(ST_WALLHALFSEPARATOR, transform, elevationHeight, HalfWallOffset);
			}
		}

		x += cell.Bounds.Size.X;
		drawFence = CanDrawFence(cell, x, y, isElevatedFence, drawPillar, elevationHeight);
		transform = FTransform::Identity;
		if (drawFence || isElevatedFence) {
			FVector position(x, y, z);
			position += FVector(0, 0.5f, 0);
			position *= GridToMeshScale;
			transform.SetLocation(position);
			transform.SetRotation(FQuat(FVector(0, 0, 1), PI / 2));
			if (drawFence) {
				AddPropSocket(ST_FENCE, transform);
			}
			if (isElevatedFence) {
				AddPropSocket(ST_WALLHALF, transform, elevationHeight, HalfWallOffset);
			}
		}
		if (drawFence || drawPillar) {
			transform.SetLocation(FVector(x, y, z) * GridToMeshScale);
			AddPropSocket(ST_FENCESEPARATOR, transform);
			if (isElevatedFence) {
				AddPropSocket(ST_WALLHALFSEPARATOR, transform, elevationHeight, HalfWallOffset);
			}
		}
	}
}

void UGridDungeonBuilder::EmitDungeonMarkers_Implementation() {
	Super::EmitDungeonMarkers_Implementation();

	gridModel = Cast<UGridDungeonModel>(model);
	gridConfig = Cast<UGridDungeonConfig>(config);

	if (gridModel->Cells.Num() == 0) return;
	ClearSockets();

	// Populate the prop sockets all over the map
	for (const FCell& cell : gridModel->Cells) {
		switch (cell.CellType) {
		case FCellType::Room:
			BuildMesh_Room(cell);
			BuildMesh_RoomDecoration(cell);
			break;

		case FCellType::Corridor:
		case FCellType::CorridorPadding:
			BuildMesh_Corridor(cell);
			break;
		}
		BuildMesh_Stairs(cell);
	}


	// Mirror the dungeon based on the mirror volumes placed in the level
    MirrorDungeon();
}

