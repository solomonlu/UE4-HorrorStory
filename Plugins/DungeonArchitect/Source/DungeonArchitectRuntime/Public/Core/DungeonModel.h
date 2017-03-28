//$ Copyright 2015 Ali Akbar, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//
#pragma once
#include "DungeonConfig.h"
#include "Core/Utils/MathUtils.h"
#include "DungeonModel.generated.h"

/**
* 
*/
UCLASS(Blueprintable)
class DUNGEONARCHITECTRUNTIME_API UDungeonModel : public UObject
{
	GENERATED_BODY()

public:

	virtual void Cleanup() {}
	virtual void Reset() {}
};
