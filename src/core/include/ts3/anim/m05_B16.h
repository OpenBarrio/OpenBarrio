#pragma once
#include "ts3/layout.h"
namespace ts3::anim::b16 {
struct Ref { void** vtable; volatile int32_t refs; };
struct State {
    void** vtable; unsigned char opaque[0x28];
    Ref* owner; void* delegate; Ref* auxiliary;
};
TS3_X86_OFFSET(Ref,refs,4);
TS3_X86_OFFSET(State,owner,0x2c);
TS3_X86_OFFSET(State,delegate,0x30);
TS3_X86_OFFSET(State,auxiliary,0x34);
// TS3W.exe 0x0056E1C0: Forward time to delegate slot 2, even without an owner.
void SetTime(State* self, float value);
// TS3W.exe 0x0056E230: Forward weight to delegate slot 4, even without an owner.
void SetWeight(State* self, float value);
// TS3W.exe 0x0056E250: Read slot 5 when owner and delegate exist, otherwise positive zero.
float Time(State* self);
// TS3W.exe 0x0056E270: Read slot 6 when owner and delegate exist, otherwise positive zero.
float Duration(State* self);
// TS3W.exe 0x0056E290: Read slot 7; an owner without a delegate uses global 0114F594.
float Rate(State* self);
// TS3W.exe 0x0056E2B0: Read slot 8; an owner without a delegate uses global 0114F594.
float Start(State* self);
// TS3W.exe 0x0056E2D0: Read slot 9; an owner without a delegate uses global 0114F594.
float End(State* self);
// TS3W.exe 0x0056E2F0: Read slot 10; an owner without a delegate uses global 0114F594.
float Weight(State* self);
// TS3W.exe 0x0056E310: Forward bool slot 11, or return false.
bool Active(State* self);
// TS3W.exe 0x0056E330: Forward bool slot 12, or return false.
bool Finished(State* self);
// TS3W.exe 0x0056E350: Forward reset slot 1 if a delegate exists; no defined EAX return.
void Reset(State* self);
// TS3W.exe 0x0056E480: Return controller interface type 073B9D8C (verified twin with changed immediate).
uint32_t Type(void* self);
// TS3W.exe 0x0056E490: Query type 073B9D8C and adjust the interface pointer back by eight bytes.
void* Query(void* self, uint32_t id);
// TS3W.exe 0x0056E640: Retain owner, publish it, then release old owner; restore last ref before deleting.
uintptr_t SetOwner(State* self, Ref* value);
// TS3W.exe 0x0056E690: Retain auxiliary, publish it, then release old auxiliary; restore last ref before deleting.
uintptr_t SetAuxiliary(State* self, Ref* value);
}
#if defined(_M_IX86)
namespace ts3::anim::b16 {
inline void __fastcall HookSetTime(State* self, void*, float value) { return SetTime(self,value); }
inline void __fastcall HookSetWeight(State* self, void*, float value) { return SetWeight(self,value); }
inline float __fastcall HookTime(State* self, void*) { return Time(self); }
inline float __fastcall HookDuration(State* self, void*) { return Duration(self); }
inline float __fastcall HookRate(State* self, void*) { return Rate(self); }
inline float __fastcall HookStart(State* self, void*) { return Start(self); }
inline float __fastcall HookEnd(State* self, void*) { return End(self); }
inline float __fastcall HookWeight(State* self, void*) { return Weight(self); }
inline bool __fastcall HookActive(State* self, void*) { return Active(self); }
inline bool __fastcall HookFinished(State* self, void*) { return Finished(self); }
inline void __fastcall HookReset(State* self, void*) { return Reset(self); }
inline uint32_t __fastcall HookType(void* self, void*) { return Type(self); }
inline void* __fastcall HookQuery(void* self, void*, uint32_t id) { return Query(self,id); }
inline uintptr_t __fastcall HookSetOwner(State* self, void*, Ref* value) { return SetOwner(self,value); }
inline uintptr_t __fastcall HookSetAuxiliary(State* self, void*, Ref* value) { return SetAuxiliary(self,value); }
}
#endif
