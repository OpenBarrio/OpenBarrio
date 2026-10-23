#pragma once
#include "ts3/render/m07_abi.h"
#include <cstddef>
#include <cstdint>
namespace ts3::render::m07::m03 {
struct Builder {
    uint8_t prefix[0x254];
    uint8_t *vertices, *verticesEnd, *verticesCapacity;
    uint8_t gap260[0x18];
    uint8_t *channels, *channelsEnd, *channelsCapacity;
    uint8_t gap284[4];
    uint8_t *streams, *streamsEnd, *streamsCapacity;
};
struct Metadata { uint32_t low,high,unused,version; };
struct Context {
    void* root; void* owned; void* stream;
    uint32_t offset,mode;
    uint8_t success,flag;
};
struct Fixup { int32_t first,second; uint32_t firstSize,secondSize; };
using Payload = Thiscall<void,Builder,Context*,uint32_t*,Fixup*>;
using IndexedPayload = Thiscall<void,Builder,Context*,uint32_t*,Fixup*,uint32_t>;
struct Dependencies {
    Thiscall<void,Context,void*,uint32_t> construct=nullptr;
    Thiscall<void,Context> destroy=nullptr;
    Thiscall<uint32_t,Builder> count=nullptr;
    uint8_t (__cdecl* word)(void*,uint32_t,uint32_t)=nullptr;
    uint8_t (__cdecl* pair)(void*,uint32_t,uint32_t,uint32_t)=nullptr;
    Thiscall<void,Builder,Context*,uint32_t,uint32_t,const Metadata*> header=nullptr;
    void* (__cdecl* allocate)(uint32_t,const char*,uint32_t,uint32_t,const char*,uint32_t)=nullptr;
    void (__cdecl* free)(void*)=nullptr;
    Payload base=nullptr, positions=nullptr, normals=nullptr, extras=nullptr;
    Thiscall<void,Builder,Context*,uint32_t,uint32_t*,Fixup*> vertex=nullptr;
    IndexedPayload stream=nullptr,channel=nullptr;
};
extern Dependencies dependencies;
// TS3W.exe 0x006D2780: Serialize ModelBuilder streams and backpatch sizes; always returns AL=1, even on write/seek failure.
uint8_t __fastcall Build(Builder*,void*,void*,const Metadata*);
#if defined(_M_IX86)
static_assert(offsetof(Builder,vertices)==0x254 && offsetof(Builder,channels)==0x278 && offsetof(Builder,streams)==0x288);
static_assert(sizeof(Context)==24 && sizeof(Fixup)==16);
#endif
}
