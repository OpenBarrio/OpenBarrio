#pragma once
#include "ts3/layout.h"
#include <xmmintrin.h>
namespace ts3::anim::m132 {
// Observed fields of the locomotion brain, not AnimatedSimBody's layout.
struct Body {
    unsigned char opaque00[0x14]; void* actor;
    unsigned char opaque18[0x2c]; void* builder;
    unsigned char opaque48[0x34]; uint32_t gait; uint32_t opaque80; uint32_t style;
    unsigned char opaque88[0x3c]; void* owner;
    unsigned char opaqueC8[4]; uint8_t blocked;
    unsigned char opaqueCD[0x44]; uint8_t disabled;
    unsigned char opaque112[0x12]; void* host;
    unsigned char opaque128[0x57]; uint8_t turning; uint8_t mirrored;
    unsigned char opaque181[4]; uint8_t routeFailed;
    unsigned char opaque186[0xce]; uint8_t requested;
};
TS3_X86_OFFSET(Body,actor,0x14); TS3_X86_OFFSET(Body,builder,0x44);
TS3_X86_OFFSET(Body,style,0x84); TS3_X86_OFFSET(Body,owner,0xc4);
TS3_X86_OFFSET(Body,blocked,0xcc); TS3_X86_OFFSET(Body,disabled,0x111);
TS3_X86_OFFSET(Body,host,0x124); TS3_X86_OFFSET(Body,turning,0x17f);
TS3_X86_OFFSET(Body,mirrored,0x180); TS3_X86_OFFSET(Body,routeFailed,0x185);
TS3_X86_OFFSET(Body,requested,0x254);
// TS3W.exe 0x0084BC70: request a turn toward a direction; keeps EA's asymmetric cleanup and angle thresholds.
char __fastcall TurnToward(Body* self, void* edx, const __m128* direction, int mode, char requested);
// TS3W.exe 0x0084C590: reconcile the actor's route orientation and start a turn controller.
char __fastcall ReconcileTurn(Body* self, void* edx);
}
