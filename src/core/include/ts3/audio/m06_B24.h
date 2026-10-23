#pragma once
#include "ts3/layout.h"
#include "ts3/misc/cls_5044c0_x.h"
namespace ts3::audio::b24 {
struct Pair { uint32_t key; void* resource; };
struct TaggedPair { uint32_t key; void* resource; uint8_t flag; };
struct Temporary { void* resource; uint8_t flag; };
struct Node { uint32_t key; uint8_t pad[0xb4]; Node* next; };
struct Map { void* unused; Node** buckets; uint32_t count; uint8_t pad[0x10]; const char* label; };
struct Sound {
    void* primary; void* secondary; void* tertiary;
    uint8_t pad0c[0x58-0x0c]; void* player; float position;
    uint8_t pad60[0xf4-0x60]; void* properties;
    uint8_t padf8[0x334-0xf8]; float variation;
    uint8_t pad338[0x1898-0x338]; int32_t state; uint32_t pad189c;
    void* curves[3]; void* identity;
    float attenuation,fade,release,maximum;
    uint8_t enabled,active; uint8_t pad18c2[2];
    uint16_t *begin,*end,*capacity; const char* label;
};
TS3_X86_OFFSET(Sound,properties,0xf4);
TS3_X86_OFFSET(Sound,curves,0x18a0);
TS3_X86_OFFSET(Sound,begin,0x18c4);
TS3_X86_OFFSET(Node,next,0xb8);
TS3_X86_OFFSET(Map,label,0x1c);
// TS3W.exe 0x00870690: load optional curves and sound properties, retaining AL-only success.
uintptr_t Load(Sound*);
// TS3W.exe 0x008708A0: copy base state and the borrowed identity pointer.
void* Copy(Sound*,Sound*);
// TS3W.exe 0x008708C0: retain before publishing and releasing the old pointer.
void** Assign(void**,void*);
// TS3W.exe 0x008708F0: replace a tree root with the cleanup helper's result.
void** Clear(void**);
// TS3W.exe 0x00870910: copy a key/resource pair and retain the resource.
Pair* CopyPair(Pair*,const Pair*);
// TS3W.exe 0x00870940: publish curve-controlled properties and the UTF16 filename.
uintptr_t Publish(Sound*);
// TS3W.exe 0x00870AE0: update fade and release parameters after the base tick.
uintptr_t Tick(Sound*,float,float,uint32_t);
// TS3W.exe 0x00870BB0: apply listener attenuation and callback-visible active transitions.
float Gain(Sound*,float,float,float,float,Sound*,uint8_t*);
// TS3W.exe 0x00870FA0: temporarily retain a resource around flag publication.
Temporary* InitTemporary(Temporary*,void*,uint8_t);
// TS3W.exe 0x00870FE0: construct a key/resource pair from two separate cells.
Pair* MakePair(Pair*,const uint32_t*,void* const*);
// TS3W.exe 0x00871010: copy a tagged pair, retaining before publishing its flag.
TaggedPair* CopyTagged(TaggedPair*,const TaggedPair*);
// TS3W.exe 0x00871040: rehash chains, reversing traversal order and installing a sentinel.
void* Rehash(Map*,uint32_t);
// TS3W.exe 0x00871100: construct the derived sound with its three interface tables.
Sound* Construct(Sound*);
// TS3W.exe 0x008711B0: return the sound class identifier.
uint32_t Type();
}

namespace ts3::audio::b24 {
inline uintptr_t __fastcall HookLoad(Sound* s,void*) { return Load(s); }
inline void* __fastcall HookCopy(Sound* s,void*,Sound* to) { return Copy(s,to); }
inline void** __fastcall HookAssign(void** s,void*,void* p) { return Assign(s,p); }
inline void** __fastcall HookClear(void** s,void*) { return Clear(s); }
inline Pair* __fastcall HookCopyPair(Pair* s,void*,const Pair* p) { return CopyPair(s,p); }
inline uintptr_t __fastcall HookPublish(Sound* s,void*) { return Publish(s); }
inline uintptr_t __fastcall HookTick(Sound* s,void*,float a,float b,uint32_t c) { return Tick(s,a,b,c); }
inline float __fastcall HookGain(Sound* s,void*,float a,float b,float c,float d,Sound* source,uint8_t* out) { return Gain(s,a,b,c,d,source,out); }
inline Temporary* __fastcall HookInitTemporary(Temporary* s,void*,void* p,uint8_t f) { return InitTemporary(s,p,f); }
inline TaggedPair* __fastcall HookCopyTagged(TaggedPair* s,void*,const TaggedPair* p) { return CopyTagged(s,p); }
inline void* __fastcall HookRehash(Map* s,void*,uint32_t n) { return Rehash(s,n); }
inline Sound* __fastcall HookConstruct(Sound* s,void*) { return Construct(s); }
}
