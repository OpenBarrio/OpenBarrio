#pragma once
#include "ts3/layout.h"
namespace ts3::audio::b52 {
struct Node { uint32_t key; union { float value; void* object; }; Node* next; };
struct Pool { void* first; const void* label; void* begin; void* end; uint32_t stride; void* allocator; };
struct Map { void* untouched; Node** buckets; uint32_t count,size; float load,growth; uint32_t threshold; Pool pool; };
struct Iterator { Node* node; Node** bucket; };
struct InsertResult { Node* node; Node** bucket; uint8_t inserted; };
struct Links { Links* next; Links* prev; void* allocator; };
struct ListNode { Links* next; Links* prev; void* value; };
struct Source { void* begin; uint8_t unused[16]; void* allocator; };
struct Sound {
    void* vtable;
    void* secondary;
    uint8_t gap8[48];
    uint32_t key[2];
    uint8_t gap40[8];
    uint32_t stamp[2];
    uint8_t gap50[8];
    void* target;
    uint8_t gap5c[8];
    float f64;
    float f68;
    float f6c;
    float f70;
    uint8_t gap74[4];
    float f78;
    float f7c;
    uint8_t gap80[4];
    float f84;
    float f88;
    float f8c;
    float f90;
    uint8_t gap94[24];
    uint8_t enabled;
    uint8_t gapad[3];
    uint8_t alternate;
    uint8_t gapb1[7];
    float position[3];
    uint8_t gapc4[12];
    uint8_t flags;
    uint8_t gapd1[3];
    float alternatePosition[3];
    uint8_t gape0[40];
    Map values;
    uint8_t gap13c[432];
    void* resource;
    uint8_t gap2f0[60];
    void* resolved;
    uint8_t gap330[4];
    float f334;
    uint8_t gap338[4];
    uint32_t u33c;
    uint32_t fallbackKey[2];
    Map curves;
    uint8_t gap37c[428];
    Map releaseCurves;
    uint8_t gap55c[1308];
    Links slaves;
    void* master;
    uint8_t gapa88[4];
    float preparedPosition[3];
    uint8_t gapa98[4];
    float preparedGain;
    uint32_t preparedState;
    uint8_t preparedFlag;
    uint8_t gapaa5[3];
    float preparedZero;
    float preparedPercent;
    uint8_t gapab0[24];
    float fac8;
    uint8_t gapacc[16];
    uint32_t propertyFlags;
    uint8_t curveA[0x360];
    uint8_t curveB[0x360];
    uint8_t curveC[0x360];
    uint8_t gap1500[904];
    char* stringBegin; char* stringEnd; char* stringCapacity;
    uint8_t gap1894[4];
    int32_t channels;
};
TS3_X86_OFFSET(Sound,values,0x108);
TS3_X86_OFFSET(Sound,releaseCurves,0x528);
TS3_X86_OFFSET(Sound,slaves,0xa78);
TS3_X86_OFFSET(Sound,channels,0x1898);
TS3_X86_SIZE(Map,0x34);
// TS3W.exe 0x008AD6F0: Publish cached sound properties and string to its player in original order.
void Publish(Sound*);
// TS3W.exe 0x008AD980: Initialize the 240-byte-arena float hash map; leave the first word untouched.
Map* Construct240(Map*,uint32_t,uintptr_t,uintptr_t,uintptr_t,uintptr_t,uintptr_t,const Source*);
// TS3W.exe 0x008ADA50: Initialize the 480-byte-arena float hash map; leave the first word untouched.
Map* Construct480(Map*,uint32_t,uintptr_t,uintptr_t,uintptr_t,uintptr_t,uintptr_t,const Source*);
// TS3W.exe 0x008ADB20: Insert a float-map entry, with allocation preceding optional rehash.
InsertResult* Insert(Map*,InsertResult*,const uint32_t*,const void*);
// TS3W.exe 0x008ADBE0: Set string/curve properties and forward non-null payloads to the player.
void SetData(Sound*,uint32_t,const void*,uint32_t);
// TS3W.exe 0x008ADCC0: Read built-in or mapped float properties; failed reads leave output untouched.
bool Get(Sound*,uint32_t,float*);
// TS3W.exe 0x008ADFA0: Test whether the secondary interface has a mapped float property.
bool Contains(void*,uint32_t);
// TS3W.exe 0x008ADFE0: Resolve the sample resource, initialize timing and publish playback properties.
bool Prepare(Sound*);
// TS3W.exe 0x008AE2E0: Copy channel gain and mapped properties to another sound.
void Copy(Sound*,Sound*);
// TS3W.exe 0x008AE3B0: Release curve values before resetting the curve hash table.
void Clear(Sound*);
// TS3W.exe 0x008AE430: Adapt unique curve-map insertion, returning the caller's output iterator.
void* FindOrInsert(Map*,void*,const void*);
// TS3W.exe 0x008AE460: Allocate and link a slave list node; allocation failure remains unchecked.
ListNode* Append(Links*,void* const*);
// TS3W.exe 0x008AE4B0: Update or create a response curve, or forward scalar properties.
void SetCurve(void*,uint32_t,float,uint32_t);
// TS3W.exe 0x008AEC10: Attach and retain a distinct unparented slave sound.
void Attach(Sound*,Sound*);
}

