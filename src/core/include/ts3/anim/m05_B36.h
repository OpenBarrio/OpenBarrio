#pragma once
#include "ts3/anim/m05_layouts.h"
namespace ts3::anim::b36 {
struct Body {
    void* vtable; unsigned char opaque04[16]; void* object14; void* secondary;
    ListLink controllers; void* controllerDescriptor; unsigned char style[0x7c];
    ListLink effects; void* effectsDescriptor; uint32_t unknownB0;
    void** slaveBegin; void** slaveEnd; void** slaveCapacity; void* slaveDescriptor; void* master;
    unsigned char opaqueC8[0x5c]; void* interface124; int32_t cachedMode;
    unsigned char opaque12C[0x40]; int32_t limit,current,upper; void* token;
    unsigned char opaque17C[2]; uint8_t flag17E; unsigned char opaque17F[0xc9];
    int32_t value248,value24C;
};
TS3_X86_OFFSET(Body,interface124,0x124);
TS3_X86_OFFSET(Body,current,0x170);
TS3_X86_OFFSET(Body,value248,0x248);
struct Motion { uint32_t kind; float values[4]; uint8_t flag14,flag15; uint16_t padding; void* resource; };
TS3_X86_OFFSET(Motion,resource,0x18);
// TS3W.exe 0x00848C90: Zero the output then ask the optional motion interface for its vector.
void GetVector(Body*,Float4Value*);
// TS3W.exe 0x00848CD0: Query an optional interface vector; absent interface clears output and returns false.
bool TryVector(Body*,Float4Value*);
// TS3W.exe 0x008491E0: Test signed progression bounds and matching cached values.
int32_t IsComplete(Body*);
// TS3W.exe 0x00849210: Remove the old master token and optionally install a nonempty replacement.
void SetToken(Body*,void**,uint32_t);
// TS3W.exe 0x00849270: Dispatch a recognized event through the preceding interface pointer.
void Dispatch(void*,void*,uint32_t);
// TS3W.exe 0x008495D0: Initialize motion state, leaving the two padding bytes untouched.
Motion* Construct(Motion*,uint32_t);
// TS3W.exe 0x00849600: Select root displacement from a retained controller and release it.
void SelectDisplacement(void*,Float4Value*,void*);
// TS3W.exe 0x00849770: Consume a retained controller and test its routing kind.
bool RoutingKind(void*);
// TS3W.exe 0x00849810: Consume a controller chain and calculate its remaining duration.
float Remaining(void*,uint64_t,uint32_t,uint32_t,bool);
// TS3W.exe 0x008499B0: Release the retained motion resource, without clearing the field.
void Destroy(Motion*);
// TS3W.exe 0x008499E0: Copy motion state and retain the resource; keep padding and quiet SNaNs only in the last float.
Motion* Copy(Motion*,const Motion*);
// TS3W.exe 0x0084AF20: Lazily cache a mode from routing state and the body flag.
int32_t Mode(Body*);
// TS3W.exe 0x0084C3B0: Create a visual effect and attach its components to the body.
void CreateEffect(Body*,uint32_t,uint32_t,bool);
// TS3W.exe 0x0084CE30: Update visual effects and optionally remove the list.
void ClearEffects(Body*,bool);
struct Range { void* begin; void* end; void* capacity; uint32_t descriptor; void* inlineBegin; unsigned char rest[0xa8]; };
TS3_X86_SIZE(Range,0xbc);
// TS3W.exe 0x0084D700: Destroy a scalar or reverse-order array, optionally freeing storage; keep the signed decremented-cookie loop.
void* DeleteRanges(Range*,uint32_t);
}

namespace ts3::anim::b36 {
inline void __fastcall HookGetVector(Body* s,void*,Float4Value* out) { return GetVector(s,out); }
inline bool __fastcall HookTryVector(Body* s,void*,Float4Value* out) { return TryVector(s,out); }
inline int32_t __fastcall HookIsComplete(Body* s,void*) { return IsComplete(s); }
inline void __fastcall HookSetToken(Body* s,void*,void** r,uint32_t f) { return SetToken(s,r,f); }
inline void __fastcall HookDispatch(void* s,void*,void* e,uint32_t f) { return Dispatch(s,e,f); }
inline Motion* __fastcall HookConstruct(Motion* s,void*,uint32_t k) { return Construct(s,k); }
inline void __fastcall HookDestroy(Motion* s,void*) { return Destroy(s); }
inline Motion* __fastcall HookCopy(Motion* s,void*,const Motion* f) { return Copy(s,f); }
inline int32_t __fastcall HookMode(Body* s,void*) { return Mode(s); }
inline void __fastcall HookCreateEffect(Body* s,void*,uint32_t n,uint32_t slot,bool f) { return CreateEffect(s,n,slot,f); }
inline void __fastcall HookClearEffects(Body* s,void*,bool f) { return ClearEffects(s,f); }
inline void* __fastcall HookDeleteRanges(Range* s,void*,uint32_t f) { return DeleteRanges(s,f); }
}
