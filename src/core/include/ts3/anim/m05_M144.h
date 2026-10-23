#pragma once
#include "ts3/layout.h"
#include <cstdint>
namespace ts3::anim {
struct M144Child { void* vtable; uint32_t refs; };
struct M144Controller {
    void* mainTable; void* secondaryTable;
    unsigned char opaque08[12];
    void* host;
    void* interfaceTable;
    unsigned char opaque1c[20];
    alignas(16) float rotation[4];
    unsigned char opaque40[0x64];
    void* rigHost;
    uint32_t opaqueA8;
    float speed; float opaqueB0; float wheelbase; float radius;
    unsigned char opaqueBC[8];
    uint32_t initialized;
    void* handle;
    uint32_t opaqueCC;
    M144Child* child;
    int32_t bones[9];
    // TS3W.exe 0x008558F0: Resolve bicycle bones, measure axle spacing and install the retained rotation controller.
    uintptr_t InitializeBicycle();
    // TS3W.exe 0x00855CF0: Update pedal speeds and projected steering rotations; null child leaves EAX unspecified.
    void UpdateBicycle();
    // TS3W.exe 0x00856340: Restore bicycle tables, release its child and optionally free; returns this even after free.
    M144Controller* Destroy(uint32_t flags);
    // TS3W.exe 0x008565D0: Construct the vehicle base and install motorcycle interfaces without clearing opaque bytes.
    M144Controller* Construct(uint32_t a, uint32_t b);
    // TS3W.exe 0x008563A0: Push twice-normalized relative rotation and speed to the child; null child is a no-op (EAX unspecified).
    void UpdateRotation();
};
TS3_X86_OFFSET(M144Controller, host, 0x14);
TS3_X86_OFFSET(M144Controller, rotation, 0x30);
TS3_X86_OFFSET(M144Controller, child, 0xd0);
TS3_X86_OFFSET(M144Controller, speed, 0xac);
// TS3W.exe 0x00856680: Adjust the secondary interface by one pointer and tail-call the motorcycle deleting destructor.
void* M144DestroySecondary(void* secondary, uint32_t flags);
}
