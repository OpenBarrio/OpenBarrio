#pragma once
#include "ts3/layout.h"
namespace ts3::resource {
// Native injection for the three unreconstructed lifetime/interface boundaries.
struct C02Boundary {
    void (*deleteMutex)(void*)=nullptr;
    void (*destroyDatabase)(void*)=nullptr;
    void (*freeResource)(void*)=nullptr;
    uintptr_t (*databaseMethod)(void*)=nullptr;
};
C02Boundary& C02Dependencies();
// TS3W.exe 0x007346E0: subtract the ref-view displacement and forward deleting destruction.
void* C02DestroyRef(void*,uint32_t);
// TS3W.exe 0x007346F0: translate database view to primary and forward the database method.
uintptr_t C02DatabaseMethod(void*);
// TS3W.exe 0x00734700: subtract the database-view displacement and forward deleting destruction.
void* C02DestroyDatabase(void*,uint32_t);
// TS3W.exe 0x00734820: destroy mutex and database, install base vptr, optionally free on flags bit zero; return self even after free.
void* C02Destroy(void*,uint32_t);
#if defined(_M_IX86)
#define C02_ABI __fastcall
#else
#define C02_ABI
#endif
// TS3W.exe 0x007346E0: thiscall adapter.
void* C02_ABI HookC02Ref(void*,void*,uint32_t);
// TS3W.exe 0x007346F0: thiscall adapter.
uintptr_t C02_ABI HookC02Method(void*,void*);
// TS3W.exe 0x00734700: thiscall adapter.
void* C02_ABI HookC02Database(void*,void*,uint32_t);
// TS3W.exe 0x00734820: thiscall adapter.
void* C02_ABI HookC02Destroy(void*,void*,uint32_t);
}
