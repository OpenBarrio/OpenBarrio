#pragma once
#include "ts3/layout.h"
#include <cstdint>
namespace ts3::world {
struct RenderContext {
    uint8_t prefix[12];
    void* device;
    uint8_t middle[124];
    uint8_t begun, blocked;
    uint8_t tail[1770];
    int32_t savedState;
    // TS3W.exe 0x0060F270: clear supported buffers, temporarily disabling state 194.
    int32_t Clear(uint8_t flags, int32_t color, float depth, int32_t stencil);
    // TS3W.exe 0x00611620: begin rendering, with at most two recovery attempts.
    uint8_t Begin();
};
TS3_X86_OFFSET(RenderContext, device, 12);
TS3_X86_OFFSET(RenderContext, begun, 140);
TS3_X86_OFFSET(RenderContext, blocked, 141);
TS3_X86_OFFSET(RenderContext, savedState, 1912);
TS3_X86_SIZE(RenderContext, 1916);
struct alignas(16) ShaderVectorState {
    uint8_t prefix[496];
    uint8_t vector[16];
    // TS3W.exe 0x009615E0: copy one aligned vector and return the source pointer.
    void* SetVector(const void* source);
};
TS3_X86_OFFSET(ShaderVectorState, vector, 496);
TS3_X86_SIZE(ShaderVectorState, 512);
struct alignas(16) PickView {
    uint8_t prefix[12]; float sizeX,sizeY; uint32_t unused20; float extent;
    uint8_t middle[20]; float matrix[16]; uint8_t tail[1677]; uint8_t ready;
};
TS3_X86_OFFSET(PickView, matrix,48);
TS3_X86_OFFSET(PickView,ready,1789);
struct PickScene { uint8_t prefix[28]; void* manager; };
struct PickRenderable { void** vtable; uint8_t prefix[48]; uint32_t id; };
struct PickItem { PickRenderable* object; void* data; };
struct PickEntry { PickItem* item; uintptr_t unused; };
struct PickEntryList { PickEntry* begin; PickEntry* end; PickEntry* capacity; const char* name; };
struct PickWorld { uint8_t prefix[272]; PickEntryList entries; uint8_t extras[16]; };
TS3_X86_OFFSET(PickWorld,entries,272);
struct PickConstant { uint32_t version; uint16_t generation,type; uint32_t dirty; const float* value; };
TS3_X86_OFFSET(PickConstant,value,12);
TS3_X86_SIZE(PickConstant,16);
struct PickConstants { uint32_t unused; PickConstant* entries; uint32_t count,unused12; uint16_t unused16,generation; };
TS3_X86_OFFSET(PickConstants,generation,18);
struct PickDrawContext {
    void* view; uint32_t mask; void* shaderA; void* shaderB;
    uint8_t flags[7],padding;
    int32_t values[7];
};
TS3_X86_SIZE(PickDrawContext,52);
struct PickFilter {
    void* begin; void* end; void* capacity; const char* name;
    void* objects; uint32_t flags; uint8_t enabled; uint8_t padding[3]; int32_t limit; uint8_t options[2];
};
TS3_X86_OFFSET(PickFilter,limit,28);
struct alignas(16) PickShader { ShaderVectorState state; uint8_t tail[1328]; };
TS3_X86_SIZE(PickShader,1840);
struct alignas(16) PickRenderer {
    PickView* source; PickScene* scene; PickWorld* world;
    uint32_t colorConstant,projectionConstant;
    uint8_t pending,completed,deferred,padding23;
    uint32_t maskA,maskB;
    uint8_t middle[12];
    float worldScale,scaleX,scaleY;
    int32_t requestedX,requestedY,capturedX,capturedY,fallbackX,fallbackY;
    uint8_t preFilter[20]; void* filter;
    uint8_t preMatrix[24]; alignas(16) float savedMatrix[16];
    uint8_t postMatrix[64];
    uint8_t unknownFlag,alternate,includeHidden,filtered,special;
    // TS3W.exe 0x006DF8E0: cull/draw pick IDs, restore render state, and clear temporary constant pointers.
    int32_t Draw(PickShader* shader);
    // TS3W.exe 0x006DFEC0: capture or render a picking request; preserve low-byte early-return artifacts.
    uint8_t Update();
};
TS3_X86_OFFSET(PickRenderer,pending,20);
TS3_X86_OFFSET(PickRenderer,filter,100);
TS3_X86_OFFSET(PickRenderer,savedMatrix,128);
TS3_X86_OFFSET(PickRenderer,alternate,257);
TS3_X86_OFFSET(PickRenderer,special,260);
}
