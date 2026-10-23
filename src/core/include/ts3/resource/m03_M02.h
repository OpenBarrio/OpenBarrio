#pragma once
#include "ts3/layout.h"
namespace ts3::resource::m03::m02 {
struct Key { uint64_t instance; uint32_t type,group; };
struct Keys { Key *begin,*end,*capacity; const char* name; };
struct ParseContext { Keys* keys; const char16_t* prefix; uint32_t length; };
struct Shadow {
    void* primary; uintptr_t unknown04; void* database; void* reference;
    uint8_t base[0xa8]; void* index; uint32_t reservedBC;
    uint8_t mutex[24],reservedD8[8]; int32_t state; uint32_t reservedE4;
    uint64_t tick,delay,stamp; uint32_t attributes,mode;
};
TS3_X86_OFFSET(Shadow,index,0xb8);
TS3_X86_OFFSET(Shadow,state,0xe0);
TS3_X86_OFFSET(Shadow,mode,0x104);
// TS3W.exe 0x00731000: parse a matching preloader line into resource keys; only ASCII spaces delimit names.
uint8_t __cdecl Parse(const char16_t*,const char16_t*,ParseContext*);
// TS3W.exe 0x007342F0: construct a shadow watcher, retaining untouched padding and reciprocal-clock delay.
Shadow* __fastcall Construct(Shadow*,void*);
// TS3W.exe 0x00734E00: serialize matching shadow indexes to a file, preserving prefix and cleanup/AL quirks.
uintptr_t __fastcall Save(void*,void*,const char16_t*,const char16_t*,uint8_t,int);
// TS3W.exe 0x00735320: refresh a pending shadow index under its mutex; allocation failure still updates file state.
void __fastcall Refresh(Shadow*,void*);
}
