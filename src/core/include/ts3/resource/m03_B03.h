#pragma once
#include "ts3/layout.h"
namespace ts3::resource {
// Optional native boundaries; null selects the original/verified dependency.
struct B03Boundary {
    void* (*destroy)(void*,uint32_t)=nullptr;
    void* (*open)(void*,const char16_t*,int)=nullptr;
};
B03Boundary& B03Dependencies();
// TS3W.exe 0x00736690: adjust the observer view by -0x1D8 and forward deleting destruction.
void* B03DestroyObserver(void*,uint32_t);
// TS3W.exe 0x007366A0: adjust the manager view by -8 and forward deleting destruction.
void* B03DestroyManager(void*,uint32_t);
// TS3W.exe 0x007381F0: open a shadow database with flags zero.
void* B03Open(void*,const char16_t*);
// TS3W.exe 0x00739270: byte-swap count consecutive 32-bit words; zero count does not touch memory.
void B03Swap(void*,uint32_t);
// TS3W.exe 0x00739320: load a temporary memory stream into the owning stream; keeps AL-only results and cleanup EAX upper bits.
uintptr_t B03Load(void**);
#if defined(_M_IX86)
#define B03_ABI __fastcall
#else
#define B03_ABI
#endif
// TS3W.exe 0x00736690: thiscall ABI adapter.
void* B03_ABI HookB03Observer(void*,void*,uint32_t);
// TS3W.exe 0x007366A0: thiscall ABI adapter.
void* B03_ABI HookB03Manager(void*,void*,uint32_t);
// TS3W.exe 0x007381F0: thiscall ABI adapter.
void* B03_ABI HookB03Open(void*,void*,const char16_t*);
}
