#pragma once
#include <cstdint>
namespace ts3::audio {
struct B31Delegates { void* primaryTable; void* secondaryTable; uint32_t refs; void* first; void* second; };
// TS3W.exe 0x008A03D0: Append handle, opcode 0x24, type 7 and nine matrix words; publish after all appends.
void* B31WriteMatrix(void* self, uint32_t handle, const uint32_t* matrix);
// TS3W.exe 0x008A0440: Initialize the two interface views and empty delegates.
B31Delegates* B31Construct(B31Delegates* self);
// TS3W.exe 0x008A0470: Adjust secondary this by one vptr and invoke the original owning destructor.
void* B31DestroySecondary(void* secondary, uint32_t flags);
// TS3W.exe 0x008A0480: Retain new delegate, publish it, then release old; equal assignment is a no-op.
void B31SetSecond(B31Delegates* self, void* value);
// TS3W.exe 0x008A0510: Forward slot four to first nonnull delegate without testing the result.
uintptr_t B31FirstOnly(B31Delegates* self, uint32_t a, uint32_t b);
// TS3W.exe 0x008A0540: Broadcast slot five in primary then fallback order.
void B31Broadcast(B31Delegates* self, uint32_t a);
// TS3W.exe 0x008A04C0: Query slot 2 with ordered short-circuit fallback; only AL is truth-tested.
uint32_t B31Query04C0(B31Delegates* self, uint32_t a, uint32_t b);
// TS3W.exe 0x008A0570: Query slot 6 with ordered short-circuit fallback; only AL is truth-tested.
uint32_t B31Query0570(B31Delegates* self, uint32_t a);
// TS3W.exe 0x008A05B0: Query slot 8 with ordered short-circuit fallback; only AL is truth-tested.
uint32_t B31Query05B0(B31Delegates* self, uint32_t a, uint32_t b);
// TS3W.exe 0x008A0600: Query slot 7 with ordered short-circuit fallback; only AL is truth-tested.
uint32_t B31Query0600(B31Delegates* self, uint32_t a, uint32_t b);
// TS3W.exe 0x008A0650: Query slot 9 with ordered short-circuit fallback; only AL is truth-tested.
uint32_t B31Query0650(B31Delegates* self, uint32_t a, uint32_t b);
// TS3W.exe 0x008A06A0: Query slot 10 with ordered short-circuit fallback; only AL is truth-tested.
uint32_t B31Query06A0(B31Delegates* self, uint32_t a, uint32_t b);
// TS3W.exe 0x008A06F0: Query slot 11 with ordered short-circuit fallback; only AL is truth-tested.
uint32_t B31Query06F0(B31Delegates* self, uint32_t a, uint32_t b);
// TS3W.exe 0x008A0740: Query slot 12 with ordered short-circuit fallback; only AL is truth-tested.
uint32_t B31Query0740(B31Delegates* self, uint32_t a, uint32_t b);
// TS3W.exe 0x008A0790: Query slot 13 with ordered short-circuit fallback; only AL is truth-tested.
uint32_t B31Query0790(B31Delegates* self, uint32_t a, uint32_t b);
}

namespace ts3::audio {

struct B31Adapter {
    uint32_t B31WriteMatrix(uint32_t a0, void* a1);
    uint32_t B31Construct();
    void B31SetSecond(void* a0);
    uint32_t B31FirstOnly(uint32_t a0, uint32_t a1);
    void B31Broadcast(uint32_t a0);
    uint32_t B31Query04C0(uint32_t a0, uint32_t a1);
    uint32_t B31Query0570(uint32_t a0);
    uint32_t B31Query05B0(uint32_t a0, uint32_t a1);
    uint32_t B31Query0600(uint32_t a0, uint32_t a1);
    uint32_t B31Query0650(uint32_t a0, uint32_t a1);
    uint32_t B31Query06A0(uint32_t a0, uint32_t a1);
    uint32_t B31Query06F0(uint32_t a0, uint32_t a1);
    uint32_t B31Query0740(uint32_t a0, uint32_t a1);
    uint32_t B31Query0790(uint32_t a0, uint32_t a1);
};
}
