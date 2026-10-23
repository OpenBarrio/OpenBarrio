// Rewritten cluster unit clu_408f10_Scratch (Scratch subsystem): the download-content
// manifest helpers (wide-string copy/assign, library loading, value glue and the
// manifest/attribute walkers) plus a handful of small dispatch and record routines.
//
// Calling conventions are the originals': __thiscall functions are declared __fastcall
// with an explicit dummy second parameter (self in ECX, dummy in EDX on x86) so the
// harness can swap them in at their TS3W.exe address; cdecl functions stay plain.
#pragma once

#include <cstdint>

#include "ts3/layout.h"

// WideString only ever appears as an opaque pointer here. Forward-declaring it instead of
// including clu_408e20_Scratch.h keeps this header includable from the harness and the
// autotest registry, which already hold clu_4b4250_Scratch.h's different WideString.
namespace ts3::scratch {
struct WideString;
}

namespace ts3::scratch {

// ---------------------------------------------------------------------------------------
// 0x00408F10 (__thiscall, ret 4). Assign the wide string `other` to `self` (through
// 0x00408E20) unless the two objects are the same; returns `self`.
WideString* __fastcall CopyWide408F10(WideString* self, void*, const WideString* other);

// 0x00409010 (__thiscall, ret 4). Assign the NUL-terminated wide string `src` to the wide
// string `self`; returns whatever 0x00408E20 left in EAX (the destination's buffer when a
// growth happened, otherwise the tested source pointer).
WideString* __fastcall AssignWideFromCStr409010(WideString* self, void*, const wchar_t* src);

// 0x004F2800 (__thiscall, ret 8). Load `path` as a library, require its
// "EASharedLibraryStartupFunction" export and, when `flag` and the object's virtual slot 5
// allow it, keep the name (0x00409010) and drop the previous one. Returns 1 in AL on
// success, 0 in AL on every failure; the upper EAX bits are the last callee's.
uint8_t __fastcall LoadSharedLibrary4F2800(void* self, void*, const wchar_t* path,
                                           uint8_t flag);

// 0x00500830 (__thiscall, ret 4). The 64-bit value `index` (0 or 1) assembled from the two
// dword pairs at self+0x00 and self+0x08; any other index returns 0.
uint64_t __fastcall GetValue500830(void* self, void*, int32_t index);

// 0x007DA1B0 (__thiscall, ret 8). When the dispatcher at self+0x48 is in state 2, forward
// the two arguments to virtual slot 6 of the object at self+0x0C; otherwise 0.
char __fastcall Dispatch7DA1B0(void* self, void*, int32_t a2, int32_t a3);

// 0x007DA210 (__thiscall, ret 0). When the dispatcher at self+0x48 is in state 2, reset it
// to 0 and return 1; otherwise 0.
char __fastcall ResetState7DA210(void* self, void*);

// 0x007DA2D0 (__thiscall, ret 8). Like 0x007DA1B0 but the third argument is read from
// *a3.
char __fastcall Dispatch7DA2D0(void* self, void*, int32_t a2, const void* a3);

// 0x007DA840 (__thiscall, ret 8). In state 1 convert `value` through 0x00471070 and assign
// the result into the wide string `dst` (0x00409010); 0 when the state is not 1, the input
// object changed state/flag, or the conversion failed.
char __fastcall ConvertAndAssign7DA840(void* self, void*, int32_t value, WideString* dst);

// 0x007DBAF0 (__thiscall, ret 4). In state 2 verify the wide string `text` through
// 0x004E33D0
// unless the object switched to state 1 with its flag set, in which case 0. The
// 0x004E33D0
// itself (EA's min-clamp idiom), and a zero-length string only continues when 0x004E33D0
// reported success.
char __fastcall AppendConverted7DBAF0(void* self, void*, const WideString* text);

// 0x008A7870 (__thiscall, ret 4). Initialise the 16-byte wide string `self` to the static
// empty string, taking the allocator name from *src. Returns `self`.
WideString* __fastcall InitName8A7870(WideString* self, void*, const void** src);

// Hook shim for 0x008A7870: hookgen renders a thiscall rewrite as a member call, and a
// non-static member gets the game object in ECX (no data members, so `this` is the object).
struct InitName8A7870Shim {
    void* InitName8A7870(const void* src) {
        return ts3::scratch::InitName8A7870(reinterpret_cast<WideString*>(this), nullptr,
                                            reinterpret_cast<const void**>(const_cast<void*>(src)));
    }
};

// 0x0093D2C0 (__thiscall, ret 0xC). Forward the range [a2, a3) and the caller's first
// argument to 0x006C8370 with the range end passed twice; the incoming ECX is forwarded as
// 0x006C8370's this.
int32_t __fastcall CopyRange93D2C0(void* self, void*, int32_t a1, int32_t a2, int32_t a3);

// 0x009B3820 (__cdecl). Walk one XML-like record: while holding `logger`, convert the
// element/attribute names and values (0x004F3750) and dispatch them through 0x007DA1B0,
// then recurse into the record's child list. Returns 0 when the record is missing or the
// logger is closed, else the logger's leave result.
char __cdecl WalkRecord9B3820(void* record, void* logger);

// 0x009B3A10 (__thiscall, ret 4). Enter `logger`, walk the vector of records at self
// through 0x009B3820 until one fails and leave the logger; returns 0 when the logger could
// not be entered or a record failed, else the leave result.
char __fastcall WalkRecords9B3A10(void* self, void*, void* logger);

// 0x009B3A90 (__thiscall, ret 4). Log the eight wide strings at self+0xA0/+0xB0/+0xC0/
// +0xD0/+0xE0/+0xF0/+0x100/+0x110 and the three 16-byte vectors at self+0x120/+0x130/
// +0x140 through the logger's enter/log/leave calls (0x007DBA60 fields follow the record).
int32_t __fastcall LogRecord9B3A90(void* self, void*, void* logger);

// 0x009B8250 (__thiscall, ret 4). Build and log the whole manifest of `a2`: package type
// and sub-type, the platform data, asset version, flags, thumbnail key, the two dependency
// lists, the key list and the package builder. Returns 1 in AL when the manifest was
// written (also when the logger could not be entered at all) and 0 in AL when a step
// failed.
char __fastcall BuildManifest9B8250(void* self, void*, void* a2);

// 0x009B95A0 (__thiscall, ret 8). Ask the loader `a2`'s virtual slot 13 to produce an
// object, load it with 0x009B8250 and release it through its own virtual slots; returns the
// 0x009B8250 result (0 when the loader is null or nothing was produced).
char __fastcall CreateAndLoad9B95A0(void* self, void*, void* loader, void* source);

// 0x009C6630 (__thiscall, ret 0xC). Write `length` bytes from the current read position:
// the container at self+0x110 must accept an offset (self+0x118 plus src+0x24) through its
// virtual slot 10, the writer `target` must accept every chunk through its virtual slot 14,
// and a 0x1000-byte scratch string must be available. Returns 1 on success.
char __fastcall WriteBuffer9C6630(void* self, void*, const void* src, void* target,
                                  uint32_t length);

// 0x009C6760 (__thiscall, ret 8). Write element `index` of the 2152-byte array at
// self+4/+8 through 0x009C6630, using the element's dword at +0x20 as the length; an
// out-of-range index passes a null element and reads the length from the absolute address
// 0x20 (the original's null dereference, kept on x86).
char __fastcall WriteIndexed9C6760(void* self, void*, uint16_t index, void* target);

}  // namespace ts3::scratch