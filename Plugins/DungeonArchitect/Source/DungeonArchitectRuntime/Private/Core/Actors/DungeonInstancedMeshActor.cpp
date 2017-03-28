//$ Copyright 2015 Ali Akbar, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//
#include "DungeonArchitectRuntimePrivatePCH.h"
#include "DungeonInstancedMeshActor.h"
#include "DungeonMesh.h"


ADungeonInstancedMeshActor::ADungeonInstancedMeshActor(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer) {
}

void ADungeonInstancedMeshActor::OnBuildStart() {
    //BuildCache();
	DestroyAllInstances();
}

void ADungeonInstancedMeshActor::OnBuildStop() {
    PurgeUsedInstances();
    InstancedComponentCache.Reset();
    
    MarkComponentsRenderStateDirty();
}

void ADungeonInstancedMeshActor::PurgeUsedInstances() {
    // Get the list of all the instanced mesh components
    TArray<UHierarchicalInstancedStaticMeshComponent*> Components;
    GetComponents<UHierarchicalInstancedStaticMeshComponent>(Components);
    for (UHierarchicalInstancedStaticMeshComponent* Component : Components) {
        if (Component->GetInstanceCount() == 0) {
			Component->DestroyComponent();
			BlueprintCreatedComponents.Remove(Component);
        }
    }
}

void ADungeonInstancedMeshActor::DestroyAllInstances() {
    InstancedComponentCache.Reset();
    
    TArray<UHierarchicalInstancedStaticMeshComponent*> InstancedComponentArray;
    GetComponents<UHierarchicalInstancedStaticMeshComponent>(InstancedComponentArray);
    for (UHierarchicalInstancedStaticMeshComponent* Component : InstancedComponentArray) {
        Component->DestroyComponent();
        BlueprintCreatedComponents.Remove(Component);
    }
}

UHierarchicalInstancedStaticMeshComponent* ADungeonInstancedMeshActor::GetInstancedComponent(UDungeonMesh* Mesh) {
    UHierarchicalInstancedStaticMeshComponent* Component = nullptr;
	if (!Mesh->HashCode) {
		// Calculate the hash code if not available (for backward compatibility)
		// TODO: Check performance for null meshes since has code will be 0
		Mesh->CalculateHashCode();
	}

	uint32 hash = Mesh->HashCode;
	if (!InstancedComponentCache.Contains(hash)) {
		Component = NewObject<UHierarchicalInstancedStaticMeshComponent>(this);
        Component->StaticMesh = Mesh->StaticMesh;
		
		// Set the material overrides
		for (const FMaterialOverride& MaterialOverride : Mesh->MaterialOverrides) {
			Component->SetMaterial(MaterialOverride.index, MaterialOverride.Material);
		}

        Component->SetupAttachment(GetRootComponent());
        Component->RegisterComponent();
        BlueprintCreatedComponents.Add(Component);
        InstancedComponentCache.Add(hash, Component);
    } else {
		Component = InstancedComponentCache[hash];
    }
    return Component;
}


void ADungeonInstancedMeshActor::AddMeshInstance(UDungeonMesh* Mesh, const FTransform& Transform) {
	if (!Mesh || !Mesh->StaticMesh) return;
    UHierarchicalInstancedStaticMeshComponent* Component = GetInstancedComponent(Mesh);
    Component->AddInstance(Transform);
}
