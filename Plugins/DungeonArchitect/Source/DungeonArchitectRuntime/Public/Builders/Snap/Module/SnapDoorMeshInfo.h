//$ Copyright 2015 Ali Akbar, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//
#pragma once
#include "SnapDoorMeshInfo.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSnapDoorMeshInfo, Log, All);

UENUM(BlueprintType)
enum class EUSnapDoorMeshType : uint8
{
	StaticMesh = 0		UMETA(DisplayName = "Static Mesh"),
	Blueprint = 1		UMETA(DisplayName = "Blueprint Class")
};

UCLASS(Blueprintable)
class DUNGEONARCHITECTRUNTIME_API USnapDoorMeshInfo : public UObject {
	GENERATED_UCLASS_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Snap Door")
	FTransform Transform;

	UPROPERTY(EditAnywhere, Category = "Snap Door")
	EUSnapDoorMeshType MeshType;

	UPROPERTY(EditAnywhere, Category = "Snap Door")
	UStaticMeshComponent* MeshComponent;

	UPROPERTY(EditAnywhere, Category = "Snap Door")
	TSubclassOf<AActor> BlueprintClass;

};
