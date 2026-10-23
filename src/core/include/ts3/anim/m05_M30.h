#pragma once
#include "ts3/anim/m05_layouts.h"
namespace ts3::anim::m30 {
struct Binding { uint32_t low,high,page,item,bone,slot; };
struct Entry { uint32_t key; void* value; };
struct Map { void** vtable; int32_t refs; Entry *begin,*end; };
struct Clip {
 Controller base;
 Binding *begin,*end,*capacity;
 void* descriptor;
 unsigned char storage[0x4b8];
 ClipIkInfo ik;
 void *resource,*secondary;
 Map* map;
 float stateDC,stateE0,stateE4,period;
 uint32_t mode,valueF0; int32_t valueF4;
 uint8_t flags[7],pad;
};
TS3_X86_OFFSET(Clip,ik,0x4f0);
TS3_X86_OFFSET(Clip,map,0x5d8);
TS3_X86_OFFSET(Clip,flags,0x5f8);
// TS3W.exe 0x00569160: rebuild resolved IK bindings and foreign-namespace latch.
void Rebuild(Clip*);
// TS3W.exe 0x00569330: update mirroring only when the input byte changes.
void SetMirror(Clip*,uint8_t);
// TS3W.exe 0x00569360: retain map before releasing old map, then refresh IK.
void SetMap(Clip*,Map*);
// TS3W.exe 0x00569420: merge source entries and refresh the retained map.
void Merge(Clip*,Map*);
// TS3W.exe 0x005694F0: apply packed sixteen-byte IK edits, then rebuild bindings.
void Apply(Clip*,const uint32_t*);
// TS3W.exe 0x005695F0: populate a clip evaluator; consume nonnegative override, preserving NaN skip.
void* Evaluate(Clip*,void*);
#if defined(_M_IX86)
namespace hooks {
void __fastcall Rebuild(Clip*,void*);
void __fastcall SetMirror(Clip*,void*,uint8_t);
void __fastcall SetMap(Clip*,void*,Map*);
void __fastcall Merge(Clip*,void*,Map*);
void __fastcall Apply(Clip*,void*,const uint32_t*);
void* __fastcall Evaluate(Clip*,void*,void*);
}
#endif
}
