//$ Copyright 2015 Ali Akbar, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//
#include "DungeonArchitectRuntimePrivatePCH.h"
#include "IDungeonArchitectRuntime.h"

class FDungeonArchitectRuntime : public IDungeonArchitectRuntime
{
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};

IMPLEMENT_MODULE( FDungeonArchitectRuntime, DungeonArchitectRuntime )

void FDungeonArchitectRuntime::StartupModule()
{
	// This code will execute after your module is loaded into memory (but after global variables are initialized, of course.)
}


void FDungeonArchitectRuntime::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

