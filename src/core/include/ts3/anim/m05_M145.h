#pragma once
#include "ts3/layout.h"
#include <cstdint>
namespace ts3::anim {
struct M145Child { void* vtable; uint32_t refs; };
struct M145 {
    void* vtable; void* secondary; unsigned char pad08[0x10]; void* interface18;
    unsigned char pad1c[0x88]; void* host; uint32_t a8; float speed; uint32_t b0;
    float length, halfLength, width, halfWidth; unsigned char padc4[12];
    M145Child* child; int32_t joints[8];
    // TS3W.exe 0x00856690: resolves wheel joints and creates the retained rotation controller.
    uintptr_t Initialize();
    // TS3W.exe 0x008569F0: tears down the vehicle controller, rearming last references before deletion.
    uintptr_t Destroy(uint32_t flags);
    // TS3W.exe 0x00856A50: sends speed times 0.3 to the optional rotation controller; null returns this.
    uintptr_t Update();
    // TS3W.exe 0x00856A80: stops rotation, notifies the host and runs base stop.
    uintptr_t Stop();
    // TS3W.exe 0x00856AC0: constructs the vehicle base and installs three interfaces.
    uintptr_t Construct(uint32_t a, uint32_t b);
};
TS3_X86_OFFSET(M145,host,0xa4);
TS3_X86_OFFSET(M145,child,0xd0);
TS3_X86_OFFSET(M145,joints,0xd4);
// TS3W.exe 0x00856B50: adjusts the secondary-interface this by one pointer and delegates deletion.
void* M145DestroySecondary(void* self,uint32_t flags);
}
