//$ Copyright 2015 Ali Akbar, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//
#include "DungeonArchitectRuntimePrivatePCH.h"
#include "DungeonDebug.h"
#include "SceneProviderCommand.h"

DEFINE_LOG_CATEGORY(LogDungeonDebug);

ADungeonDebug::ADungeonDebug(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{

}

void ADungeonDebug::ExecuteDebugCommand(int32 CommandID)
{
	switch (CommandID) {
	case 0: ExecuteCommand0(); break;
	case 1: ExecuteCommand1(); break;
	case 2: ExecuteCommand2(); break;
	case 3: ExecuteCommand3(); break;
	case 4: ExecuteCommand4(); break;
	case 5: ExecuteCommand5(); break;

	}
}

void ADungeonDebug::ExecuteCommand0()
{
	SceneProviderCommand_CloneActor Command(nullptr, CloneTemplate, FTransform::Identity, "NODE-x");
	Command.Execute(GetWorld());
}
