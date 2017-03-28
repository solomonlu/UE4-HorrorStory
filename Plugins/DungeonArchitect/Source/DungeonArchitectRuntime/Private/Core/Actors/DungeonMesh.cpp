//$ Copyright 2015 Ali Akbar, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//
#include "DungeonArchitectRuntimePrivatePCH.h"
#include "DungeonMesh.h"

UDungeonMesh::UDungeonMesh(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{
	Template = ObjectInitializer.CreateDefaultSubobject<UStaticMeshComponent>(this, "Template");
	Template->StaticMesh = StaticMesh;
	Template->SetMobility(EComponentMobility::Static);
	Template->SetCanEverAffectNavigation(true);
	Template->bCastStaticShadow = true;

}

void UDungeonMesh::CalculateHashCode()
{
	FString HashData;
	if (StaticMesh) {
		HashData += StaticMesh->GetFullName();
	}

	for (const FMaterialOverride& MaterialOverride : MaterialOverrides) {
		HashData += "|";
		HashData += FString::FromInt(MaterialOverride.index);
		if (MaterialOverride.Material) {
			HashData += "|";
			HashData += MaterialOverride.Material->GetFullName();
		}
	}

	HashCode = GetTypeHash(HashData);
}
