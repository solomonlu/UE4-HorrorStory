//$ Copyright 2015 Ali Akbar, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//
#pragma once

#include "SnapDoorAsset.h"
#include "SnapDoor.generated.h"

UENUM()
enum class ESnapDoorMode : uint8 {
	Door				UMETA(DisplayName = "Door"),
	Closed				UMETA(DisplayName = "Closed")
};

class USnapDoorAsset;

UCLASS(Blueprintable)
class DUNGEONARCHITECTRUNTIME_API ASnapDoor : public AActor
{
	GENERATED_UCLASS_BODY()

	virtual void PostLoad() override;
	virtual void PostActorCreated() override;
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void PostInitializeComponents() override;
	FString GetDoorAssetPath() const;
	void BuildDoor();

	/** Lets you emit your own markers into the scene */
	UPROPERTY()
	UChildActorComponent* ActorComponent;

	/** Lets you emit your own markers into the scene */
	UPROPERTY()
	UStaticMeshComponent* MeshComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Snap Door")
	TArray<FString> AllowedModuleTypes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Misc)
	ESnapDoorMode VisualMode;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Misc)
	USnapDoorAsset* DoorAsset;


#if WITH_EDITORONLY_DATA
	UBillboardComponent* GetSpriteComponent() const { return SpriteComponent; }

private_subobject:
	UPROPERTY()
	UBillboardComponent* SpriteComponent;

	UPROPERTY()
	UTextRenderComponent* CaptionComponent;

	UPROPERTY()
	UArrowComponent* ArrowComponent;

#endif //WITH_EDITORONLY_DATA

private:

};