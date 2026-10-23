#pragma once
#include "ts3/anim/m05_layouts.h"
namespace ts3::anim::m73 {
struct alignas(16) Record { unsigned char vectors[32]; uint32_t words[4]; float weight; unsigned char untouched[12]; };
struct Records { Record* begin; Record* end; Record* capacity; unsigned char opaque[0x2e4]; float total; };
struct Workspace { void* begin; void* end; void* capacity; uint32_t opaque; unsigned char storage[0x3000]; float scalar; unsigned char tail[12]; };
struct alignas(16) Pose {
    void** vtable; uint32_t refs; unsigned char opaque08[8];
    Float4Value translation,rotation; RigResource* rig; SrtValue* srt;
    void* auxiliary38; void* auxiliary3c; void* auxiliary40; void* workspace;
    void* opaque48; void* weights; uint16_t flags,boneCount;
    unsigned char opaque54[0x708c]; float ones[300]; float one;
    unsigned char opaque7594[12]; Workspace work[6];
    unsigned char opaque19660[0x4b0]; Float4Value finalRotation,finalTranslation;
};
TS3_X86_OFFSET(Records,total,0x2f0);
TS3_X86_OFFSET(Pose,rig,0x30);
TS3_X86_OFFSET(Pose,ones,0x70e0);
TS3_X86_OFFSET(Pose,work,0x75a0);
TS3_X86_OFFSET(Pose,finalRotation,0x19b10);
// TS3W.exe 0x00574E70: Append only the live 0x34 bytes, leaving record padding untouched.
uintptr_t Append(Records*,const Record*);
// TS3W.exe 0x00574ED0: Append strictly positive weights and accumulate in single precision.
void AddWeighted(Records*,const Record*);
// TS3W.exe 0x00574F10: Destroy and optionally return the object to the synchronized scratch pool.
Pose* Delete(Pose*,uint32_t);
// TS3W.exe 0x00574F50: Initialize scratch pose state, preserving opaque bytes and temporary rig refs.
Pose* Construct(Pose*,RigResource*);
// TS3W.exe 0x00575120: Lazily create weights then assign a nonnull source vector.
void SetWeights(Pose*,void*);
// TS3W.exe 0x00575430: Hash lowercased unsigned bytes with multiply-before-XOR FNV ordering.
uint32_t Hash(void*,const char*);
}
#if defined(_M_IX86)
namespace ts3::anim::m73::hooks {
inline uintptr_t __fastcall Append(Records* s,void*,const Record* r){return m73::Append(s,r);}
inline void __fastcall AddWeighted(Records* s,void*,const Record* r){m73::AddWeighted(s,r);}
inline Pose* __fastcall Delete(Pose* s,void*,uint32_t f){return m73::Delete(s,f);}
inline Pose* __fastcall Construct(Pose* s,void*,RigResource* r){return m73::Construct(s,r);}
inline void __fastcall SetWeights(Pose* s,void*,void* r){m73::SetWeights(s,r);}
inline uint32_t __fastcall Hash(void* s,void*,const char* r){return m73::Hash(s,r);}
}
#endif
