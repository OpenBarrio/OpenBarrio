#pragma once
#include "ts3/layout.h"
#include <cstdint>
namespace ts3::anim::M140 {
struct Controller;
struct Link { Link* next; Link* previous; Controller* controller; int32_t kind; };
struct Controller { void** vtable; int32_t refs; unsigned char unknown08[4]; unsigned char delegates[16]; void* host; uint32_t id; uint8_t finished; };
struct Body {
    void** vtable;
    unsigned char unknown04[16];
    void* actor; // +14
    void* secondary;
    Link controllers; // Only the first two pointer fields belong to the sentinel.
    unsigned char unknown2C[0x7c-0x2c];
    int32_t routeKind;
    unsigned char unknown80[4];
    uint32_t routeMask;
    unsigned char unknown88[0xc4-0x88];
    void* motionActor;
    int32_t slaveCount;
    uint8_t useSlaves;
    unsigned char unknownCD[0xf0-0xcd];
    unsigned char progression[0x1c];
    void* pending;
    uint8_t unknown110;
    uint8_t hasPending;
    unsigned char unknown112[0x16c-0x112];
    int32_t target, scheduled, completed;
    unsigned char unknown178[4];
    uint8_t active, transitioning, running, stopping, unknown180, disabled;
    unsigned char unknown182[2];
    uint8_t allowDelta;
    unsigned char unknown185[0x248-0x185];
    int32_t balance;
    uint32_t remaining;
    uint8_t flag250, flag251, flag252, flag253;

    // TS3W.exe 0x00852700: advances root-motion/controller state; preserves the active latch and low-byte predicates.
    uint8_t Advance(int32_t mode);
    // TS3W.exe 0x00853500: removes expired controllers and performs the post-update transition pass (void ABI).
    void Finish(int32_t mode);
    // TS3W.exe 0x00853820: holds the actor controller across both update passes (void ABI; stack argument unused).
    void Update(uint32_t unused);
};
TS3_X86_OFFSET(Body, actor, 0x14);
TS3_X86_OFFSET(Body, controllers, 0x1c);
TS3_X86_OFFSET(Body, routeKind, 0x7c);
TS3_X86_OFFSET(Body, motionActor, 0xc4);
TS3_X86_OFFSET(Body, progression, 0xf0);
TS3_X86_OFFSET(Body, pending, 0x10c);
TS3_X86_OFFSET(Body, target, 0x16c);
TS3_X86_OFFSET(Body, active, 0x17c);
TS3_X86_OFFSET(Body, balance, 0x248);
TS3_X86_OFFSET(Body, flag253, 0x253);
TS3_X86_OFFSET(Controller, host, 0x1c);
TS3_X86_OFFSET(Controller, finished, 0x24);
}
