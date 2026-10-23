#pragma once
#include "ts3/render/m07_abi.h"
#include <cstdint>
namespace ts3::render::m07::m08 {
struct Binding { uint32_t revision; uint16_t generation,type; uint32_t count; const void* value; };
struct Bindings { void* vtable; Binding* entries; uint32_t count; uint8_t opaque0c[6]; uint16_t generation; };
struct Camera { uint8_t opaque[0x70]; alignas(16) float rows[4][4]; };
struct ScenePass {
    uint8_t opaque00[0x24]; Camera* camera;
    uint8_t opaque28[0x2AC-0x28]; float waterHeight;
    uint8_t opaque2b0[4]; float sea[3];
    uint8_t opaque2c0[0x300-0x2c0]; void* shaders;
    uint8_t opaque304[0x538-0x304]; uint32_t passes[4];
};
TS3_X86_OFFSET(ScenePass,camera,0x24);
TS3_X86_OFFSET(ScenePass,waterHeight,0x2AC);
TS3_X86_OFFSET(ScenePass,sea,0x2B4);
TS3_X86_OFFSET(ScenePass,shaders,0x300);
TS3_X86_OFFSET(ScenePass,passes,0x538);
TS3_X86_SIZE(Binding,16);
TS3_X86_OFFSET(Bindings,generation,18);
struct TextureSize { uint8_t opaque[20]; uint32_t width,height; };
struct PassGlobals {
    Bindings* bindings;
    uint32_t* flags; uint32_t* ids[5]; uint32_t* shadowId;
    float* shimmer; float* underSea; const float* bloom;
    uint8_t* texturesEnabled; void** textures[4]; uint32_t* textureIds[4]; void* signal;
};
struct PassCalls {
    uint32_t (*intern)(Bindings*,const char*,uint32_t,uint32_t);
    void* (*device)(); void* (*resources)(); TextureSize** (*lookup)(void*,uint32_t);
    void (*enable)(ScenePass*,uint32_t);
    void (*select)(void*,uint32_t); void (*upload)(Bindings*,uint32_t,const float*);
    uint8_t (*signalA)(void*); uint8_t (*signalB)(void*);
    void* (*texture)(void*); void (*bind)(Bindings*,uint32_t,void*);
    uint32_t (*find)(void*,const char*);
    void (*draw)(void*,uint32_t); void (*clear)(void*,uint32_t,uint32_t);
};
const PassCalls& OriginalPassCalls();
PassGlobals OriginalPassGlobals();
// TS3W.exe 0x006EDE30: binds and submits a postprocessing pass, retaining AL-only signal checks and cleanup-dependent EAX.
uintptr_t SubmitPass(ScenePass*,int32_t,const PassGlobals&,const PassCalls& = OriginalPassCalls());
struct TargetSystem {
    uint8_t opaque00[0xF0]; uint8_t enhanced;
    uint8_t opaqueF1[0x108-0xF1]; uint32_t pendingBytes,totalBytes;
    uint8_t opaque110[0x12C-0x110]; void* textures[6];
    uint8_t opaque144[0x178-0x144]; uint8_t feature178,feature179,flag17a,feature17b,feature17c,flag17d,flag17e,ready,feature180;
};
TS3_X86_OFFSET(TargetSystem,pendingBytes,0x108);
TS3_X86_OFFSET(TargetSystem,textures,0x12C);
TS3_X86_OFFSET(TargetSystem,feature178,0x178);
// TS3W.exe 0x006E2760: creates render targets and stock textures and registers pick resources; keeps unchecked stock allocation failure and last-release EAX.
uintptr_t InitializeTargets(TargetSystem*,uint8_t);
#if defined(_M_IX86)
uintptr_t __fastcall HookInitializeTargets(TargetSystem*,void*,uint8_t);
uintptr_t __fastcall HookSubmitPass(ScenePass*,void*,int32_t);
#endif
}
