// Rewritten cluster unit clu_4dbbd0_Scratch (Scratch subsystem): the TS3W.exe
// DownloadContent / world-archive scratch functions 0x004DBBD0 .. 0x00D15C50.
//
// The entry points keep the originals' calling conventions so the harness can swap
// them in at their TS3W.exe address: __thiscall entries are declared __fastcall with an
// explicit dummy second parameter (self in ECX, dummy in EDX on x86; the stack arguments
// are callee-popped, matching the original's ret N). cdecl/stdcall entries stay as
// declared. Private helpers of this unit are called directly.
//
// The game's 16-byte wide string as this unit uses it: {begin, end (one past the last
// code unit), capacity end, allocation tag}. This is deliberately a unit-local type
// rather than clu_408e20_Scratch.h's WideString: clu_4b4250_Scratch.h defines a
// different 16-byte struct with that same name (wchar_t begin/cur/cap/tag) in this
// namespace, and the harness and autotest translation units include both units' headers.
#pragma once

#include <cstddef>
#include <cstdint>

#include "ts3/layout.h"

namespace ts3::scratch {

struct ScratchWideString {
    void* begin;      // +0x00
    void* end;        // +0x04
    void* cap;        // +0x08
    void* allocName;  // +0x0C
};
TS3_X86_OFFSET(ScratchWideString, begin, 0x00);
TS3_X86_OFFSET(ScratchWideString, end, 0x04);
TS3_X86_OFFSET(ScratchWideString, cap, 0x08);
TS3_X86_OFFSET(ScratchWideString, allocName, 0x0C);
TS3_X86_SIZE(ScratchWideString, 16);

// The node of the recursive list freed by 0x009886E0: the child list at +0x00, the
// next sibling at +0x04 and the owned pointer at +0x14 (the remaining words are left
// alone by the original).
struct ScratchListNode {
    ScratchListNode* child;  // +0x00
    ScratchListNode* next;   // +0x04
    void* unknown08;         // +0x08
    void* unknown0C;         // +0x0C
    void* unknown10;         // +0x10
    void* owned;             // +0x14
};
TS3_X86_OFFSET(ScratchListNode, child, 0x00);
TS3_X86_OFFSET(ScratchListNode, next, 0x04);
TS3_X86_OFFSET(ScratchListNode, owned, 0x14);

// TS3W.exe 0x004DBBD0 (__cdecl, 18 bytes): DeleteFileW, normalized with neg/sbb/neg to
// exactly 0 or 1. Side-effecting.
int32_t DeleteFile004DBBD0(const wchar_t* lpFileName);

// TS3W.exe 0x004DCFF0 (__cdecl, 50 bytes): null/empty -> 0; an existing directory
// (attribute bit 0x10) -> 1; otherwise 0x004DCD40 (create the directory tree, cdecl).
// Side-effecting.
char EnsureDirectory004DCFF0(const wchar_t* lpFileName);

// TS3W.exe 0x004ED5E0 (__thiscall, 31 bytes): the path travels in ECX and is split by
// 0x004EC8D0 into three locals; the returned value is the third slot (the extension
// pointer), not 0x004EC8D0's own EAX.
const wchar_t* __fastcall GetSplitPathExtension004ED5E0(const wchar_t* path,
                                                        void* /*edx*/);

// TS3W.exe 0x004EDA20 (__thiscall, ret 4, 45 bytes): zero the first code unit, then
// 0x004ECE50(out, src, &zero, 5); returns out (the original keeps ECX at its entry
// value, which is the raw `src`; the rewrite calls the rewritten NormalizePath).
char16_t* __fastcall ConstructWideFromPath004EDA20(char16_t* out, void* /*edx*/,
                                                   const char16_t* src);

// TS3W.exe 0x004EDD80 (__cdecl, 50 bytes): when the string is non-empty and its last
// character is '\', '/' or ':' return 0; otherwise append '\' through 0x00412CC0
// (__thiscall, ret 4) and return 1. The append call is unconditional in the original,
// even for an empty string.
char EnsureTrailingSeparator004EDD80(ScratchWideString* string);

// TS3W.exe 0x004EF330 (__thiscall, 27 bytes): free begin when (cap - begin)&~1 > 2 and
// begin is non-null; returns begin, or the EAX of the free on the freeing path.
void* __fastcall DestroyString004EF330(ScratchWideString* self, void* /*edx*/);

// TS3W.exe 0x004FF6D0 (__cdecl, 97 bytes): fold every 16-bit character below 0x100
// through the table at 0x0114CB30 (unsigned index) and compare; a mismatch returns
// fold(a) - fold(b) of the zero-extended 16-bit folds, both NUL returns 0.
int32_t CompareWideFold004FF6D0(const char16_t* a, const char16_t* b);

// TS3W.exe 0x00594EC0 (__stdcall, ret 8, 57 bytes): walk 20-byte records over [first,
// last) and free *(record+4) when non-null and ((*(record+12) - it)&~1) > 2.
void __stdcall DestroyRecordArray00594EC0(uintptr_t first, uintptr_t last);

// TS3W.exe 0x0077F620 (__cdecl, 108 bytes): ask 0x007692C0 for the scripting bus,
// allocate a 24-byte message through 0x004E3F90 (cdecl, 6 args), build it with
// 0x0077F5A0, dispatch it at vtable +0x1C as 0xE777FB0D, then release the message at
// vtable +8 (which is also the tail-called return value).
int32_t NotifyHotInstall0077F620(void* src);

// TS3W.exe 0x00875950 (__thiscall, ret 0xC, 190 bytes): copy this+0x268 over the target
// through 0x004DBBF0, retrying every 500 ms while GetLastError() == 5; 0x004F9F40 fills
// the 64-bit "now" local (ret 4) and the deadline test is signed 64-bit. Returns 1 when
// the copy succeeded and the 0x008992C0 poll is clean; deletes the target otherwise.
char __fastcall CopyFileRetry00875950(void* self, void* /*edx*/, wchar_t* lpFileName,
                                      uint8_t* outError, int64_t* deadline);

// TS3W.exe 0x00987D50 (__thiscall, ret 0x10, 115 bytes): hash-map membership probe.
// Miss -> 0; hit only when the entry's +0x10 is non-null and appears in the 4-byte
// range [(list->next + 8) + 4, (list->next + 8) + 8) of the list at this+0xC.
bool __fastcall IsEntryInTable00987D50(void* self, void* /*edx*/, int32_t key,
                                       int32_t unusedA3, int32_t unusedA4,
                                       int32_t unusedA5);

// TS3W.exe 0x00987E40 (__thiscall, ret 4, 65 bytes): look the key up in the map at
// this+0x24 through 0x00D38B00; a miss (the 0x00D38B00 end sentinel) returns 0, a hit
// the entry's +4 field.
int32_t __fastcall LookupEntryField00987E40(void* self, void* /*edx*/, int32_t key);

// TS3W.exe 0x009886E0 (__thiscall, ret 4, 66 bytes): recursively free a node list,
// child first then the owned pointer then the node. For a null list the original
// returns whatever EAX held at entry; the rewrite returns 0 (no caller reads it).
int32_t __fastcall FreeNodeList009886E0(void* self, void* /*edx*/,
                                        ScratchListNode* node);

// TS3W.exe 0x009893C0 (__thiscall, ret 0, 193 bytes): when the list at this+0xC is
// non-empty or this[54] is set return 1; otherwise create this[56]'s directory, format
// the temp DB cache path through 0x0040E920, delete a stale file and pass it to
// 0x00987350 (which writes this[56]).
char __fastcall EnsureTempDbCache009893C0(void* self, void* /*edx*/);

// TS3W.exe 0x0098AE00 (__thiscall, 205 bytes): shrink the ten vector-ish spans of the
// content manager, drop the hash table through 0x0071F3B0 when present and reinitialize
// the +0x80 map. Returns the last vector-begin the original left in EAX (the map
// sentinel this+0x84 on the wipe path).
void* __fastcall ResetContentVectors0098AE00(int32_t* self, void* /*edx*/);

// TS3W.exe 0x0098C000 (__thiscall, ret 8, 188 bytes): format the world path of this+0xE0
// into `out` through 0x0040E920 (extensions selected by mode), retrying with an
// increasing index while the formatted file exists; then assign the string to itself
// through 0x00408E20 (__thiscall, ret 8) and return its result.
int32_t __fastcall BuildWorldPath0098C000(void* self, void* /*edx*/, ScratchWideString* out,
                                          int32_t mode);

// TS3W.exe 0x0098D430 (__thiscall, ret 4, 181 bytes): 0 when this[54] is set or the
// directory cannot be created; otherwise build the world path, hand it to 0x00987350
// and return the this[4] object + 8.
int32_t __fastcall GetWorldDir0098D430(int32_t* self, void* /*edx*/, int32_t a2);

// TS3W.exe 0x0098DD00 (__thiscall, ret 8, 180 bytes): find the content whose id is a3.
// An empty list is filled through 0x009893C0; a single-entry list (and the empty list)
// goes through 0x0098D430; otherwise the list is walked and each candidate is checked
// by file size (0x004DBC60) against this[55] before and after the 0x004DBC60 fixup.
int32_t __fastcall FindContent0098DD00(int32_t* self, void* /*edx*/, int32_t a2,
                                        int32_t a3);

// TS3W.exe 0x0098FD70 (__thiscall, ret 4, 174 bytes): commit the world package; the
// list must be non-empty. A single entry is resolved through 0x0098D430, several
// through 0x0098DD00; 0x007D8F00 copies the database records, 0x00D15C50 appends the
// id, 0x00985700 rebuilds and 0x00989490 shrinks the list. Returns 1, or 0 on failure.
char __fastcall CommitWorldPackage0098FD70(int32_t* self, void* /*edx*/, int32_t a2);

// TS3W.exe 0x0099C040 (__cdecl, 131 bytes): true when the 16-byte-element vector is
// empty, or every element is all-zero (0x00500A30 builds the zero key, 0x00500AC0
// compares), or the directory manager accepts it (0x0072E7F0) and rejects it
// (0x0072E8A0).
bool __cdecl VerifyContent0099C040(int32_t* vector);

// TS3W.exe 0x0099C140 (__cdecl, 25 bytes): null -> false, else 0x0099C040(object+0xBC).
bool __cdecl VerifyContentWrapper0099C140(int32_t object);

// TS3W.exe 0x009A90B0 (__cdecl, 67 bytes): 0x0072F1C0 returns the world resource
// service, 0x0058A900 (cdecl, 3 args) resolves the version from the 0x011808B0/B4/B8
// globals, the service's vtable +0x24 returns the installed-worlds path and 0x0040E920
// formats (out, 0x00FD83A4, path, version).
int32_t __cdecl BuildInstalledWorldPath009A90B0(int32_t out);

// TS3W.exe 0x009A9970 (__cdecl, 422 bytes): commit one world package file. Missing
// file or null -> 1; a2 == 0 -> delete the file, 4096 when it was removed else 1;
// otherwise build the target path under the installed-worlds directory and return 15
// when it exists, 4096 when the copy succeeded or 1 when the source was deleted.
int32_t __cdecl CommitWorldFile009A9970(const wchar_t* lpFileName, char a2);

// TS3W.exe 0x009B00D0 (__cdecl, 91 bytes): true when either 4-dword timestamp is
// all-zero (all four dwords), or a[3]/1000 >= b[3]/1000 with the unsigned 0x10624DD3
// division. `const void*` like the other raw 16-byte compares so the harness's compare
// hook adapter (const void*) can call it directly.
bool __cdecl CompareTimestamps009B00D0(const void* a, const void* b);

// TS3W.exe 0x009B0150 (__thiscall, ret 4, 17 bytes): 0x009B00D0(this, b).
bool __fastcall CompareTimestamps009B0150(const void* self, void* /*edx*/,
                                          const void* b);

// TS3W.exe 0x009C6800 (__thiscall, ret 8, 291 bytes): save a Sims3Pack into the
// content manager. Guards this[0xB4] == 1, this[0x114] != 0, this[0x110] != 0 and
// this[0x118] != 0, then 0x004DCFF0(src), item 0x009C5EE0(a2), 0x00408890/0x00409010
// build the save path, 0x004EDD80 appends a separator, 0x004F1F50 the item name,
// 0x004DB970 opens the FileStream (0x004DB4C0, ret 16) and 0x009C6760 writes it.
char __fastcall SaveSims3Pack009C6800(void* self, void* /*edx*/, int16_t a2,
                                      wchar_t* src);

// TS3W.exe 0x009CA860 (__cdecl, 76 bytes): for every 16-byte element of [a2, a3) call
// the __thiscall callback as callback(a4 + a7, element); then store {a4, a5, callback,
// a7} into dest and return dest.
int32_t* __cdecl ConstructCallback009CA860(int32_t* dest, int32_t a2, int32_t a3,
                                           int32_t a4, int32_t a5, void* callback,
                                           int32_t a7);

// TS3W.exe 0x009CAE60 (__thiscall, ret 4, 77 bytes): build the callback context
// {this[15], <uninitialized second word>, 0x0051B880, 0} and run 0x009CA860 over
// [a2[0], a2[1]). The original reads the second word from an uninitialized stack slot
// (EA bug); this rewrite zeroes it and returns the context local (a stack address,
// like the original; the only caller discards it).
void* __fastcall CallbackSetup009CAE60(int32_t* self, void* /*edx*/, int32_t* a2);

// TS3W.exe 0x00D15C50 (__cdecl, 82 bytes): append every dword of [a2, a3) to the
// vector-ish object a4 ({?, cur, end}); when cur >= end the original calls 0x008621C0
// and does not advance cur, and a null cur advances without storing. *dest = a4;
// returns dest.
int32_t* __cdecl AppendRecords00D15C50(int32_t* dest, int32_t a2, int32_t a3,
                                       int32_t a4);

}  // namespace ts3::scratch