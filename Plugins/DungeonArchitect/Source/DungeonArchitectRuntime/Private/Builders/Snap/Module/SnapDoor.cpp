//$ Copyright 2015 Ali Akbar, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//
#include "DungeonArchitectRuntimePrivatePCH.h"
#include "SnapDoor.h"
#include "SnapDoorMeshInfo.h"


ASnapDoor::ASnapDoor(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	USceneComponent* SceneComponent = ObjectInitializer.CreateDefaultSubobject<USceneComponent>(this, "SceneRoot");
	SceneComponent->SetRelativeTransform(FTransform::Identity);
	SceneComponent->SetMobility(EComponentMobility::Static);
	RootComponent = SceneComponent;
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;

	//ActorComponent = ObjectInitializer.CreateDefaultSubobject<UChildActorComponent>(this, "ActorComponent");
	//ActorComponent->SetupAttachment(RootComponent);
	ActorComponent = nullptr;
	MeshComponent = nullptr; // ObjectInitializer.CreateDefaultSubobject<UStaticMeshComponent>(this, "MeshComponent");
	DoorAsset = nullptr;

#if WITH_EDITORONLY_DATA
	// Add an arrow component
	ArrowComponent = ObjectInitializer.CreateDefaultSubobject<UArrowComponent>(this, "Arrow");
	if (ArrowComponent) {
		FTransform ArrowTransform = FTransform::Identity;
		ArrowTransform.SetRotation(FQuat::MakeFromEuler(FVector(0, 0, 90)));
		ArrowComponent->SetRelativeTransform(ArrowTransform);
		ArrowComponent->SetupAttachment(RootComponent);
	}

	// Add a sprite component
	SpriteComponent = CreateEditorOnlyDefaultSubobject<UBillboardComponent>(TEXT("Sprite"));

	// Create a text component to show the door
	CaptionComponent = CreateEditorOnlyDefaultSubobject<UTextRenderComponent>(TEXT("Caption"));
	if (CaptionComponent) {
		CaptionComponent->SetHorizontalAlignment(EHTA_Center);
		CaptionComponent->SetRelativeLocation(FVector(0, 0, 30));
		CaptionComponent->SetTextRenderColor(FColor::White);
		CaptionComponent->SetRelativeRotation(FQuat::MakeFromEuler(FVector(0, 0, 90)));
		CaptionComponent->SetupAttachment(RootComponent);
		CaptionComponent->bHiddenInGame = true;
	}

	if (!IsRunningCommandlet()) {
		// Structure to hold one-time initialization
		struct FConstructorStatics
		{
			ConstructorHelpers::FObjectFinderOptional<UTexture2D> DungeonSpriteObject;
			FName ID_Dungeon;
			FText NAME_Dungeon;
			FConstructorStatics()
				: DungeonSpriteObject(TEXT("/DungeonArchitect/Textures/S_SnapDoor"))
				, ID_Dungeon(TEXT("SnapDoor"))
				, NAME_Dungeon(NSLOCTEXT("SpriteCategory", "SnapDoor", "SnapDoor"))
			{
			}
		};
		static FConstructorStatics ConstructorStatics;

		if (SpriteComponent)
		{
			SpriteComponent->Sprite = ConstructorStatics.DungeonSpriteObject.Get();
			SpriteComponent->RelativeScale3D = FVector(1, 1, 1);
			SpriteComponent->SpriteInfo.Category = ConstructorStatics.ID_Dungeon;
			SpriteComponent->SpriteInfo.DisplayName = ConstructorStatics.NAME_Dungeon;
			SpriteComponent->SetupAttachment(RootComponent);
			SpriteComponent->Mobility = EComponentMobility::Static;
		}
	}

#endif //WITH_EDITORONLY_DATA
}

void ASnapDoor::PostLoad()
{
	Super::PostLoad();
	//BuildDoor();
}

void ASnapDoor::PostActorCreated()
{
	Super::PostActorCreated();
	//BuildDoor();
}

void ASnapDoor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	BuildDoor();
}

void ASnapDoor::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	//BuildDoor();
}

FString ASnapDoor::GetDoorAssetPath() const
{
	return DoorAsset ? DoorAsset->GetName() : FString();
}

void ASnapDoor::BuildDoor() {
	if (!DoorAsset) return;
	
	// Remove the old components
	if (MeshComponent) {
		MeshComponent->DestroyComponent();
		MeshComponent = nullptr;
	}

	if (ActorComponent) {
		ActorComponent->DestroyComponent();
		ActorComponent = nullptr;
	}

#if WITH_EDITORONLY_DATA
	FString Name = DoorAsset->GetName();
	if (CaptionComponent) {
		CaptionComponent->SetText(FText::FromString(Name));
	}
#endif

	// Set the appropriate child actor class and create the child actor
	USnapDoorMeshInfo* MeshInfo = (VisualMode == ESnapDoorMode::Door) ? DoorAsset->DoorMesh : DoorAsset->ClosedMesh;
	if (MeshInfo->MeshType == EUSnapDoorMeshType::StaticMesh) {
		MeshComponent = NewObject<UStaticMeshComponent>(this, UStaticMeshComponent::StaticClass(), "MeshComponent", RF_NoFlags, MeshInfo->MeshComponent);
		MeshComponent->SetRelativeTransform(MeshInfo->Transform);
		MeshComponent->SetupAttachment(RootComponent);
		MeshComponent->RegisterComponent();
	}
	else {
		ActorComponent = NewObject<UChildActorComponent>(this, "ActorComponent");
		ActorComponent->SetupAttachment(RootComponent);
		ActorComponent->SetChildActorClass(MeshInfo->BlueprintClass);
		ActorComponent->RegisterComponent();

		ActorComponent->CreateChildActor();
		ActorComponent->SetRelativeTransform(MeshInfo->Transform);
	}
}

