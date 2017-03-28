//$ Copyright 2015 Ali Akbar, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//


#include "DungeonArchitectRuntimePrivatePCH.h"
#include "PMRandom.h"

PMRandom::PMRandom()
{
}

void PMRandom::Init(int32 seed)
{
	random.Initialize((int32)seed);
}

float PMRandom::NextGaussianFloat() {
	float u1 = random.FRand();
	float u2 = random.FRand();
	
	float randStdNormal = FMath::Sqrt(-2.0f * FMath::Log2(u1)) *
		FMath::Sin(2.0f * PI * u2); // random normal(0,1)

	return randStdNormal;
}

float PMRandom::NextGaussianFloat(float mean, float stdDev) {
	return mean + stdDev * NextGaussianFloat();
}

float PMRandom::GetNextUniformFloat() {
	//return FMath::Clamp(NextGaussianFloat(0.5f, 0.1f), 0.0f, 1.0f);
	return random.FRand();
}

