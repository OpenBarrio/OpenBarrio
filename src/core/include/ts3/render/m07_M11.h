#pragma once
#include "ts3/render/m07_abi.h"
#include <cstdint>
namespace ts3::render::m07::m11 {
// Optional address resolver for native fixtures; the x64 original bridge fails explicitly.
extern void* (*resolve)(uint32_t);
struct String { wchar_t* begin; wchar_t* end; wchar_t* capacity; const char* tag; };
struct Sculpture { uint32_t prefix[2]; uint32_t flags; uint32_t opaque[5]; uint32_t multiplier[4]; };
struct Binding { uint32_t type,key,opaque[2],resource[4],tail[2]; };
struct SurfaceParams { const wchar_t* diffuse; const wchar_t* normal; uint32_t rgb[3],power,shine; };
struct FixupContext { uint32_t header; uint8_t map[0x1024]; void* sentinel; };
struct Shadow { uint32_t header,size; uint8_t depthColor,depthTexture,pad[2]; uint32_t colorSlot,depthSlot; uint32_t requested; };
// TS3W.exe 0x0070B1E0: appends source, wrapped UTF-16 index+49 and _d/_n to two strings, returning the final append result.
uintptr_t MakeNames(const wchar_t* source,String* diffuse,String* normal,uint32_t index);
// TS3W.exe 0x0070B1E0: original ESI source / EDI normal, stack diffuse and index, caller cleanup ABI adapter.
uintptr_t __cdecl MakeNamesEntry();
// TS3W.exe 0x0070B2F0: builds sculpture bindings and lazily interns shader identifiers, preserving untouched output holes.
int __fastcall SculptureBindings(Sculpture*,void*,Binding*,uint32_t* shader);
// TS3W.exe 0x006FF190: applies/removes model LOD fixups; remove reads only the low argument byte, returns AL only.
uint8_t __fastcall ModelFixup(FixupContext*,void*,uint32_t keyLo,uint32_t keyHi,uint32_t kind,uint32_t remove,uint32_t* data,uint32_t count);
// TS3W.exe 0x0070BEB0: creates shadow targets with halving retries; preserves stale BL when the initial size is below the cached minimum.
uint8_t __fastcall ShadowTargets(Shadow*,void*);
}
