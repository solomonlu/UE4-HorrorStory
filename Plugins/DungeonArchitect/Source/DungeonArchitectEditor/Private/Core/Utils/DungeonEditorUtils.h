//$ Copyright 2015 Ali Akbar, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//
#pragma once
#include "DetailLayoutBuilder.h"

class ADungeon;

class DUNGEONARCHITECTEDITOR_API FDungeonEditorUtils {
public:
	static ADungeon* GetDungeonActorFromLevelViewport();

	template<typename T>
	static T* GetBuilderObject(IDetailLayoutBuilder* DetailBuilder) {
		TArray<TWeakObjectPtr<UObject>> OutObjects;
		DetailBuilder->GetObjectsBeingCustomized(OutObjects);
		T* Obj = nullptr;
		if (OutObjects.Num() > 0) {
			Obj = Cast<T>(OutObjects[0].Get());
		}
		return Obj;
	}
};
