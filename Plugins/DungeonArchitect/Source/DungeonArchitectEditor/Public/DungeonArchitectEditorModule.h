//$ Copyright 2015 Ali Akbar, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//


#pragma once

#include "UnrealEd.h"
#include "ModuleManager.h"
#include "ModuleInterface.h"
#include "Toolkits/AssetEditorToolkit.h"
#include "Toolkits/IToolkit.h"
#include "IDungeonArchitectEditor.h"
#include "AssetTypeCategories.h"

/**
 * The public interface to this module
 */
class IDungeonArchitectEditorModule : public IModuleInterface
{

public:

	/**
	 * Singleton-like access to this module's interface.  This is just for convenience!
	 * Beware of calling this during the shutdown phase, though.  Your module might have been unloaded already.
	 *
	 * @return Returns singleton instance, loading the module on demand if needed
	 */
	static inline IDungeonArchitectEditorModule& Get()
	{
		return FModuleManager::LoadModuleChecked< IDungeonArchitectEditorModule >("DungeonArchitectEditor");
	}

	/**
	 * Checks to see if this module is loaded and ready.  It is only valid to call Get() if IsAvailable() returns true.
	 *
	 * @return True if the module is loaded and ready to use
	 */
	static inline bool IsAvailable()
	{
		return FModuleManager::Get().IsModuleLoaded( "DungeonArchitectEditor" );
	}

	/** Creates a new dungeon editor */
	virtual TSharedRef<IDungeonArchitectEditor> CreateDungeonEditor(const EToolkitMode::Type Mode, const TSharedPtr< IToolkitHost >& InitToolkitHost) = 0;
	virtual EAssetTypeCategories::Type GetDungeonAssetCategoryBit() const = 0;
};

