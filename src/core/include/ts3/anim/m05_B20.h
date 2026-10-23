#pragma once
#include "ts3/anim/m05_layouts.h"
namespace ts3::anim::b20 {
struct alignas(16) Pose {
    unsigned char prefix[0x30];
    RigResource* rig;
    SrtValue* local;
    SrtValue* world;
    float* weights;
    void* auxiliary;
    void* workspace;
    void* reserved;
    void* vertexWeights;
    uint16_t flags, count;
    unsigned char gap[12];
    SrtValue localStorage[300];
    SrtValue worldStorage[300];
    uint32_t weightStorage[301];
    unsigned char gap7594[12];
    unsigned char auxiliaryStorage[0x120c0];
    float weightsStorage[300];
    Float4Value identity,zero;
};
TS3_X86_OFFSET(Pose,localStorage,0x60);
TS3_X86_OFFSET(Pose,worldStorage,0x38a0);
TS3_X86_OFFSET(Pose,weightStorage,0x70e0);
TS3_X86_OFFSET(Pose,auxiliaryStorage,0x75a0);
TS3_X86_OFFSET(Pose,weightsStorage,0x19660);
TS3_X86_OFFSET(Pose,identity,0x19b10);
struct Coherent { unsigned char prefix[0x24]; ResourcePrefix* resource; unsigned char gap[12]; uint32_t flags; };
// TS3W.exe 0x00573ED0: Clear a retained pointer and release it atomically, restoring one before destruction.
uintptr_t Release(ResourcePrefix**);
// TS3W.exe 0x005740B0: Replace the coherent resource and mark its state dirty.
void SetResource(Coherent*,void*);
// TS3W.exe 0x00574190: Obtain local SRT storage and clear the valid flag.
SrtValue* Local(Pose*);
// TS3W.exe 0x005741B0: Fill 301 scalar words with the supplied float bits.
uint32_t* Fill(Pose*,float);
// TS3W.exe 0x005741E0: Clear the scalar storage pointer (verified twin adapted from InitBaseVtable).
void ClearFill(Pose*);
// TS3W.exe 0x005741F0: Select inline per-bone weights.
float* Weights(Pose*);
// TS3W.exe 0x00574200: Clear the weights pointer (verified twin's store offset adapted).
void ClearWeights(Pose*);
// TS3W.exe 0x00574210: Select inline auxiliary storage.
void* Auxiliary(Pose*);
// TS3W.exe 0x00574220: Clear the auxiliary pointer (verified twin's store offset adapted).
void ClearAuxiliary(Pose*);
// TS3W.exe 0x00574360: Initialize the shared pose pool once, returning true in AL.
bool InitializePool();
// TS3W.exe 0x005743A0: Copy bind poses and reset the terminal vectors, setting the valid flag.
void* Reset(Pose*);
// TS3W.exe 0x005745E0: Select inline per-bone weights and clear count scalars.
float* ZeroWeights(Pose*);
// TS3W.exe 0x00574750: Construct zeroed BoneDeltas; allocation failure still faults.
BoneDeltas* Construct(BoneDeltas*,uint32_t);
// TS3W.exe 0x005747C0: Destroy BoneDeltas and optionally free it, returning the freed address.
BoneDeltas* Destroy(BoneDeltas*,uint32_t);
// TS3W.exe 0x00574800: Clone a BoneDeltas payload without copying its flag.
BoneDeltas* Clone(BoneDeltas*);
}

#if defined(_M_IX86)
namespace ts3::anim::b20 {
uintptr_t __fastcall HookRelease(ResourcePrefix** p, void*);
void __fastcall HookSetResource(Coherent* p, void*, void* a);
SrtValue* __fastcall HookLocal(Pose* p, void*);
uint32_t* __fastcall HookFill(Pose* p, void*, float a);
void __fastcall HookClearFill(Pose* p, void*);
float* __fastcall HookWeights(Pose* p, void*);
void __fastcall HookClearWeights(Pose* p, void*);
void* __fastcall HookAuxiliary(Pose* p, void*);
void __fastcall HookClearAuxiliary(Pose* p, void*);
void* __fastcall HookReset(Pose* p, void*);
float* __fastcall HookZeroWeights(Pose* p, void*);
BoneDeltas* __fastcall HookConstruct(BoneDeltas* p, void*, uint32_t a);
BoneDeltas* __fastcall HookDestroy(BoneDeltas* p, void*, uint32_t a);
BoneDeltas* __fastcall HookClone(BoneDeltas* p, void*);
}
#endif
