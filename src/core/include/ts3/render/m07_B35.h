#pragma once
#include "ts3/layout.h"
#include "ts3/render/m07_layouts.h"
namespace ts3::render::m07::b35 {
using Resolver=void*(*)(uint32_t);
extern Resolver resolve;
struct Vector { void* begin; void* end; void* capacity; const char* tag; };
struct EffectBin {
    uint8_t ready; uint8_t padding[3];
    uint32_t effects,source,distortion,value,red,green,blue,scale,controls;
    uint32_t records[20][4]; Vector filters,ramps;
    uint32_t random; uint32_t styles;
    uint32_t state[12]; uint32_t styleTexture;
};
TS3_X86_OFFSET(EffectBin,filters,0x168);
TS3_X86_OFFSET(EffectBin,random,0x188);
TS3_X86_OFFSET(EffectBin,styleTexture,0x1c0);
struct Link { Link* next; Link* previous; };
struct Dependency { void* owner; Link ownerLink; Link link; uint32_t unused,key,count; };
struct Dependencies { uint8_t prefix[16]; Link head; };
TS3_X86_OFFSET(Dependency,link,12);
TS3_X86_OFFSET(Dependency,key,24);
// TS3W.exe 0x00706AA0: create the normalized screen capture temporary texture, retaining AL-only tests.
void* __cdecl CaptureTexture(int32_t width,int32_t height);
// TS3W.exe 0x00707410: initialize sparse effect-bin state without overwriting untouched identifiers.
EffectBin* __fastcall ConstructEffects(EffectBin*,void*);
// TS3W.exe 0x00709200: lazily intern effect shader identifiers and populate the bins.
void __fastcall InitializeEffects(EffectBin*,void*);
// TS3W.exe 0x006DED60: find or append a dependency entry, returning its embedded list link.
Link* __fastcall AddDependency(Dependencies*,void*,uint32_t key);
// TS3W.exe 0x006E1B80: create and bind a simple render-to-texture pair; sixth argument is unused.
uint8_t __stdcall SimpleRTT(int32_t,int32_t,uint32_t,uint32_t,uint32_t,uint32_t);
// TS3W.exe 0x006E1C70: create and bind a render target, defaulting format zero to 141.
uint8_t __stdcall RenderTarget(int32_t,int32_t,uint32_t,uint32_t);
// TS3W.exe 0x006E1D00: create a cube render texture and return Release, including on create failure.
uintptr_t __stdcall CubeRTT(void*,int32_t,uint32_t);
// TS3W.exe 0x0070A810: send a sculpture-capture message and return the final Release result.
uintptr_t __cdecl SendSculpture(uint32_t,uint32_t);
struct MaterialList { uint8_t prefix[0x40]; void** materials; };
struct DiffuseSelection { uint8_t prefix[0x10]; uint32_t value[4]; };
struct BindingData { uint8_t prefix[12]; uintptr_t offset; };
struct Binding { uint8_t prefix[10]; uint16_t type; uint32_t unknown; BindingData* data; uint8_t gap[10]; uint8_t kind; };
TS3_X86_OFFSET(Binding,kind,30);
// TS3W.exe 0x0070AE30: copy the diffuse-map key from the first Phong or SolidPhong material.
uintptr_t __fastcall SelectDiffuse(DiffuseSelection*,void*,MaterialList*);
extern uint8_t* fading;
// TS3W.exe 0x006DE7E0: change object fading and notify the message bus; allocation failure still dereferences null.
uint8_t __cdecl SetFading(uint8_t enabled);
struct SculptureIds { uint32_t flags,diffuse,normal,shininess,specular,specStyle; };
extern SculptureIds* sculptureIds;
struct SculptureStyle { const wchar_t* diffuse; const wchar_t* normal; float specular[3]; float shininess,specStyle; };
struct SculptureParameters { uint8_t prefix[48]; float* next; };
struct SculptureBinding { uint32_t type,id,unknown[2],key[4]; float* values; uint32_t count; };
TS3_X86_SIZE(SculptureBinding,40);
// TS3W.exe 0x0070AF40: append texture and scalar sculpture bindings, leaving record holes untouched.
float* __fastcall BindSculpture(SculptureParameters*,void*,SculptureBinding*,int32_t*,const SculptureStyle*,uint32_t);
struct Filter { uint32_t unknown[4],minimum,maximum,mask,tail; };
struct Effect { uint32_t unknown[8],minimum,maximum,mask,tail; };
// TS3W.exe 0x00709330: filter effects by mask and inclusive range, retaining the random-index EAX return.
uint32_t __cdecl ChooseEffects(uint32_t,uint32_t,uint32_t*,uint32_t*);
struct CaptureDoneMessage { void** table; uint32_t refs; uint32_t key[4]; };
struct CaptureKey { uint64_t instance; uint32_t type,group; };
// TS3W.exe 0x0070A890: persist a sculpture image under a collision-free timer key and notify listeners.
uintptr_t __stdcall FinishSculpture(void*);
struct SculptureBuilder { uint8_t* builder; uint32_t state; };
// TS3W.exe 0x0070A5B0: configure and populate a sculpture model builder from a drawable's mesh pool.
uintptr_t __fastcall BuildSculpture(SculptureBuilder*,void*,void*);
struct alignas(16) PickState {
    RenderPickManager base;
    uint8_t flag20,dirty; uint8_t pad22[26];
    float inverseWidth,inverseHeight; uint8_t pad38[8];
    int32_t x,y,savedX,savedY;uint8_t pad50[8];
    int32_t objectIndex,partIndex;uint32_t pixel;uint8_t pad64[12];
    alignas(16) float position[4];float transform[4][4],savedTransform[4][4];
    uint8_t flags100,partPick;
};
TS3_X86_OFFSET(PickState,x,0x40);
TS3_X86_OFFSET(PickState,position,0x70);
TS3_X86_OFFSET(PickState,partPick,0x101);
struct PickLock { uint8_t* data; uint32_t pitch,words[8]; };
struct PickNode { uint32_t key,distance,pixel; PickNode* next; };
struct PickMap { uint32_t unknown;PickNode** buckets;uint32_t bucketCount,size;float load,growth;uint32_t policy;const char* tag; };
struct PickIterator { PickNode* node;PickNode** bucket; };
struct PickEntry { uint32_t key,distance,pixel; };
struct PickInsertResult { PickIterator iterator;uint8_t inserted; };
// TS3W.exe 0x006E01B0: consume a pending pick buffer, select the nearest eligible object and reconstruct its position.
void __fastcall ResolvePick(PickState*,void*);
}
