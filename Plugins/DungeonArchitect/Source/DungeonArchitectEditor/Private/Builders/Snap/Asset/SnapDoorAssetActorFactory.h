//$ Copyright 2015 Ali Akbar, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once

#include "ActorFactories/ActorFactory.h"
#include "SnapDoorAssetActorFactory.generated.h"

UCLASS()
class DUNGEONARCHITECTEDITOR_API USnapDoorAssetActorFactory : public UActorFactory {
	GENERATED_UCLASS_BODY()

	//~ Begin UActorFactory Interface
	virtual AActor* SpawnActor(UObject* Asset, ULevel* InLevel, const FTransform& Transform, EObjectFlags ObjectFlags, const FName Name) override;
	virtual bool CanCreateActorFrom(const FAssetData& AssetData, FText& OutErrorMsg) override;
	//~ End UActorFactory Interface
};
