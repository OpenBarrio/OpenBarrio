#pragma once
#include "ts3/layout.h"
#include <cstdint>
namespace ts3::render::m07::m13 {
struct Service {
    void** table;
    uint8_t opaque04[12];
    uint32_t listener, callback;
    uint8_t initialized;
    uint8_t pad19[3];
    void* swarm;
    void* opaque20;
    void* resources;
    void* renderer;
    uint32_t opaque2c;
    uint8_t lock[24];
    void* decals;
};
TS3_X86_OFFSET(Service,swarm,0x1c);
TS3_X86_OFFSET(Service,resources,0x24);
TS3_X86_OFFSET(Service,decals,0x48);
// TS3W.exe 0x00720C70: initialize Swarm renderer services; keep AL-only status tests and partial-init cleanup ordering.
uint8_t __fastcall Initialize(Service* self, void*);
}
