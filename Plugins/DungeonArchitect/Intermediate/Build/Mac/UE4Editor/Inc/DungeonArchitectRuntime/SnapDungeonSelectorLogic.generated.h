// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	C++ class header boilerplate exported from UnrealHeaderTool.
	This is automatically generated by the tools.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

#include "ObjectBase.h"

PRAGMA_DISABLE_DEPRECATION_WARNINGS
class USnapDungeonModel;
#ifdef DUNGEONARCHITECTRUNTIME_SnapDungeonSelectorLogic_generated_h
#error "SnapDungeonSelectorLogic.generated.h already included, missing '#pragma once' in SnapDungeonSelectorLogic.h"
#endif
#define DUNGEONARCHITECTRUNTIME_SnapDungeonSelectorLogic_generated_h

#define HostProject_Plugins_DungeonArchitect_Source_DungeonArchitectRuntime_Public_Builders_Snap_SnapDungeonSelectorLogic_h_16_RPC_WRAPPERS \
	virtual bool SelectNode_Implementation(USnapDungeonModel* Model); \
 \
	DECLARE_FUNCTION(execSelectNode) \
	{ \
		P_GET_OBJECT(USnapDungeonModel,Z_Param_Model); \
		P_FINISH; \
		P_NATIVE_BEGIN; \
		*(bool*)Z_Param__Result=this->SelectNode_Implementation(Z_Param_Model); \
		P_NATIVE_END; \
	}


#define HostProject_Plugins_DungeonArchitect_Source_DungeonArchitectRuntime_Public_Builders_Snap_SnapDungeonSelectorLogic_h_16_RPC_WRAPPERS_NO_PURE_DECLS \
 \
	DECLARE_FUNCTION(execSelectNode) \
	{ \
		P_GET_OBJECT(USnapDungeonModel,Z_Param_Model); \
		P_FINISH; \
		P_NATIVE_BEGIN; \
		*(bool*)Z_Param__Result=this->SelectNode_Implementation(Z_Param_Model); \
		P_NATIVE_END; \
	}


#define HostProject_Plugins_DungeonArchitect_Source_DungeonArchitectRuntime_Public_Builders_Snap_SnapDungeonSelectorLogic_h_16_EVENT_PARMS \
	struct SnapDungeonSelectorLogic_eventSelectNode_Parms \
	{ \
		USnapDungeonModel* Model; \
		bool ReturnValue; \
 \
		/** Constructor, initializes return property only **/ \
		SnapDungeonSelectorLogic_eventSelectNode_Parms() \
			: ReturnValue(false) \
		{ \
		} \
	};


extern DUNGEONARCHITECTRUNTIME_API  FName DUNGEONARCHITECTRUNTIME_SelectNode;
#define HostProject_Plugins_DungeonArchitect_Source_DungeonArchitectRuntime_Public_Builders_Snap_SnapDungeonSelectorLogic_h_16_CALLBACK_WRAPPERS
#define HostProject_Plugins_DungeonArchitect_Source_DungeonArchitectRuntime_Public_Builders_Snap_SnapDungeonSelectorLogic_h_16_INCLASS_NO_PURE_DECLS \
	private: \
	static void StaticRegisterNativesUSnapDungeonSelectorLogic(); \
	friend DUNGEONARCHITECTRUNTIME_API class UClass* Z_Construct_UClass_USnapDungeonSelectorLogic(); \
	public: \
	DECLARE_CLASS(USnapDungeonSelectorLogic, UDungeonSelectorLogic, COMPILED_IN_FLAGS(0), 0, TEXT("/Script/DungeonArchitectRuntime"), NO_API) \
	DECLARE_SERIALIZER(USnapDungeonSelectorLogic) \
	/** Indicates whether the class is compiled into the engine */ \
	enum {IsIntrinsic=COMPILED_IN_INTRINSIC};


#define HostProject_Plugins_DungeonArchitect_Source_DungeonArchitectRuntime_Public_Builders_Snap_SnapDungeonSelectorLogic_h_16_INCLASS \
	private: \
	static void StaticRegisterNativesUSnapDungeonSelectorLogic(); \
	friend DUNGEONARCHITECTRUNTIME_API class UClass* Z_Construct_UClass_USnapDungeonSelectorLogic(); \
	public: \
	DECLARE_CLASS(USnapDungeonSelectorLogic, UDungeonSelectorLogic, COMPILED_IN_FLAGS(0), 0, TEXT("/Script/DungeonArchitectRuntime"), NO_API) \
	DECLARE_SERIALIZER(USnapDungeonSelectorLogic) \
	/** Indicates whether the class is compiled into the engine */ \
	enum {IsIntrinsic=COMPILED_IN_INTRINSIC};


