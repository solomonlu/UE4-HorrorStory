// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	C++ class header boilerplate exported from UnrealHeaderTool.
	This is automatically generated by the tools.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

PRAGMA_DISABLE_DEPRECATION_WARNINGS
struct FVector;
class ADungeon;
class UGridDungeonModel;
class UGridDungeonConfig;
struct FCellDoor;
struct FCell;
struct FRectangle;
#ifdef DUNGEONARCHITECTRUNTIME_GridDungeonModelHelper_generated_h
#error "GridDungeonModelHelper.generated.h already included, missing '#pragma once' in GridDungeonModelHelper.h"
#endif
#define DUNGEONARCHITECTRUNTIME_GridDungeonModelHelper_generated_h

#define HorrorStory_Plugins_DungeonArchitect_Source_DungeonArchitectRuntime_Public_Builders_Grid_GridDungeonModelHelper_h_16_RPC_WRAPPERS \
 \
	DECLARE_FUNCTION(execRemovePaintCell) \
	{ \
		P_GET_STRUCT_REF(FVector,Z_Param_Out_CellLocation); \
		P_GET_OBJECT(ADungeon,Z_Param_Dungeon); \
		P_GET_UBOOL(Z_Param_bAutomaticRebuild); \
		P_FINISH; \
		P_NATIVE_BEGIN; \
		UGridDungeonModelHelper::RemovePaintCell(Z_Param_Out_CellLocation,Z_Param_Dungeon,Z_Param_bAutomaticRebuild); \
		P_NATIVE_END; \
	} \
 \
	DECLARE_FUNCTION(execAddPaintCell) \
	{ \
		P_GET_STRUCT_REF(FVector,Z_Param_Out_CellLocation); \
		P_GET_OBJECT(ADungeon,Z_Param_Dungeon); \
		P_GET_UBOOL(Z_Param_bAutomaticRebuild); \
		P_FINISH; \
		P_NATIVE_BEGIN; \
		UGridDungeonModelHelper::AddPaintCell(Z_Param_Out_CellLocation,Z_Param_Dungeon,Z_Param_bAutomaticRebuild); \
		P_NATIVE_END; \
	} \
 \
	DECLARE_FUNCTION(execGetRandomCellLocation) \
	{ \
		P_GET_OBJECT(UGridDungeonModel,Z_Param_Model); \
		P_GET_OBJECT(UGridDungeonConfig,Z_Param_Config); \
		P_FINISH; \
		P_NATIVE_BEGIN; \
		*(FVector*)Z_Param__Result=UGridDungeonModelHelper::GetRandomCellLocation(Z_Param_Model,Z_Param_Config); \
		P_NATIVE_END; \
	} \
 \
	DECLARE_FUNCTION(execGetDoorExits) \
	{ \
		P_GET_STRUCT_REF(FCellDoor,Z_Param_Out_Door); \
		P_GET_STRUCT_REF(FVector,Z_Param_Out_ExitA); \
		P_GET_STRUCT_REF(FVector,Z_Param_Out_ExitB); \
		P_FINISH; \
		P_NATIVE_BEGIN; \
		UGridDungeonModelHelper::GetDoorExits(Z_Param_Out_Door,Z_Param_Out_ExitA,Z_Param_Out_ExitB); \
		P_NATIVE_END; \
	} \
 \
	DECLARE_FUNCTION(execGetRoomFreeEdge) \
	{ \
		P_GET_OBJECT(UGridDungeonModel,Z_Param_Model); \
		P_GET_STRUCT_REF(FCell,Z_Param_Out_RoomCell); \
		P_GET_TARRAY_REF(FVector,Z_Param_Out_FreeEdgeCenters); \
		P_GET_TARRAY_REF(float,Z_Param_Out_FreeEdgeAngles); \
		P_FINISH; \
		P_NATIVE_BEGIN; \
		UGridDungeonModelHelper::GetRoomFreeEdge(Z_Param_Model,Z_Param_Out_RoomCell,Z_Param_Out_FreeEdgeCenters,Z_Param_Out_FreeEdgeAngles); \
		P_NATIVE_END; \
	} \
 \
	DECLARE_FUNCTION(execGetBoundingBox) \
	{ \
		P_GET_TARRAY_REF(FCell,Z_Param_Out_Cells); \
		P_GET_STRUCT_REF(FRectangle,Z_Param_Out_Bounds); \
		P_FINISH; \
		P_NATIVE_BEGIN; \
		UGridDungeonModelHelper::GetBoundingBox(Z_Param_Out_Cells,Z_Param_Out_Bounds); \
		P_NATIVE_END; \
	} \
 \
	DECLARE_FUNCTION(execToWorldCoords) \
	{ \
		P_GET_STRUCT_REF(FRectangle,Z_Param_Out_Bounds); \
		P_GET_STRUCT_REF(FVector,Z_Param_Out_GridSize); \
		P_GET_STRUCT_REF(FVector,Z_Param_Out_Location); \
		P_GET_STRUCT_REF(FVector,Z_Param_Out_Size); \
		P_FINISH; \
		P_NATIVE_BEGIN; \
		UGridDungeonModelHelper::ToWorldCoords(Z_Param_Out_Bounds,Z_Param_Out_GridSize,Z_Param_Out_Location,Z_Param_Out_Size); \
		P_NATIVE_END; \
	} \
 \
	DECLARE_FUNCTION(execGetCellCenter) \
	{ \
		P_GET_STRUCT_REF(FCell,Z_Param_Out_Cell); \
		P_GET_STRUCT_REF(FVector,Z_Param_Out_Center); \
		P_FINISH; \
		P_NATIVE_BEGIN; \
		UGridDungeonModelHelper::GetCellCenter(Z_Param_Out_Cell,Z_Param_Out_Center); \
		P_NATIVE_END; \
	} \
 \
	DECLARE_FUNCTION(execGetCellMSTRooms) \
	{ \
		P_GET_STRUCT_REF(FCell,Z_Param_Out_Cell); \
		P_GET_TARRAY_REF(int32,Z_Param_Out_MSTRooms); \
		P_FINISH; \
		P_NATIVE_BEGIN; \
		UGridDungeonModelHelper::GetCellMSTRooms(Z_Param_Out_Cell,Z_Param_Out_MSTRooms); \
		P_NATIVE_END; \
	} \
 \
	DECLARE_FUNCTION(execGetCellConnectedRooms) \
	{ \
		P_GET_STRUCT_REF(FCell,Z_Param_Out_Cell); \
		P_GET_TARRAY_REF(int32,Z_Param_Out_ConnectedRooms); \
		P_FINISH; \
		P_NATIVE_BEGIN; \
		UGridDungeonModelHelper::GetCellConnectedRooms(Z_Param_Out_Cell,Z_Param_Out_ConnectedRooms); \
		P_NATIVE_END; \
	}


