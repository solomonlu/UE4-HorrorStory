//$ Copyright 2015 Ali Akbar, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//
#pragma once

class UDungeonBuilder;
class UDungeonEdModeHandler;


class DUNGEONARCHITECTEDITOR_API FDungeonEdModeHandlerFactory {
public:
	static TSharedPtr<FDungeonEdModeHandlerFactory> Get();
	static void Register();

	void RegisterHandler(TSubclassOf<UDungeonBuilder> BuilderClass, TSubclassOf<UDungeonEdModeHandler> HandlerClass);
	UDungeonEdModeHandler* CreateHandler(TSubclassOf<UDungeonBuilder> BuilderClass, UObject* Outer = (UObject*)GetTransientPackage());

private:
	/**
	* The static object referencer object that is shared across all
	* garbage collectible non-UObject objects.
	*/
	static TSharedPtr<FDungeonEdModeHandlerFactory> Instance;

	/**
	* Initializes the global object referencer and adds it to the root set.
	*/
	static void StaticInit(void);


private:
	FDungeonEdModeHandlerFactory() {}
	TMap<TSubclassOf<UDungeonBuilder>, TSubclassOf<UDungeonEdModeHandler>> BuilderToHandlerMapping;
};