#define HostProject_Plugins_DungeonArchitect_Source_DungeonArchitectRuntime_Public_Builders_Snap_SnapDungeonSelectorLogic_h_16_STANDARD_CONSTRUCTORS \
	/** Standard constructor, called after all reflected properties have been initialized */ \
	NO_API USnapDungeonSelectorLogic(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get()); \
	DEFINE_DEFAULT_OBJECT_INITIALIZER_CONSTRUCTOR_CALL(USnapDungeonSelectorLogic) \
	DECLARE_VTABLE_PTR_HELPER_CTOR(NO_API, USnapDungeonSelectorLogic); \
DEFINE_VTABLE_PTR_HELPER_CTOR_CALLER(USnapDungeonSelectorLogic); \
private: \
	/** Private move- and copy-constructors, should never be used */ \
	NO_API USnapDungeonSelectorLogic(USnapDungeonSelectorLogic&&); \
	NO_API USnapDungeonSelectorLogic(const USnapDungeonSelectorLogic&); \
public:


#define HostProject_Plugins_DungeonArchitect_Source_DungeonArchitectRuntime_Public_Builders_Snap_SnapDungeonSelectorLogic_h_16_ENHANCED_CONSTRUCTORS \
	/** Standard constructor, called after all reflected properties have been initialized */ \
	NO_API USnapDungeonSelectorLogic(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get()) : Super(ObjectInitializer) { }; \
private: \
	/** Private move- and copy-constructors, should never be used */ \
	NO_API USnapDungeonSelectorLogic(USnapDungeonSelectorLogic&&); \
	NO_API USnapDungeonSelectorLogic(const USnapDungeonSelectorLogic&); \
public: \
	DECLARE_VTABLE_PTR_HELPER_CTOR(NO_API, USnapDungeonSelectorLogic); \
DEFINE_VTABLE_PTR_HELPER_CTOR_CALLER(USnapDungeonSelectorLogic); \
	DEFINE_DEFAULT_OBJECT_INITIALIZER_CONSTRUCTOR_CALL(USnapDungeonSelectorLogic)


#define HostProject_Plugins_DungeonArchitect_Source_DungeonArchitectRuntime_Public_Builders_Snap_SnapDungeonSelectorLogic_h_13_PROLOG \
	HostProject_Plugins_DungeonArchitect_Source_DungeonArchitectRuntime_Public_Builders_Snap_SnapDungeonSelectorLogic_h_16_EVENT_PARMS


#define HostProject_Plugins_DungeonArchitect_Source_DungeonArchitectRuntime_Public_Builders_Snap_SnapDungeonSelectorLogic_h_16_GENERATED_BODY_LEGACY \
PRAGMA_DISABLE_DEPRECATION_WARNINGS \
public: \
	HostProject_Plugins_DungeonArchitect_Source_DungeonArchitectRuntime_Public_Builders_Snap_SnapDungeonSelectorLogic_h_16_RPC_WRAPPERS \
	HostProject_Plugins_DungeonArchitect_Source_DungeonArchitectRuntime_Public_Builders_Snap_SnapDungeonSelectorLogic_h_16_CALLBACK_WRAPPERS \
	HostProject_Plugins_DungeonArchitect_Source_DungeonArchitectRuntime_Public_Builders_Snap_SnapDungeonSelectorLogic_h_16_INCLASS \
	HostProject_Plugins_DungeonArchitect_Source_DungeonArchitectRuntime_Public_Builders_Snap_SnapDungeonSelectorLogic_h_16_STANDARD_CONSTRUCTORS \
public: \
PRAGMA_ENABLE_DEPRECATION_WARNINGS


#define HostProject_Plugins_DungeonArchitect_Source_DungeonArchitectRuntime_Public_Builders_Snap_SnapDungeonSelectorLogic_h_16_GENERATED_BODY \
PRAGMA_DISABLE_DEPRECATION_WARNINGS \
public: \
	HostProject_Plugins_DungeonArchitect_Source_DungeonArchitectRuntime_Public_Builders_Snap_SnapDungeonSelectorLogic_h_16_RPC_WRAPPERS_NO_PURE_DECLS \
	HostProject_Plugins_DungeonArchitect_Source_DungeonArchitectRuntime_Public_Builders_Snap_SnapDungeonSelectorLogic_h_16_CALLBACK_WRAPPERS \
	HostProject_Plugins_DungeonArchitect_Source_DungeonArchitectRuntime_Public_Builders_Snap_SnapDungeonSelectorLogic_h_16_INCLASS_NO_PURE_DECLS \
	HostProject_Plugins_DungeonArchitect_Source_DungeonArchitectRuntime_Public_Builders_Snap_SnapDungeonSelectorLogic_h_16_ENHANCED_CONSTRUCTORS \
private: \
PRAGMA_ENABLE_DEPRECATION_WARNINGS


#undef CURRENT_FILE_ID
#define CURRENT_FILE_ID HostProject_Plugins_DungeonArchitect_Source_DungeonArchitectRuntime_Public_Builders_Snap_SnapDungeonSelectorLogic_h


PRAGMA_ENABLE_DEPRECATION_WARNINGS