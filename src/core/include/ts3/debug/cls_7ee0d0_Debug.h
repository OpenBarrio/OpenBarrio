// The Debug unit around TS3W.exe 0x007EE0D0 (subsystem Debug): one small refcounted
// object with the UI-spy message router hanging off its vtable.
//
//   DebugObject   ctor 0x007EE0D0 (thiscall, one byte argument, ret 4), vtable 0x0101D964
//                         slot 0 +0x00 0x008ABE50 destructor (game code, not rewritten)
//                         slot 1 +0x04 0x007EE0F0 Release (this unit)
//                         slot 2 +0x08 0x0053B170 destroy (game code; Release calls it with 1)
//                         slot 3 +0x0C 0x007EF650 RouteMessage (this unit, ret 0xC)
//
// The object is 0x10 bytes: the vtable, a dword the constructor clears at +0x04, a
// single byte at +0x08 (the other three bytes of that dword are never written — kept
// from the original), and the atomic refcount at +0x0C. The sizes below are the touched
// extent; the game's objects may be larger.
#pragma once

#include <cstdint>

#include "ts3/layout.h"

namespace ts3::debug {

// What RouteMessage takes a pointer to: only +0x04 (0 = skip the whole message) and
// +0x08 (the message id) are read here.
struct UipyMessage {
    uint32_t unused00;  // +0x00 untouched by this unit
    uint32_t present;   // +0x04 nonzero = the message is live
    uint32_t id;        // +0x08 routed unless 0x0C, 0x0D, 0x14 or 0x7893FADB
};
TS3_X86_OFFSET(UipyMessage, present, 0x04);
TS3_X86_OFFSET(UipyMessage, id, 0x08);
TS3_X86_SIZE(UipyMessage, 0x0C);

// The stack vector RouteMessage builds for the game's router 0x007EF1D0: an EASTL-like
// pointer triple over a 0x100-byte inline buffer at +0x14, the channel name at +0x0C and
// the inline buffer's address again at +0x10, so the callee can tell an inline buffer
// from a heap one. Only the first byte of the buffer is written — the original leaves
// the rest of the stack slot uninitialized.
struct RouteBuffer {
    char* begin;             // +0x00 start of the data
    char* end;               // +0x04 one past the last byte written
    char* cap;               // +0x08 one past the inline buffer (inlineBuf + 0x100)
    const char* channel;     // +0x0C the game's "Debug/UISpy" at 0x0101E1BC
    char* inlinePtr;         // +0x10 == begin while the data lives inline
    char inlineBuf[0x100];   // +0x14 inline storage
};
TS3_X86_OFFSET(RouteBuffer, end, 0x04);
TS3_X86_OFFSET(RouteBuffer, cap, 0x08);
TS3_X86_OFFSET(RouteBuffer, channel, 0x0C);
TS3_X86_OFFSET(RouteBuffer, inlinePtr, 0x10);
TS3_X86_OFFSET(RouteBuffer, inlineBuf, 0x14);
TS3_X86_SIZE(RouteBuffer, 0x114);

struct DebugObject;

// vtable 0x0101D964. The destructor slot and `destroy` are game code; `release` and
// `route` are this unit's entries, so an object built by the original constructor can
// be driven by the rewrite and vice versa.
struct DebugVtable {
    void* dtor;                                                      // +0x00 0x008ABE50 (game)
    int32_t(__thiscall* release)(DebugObject* self);                 // +0x04 0x007EE0F0 (this unit)
    void(__thiscall* destroy)(DebugObject* self, int32_t arg);       // +0x08 0x0053B170 (game)
    void(__thiscall* route)(DebugObject* self, uint32_t kind, void* extra, const UipyMessage* msg);  // +0x0C
};
TS3_X86_SIZE(DebugVtable, 0x10);

// The object itself (touched extent 0x10 bytes).
struct DebugObject {
    void* vtable;         // +0x00 the game's 0x0101D964 on x86
    uint32_t unused04;    // +0x04 cleared by Construct
    uint8_t mode;         // +0x08 Construct's byte argument (only this byte is written)
    uint8_t unused09[3];  // +0x09 never touched, whatever the object held survives
    uint32_t refcount;    // +0x0C starts at 0, Release decrements it

    // TS3W.exe 0x007EE0D0, ret 4 (thiscall, one byte argument): store the byte at +0x08,
    // install the vtable, clear +0x04 and the refcount. Returns `this`.
    DebugObject* Construct(uint8_t modeArg);
    // TS3W.exe 0x007EE0F0 (thiscall): atomically decrement the refcount and return the
    // new value; when it reaches 0, dispatch vtable slot 2 (destroy) with 1. The
    // original's null-`this` test after the decrement is dead code (the xadd above
    // already dereferenced `this`) and is not reproduced.
    int32_t Release();
    // TS3W.exe 0x007EF650, ret 0xC (thiscall — IDA calls it __stdcall, but ecx is never
    // touched here and is passed through to the router 0x007EF1D0, which is what makes
    // it thiscall): drop uninteresting UISpy messages, build a RouteBuffer on the stack,
    // hand it to the game's router, then release the heap buffer the router may have
    // left in it.
    void RouteMessage(uint32_t kind, void* extra, const UipyMessage* msg);
};
TS3_X86_OFFSET(DebugObject, unused04, 0x04);
TS3_X86_OFFSET(DebugObject, mode, 0x08);
TS3_X86_OFFSET(DebugObject, refcount, 0x0C);
TS3_X86_SIZE(DebugObject, 0x10);

}  // namespace ts3::debug
