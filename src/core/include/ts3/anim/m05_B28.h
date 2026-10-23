#pragma once
#include "ts3/layout.h"
namespace ts3::anim::b28 {
struct Key { uint32_t low,high; };
struct Parent { Key key; uint32_t slot; unsigned char opaque[68]; uint8_t enabled,mode; };
struct Entity { void* vtable; int32_t refs; void* interfaceTable; uint32_t flags; Key key; Parent* parent; };
struct Entry { uint32_t key; Entity* value; };
struct Map { void* vtable; int32_t refs; Entry *begin,*end,*capacity; const char* descriptor; };
struct Stream { void* opaque[2]; void* data; uintptr_t reserved; uint32_t version; };
// TS3W.exe 0x00581560: Resolves an entity key and optionally clears a stale key.
void* Resolve(Entity*, uint8_t);
// TS3W.exe 0x005815A0: Frees and replaces the parent; returns null when previously empty.
Parent* SetParent(Entity*, Parent*);
// TS3W.exe 0x005815D0: Resolves a key without checking the manager.
void* ResolveKey(Key*);
// TS3W.exe 0x00581700: Resolves the parent slot through the world component.
void* ResolveParent(Parent*);
// TS3W.exe 0x00581750: Replaces a matching entry, releasing the old entity after retaining the new.
uint8_t Replace(Map*, uint32_t, Entity*);
// TS3W.exe 0x00581810: Returns the first matching entry.
Entity* Find(Map*, uint32_t);
// TS3W.exe 0x00581840: Resolves the first matching entry.
void* FindResolved(Map*, uint32_t);
// TS3W.exe 0x00581890: Initializes the observed entity prefix.
Entity* ConstructEntity(Entity*, uint32_t);
// TS3W.exe 0x00581910: Latches parent validity and optionally detaches the parent.
Parent* TakeParent(Entity*, uint8_t);
// TS3W.exe 0x00581960: Serializes entity and optional parent in original order.
uint8_t SaveEntity(Entity*, Stream*, uint32_t);
// TS3W.exe 0x00581AD0: Loads entity and optional parent, preserving legacy flag conversion.
uint8_t LoadEntity(Entity*, Stream*, uint32_t);
// TS3W.exe 0x00581CA0: Releases entries then frees storage and restores the base table.
uintptr_t DestroyMap(Map*);
// TS3W.exe 0x00581CE0: Serializes the ordered map and each nonnull entity.
uint8_t SaveMap(Map*, Stream*, uint32_t);
// TS3W.exe 0x00581EE0: Destroys then optionally frees, returning the original pointer.
Map* DeleteMap(Map*, uint32_t);
// TS3W.exe 0x00582010: Initializes the map and reserves eight entries.
Map* ConstructMap(Map*);
}

namespace ts3::anim {
// x86 thiscall registration adapters; pointer-bearing layouts above remain native-width.
struct b28_hooks {
 uint32_t Resolve(uint32_t clear) { return static_cast<uint32_t>(reinterpret_cast<uintptr_t>(b28::Resolve(reinterpret_cast<b28::Entity*>(this),uint8_t(clear)))); }
 uint32_t SetParent(void* parent) { return static_cast<uint32_t>(reinterpret_cast<uintptr_t>(b28::SetParent(reinterpret_cast<b28::Entity*>(this),static_cast<b28::Parent*>(parent)))); }
 uint32_t ResolveKey() { return static_cast<uint32_t>(reinterpret_cast<uintptr_t>(b28::ResolveKey(reinterpret_cast<b28::Key*>(this)))); }
 uint32_t ResolveParent() { return static_cast<uint32_t>(reinterpret_cast<uintptr_t>(b28::ResolveParent(reinterpret_cast<b28::Parent*>(this)))); }
 bool Replace(uint32_t key,void* value) { return b28::Replace(reinterpret_cast<b28::Map*>(this),key,static_cast<b28::Entity*>(value)) != 0; }
 uint32_t Find(uint32_t key) { return static_cast<uint32_t>(reinterpret_cast<uintptr_t>(b28::Find(reinterpret_cast<b28::Map*>(this),key))); }
 uint32_t FindResolved(uint32_t key) { return static_cast<uint32_t>(reinterpret_cast<uintptr_t>(b28::FindResolved(reinterpret_cast<b28::Map*>(this),key))); }
 uint32_t ConstructEntity(uint32_t flags) { return static_cast<uint32_t>(reinterpret_cast<uintptr_t>(b28::ConstructEntity(reinterpret_cast<b28::Entity*>(this),flags))); }
 uint32_t TakeParent(uint32_t take) { return static_cast<uint32_t>(reinterpret_cast<uintptr_t>(b28::TakeParent(reinterpret_cast<b28::Entity*>(this),uint8_t(take)))); }
 bool SaveEntity(void* stream,uint32_t flags) { return b28::SaveEntity(reinterpret_cast<b28::Entity*>(this),static_cast<b28::Stream*>(stream),flags) != 0; }
 bool LoadEntity(void* stream,uint32_t flags) { return b28::LoadEntity(reinterpret_cast<b28::Entity*>(this),static_cast<b28::Stream*>(stream),flags) != 0; }
 uint32_t DestroyMap() { return static_cast<uint32_t>(b28::DestroyMap(reinterpret_cast<b28::Map*>(this))); }
 bool SaveMap(void* stream,uint32_t flags) { return b28::SaveMap(reinterpret_cast<b28::Map*>(this),static_cast<b28::Stream*>(stream),flags) != 0; }
 uint32_t DeleteMap(uint32_t flags) { return static_cast<uint32_t>(reinterpret_cast<uintptr_t>(b28::DeleteMap(reinterpret_cast<b28::Map*>(this),flags))); }
 uint32_t ConstructMap() { return static_cast<uint32_t>(reinterpret_cast<uintptr_t>(b28::ConstructMap(reinterpret_cast<b28::Map*>(this)))); }
};
}
