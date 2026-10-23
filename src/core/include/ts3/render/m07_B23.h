#pragma once
#include "ts3/layout.h"
namespace ts3::render::b23 {
struct Drawables { unsigned char prefix[0x40]; void** items; uint32_t count; };
struct Substitution {
    void* first[4]; uint8_t flags[2], pad12[6]; void* buckets;
    uint32_t bucketCount,count; float minLoad,maxLoad; void* storage; const char* tag;
};
struct Link { Link* next; Link* prev; };
struct SwapParams { void* first; void* second; void* callback; uint32_t untouched; };
struct Proxy {
    void* first[5]; uint32_t invalid[2],one; uint16_t selector; uint8_t enabled,mask;
    uint8_t pad24[4]; void* middle[6]; void* model; SwapParams* delayed;
    Link callbacks; const char* callbackTag; uint32_t callbackCount; void* context;
    uint8_t flags[3],pad5f; void* selection[3]; const char* selectionTag; void* tail;
};
TS3_X86_OFFSET(Drawables,items,0x40); TS3_X86_OFFSET(Substitution,buckets,0x18);
TS3_X86_SIZE(Substitution,0x34); TS3_X86_OFFSET(Proxy,delayed,0x44);
TS3_X86_OFFSET(Proxy,context,0x58); TS3_X86_SIZE(Proxy,0x74);
struct Drawable { uint8_t opaque[0xc1]; uint8_t flags1,flags2; };
struct AdapterOwner { uint8_t opaque[0x158]; Drawables* target; };
struct Surface { uint8_t opaque[0x54]; void* shader; };
struct ShaderOverride {
    uint32_t kind,key,unused[2],zeros[4]; const float* value; uint32_t count;
};
struct LOD : Drawables { uint8_t opaque48[0x2c]; int16_t materialSelector; };
struct MaterialTree {
    void* unused; Link head; void* root; uint8_t flag,pad[3]; uint32_t count; const char* tag;
};
struct CachedMaterialNode { void* opaque[5]; void* material; };
struct ShaderIdentity { uint8_t opaque[8]; uint32_t id; };
struct RemapNode { void* first; void* second; };
struct Remap { uint32_t unused; RemapNode** buckets; uint32_t bucketCount; };
struct SubstitutionArgs {
    ShaderOverride* overrides; uint32_t count; void* fallback; uint32_t skipId;
    uint8_t modeA,modeB,pad[2]; Remap remap;
};
struct CoverageObject { uint8_t opaque[0x10]; uint32_t key[2]; };
struct CoverageNode { CoverageObject* object; uint32_t extra; CoverageNode* next; };
struct CoverageSet {
    uint32_t unused; CoverageNode** buckets; uint32_t bucketCount,count;
    float minLoad,maxLoad; void* storage; const char* tag;
};
struct CoverageVectorView { uint64_t* begin; uint64_t* end; uint64_t* capacity; };
struct CoverageHost { uint8_t opaque[8]; uint8_t mutex[0x30]; CoverageVectorView output; };
TS3_X86_OFFSET(LOD,materialSelector,0x74); TS3_X86_SIZE(MaterialTree,0x1c);
TS3_X86_SIZE(CoverageSet,0x20); TS3_X86_OFFSET(CoverageHost,output,0x38);
// TS3W.exe 0x006DB480: Substitute LOD materials, caching reused material identities in a temporary tree.
uintptr_t __fastcall SubstituteMaterials(LOD*,void*,Drawables*,SubstitutionArgs*,void*);
// TS3W.exe 0x0059E7C0: Collect eligible object coverage keys and packed coverage bytes under the host mutex.
uintptr_t __fastcall CollectCoverage(CoverageHost*,void*,void*,void*);
// TS3W.exe 0x0059EAD0: Lazily initialize the object coverage vector using the unsynchronized low-bit guard.
void* __cdecl CoverageVector();
// TS3W.exe 0x005CAB50: Clone a hair drawable, adding HairAlphaBlend and transferring its alpha flag.
uint8_t __fastcall CloneHair(AdapterOwner*,void*,void*,Drawable*,int32_t);
// TS3W.exe 0x005CAC80: Clone an ordinary drawable with primitive mode five.
uint8_t __fastcall CloneOrdinary(AdapterOwner*,void*,void*,Drawable*,int32_t);
// TS3W.exe 0x005CAD80: Clone a fin drawable, doubling vertices and multiplying primitive count by nine.
uint8_t __fastcall CloneFin(AdapterOwner*,void*,void*,Drawable*,int32_t);
// TS3W.exe 0x005CAEF0: Clone a second fin drawable, doubling both counts with unsigned wrapping.
uint8_t __fastcall CloneFin2(AdapterOwner*,void*,void*,Drawable*,int32_t);
// TS3W.exe 0x006DAC10: Query the four supported resource interfaces without adjusting this.
void* __fastcall Query(void*,void*,uint32_t);
// TS3W.exe 0x006DACA0: Replace the drawable array, retaining partial initialization on failure.
uint8_t __fastcall Resize(Drawables*,void*,uint32_t);
// TS3W.exe 0x006DADB0: Append a borrowed drawable using a newly allocated array.
uint8_t __fastcall Append(Drawables*,void*,void*);
// TS3W.exe 0x006DB3B0: Initialize material substitution data without writing padding.
Substitution* __fastcall InitSubstitution(Substitution*,void*);
// TS3W.exe 0x006DB6E0: Allocate a LOD clone, copy it and optionally substitute materials.
uint8_t __fastcall Clone(void*,void*,void**,void*,void*);
// TS3W.exe 0x006DD160: Replace delayed swap data and retain the model and callback in EA order.
uintptr_t __fastcall DelaySwap(Proxy*,void*,void*,void*,void*);
// TS3W.exe 0x006DD580: Initialize proxy callback and material-selector state, preserving holes.
Proxy* __fastcall InitProxy(Proxy*,void*);
}
