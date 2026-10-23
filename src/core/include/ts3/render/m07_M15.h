#pragma once
#include "ts3/layout.h"
#include <cstdint>
namespace ts3::render::m07 {
struct ShaderReaderM15 {
    uint32_t prefix[2]; void* stream; uint32_t opaque; uint32_t mode; uint8_t good;
};
struct ParamM15 { uint32_t words[7]; };
template<class T> struct VectorM15 { T* begin; T* end; T* capacity; uint32_t tag; };
struct ShaderPassM15 {
    uint32_t id, flags, values[9]; // +00..+28
    uint8_t opaque2c[8];
    void* states; uint32_t stateCount, dynamicCount; void* dynamic;
    uint32_t mask, sampler[16], texture[16], textureCount;
    void* dynamicTexture; uint32_t samplerValue[16]; uint8_t opaque110[64];
    uint32_t vertexId, vertexCount, vertexExtent, pixelId, pixelCount, pixelExtent;
    VectorM15<ParamM15> vertex, pixel;
    uint8_t ReadM15(void* reader, void* library);
};
TS3_X86_OFFSET(ShaderReaderM15, good, 0x14);
TS3_X86_OFFSET(ShaderPassM15, states, 0x34);
TS3_X86_OFFSET(ShaderPassM15, vertexId, 0x150);
TS3_X86_OFFSET(ShaderPassM15, pixel, 0x178);
// TS3W.exe 0x0079C820: deserialize an annotated shader set, retaining partial read failure behavior.
uint8_t __stdcall ReadShaderSetM15(ShaderReaderM15*, void** output, void* context);
// TS3W.exe 0x0079EED0: deserialize a shader pass; inactive sampler values and partial failure writes are preserved.
uint8_t ReadShaderPassM15(ShaderPassM15*, ShaderReaderM15*, void* library);
inline uint8_t ShaderPassM15::ReadM15(void* reader, void* library) {
    return ReadShaderPassM15(this, static_cast<ShaderReaderM15*>(reader), library);
}
}
