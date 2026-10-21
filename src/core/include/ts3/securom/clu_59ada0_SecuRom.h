// Cluster unit clu_59ada0_SecuRom (subsystem SecuRom): the cached
// LocalizedStringService getter, its "look one message up by key" method, one bridge
// forwarder, and the five SIGS job reporters that fetch the
// L"SecuRom/ErrorMessage:FailToServer" text and send it to the SimCity/InGame-Store
// network layer as a MsgData message.
//
//   0x0059ADA0  GetLocalizedStringService0059ADA0  (leaf, 49 callers): the cached
//                                    service pointer in the global at 0x011CE2D0.
//   0x0059CD10  LocalizedStringService::GetErrorString0059CD10 (thiscall, ret 8):
//                                    FNV-64 the key (game 0x007CFE40), ask the service
//                                    (game 0x0059CB00) for that hash; on a miss clear
//                                    the string through game 0x0040E920 and return 0.
//   0x00743790  Forward59C5B000743790 (cdecl): the service getter plus a tail call to
//                                    the service's 0x0059C5B0 with both arguments.
//   0x0088ED80  ReportFsiParamsFailToServer0088ED80 (caller passes ecx, never read):
//                                    also presets four job-tuning dwords and one byte.
//   0x0088F720  ReportStoreSimPointsFailToServer0088F720
//   0x0088F940  ReportPngDownloadFailToServer0088F940
//   0x0088FCE0  IgsPurchaseJob::ReportFailToServer0088FCE0 (thiscall): forwards
//                                    this+0x18 into the message.
//   0x00890A30  IgsPurchaseJob::ReportFailToServerTo00890A30 (thiscall, ret 4): takes
//                                    the dispatcher as an argument instead of reading
//                                    the singleton, and lets game 0x0088FDB0 build the
//                                    message.
//
// Shape of the five reporters (identical apart from the constants): build a stack
// SecuRomString, look the fail-to-server text up, and when it was found get the SIGS
// dispatcher (0x007692C0), allocate a message (0x004E3F90, "SIGS/pMsg"), construct it,
// AddRef it, dispatch vtable slot 7 with the message id, Release it; then release the
// string buffer if it is heap-owned and return the string's data pointer (or operator
// delete's EAX on that path).
//
// EA quirks kept:
//  - the ownership test is signed: the buffer is freed only when (capacityEnd - begin),
//    with its lowest bit cleared, is greater than 2 - a capacityEnd below begin wraps to a negative value and
//    does NOT free, and a 3-byte span masks down to 2 and does NOT free either;
//  - 0x00890A30 ignores the lookup's result: it sends the message even when the key was
//    not found (the other four gate on it);
//  - 0x0088ED80 writes its four tuning globals and its flag byte BEFORE the lookup, so
//    they are written even when the lookup fails;
//  - the reporters return whatever EAX the delete call left, not the string pointer, on
//    the heap-owned path;
//  - 0x0059CD10 sets only AL (1 on a hit, 0 on a miss): the upper bytes of EAX stay
//    dirty, so the return contract is one byte.
//
// Nothing here is rewritten but this unit: the service's own lookup, the message
// constructors and the allocator run through the ts3 layout.h bridge (x64 builds abort
// there - no game code to call).
#pragma once

#include <cstdint>

#include "ts3/layout.h"

namespace ts3::securom {

// The game's static empty wide-string buffer (NewFileName at 0x011C7E54, one wchar_t):
// every stack SecuRomString of this unit starts as {begin = cursor = buffer, capacityEnd
// = buffer + 2}, which is what IsHeapBuffer() below recognises as "not mine".
constexpr uintptr_t kStaticEmptyString = 0x011C7E54u;  // NewFileName
constexpr uintptr_t kStaticEmptyStringEnd = 0x011C7E56u;

// 0x0059CD10's value argument: the four pointer-sized fields the game's job reporters
// keep on the stack. The reporters only write begin/capacityEnd/tag and read begin; the
// service's lookup (0x0059CB00) fills the text and moves cursor/capacityEnd, and the
// miss path (0x0040E920) sets cursor = begin. `tag` is the "SIGS/<Job>/errString" label.
struct SecuRomString {
    void* begin;        // +0x00 the text (returned by the reporters)
    void* cursor;       // +0x04
    void* capacityEnd;  // +0x08 end of the storage begin points into
    const void* tag;    // +0x0C

