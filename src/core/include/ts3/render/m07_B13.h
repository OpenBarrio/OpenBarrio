#pragma once
#include "ts3/render/m07_abi.h"
#include "ts3/render/m07_layouts.h"
namespace ts3::render::m07::b13 {
// Optional native/test address binding; absent bindings use the explicit x86 bridge.
extern void* (*resolve)(uint32_t);
struct Vector { void** table; volatile long refs; void** begin; void** end; void** capacity; const char* tag; float period; };
struct Service { uint8_t opaque0000[0x10]; void* world; };
struct Node { Node* next; Node* prev; void* key; void* value; float rate,time; void* a; void* b; float c; uint8_t flag; };
struct Manager { uint8_t opaque0000[0x10]; Node head; };
struct ImageData { void* pixels; uint32_t width,height,pitch,format; };
// Refines the scaffold's GPU texture word30: 006196B0 dereferences it and
// calls COM Release, proving this slot holds a pointer, not a native uint32_t.
struct TextureView {
    void* primary; uint32_t refs; void* secondary; uint8_t opaque000C[8];
    void* handle14; uint32_t word18,levels,flags,word24,format,word2C; void* handle30;
};
struct ImageTexture {
    void* primary; uint32_t refs; void* secondary; uint8_t opaque000C[8];
    void* handle14; uint32_t word18,levels,flags,word24,format;
};
struct Animated { void* primary; uint8_t opaque0004[0x24]; Vector textures; volatile long loaded; }; 
TS3_X86_OFFSET(TextureView,secondary,0x8); TS3_X86_OFFSET(TextureView,handle30,0x30);
TS3_X86_OFFSET(Vector,period,0x18); TS3_X86_OFFSET(Animated,loaded,0x44);
TS3_X86_OFFSET(Manager,head,0x10); TS3_X86_OFFSET(ImageTexture,format,0x28);
// TS3W.exe 0x00687310: allocate, replace and initialize the world service, publishing only on success.
uint8_t __fastcall Initialize(Service*,void*);
// TS3W.exe 0x00687650: initialize the animated texture vector and reference counter.
Vector* __fastcall Construct(Vector*,void*);
// TS3W.exe 0x00687690: release texture entries in reverse order and destroy vector storage.
uintptr_t __fastcall Destruct(Vector*,void*);
// TS3W.exe 0x006876E0: destroy the vector, optionally free it, and return the receiver.
Vector* __fastcall Delete(Vector*,void*,uint32_t);
// TS3W.exe 0x00687700: allocate and construct a 72-byte animated texture resource.
void* __fastcall Create(void*,void*);
// TS3W.exe 0x00687BA0: construct and submit an animated texture LOD fixup, then release it.
uintptr_t __stdcall Fixup(void*,const void*);
// TS3W.exe 0x00687CE0: update or append material data while holding the manager mutex.
uintptr_t __fastcall Material(Manager*,void*,void*,uint32_t,uint32_t);
// TS3W.exe 0x00687E90: register the animated texture manager if the resource service exists.
uint8_t __cdecl RegisterManager();
// TS3W.exe 0x006880F0: load animated texture data, retaining EA's partial-load cleanup ordering.
uint8_t __fastcall Load(Animated*,void*,void*,uint32_t,uint32_t);
// TS3W.exe 0x00618930: convert image levels, replicating the final compressed block for small mips.
uint8_t __cdecl Convert(ImageTexture*,const ImageData*,uint8_t);
// TS3W.exe 0x00619B20: return true for a nonnull handle30, otherwise create the texture (AL-only result).
uint8_t __fastcall Ensure(void*,void*);
// TS3W.exe 0x00619B40: release the adjusted texture unless flags 0x102 suppress it; no defined result on the no-op path.
void __fastcall Unload(void*,void*);
}
