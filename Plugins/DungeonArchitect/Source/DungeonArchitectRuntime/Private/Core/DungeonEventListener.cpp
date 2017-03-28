//$ Copyright 2015 Ali Akbar, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "DungeonArchitectRuntimePrivatePCH.h"
#include "DungeonEventListener.h"
#include "Dungeon.h"

DEFINE_LOG_CATEGORY(DungeonEventListenerLog);


void UDungeonEventListener::OnPreDungeonBuild_Implementation(ADungeon* Dungeon)
{

}

void UDungeonEventListener::OnDungeonLayoutBuilt_Implementation(ADungeon* Dungeon)
{

}

void UDungeonEventListener::OnMarkersEmitted_Implementation(ADungeon* Dungeon)
{

}

void UDungeonEventListener::OnPostDungeonBuild_Implementation(ADungeon* Dungeon)
{

}

void UDungeonEventListener::OnPreDungeonDestroy_Implementation(ADungeon* Dungeon)
{

}

void UDungeonEventListener::OnDungeonDestroyed_Implementation(ADungeon* Dungeon)
{

}

void DungeonUtils::FDungeonEventListenerNotifier::NotifyPreDungeonBuild(ADungeon* Dungeon)
{
	if (!Dungeon) return;
	for (UDungeonEventListener* Listener : Dungeon->EventListeners) {
		if (!Listener) continue;
		Listener->OnPreDungeonBuild(Dungeon);
	}
}

void DungeonUtils::FDungeonEventListenerNotifier::NotifyDungeonLayoutBuilt(ADungeon* Dungeon)
{
	if (!Dungeon) return;
	for (UDungeonEventListener* Listener : Dungeon->EventListeners) {
		if (!Listener) continue;
		Listener->OnDungeonLayoutBuilt(Dungeon);
	}
}

void DungeonUtils::FDungeonEventListenerNotifier::NotifyMarkersEmitted(ADungeon* Dungeon)
{
	if (!Dungeon) return;
	for (UDungeonEventListener* Listener : Dungeon->EventListeners) {
		if (!Listener) continue;
		Listener->OnMarkersEmitted(Dungeon);
	}
}

void DungeonUtils::FDungeonEventListenerNotifier::NotifyPostDungeonBuild(ADungeon* Dungeon)
{
	if (!Dungeon) return;
	for (UDungeonEventListener* Listener : Dungeon->EventListeners) {
		if (!Listener) continue;
		Listener->OnPostDungeonBuild(Dungeon);
	}
}

void DungeonUtils::FDungeonEventListenerNotifier::NotifyPreDungeonDestroy(ADungeon* Dungeon)
{
	if (!Dungeon) return;
	for (UDungeonEventListener* Listener : Dungeon->EventListeners) {
		if (!Listener) continue;
		Listener->OnPreDungeonDestroy(Dungeon);
	}
}

void DungeonUtils::FDungeonEventListenerNotifier::NotifyDungeonDestroyed(ADungeon* Dungeon)
{
	if (!Dungeon) return;
	for (UDungeonEventListener* Listener : Dungeon->EventListeners) {
		if (!Listener) continue;
		Listener->OnDungeonDestroyed(Dungeon);
	}
}
