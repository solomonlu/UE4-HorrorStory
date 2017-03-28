//$ Copyright 2015 Ali Akbar, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "DungeonArchitectRuntimePrivatePCH.h"
#include "MathUtils.h"

TArray<int32> FMathUtils::GetShuffledIndices(int32 Count, FRandomStream& Random)
{
	TArray<int32> Indices;
	for (int i = 0; i < Count; i++) {
		Indices.Add(i);
	}

	Shuffle(Indices, Random);

	return Indices;
}