    // The state the original's four constant stack stores set up, with `tag` = the caller's
    // "SIGS/<Job>/errString" label (a game string address).
    static SecuRomString Empty(const void* tag);

    // The original's `(capacityEnd - begin) & ~1 > 2` signed test: true when the buffer
    // is not the 2-byte static empty one. Deliberately pointer-width unsigned arithmetic
    // cast back to signed, so a capacityEnd below begin stays "do not free".
    bool IsHeapBuffer() const;

    // `begin` if it is not heap-owned, else the result of the game's operator delete
    // (0x004E3F70) - which is the reporters' return value on that path (EA quirk).
    void* Release();
};

// The service the global at 0x011CE2D0 points at; its fields are only read by the
// functions this unit calls through the bridge.
struct LocalizedStringService {
    // TS3W.exe 0x0059CD10 (thiscall, ret 8, AL only): hash `key` with the game's
    // FNV-64-lower-wide (0x007CFE40) and ask the service (0x0059CB00) for that string;
    // 1 when found, else the miss helper 0x0040E920 clears `out` and this returns 0.
    uint8_t GetErrorString0059CD10(const wchar_t* key, SecuRomString* out) const;
};

// TS3W.exe 0x0059ADA0 (cdecl leaf, `ret`): returns the pointer stored at 0x011CE2D0 - the cached service.
// It pops nothing, so callers that pushed arguments balance them with their next callee.
LocalizedStringService* __cdecl GetLocalizedStringService0059ADA0();

// TS3W.exe 0x00743790 (cdecl): svc = 0x0059ADA0(); return 0x0059C5B0(svc, a1, a2). The
// game's callee declares its first argument as a char but reads the pushed dword only
// where it uses it, so both are forwarded as full dwords. No caller in the image
// reaches it - covered by the exehost test instead of a game-compare hook.
int32_t __cdecl Forward59C5B000743790(uint32_t a1, int32_t a2);

// TS3W.exe 0x0088ED80: presets 0x01169378=100, 0x0116937C=5, 0x01169380=1000,
// 0x01169384=20, byte 0x011D985C=1, then the shared reporter shape with the
// "SIGS/DownloadFSIParamsJob/errString" tag, a 44-byte message built by 0x0088ED20 with
// (29, begin, szPassword) and message id 0x526701D1. `this` is passed by the callers but
// never read.
void* __cdecl ReportFsiParamsFailToServer0088ED80();

// TS3W.exe 0x0088F720: the same without the globals, tag
// "SIGS/DownloadStoreSimPointsJob/errString", message 0x0088ED20(38, begin, szPassword),
// id 0x526701D1. `this` unused.
void* __cdecl ReportStoreSimPointsFailToServer0088F720();

// TS3W.exe 0x0088F940: tag "SIGS/DownloadPngJob/errString", a 28-byte message built by
// 0x00884320(27, begin) and id 0x49AFD6A0. `this` unused.
void* __cdecl ReportPngDownloadFailToServer0088F940();

// The SIGS InGameStore purchase job. Only +0x18 is observed by this unit (both entries
// read it); the game's object is larger.
struct IgsPurchaseJob {
    uint8_t pad00[0x18];
    void* d18;  // +0x18 forwarded as the message's second string source

    // TS3W.exe 0x0088FCE0 (thiscall): tag "SIGS/IgsPurchaseJob/errString", a 48-byte
    // message built by 0x0088FC10(25, begin, this+0x18, 4), id 0x0CCFB69D.
    void* ReportFailToServer0088FCE0();

    // TS3W.exe 0x00890A30 (thiscall, ret 4): same key and tag, but the dispatcher is the
    // argument, the lookup's result is ignored, and 0x0088FDB0(dispatcher, &string, 6) does
    // the message building itself (it reads `begin` from the struct base); only the string
    // release is left here.
    void* ReportFailToServerTo00890A30(uint32_t dispatcher);
};
TS3_X86_OFFSET(IgsPurchaseJob, d18, 0x18);
TS3_X86_SIZE(SecuRomString, 16);

}  // namespace ts3::securom
