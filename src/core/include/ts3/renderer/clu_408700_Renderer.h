#pragma once
#include "ts3/layout.h"
#include <cstdint>

namespace ts3::renderer {
struct RenderTimer {
    uint32_t low, high;
    uint32_t reserved[2];
    uint32_t clock;
};
TS3_X86_OFFSET(RenderTimer, clock, 16);
TS3_X86_SIZE(RenderTimer, 20);
struct ResourceProxy {
    uint32_t key[4];
    uintptr_t resource;
};
TS3_X86_OFFSET(ResourceProxy, resource, 16);
TS3_X86_SIZE(ResourceProxy, 20);
// TS3W.exe 0x00408700: start only an idle timer; return EAX (high counter half for QPC).
uint32_t StartTimer408700(RenderTimer* self);
// TS3W.exe 0x00722F50: request a resource, optionally waiting for an untagged handle.
bool LoadProxy722F50(ResourceProxy* self, const void* key, bool wait, int priority, int flags);
// TS3W.exe 0x00722FC0: cancel the request and release only nonzero untagged resources.
uintptr_t ReleaseProxy722FC0(ResourceProxy* self);
// TS3W.exe 0x00723020: initialize the proxy and request its resource, ignoring failure.
ResourceProxy* ConstructProxy723020(ResourceProxy* self, const void* key, bool wait);
// TS3W.exe 0x00C88BF0: replace a texture proxy and update the shader parameter table.
bool BindTextureC88BF0(ResourceProxy* self, const char* name, const wchar_t* path, uint32_t* parameter);
// TS3W.exe 0x00C88FB0: texture binding without returning the parameter index.
bool BindTextureC88FB0(ResourceProxy* self, const char* name, const wchar_t* path);
// Field suffixes retain x86 offsets; opaque regions are untouched by this unit.
struct PickManager {
 void* p0; void* p4; void* p8; uint32_t u12,u16;
 uint8_t b20,b21,b22,b23;
 uint32_t u24,u28,u32,u36; float f40,f44,f48,f52;
 uint32_t ids56[10]; uint32_t u96,u100;
 std::byte untouched104[152]; uint8_t b256,b257,b258,b259,b260;
 std::byte untouched261[11];
 // TS3W.exe 0x006DEEE0: allocate a display list and register pick parameters.
 bool Initialize(void*,void*);
 // TS3W.exe 0x006DEF60: register the readback delay with the tuning service.
 int RegisterOptions();
 // TS3W.exe 0x006DF0E0: initialize defaults, preserving unspecified allocation bytes.
 PickManager* Construct();
};
TS3_X86_OFFSET(PickManager,b20,20); TS3_X86_OFFSET(PickManager,u24,24);
TS3_X86_OFFSET(PickManager,ids56,56); TS3_X86_OFFSET(PickManager,b256,256);
TS3_X86_SIZE(PickManager,272);
struct SceneManager {
 std::byte unknown0[8]; void* p8; std::byte unknown12[24]; void* p36;
 std::byte unknown40[16]; PickManager* p56; std::byte unknown60[380];
 uint32_t u440,u444; void* p448; void* p452; void* p456; uint32_t u460;
 uint32_t shader464[26]; RenderTimer timer568; std::byte unknown588[152];
 void* p740; uint32_t u744; void* p748; void* p752; uint32_t u756;
 ResourceProxy* p760; uint32_t u764; void* p768; void* p772; void* p776;
 void* p780; void* p784; void* p788; void* p792; void* p796; void* p800;
 void* p804; std::byte unknown808[152]; uint32_t u960;
 std::byte unknown964[376]; uint32_t u1340,u1344,u1348;
 uint32_t u1352,u1356,u1360;
 // TS3W.exe 0x006ED2B0: build the per-frame material and sixty identity joint matrices.
 bool InitializePerFrameMaterial();
 // TS3W.exe 0x006F40B0: initialize scene materials, managers, callbacks and texture bindings.
 bool Initialize();
};
TS3_X86_OFFSET(SceneManager,p56,56); TS3_X86_OFFSET(SceneManager,u440,440);
TS3_X86_OFFSET(SceneManager,p448,448); TS3_X86_OFFSET(SceneManager,shader464,464);
TS3_X86_OFFSET(SceneManager,timer568,568); TS3_X86_OFFSET(SceneManager,p740,740);
TS3_X86_OFFSET(SceneManager,p804,804); TS3_X86_OFFSET(SceneManager,u960,960);
TS3_X86_OFFSET(SceneManager,u1340,1340); TS3_X86_OFFSET(SceneManager,u1360,1360);
TS3_X86_SIZE(SceneManager,1364);
}