#define HorrorStory_Plugins_DungeonArchitect_Source_DungeonArchitectRuntime_Public_Builders_Grid_GridDungeonModelHelper_h_16_RPC_WRAPPERS_NO_PURE_DECLS \
 \
	DECLARE_FUNCTION(execRemovePaintCell) \
	{ \
		P_GET_STRUCT_REF(FVector,Z_Param_Out_CellLocation); \
		P_GET_OBJECT(ADungeon,Z_Param_Dungeon); \
		P_GET_UBOOL(Z_Param_bAutomaticRebuild); \
		P_FINISH; \
		P_NATIVE_BEGIN; \
		UGridDungeonModelHelper::RemovePaintCell(Z_Param_Out_CellLocation,Z_Param_Dungeon,Z_Param_bAutomaticRebuild); \
		P_NATIVE_END; \
	} \
 \
	DECLARE_FUNCTION(execAddPaintCell) \
	{ \
		P_GET_STRUCT_REF(FVector,Z_Param_Out_CellLocation); \
		P_GET_OBJECT(ADungeon,Z_Param_Dungeon); \
		P_GET_UBOOL(Z_Param_bAutomaticRebuild); \
		P_FINISH; \
		P_NATIVE_BEGIN; \
		UGridDungeonModelHelper::AddPaintCell(Z_Param_Out_CellLocation,Z_Param_Dungeon,Z_Param_bAutomaticRebuild); \
		P_NATIVE_END; \
	} \
 \
	DECLARE_FUNCTION(execGetRandomCellLocation) \
	{ \
		P_GET_OBJECT(UGridDungeonModel,Z_Param_Model); \
		P_GET_OBJECT(UGridDungeonConfig,Z_Param_Config); \
		P_FINISH; \
		P_NATIVE_BEGIN; \
		*(FVector*)Z_Param__Result=UGridDungeonModelHelper::GetRandomCellLocation(Z_Param_Model,Z_Param_Config); \
		P_NATIVE_END; \
	} \
 \
	DECLARE_FUNCTION(execGetDoorExits) \
	{ \
		P_GET_STRUCT_REF(FCellDoor,Z_Param_Out_Door); \
		P_GET_STRUCT_REF(FVector,Z_Param_Out_ExitA); \
		P_GET_STRUCT_REF(FVector,Z_Param_Out_ExitB); \
		P_FINISH; \
		P_NATIVE_BEGIN; \
		UGridDungeonModelHelper::GetDoorExits(Z_Param_Out_Door,Z_Param_Out_ExitA,Z_Param_Out_ExitB); \
		P_NATIVE_END; \
	} \
 \
	DECLARE_FUNCTION(execGetRoomFreeEdge) \
	{ \
		P_GET_OBJECT(UGridDungeonModel,Z_Param_Model); \
		P_GET_STRUCT_REF(FCell,Z_Param_Out_RoomCell); \
		P_GET_TARRAY_REF(FVector,Z_Param_Out_FreeEdgeCenters); \
		P_GET_TARRAY_REF(float,Z_Param_Out_FreeEdgeAngles); \
		P_FINISH; \
		P_NATIVE_BEGIN; \
		UGridDungeonModelHelper::GetRoomFreeEdge(Z_Param_Model,Z_Param_Out_RoomCell,Z_Param_Out_FreeEdgeCenters,Z_Param_Out_FreeEdgeAngles); \
		P_NATIVE_END; \
	} \
 \
	DECLARE_FUNCTION(execGetBoundingBox) \
	{ \
		P_GET_TARRAY_REF(FCell,Z_Param_Out_Cells); \
		P_GET_STRUCT_REF(FRectangle,Z_Param_Out_Bounds); \
		P_FINISH; \
		P_NATIVE_BEGIN; \
		UGridDungeonModelHelper::GetBoundingBox(Z_Param_Out_Cells,Z_Param_Out_Bounds); \
		P_NATIVE_END; \
	} \
 \
	DECLARE_FUNCTION(execToWorldCoords) \
	{ \
		P_GET_STRUCT_REF(FRectangle,Z_Param_Out_Bounds); \
		P_GET_STRUCT_REF(FVector,Z_Param_Out_GridSize); \
		P_GET_STRUCT_REF(FVector,Z_Param_Out_Location); \
		P_GET_STRUCT_REF(FVector,Z_Param_Out_Size); \
		P_FINISH; \
		P_NATIVE_BEGIN; \
		UGridDungeonModelHelper::ToWorldCoords(Z_Param_Out_Bounds,Z_Param_Out_GridSize,Z_Param_Out_Location,Z_Param_Out_Size); \
		P_NATIVE_END; \
	} \
 \
	DECLARE_FUNCTION(execGetCellCenter) \
	{ \
		P_GET_STRUCT_REF(FCell,Z_Param_Out_Cell); \
		P_GET_STRUCT_REF(FVector,Z_Param_Out_Center); \
		P_FINISH; \
		P_NATIVE_BEGIN; \
		UGridDungeonModelHelper::GetCellCenter(Z_Param_Out_Cell,Z_Param_Out_Center); \
		P_NATIVE_END; \
	} \
 \
	DECLARE_FUNCTION(execGetCellMSTRooms) \
	{ \
		P_GET_STRUCT_REF(FCell,Z_Param_Out_Cell); \
		P_GET_TARRAY_REF(int32,Z_Param_Out_MSTRooms); \
		P_FINISH; \
		P_NATIVE_BEGIN; \
		UGridDungeonModelHelper::GetCellMSTRooms(Z_Param_Out_Cell,Z_Param_Out_MSTRooms); \
		P_NATIVE_END; \
	} \
 \
	DECLARE_FUNCTION(execGetCellConnectedRooms) \
	{ \
		P_GET_STRUCT_REF(FCell,Z_Param_Out_Cell); \
		P_GET_TARRAY_REF(int32,Z_Param_Out_ConnectedRooms); \
		P_FINISH; \
		P_NATIVE_BEGIN; \
		UGridDungeonModelHelper::GetCellConnectedRooms(Z_Param_Out_Cell,Z_Param_Out_ConnectedRooms); \
		P_NATIVE_END; \
	}


