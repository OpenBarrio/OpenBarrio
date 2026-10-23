// Rewritten cluster unit clu_408e20_Scratch (Scratch subsystem): the game's 16-byte
// wide-string object helpers (assign range / append / copy / insert / erase) plus a
// handful of unrelated record copy, init and destroy routines.
//
// Calling conventions are the originals': __thiscall functions are declared __fastcall
// with an explicit dummy second parameter (self in ECX, dummy in EDX on x86) so the
// harness can swap them in at their TS3W.exe address; cdecl functions stay plain.
#pragma once

#include <cstddef>
#include <cstdint>

#include "ts3/layout.h"

namespace ts3::scratch {

// The game's wide-string object used by the 0x00408E20 family. Pointer-sized fields;
// +0x0C is the allocator/name tag which the game's global heap helper 0x004E3F90
// ignores. Empty strings point at the static sentinel 0x011C7E54 (cap 0x011C7E56).
struct WideString {
    void* begin;      // +0x00
    void* end;        // +0x04
    void* cap;        // +0x08
    void* allocName;  // +0x0C
};
TS3_X86_OFFSET(WideString, begin, 0x00);
TS3_X86_OFFSET(WideString, end, 0x04);
TS3_X86_OFFSET(WideString, cap, 0x08);
TS3_X86_OFFSET(WideString, allocName, 0x0C);
TS3_X86_SIZE(WideString, 16);

// The vector of these strings used by 0x004EA630: {begin, end, capEnd, allocator*, name,
// flags}; the allocator interface object is called through its vtable (slot 2 alloc,
// slot 3 free) and the vector's element destroyer is 0x0073ECC0.
struct StringVector {
    void* begin;      // +0x00
    void* end;        // +0x04
    void* capEnd;     // +0x08
    void* allocator;  // +0x0C, interface object (vtable +8 allocates, +0xC frees)
    void* field10;    // +0x10, the allocator's 3rd argument
    void* field14;    // +0x14, the allocator's 2nd argument
};
TS3_X86_OFFSET(StringVector, begin, 0x00);
TS3_X86_OFFSET(StringVector, end, 0x04);
TS3_X86_OFFSET(StringVector, capEnd, 0x08);
TS3_X86_OFFSET(StringVector, allocator, 0x0C);
TS3_X86_SIZE(StringVector, 0x18);

// The 0x001C-byte element of the vector driven by 0x0060A0B0/0x0060A300/0x0060A780.
struct Element1C {
    WideString text;  // +0x00
    uint32_t f10;     // +0x10
    uint32_t f14;     // +0x14
    uint32_t f18;     // +0x18
};
TS3_X86_OFFSET(Element1C, text, 0x00);
TS3_X86_OFFSET(Element1C, f10, 0x10);
TS3_X86_SIZE(Element1C, 0x1C);

// The 0x0070-byte record built by 0x00D28930, destroyed by 0x00D289A0, copied by
// 0x00D28A90
// own copy (0x00621DB0) and destroy (0x005BF1E0) helpers.
struct RecordD28930 {
    uint32_t d00;       // +0x00
    uint32_t d04;       // +0x04
    uint32_t d08;       // +0x08
    uint32_t d0C;       // +0x0C
    uint32_t d10;       // +0x10
    uint32_t d14;       // +0x14
    uint32_t d18;       // +0x18
    uint32_t d1C;       // +0x1C
    uint32_t d20;       // +0x20
    uint32_t d24;       // +0x24
    uint32_t d28;       // +0x28
    uint32_t d2C;       // +0x2C
    uint32_t d30;       // +0x30
    uint32_t d34;       // +0x34
    uint32_t d38;       // +0x38
    uint32_t d3C;       // +0x3C
    uint8_t block40[0x10];  // +0x40
    WideString text50;  // +0x50
    WideString text60;  // +0x60
};
TS3_X86_OFFSET(RecordD28930, block40, 0x40);
TS3_X86_OFFSET(RecordD28930, text50, 0x50);
TS3_X86_OFFSET(RecordD28930, text60, 0x60);
TS3_X86_SIZE(RecordD28930, 0x70);

// ---------------------------------------------------------------------------------------
// 0x00408E20 (__thiscall, ret 8). Assign the character range [src, end) to the wide
// string at `self`. When the range is no longer than the current length the existing
// buffer is reused (memmove, fix the terminator, set end); otherwise the existing
// characters are memmoved and the remainder is appended through 0x00408B90 (which grows
// the buffer). The length compare is unsigned, so an inverted range (src > end) takes the
// growth path with a negative count - kept. Returns `self`.
void* __fastcall AssignWide408E20(WideString* self, void*, const void* src, const void* end);

// 0x00412BD0 (__thiscall, ret 8). 0x00408E20 with a character count: end = src + 2*count.
void* __fastcall AssignWideCount412BD0(WideString* self, void*, const void* src, int32_t count);

// 0x004EA340 (__cdecl). Copy the 0x10-byte strings in [src, srcEnd) backwards to `dst`;
// elements whose source and destination addresses coincide are left alone. Returns the
// first destination element (dst - 0x10*count), or `dst` when the range is empty.
uint8_t* __cdecl CopyBackwardStrings4EA340(const uint8_t* src, const uint8_t* srcEnd, uint8_t* dst);

// 0x004EA400 (__cdecl). Lazily import GetModuleFileNameW from Kernel32.dll (flag at
// 0x011CB8C8, pointer at 0x011CB8C4) and assign the module file name into `out`; returns
// the resulting character count. The 260-character buffer and the "only when the import
// resolves" check are the original's.
int32_t __cdecl GetExecutablePath4EA400(WideString* out);

// 0x004EA630 (__thiscall, ret 8). Insert the element at `value` before `pos` in the
// vector. Without reallocation the last element is copy-constructed at `end` (through
// 0x00408EB0), the tail is copied backwards and the new value assigned at `pos`; a
// `value` inside [pos, end) moves with the shift, so it is taken from value+0x10. The
// reallocating path doubles the capacity through the allocator interface at +0x0C and
// copy-constructs the elements with 0x009A3720. Returns the new capacity end.
uint8_t* __fastcall InsertVectorElement4EA630(StringVector* self, void*, uint8_t* pos,
                                              const uint8_t* value);

// 0x00595330 (__thiscall, ret 8). Copy element `index` of the vector at self+0x488 into
// `out`; returns 0 (and leaves `out` alone) when index is out of range. The range check is
// unsigned. Returns 1 on success.
uint8_t __fastcall GetVectorElement595330(void* self, void*, uint32_t index, void* out);

// 0x0059D8A0 / 0x0059D8D0 / 0x0059D900 / 0x0059D930 (__thiscall, ret 4). Assign the
// NUL-terminated wide string `src` (length by scanning) to the string at self+0x40 /
// +0x50 / +0x60 / +0x70. Each returns the destination string object.
void* __fastcall SetWide40_59D8A0(void* self, void*, const wchar_t* src);
void* __fastcall SetWide50_59D8D0(void* self, void*, const wchar_t* src);
void* __fastcall SetWide60_59D900(void* self, void*, const wchar_t* src);
void* __fastcall SetWide70_59D930(void* self, void*, const wchar_t* src);

// 0x005AFBB0 (__thiscall, ret 8). Look up preset index `index` in the object at
// self+0x230 (a vector of pointers, stride 8); when it exists inside the vector call
// 0x005BF4C0 on it, otherwise fetch entry `index - count` from the containers at 0x00609C10
// / 0x00609CA0. A scoped lock token from 0x00B319C0/0x00B32EF0 is released through its
// vtable slot 1 on every path. Returns 1 when `out` was written.
uint8_t __fastcall FindOrLoad5AFBB0(void* self, void*, uint32_t index, void* out);

// 0x0060A0B0 (__cdecl). Copy the 0x1C-byte elements in [src, srcEnd) to `dst`: assign the
// string at +0x00 and copy the three dwords +0x10..+0x18; equal addresses skip the string
// assign. Returns the one-past-end destination.
uint8_t* __cdecl CopyElements60A0B0(const uint8_t* src, const uint8_t* srcEnd, uint8_t* dst);

// 0x0060A300 (__thiscall, ret 4). Erase the element at `elem` of the vector `self`:
// shift the following elements down, shrink the end by 0x1C and free the dropped
// element's string buffer when it owns one. Returns `elem`.
uint8_t* __fastcall EraseElement60A300(void* self, void*, uint8_t* elem);

// 0x0060A780 (__thiscall, ret 8). Erase element `elementIndex` of the vector `self +
// 0x28 + 0x10*vectorIndex`; returns 0 when the vector index is >= 8 (signed), when the
// element index is out of range (unsigned compare against the signed count) or when
// nothing was erased.
uint8_t __fastcall EraseIndexedElement60A780(void* self, void*, int32_t vectorIndex,
                                             uint32_t elementIndex);

// 0x0073FBC0 (__thiscall, ret 0). Append a default (empty) element to the string vector
// `self` and return it. The in-capacity path writes the static empty-string sentinel and
// the allocator name 0x01000400 and bumps the end even when the old end was null; the
// full path pushes a temporary through 0x0051D700 and returns the temporary's begin
// pointer (or 0x004E3F70's value when the temporary owned a buffer) - kept.
void* __fastcall PushBackCommandList73FBC0(void* self, void*);

// 0x0073FCB0 (__cdecl). Append a default element to the vector `list` and, unless the
// byte at record+0x0C is set, fill it with the string at record+0x04, optionally " - "
// plus record+0x08, and a trailing newline. Returns the last callee's value (the element
// on the append paths, 0x0073FBC0's value when it returns early).
void* __cdecl AppendCommandLine73FCB0(const void* record, void* list);

// 0x007467F0 (__cdecl). Under the critical section at 0x011EA210, assign the string
// [src[0], src[1]) to the global string at 0x011818CC unless src is that global itself.
void __cdecl SetGlobalString7467F0(const void* src);

// 0x00793D30 (__cdecl). Build the option string for `a1` (0x004F5E60/0x004F62C0/
// 0x0078F2B0/0x007935B0) and copy the substring between the second character and the last
// one into `out`; returns 0 when the substring was written and 1 when the option list was
// empty. The "second character" start and the trailing -1/`count-1` clamp are the
// original's.
char __cdecl LookupOption793D30(char a1, int32_t a2, WideString* out);

// 0x0083BE10 (__thiscall, ret 4). Reset the passed record (release the twenty pointers
// 0x00873360, copy the five dwords +0x50..+0x60 and the six bytes +0x64..+0x69 and assign
// the string at +0x6C into self+0x6C). Returns `self + 0x6C` when the strings differ, else
// the record's own +0x6C pointer (the original's EAX on that path).
void* __fastcall ResetAndCopy83BE10(void* self, void*, uint8_t* record);

// 0x00884250 (__thiscall, ret 0x10). Move a temporary string into self+0x18: assign
// [begin, end) and then release the temporary buffer at `allocEnd` when it owns one
// (unchanged, >2 bytes and non-null). Returns the (allocEnd - begin) & ~1 value, or
// 0x004E3F70's result when the temporary was freed. The 5th argument is unused.
int32_t __fastcall MoveStringInto18_884250(void* self, void*, const char* begin, const char* end,
                                           const char* allocEnd, const char* unused);
// 0x00884290 (__thiscall, ret 0x10). Same as 0x00884250 for self+0x28.
int32_t __fastcall MoveStringInto28_884290(void* self, void*, const char* begin, const char* end,
                                           const char* allocEnd, const char* unused);

// 0x00894DE0 (__thiscall, ret 4). Ask the object's virtual slot 18 for a record, and when
// it has a non-empty name at +0x430 assign the name converted by 0x004F3750 into `out`.
// Returns 0x004FE0D0's result when the record is empty.
int32_t __fastcall GetOwnerPath894DE0(void* self, void*, WideString* out);

// 0x00894E70 (__thiscall, ret 4). Same as 0x00894DE0 with the name at this+0x0CD8.
int32_t __fastcall GetOwnerPath894E70(void* self, void*, WideString* out);

// 0x00956210 (__thiscall, ret 4). Assign [a2, a2+4) into this+0x9C and [a2+0x10, a2+0x14)
// into this+0xAC, then release the reference taken through this+8 (0x00580420), calling
// 0x0085A760 with this+0x9C when the object claims it and dropping the last reference
// through vtable slot 0 and slot 1 (with the count clamped to 1 first).
void* __fastcall AssignPairCopy956210(void* self, void*, const void* source);

// 0x009A3410 (__thiscall, ret 4). Copy the string [this+0x24, this+0x28) into `out` when
// it is non-empty; returns 0 without touching `out` when begin == end.
char __fastcall GetString24_9A3410(void* self, void*, WideString* out);

// 0x009A36A0 (__cdecl). Deep-swap the 0x14-byte nodes *a and *b: the leading dword and
// the 16-byte string at +0x04, through a copy-constructed temporary (0x00408EB0).
void* __cdecl SwapNode9A36A0(void* a, void* b);

// 0x009E7BF0 / 0x009E7D20 / 0x009E80C0 (__cdecl). Build the IGS registration URL into
// `out`: "https://" plus the host name built by 0x004F3890 (then ".thesims3.com" is
// detected by 0x009C1F70: the name is appended without its first character, else "www"
// and the name are appended) plus the page suffix. 0x009E7D20/0x009E80C0 append the name
// a second time after the suffix. Return the local string's begin pointer (0x004E3F70's
// value when it was freed) - the original's EAX.
void* __cdecl BuildIgsRegisterUrl9E7BF0(WideString* out);
void* __cdecl BuildIgsRegisterUrl9E7D20(WideString* out);
void* __cdecl BuildIgsRegisterUrl9E80C0(WideString* out);

// 0x00AAC380 (__thiscall, ret 4). Assign the string [this+8, this+0x0C) into `out`;
// always returns 1.
char __fastcall GetPath8AAC380(void* self, void*, WideString* out);

// 0x00AAC680 (__thiscall, ret 4). Assign the NUL-terminated wide string `src` (length by
// scanning) to the string at self+0x410. Returns the destination string object.
void* __fastcall SetWide410AAC680(void* self, void*, const wchar_t* src);

// 0x00ABE890 (__thiscall, ret 4). Clear `out`, then when 0x0059ADA0 reports a container,
// ask 0x0059CB00 for the entry at this+0x3E8/this+0x3EC and, if it is missing and the
// container at this+0x3D8 has entries, assign its first string into `out`. Returns the
// last callee's value.
void* __fastcall FindAndCopyABE890(void* self, void*, WideString* out);

// 0x00D00D40 (__thiscall, ret 4). Copy the eight dwords +0x00..+0x1C, the string at +0x20
// (unless the source and destination addresses are equal) and the dwords +0x30/+0x34 into
// `self`. Returns `self`.
void* __fastcall CopyRecordD00D40(void* self, void*, void* source);

// 0x00D28930 (__thiscall, ret 0). Build the 0x70-byte record: zero the eighteen dwords
// +0x00..+0x44, tag the three allocator-name slots (+0x4C, +0x5C, +0x6C) with 0x01085DF4
// and initialise the two strings at +0x50 and +0x60 to the static empty string. Returns
// `self`.
RecordD28930* __fastcall InitRecordD28930(RecordD28930* self, void*);

// 0x00D289A0 (__thiscall, ret 0). Release the buffers of the strings at +0x60 and +0x50
// (when owned) and destroy the block at +0x40 through 0x005BF1E0.
int32_t __fastcall DestroyRecordD289A0(RecordD28930* self, void*);

// 0x00D28A90 (__thiscall, ret 4). Copy sixteen dwords +0x00..+0x3C, the block at +0x40
// through 0x00621DB0 and the two strings at +0x50/+0x60 into `self`. Returns `self`.
RecordD28930* __fastcall CopyRecordD28A90(RecordD28930* self, void*, const RecordD28930* source);

// 0x00D28E40 (__thiscall, ret 0x10). Insert key `key` into the ordered tree rooted at
// this+4, next to `node`: at the end when `node` is the root/sentinel, otherwise between
// `node` and its successor when the key lies inside (0x004E0B20 gives the successor);
// otherwise 0x00D28D50 finds the position. The 64-bit compares are the original's.
void* __fastcall TreeInsertD28E40(void* self, void*, void* out, void* node, const void* key,
                                  int32_t flagLeast);

// 0x00D29060 (__thiscall, ret 4). Find the node of the tree at self+4 whose 64-bit key
// (node+0x10) matches [a2, a2+4); build a new record (0x00D28930), copy the key in, and
// when the key is absent (or maps before the found node's key) insert it. Returns
// node+0x18 of the found/inserted node.
int32_t __fastcall TreeFindOrInsertD29060(void* self, void*, const void* key);

// 0x00D291B0 (__thiscall, ret 0x14). Record an undoable FabricChange for entity `a2`:
// when 0x00939560 accepts it and the object 0x00935770(this = the 0x011E8FC4 global)
// exists, either update the existing node found by 0x006FE020 (writing a4's four dwords,
// a5's four dwords and the string a6) or fill a new record and insert it through
// 0x00D29060/0x00D28A90. Returns the 0x00939..., 0x00935770 or 0x0094A990 value of the
// path taken.
int32_t __fastcall RecordFabricChangeD291B0(void* self, void*, char a2, int32_t a3, void* a4,
                                            void* a5, void* a6);

// 0x00EC3850 (__cdecl). Look the object `key` up in the registry at 0x01196068
// (0x0061BA60): when it is the sentinel 0x0119606C, copy `text` into a temporary string,
// hand it to 0x00EC37A0 (which returns its byte through the frame) and return that byte;
// otherwise assign `text`'s string into the looked-up object's string at +0x14 and return 1.
char __cdecl SetOrCopyEC3850(int32_t key, WideString* text);

}  // namespace ts3::scratch