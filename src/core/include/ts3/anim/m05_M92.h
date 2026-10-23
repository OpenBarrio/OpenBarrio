#pragma once
#include "ts3/anim/m05_B25.h"
namespace ts3::anim::m92 {
struct PairEvent { b25::Event base; uint32_t keys[2]; };
struct KeyEvent { b25::Event base; uint32_t opaque[4]; uint32_t key; };
struct Events { b25::Event** begin; b25::Event** end; b25::Event** capacity; uint32_t descriptor; int32_t cached; };
// TS3W.exe 0x0057FE70: Compare base and two keys, preserving upper EAX.
uint32_t __fastcall EqualPair(PairEvent*,void*,const PairEvent*);
// TS3W.exe 0x0057FEA0: Count pointer entries using an arithmetic span shift, like verified 004AAFE0.
int32_t __fastcall Count(Events*,void*);
// TS3W.exe 0x0057FEB0: Lazily cache the LAST event with flag 4, leaving other cached values untouched.
int32_t __fastcall FindFlagged(Events*,void*);
// TS3W.exe 0x0057FF00: Return the unchecked indexed event pointer.
b25::Event* __fastcall At(Events*,void*,uint32_t);
// TS3W.exe 0x0057FF30: Delete nonnull entries in order and free storage without clearing dangling fields.
void __fastcall Destroy(Events*,void*);
// TS3W.exe 0x0057FFD0: Insert the key at +38 and forward the insertion result (normally an expired local iterator).
void* __fastcall CollectKey(KeyEvent*,void*,void*);
}