#define HorrorStory_Plugins_DungeonArchitect_Source_DungeonArchitectRuntime_Public_Builders_Grid_GridDungeonModelHelper_h_16_INCLASS_NO_PURE_DECLS \
	private: \
	static void StaticRegisterNativesUGridDungeonModelHelper(); \
	friend DUNGEONARCHITECTRUNTIME_API class UClass* Z_Construct_UClass_UGridDungeonModelHelper(); \
	public: \
	DECLARE_CLASS(UGridDungeonModelHelper, UDungeonModelHelper, COMPILED_IN_FLAGS(0), 0, TEXT("/Script/DungeonArchitectRuntime"), NO_API) \
	DECLARE_SERIALIZER(UGridDungeonModelHelper) \
	/** Indicates whether the class is compiled into the engine */ \
	enum {IsIntrinsic=COMPILED_IN_INTRINSIC};


#define HorrorStory_Plugins_DungeonArchitect_Source_DungeonArchitectRuntime_Public_Builders_Grid_GridDungeonModelHelper_h_16_INCLASS \
	private: \
	static void StaticRegisterNativesUGridDungeonModelHelper(); \
	friend DUNGEONARCHITECTRUNTIME_API class UClass* Z_Construct_UClass_UGridDungeonModelHelper(); \
	public: \
	DECLARE_CLASS(UGridDungeonModelHelper, UDungeonModelHelper, COMPILED_IN_FLAGS(0), 0, TEXT("/Script/DungeonArchitectRuntime"), NO_API) \
	DECLARE_SERIALIZER(UGridDungeonModelHelper) \
	/** Indicates whether the class is compiled into the engine */ \
	enum {IsIntrinsic=COMPILED_IN_INTRINSIC};


