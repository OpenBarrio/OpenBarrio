#pragma once
#include "ts3/render/m07_abi.h"
namespace ts3::render::m07::m05 {
using Resolver=void*(*)(uint32_t);
extern Resolver resolve;
template<class T> struct Vector { T* begin; T* end; T* capacity; const char* tag; };
struct Element { uint32_t type, semantic, index, offset; };
struct Format { Vector<Element> elements; int32_t slots[13]; uint32_t stride; };
struct Ref { uint32_t bank, part; };
struct Part {
    uint32_t resource, resourceIndex, unknown08, unknown0c;
    uint32_t vertexBegin,vertexEnd,indexBegin,indexEnd;
    alignas(16) float bounds[8];
    uint32_t unknown40,minIndex,maxIndex,unknown4c;
};
struct Bank { Part* begin; Part* end; Part* capacity; uintptr_t opaque[3]; };
struct Command { uint32_t resource,begin,end; };
struct Builder {
    uint8_t prefix[19]; uint8_t expand;
    Vector<uint32_t> vertices,indices;
    uint32_t vertexCount;
    Vector<Ref> refs;
    Vector<uint32_t> input[13];
    Vector<uint32_t> interleaved;
    uint8_t opaque128[0x5c];
    Vector<void*> sources[13];
    Vector<Bank> banks;
    uint32_t unknown264;
    Vector<Command> commands;
    uint8_t opaque278[0x10];
    Vector<Format> formats;
    uint8_t opaque298[8];
    alignas(16) float bounds[8];
};
TS3_X86_SIZE(Format,0x48);
TS3_X86_SIZE(Part,0x50);
TS3_X86_OFFSET(Builder,input,0x48);
TS3_X86_OFFSET(Builder,sources,0x184);
TS3_X86_OFFSET(Builder,banks,0x254);
TS3_X86_OFFSET(Builder,commands,0x268);
TS3_X86_OFFSET(Builder,formats,0x288);
TS3_X86_OFFSET(Builder,bounds,0x2a0);
// TS3W.exe 0x006D6770: flush pending mesh indices, expand unique attribute tuples, and accumulate part bounds. Keeps raw early returns and signed-short homogeneous positions.
uintptr_t __fastcall Flush(Builder*,void*);
}