#if defined(_M_IX86)
namespace ts3::audio::b52 {
// TS3W.exe 0x008AD6F0: Typed ABI adapter for Publish.
inline void __fastcall HookPublish(Sound* s,void*) { return Publish(s); }
// TS3W.exe 0x008AD980: Typed ABI adapter for Construct240.
inline Map* __fastcall HookConstruct240(Map* s,void*,uint32_t n,uintptr_t a,uintptr_t b,uintptr_t c,uintptr_t d,uintptr_t e,const Source* f) { return Construct240(s,n,a,b,c,d,e,f); }
// TS3W.exe 0x008ADA50: Typed ABI adapter for Construct480.
inline Map* __fastcall HookConstruct480(Map* s,void*,uint32_t n,uintptr_t a,uintptr_t b,uintptr_t c,uintptr_t d,uintptr_t e,const Source* f) { return Construct480(s,n,a,b,c,d,e,f); }
// TS3W.exe 0x008ADB20: Typed ABI adapter for Insert.
inline InsertResult* __fastcall HookInsert(Map* s,void*,InsertResult* out,const uint32_t* key,const void* value) { return Insert(s,out,key,value); }
// TS3W.exe 0x008ADBE0: Typed ABI adapter for SetData.
inline void __fastcall HookSetData(Sound* s,void*,uint32_t key,const void* data,uint32_t bytes) { return SetData(s,key,data,bytes); }
// TS3W.exe 0x008ADCC0: Typed ABI adapter for Get.
inline bool __fastcall HookGet(Sound* s,void*,uint32_t key,float* out) { return Get(s,key,out); }
// TS3W.exe 0x008ADFA0: Typed ABI adapter for Contains.
inline bool __fastcall HookContains(void* s,void*,uint32_t key) { return Contains(s,key); }
// TS3W.exe 0x008ADFE0: Typed ABI adapter for Prepare.
inline bool __fastcall HookPrepare(Sound* s,void*) { return Prepare(s); }
// TS3W.exe 0x008AE2E0: Typed ABI adapter for Copy.
inline void __fastcall HookCopy(Sound* s,void*,Sound* dest) { return Copy(s,dest); }
// TS3W.exe 0x008AE3B0: Typed ABI adapter for Clear.
inline void __fastcall HookClear(Sound* s,void*) { return Clear(s); }
// TS3W.exe 0x008AE430: Typed ABI adapter for FindOrInsert.
inline void* __fastcall HookFindOrInsert(Map* s,void*,void* out,const void* key) { return FindOrInsert(s,out,key); }
// TS3W.exe 0x008AE460: Typed ABI adapter for Append.
inline ListNode* __fastcall HookAppend(Links* s,void*,void* const* value) { return Append(s,value); }
// TS3W.exe 0x008AE4B0: Typed ABI adapter for SetCurve.
inline void __fastcall HookSetCurve(void* s,void*,uint32_t key,float value,uint32_t flag) { return SetCurve(s,key,value,flag); }
// TS3W.exe 0x008AEC10: Typed ABI adapter for Attach.
inline void __fastcall HookAttach(Sound* s,void*,Sound* slave) { return Attach(s,slave); }
}
#endif