#define HorrorStory_Plugins_DungeonArchitect_Source_DungeonArchitectRuntime_Public_Builders_Grid_GridDungeonModelHelper_h_16_STANDARD_CONSTRUCTORS \
	/** Standard constructor, called after all reflected properties have been initialized */ \
	NO_API UGridDungeonModelHelper(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get()); \
	DEFINE_DEFAULT_OBJECT_INITIALIZER_CONSTRUCTOR_CALL(UGridDungeonModelHelper) \
	DECLARE_VTABLE_PTR_HELPER_CTOR(NO_API, UGridDungeonModelHelper); \
DEFINE_VTABLE_PTR_HELPER_CTOR_CALLER(UGridDungeonModelHelper); \
private: \
	/** Private move- and copy-constructors, should never be used */ \
	NO_API UGridDungeonModelHelper(UGridDungeonModelHelper&&); \
	NO_API UGridDungeonModelHelper(const UGridDungeonModelHelper&); \
public:


#define HorrorStory_Plugins_DungeonArchitect_Source_DungeonArchitectRuntime_Public_Builders_Grid_GridDungeonModelHelper_h_16_ENHANCED_CONSTRUCTORS \
	/** Standard constructor, called after all reflected properties have been initialized */ \
	NO_API UGridDungeonModelHelper(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get()) : Super(ObjectInitializer) { }; \
