//$ Copyright 2015 Ali Akbar, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//
#pragma once

class DUNGEONARCHITECTRUNTIME_API FAssetUtils {
public:
	static UTexture2D* GetSprite(const FString& Path);
	static UTexture2D* GetPointLightSprite();
	static UTexture2D* GetSpotLightSprite();
	static FString GetAssetPath(UObject* Object);
};