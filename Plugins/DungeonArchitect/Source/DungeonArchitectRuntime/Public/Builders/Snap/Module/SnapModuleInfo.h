//$ Copyright 2015 Ali Akbar, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//
#pragma once

#include "SnapModuleInfo.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSnapModuleInfo, Log, All);

USTRUCT()
struct DUNGEONARCHITECTRUNTIME_API FSnapModuleMetaData {
	GENERATED_BODY()

	FSnapModuleMetaData() : bContributesToModuleCount(true), bCanBeEndNode(true) {}

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SnapModule")
	FString ModuleType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SnapModule")
	bool bContributesToModuleCount;

	/**
	Indicates if this module can be an end node (i.e. has only one connected door).
	You might want to uncheck this for your corridor nodes since you want them to connect two or more modules
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SnapModule")
	bool bCanBeEndNode;
};

UCLASS()
class DUNGEONARCHITECTRUNTIME_API ASnapModuleInfo : public AActor {
	GENERATED_UCLASS_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SnapModule")
	FSnapModuleMetaData ModuleInfo;
};