private: \
	/** Private move- and copy-constructors, should never be used */ \
	NO_API UGridDungeonModelHelper(UGridDungeonModelHelper&&); \
	NO_API UGridDungeonModelHelper(const UGridDungeonModelHelper&); \
public: \
	DECLARE_VTABLE_PTR_HELPER_CTOR(NO_API, UGridDungeonModelHelper); \
DEFINE_VTABLE_PTR_HELPER_CTOR_CALLER(UGridDungeonModelHelper); \
	DEFINE_DEFAULT_OBJECT_INITIALIZER_CONSTRUCTOR_CALL(UGridDungeonModelHelper)


#define HorrorStory_Plugins_DungeonArchitect_Source_DungeonArchitectRuntime_Public_Builders_Grid_GridDungeonModelHelper_h_16_PRIVATE_PROPERTY_OFFSET
#define HorrorStory_Plugins_DungeonArchitect_Source_DungeonArchitectRuntime_Public_Builders_Grid_GridDungeonModelHelper_h_14_PROLOG
#define HorrorStory_Plugins_DungeonArchitect_Source_DungeonArchitectRuntime_Public_Builders_Grid_GridDungeonModelHelper_h_16_GENERATED_BODY_LEGACY \
PRAGMA_DISABLE_DEPRECATION_WARNINGS \
public: \
	HorrorStory_Plugins_DungeonArchitect_Source_DungeonArchitectRuntime_Public_Builders_Grid_GridDungeonModelHelper_h_16_PRIVATE_PROPERTY_OFFSET \
	HorrorStory_Plugins_DungeonArchitect_Source_DungeonArchitectRuntime_Public_Builders_Grid_GridDungeonModelHelper_h_16_RPC_WRAPPERS \
	HorrorStory_Plugins_DungeonArchitect_Source_DungeonArchitectRuntime_Public_Builders_Grid_GridDungeonModelHelper_h_16_INCLASS \
	HorrorStory_Plugins_DungeonArchitect_Source_DungeonArchitectRuntime_Public_Builders_Grid_GridDungeonModelHelper_h_16_STANDARD_CONSTRUCTORS \
public: \
PRAGMA_ENABLE_DEPRECATION_WARNINGS


#define HorrorStory_Plugins_DungeonArchitect_Source_DungeonArchitectRuntime_Public_Builders_Grid_GridDungeonModelHelper_h_16_GENERATED_BODY \
PRAGMA_DISABLE_DEPRECATION_WARNINGS \
public: \
	HorrorStory_Plugins_DungeonArchitect_Source_DungeonArchitectRuntime_Public_Builders_Grid_GridDungeonModelHelper_h_16_PRIVATE_PROPERTY_OFFSET \
	HorrorStory_Plugins_DungeonArchitect_Source_DungeonArchitectRuntime_Public_Builders_Grid_GridDungeonModelHelper_h_16_RPC_WRAPPERS_NO_PURE_DECLS \
	HorrorStory_Plugins_DungeonArchitect_Source_DungeonArchitectRuntime_Public_Builders_Grid_GridDungeonModelHelper_h_16_INCLASS_NO_PURE_DECLS \
	HorrorStory_Plugins_DungeonArchitect_Source_DungeonArchitectRuntime_Public_Builders_Grid_GridDungeonModelHelper_h_16_ENHANCED_CONSTRUCTORS \
static_assert(false, "Unknown access specifier for GENERATED_BODY() macro in class GridDungeonModelHelper."); \
PRAGMA_ENABLE_DEPRECATION_WARNINGS


#undef CURRENT_FILE_ID
#define CURRENT_FILE_ID HorrorStory_Plugins_DungeonArchitect_Source_DungeonArchitectRuntime_Public_Builders_Grid_GridDungeonModelHelper_h


PRAGMA_ENABLE_DEPRECATION_WARNINGS
