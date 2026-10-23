#pragma once
#include "ts3/layout.h"
namespace ts3::anim::m115 {
struct Builder { void* vtable; int32_t refs; unsigned char state[0x21c]; float value; };
TS3_X86_OFFSET(Builder,value,0x224);
// TS3W.exe 0x0083F5B0: Construct the first specialized locomotion builder.
Builder* ConstructF5B0(Builder*);
// TS3W.exe 0x0083F610: Construct the second specialized locomotion builder.
Builder* ConstructF610(Builder*);
// TS3W.exe 0x0083F660: Construct the third specialized locomotion builder.
Builder* ConstructF660(Builder*);
// TS3W.exe 0x0083F6A0: Construct the fourth specialized locomotion builder.
Builder* ConstructF6A0(Builder*);
// TS3W.exe 0x0083F740: Destroy the embedded state, optionally free, and return this even after free.
Builder* Delete(Builder*,uint32_t);
struct String { char *begin,*end,*capacity; const char* descriptor; };
struct Key { uint32_t words[4]; };
struct Record { Key key; uint16_t weather,style; String name; uint8_t flag; };
struct Registry { unsigned char opaque[0x20]; Record *begin,*end,*capacity; uint32_t descriptor,count; uint8_t enabled; };
TS3_X86_OFFSET(Registry,enabled,0x34);
TS3_X86_SIZE(Record,0x28);
// TS3W.exe 0x0083F920: Register three weather variants per style; retain uint16 index wrap and truncated masks.
void __stdcall Register(Registry*,const char*,uint32_t,uint32_t,uint32_t,uint32_t);
}
#if defined(_M_IX86)
namespace ts3::anim::m115::hooks {
inline Builder* __fastcall F5B0(Builder* p,void*) {return ConstructF5B0(p);}
inline Builder* __fastcall F610(Builder* p,void*) {return ConstructF610(p);}
inline Builder* __fastcall F660(Builder* p,void*) {return ConstructF660(p);}
inline Builder* __fastcall F6A0(Builder* p,void*) {return ConstructF6A0(p);}
inline Builder* __fastcall Destroy(Builder* p,void*,uint32_t f) {return Delete(p,f);}
}
#endif
