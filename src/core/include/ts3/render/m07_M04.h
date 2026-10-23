#pragma once
#include "ts3/render/m07_abi.h"
#include <cstddef>
namespace ts3::render::m07::m04 {
using Resolver=void*(*)(uint32_t);
extern Resolver resolve;
struct Part { uint32_t words[19]; uint8_t indexed; uint8_t pad[3]; };
struct Group { Part* begin; Part* end; Part* capacity; const char* tag; uint32_t nearBits,farBits; };
struct MaterialInput { uint32_t key; void* begin; uint32_t count; };
struct Builder {
    uint8_t prefix[0x12]; uint8_t disabled; uint8_t pad13;
    uint32_t* vertices; uint32_t* verticesEnd; uint32_t* verticesCap; const char* vertexTag;
    uint32_t* indices; uint32_t* indicesEnd; uint32_t* indicesCap; const char* indexTag;
    uint8_t pad34[0x220]; Group* groups; Group* groupsEnd; Group* groupsCap; const char* groupTag;
    uint8_t pad264[0x14]; MaterialInput* materials; uint8_t pad27c[0xc];
    uint8_t* formats; uint8_t pad28c[0x34]; uint8_t mode,option;
};
struct DependencyLod { void** table; uint8_t pad[0x3c]; void** parts; };
struct Elements { uint8_t* begin; uint8_t* end; uint8_t* capacity; const char* tag; };
#if defined(_M_IX86)
static_assert(offsetof(Builder,groups)==0x254 && offsetof(Builder,formats)==0x288 && offsetof(Builder,mode)==0x2c0);
static_assert(sizeof(Group)==24 && sizeof(Part)==80 && offsetof(DependencyLod,parts)==0x40);
#endif
// TS3W.exe 0x006D4C80: Build buffers and every model LOD; retain partial output on format failure and the 0xFFDC cutoff.
uint8_t __fastcall Build(Builder* self,void*,void** output);
}
