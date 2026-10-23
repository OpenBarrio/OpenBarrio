// Rewrite of cluster unit clu_405a50_Swarm (the Swarm data layer's byte-vector helper and
// its users: object copy/assign primitives, the locale/adapter-string builders and the
// Swarm catalogue stores). Every entry point keeps the original TS3W.exe ABI (thiscall
// with callee cleanup, cdecl, or the register-argument usercalls of 0x8780F0/0x878180);
// dependencies that are not rewritten go through ts3/layout.h.
//
// Object layouts here are expressed as raw byte regions plus ByteVector accesses at the
// original offsets: the objects are game structures whose non-vector bytes are opaque at
// this level, and x86-only TS3_X86_OFFSET/TS3_X86_SIZE checks pin the offsets the
// disassembly proves. The rewrite never converts a game address to a 32-bit integer.
#pragma once

#include <cstddef>
#include <cstdint>

#include "ts3/layout.h"

namespace ts3::swarm {

// ---------------------------------------------------------------------------------------
// The growable byte vector this unit is built around: begin/end/capacity, with the
// NUL-terminator byte living at *end (a terminator is stored at *end by every grow and by
// a shrinking assign). 0x00405790 (grow+append) and the game allocators behind it are not
// rewritten, so they are reached through the bridge.
struct ByteVector {
    uint8_t* begin;      // +0x00
    uint8_t* end;        // +0x04 logical end; *end is the 0 terminator
    uint8_t* capacity;   // +0x08 one past the last writable byte

    // 0x00405A50 (__thiscall, ret 8): assign [src,end) into the vector. When the request
    // does not fit, the already-copied bytes stay and 0x00405790 appends the rest. The
    // single-byte move that carries the terminator runs only when the end moves; a
    // shrinking assign moves the byte at the *old* end (not necessarily a NUL) to the new
    // end and then moves the end pointer by the same delta - kept verbatim.
    ByteVector* Assign(const char* src, const char* endPtr);

    // 0x004F3A40 (__thiscall, ret 8): Assign(src, src + bytes).
    ByteVector* AssignCount(const char* src, uint32_t bytes);
};
TS3_X86_OFFSET(ByteVector, begin, 0x00);
TS3_X86_OFFSET(ByteVector, end, 0x04);
TS3_X86_OFFSET(ByteVector, capacity, 0x08);
TS3_X86_SIZE(ByteVector, 0x0C);

// 0x00405790: grow the vector to hold an append of [src,end) and copy it. Not this unit.
uint8_t* __fastcall Grow405790(ByteVector* self, void*, const char* src, const char* end);

// 0x00405AD0: construct one 20-byte string element at `dst` from `src` (vector copy plus
// the allocator name at +0x0C). 0x00405AD0 is not in this unit; declared here for the
// element layout used by 0x0083EC40.
void* __fastcall ElementCtor405AD0(void* dst, void*, const void* src);

// 0x00408E20: the wide/other-allocator variant of Assign, used at +0x10 of the 0x3C-byte
// element. 0x0089DEB0: string assignment into the element's last field.
void* __fastcall Assign408E20(void* dst, void*, const char* src, const char* end);
void* __fastcall Assign89DEB0(void* dst, void*, const void* src);

// ---------------------------------------------------------------------------------------
// 0x00575CE0 (__thiscall, ret 4). Copy-construct the used 112-byte fields from `src`:
// the dword at +0x00, the three 16-byte blocks +0x10/+0x20/+0x30, the vector at +0x40 and
// the five dwords +0x50..+0x60. Bytes +0x04..+0x0F and +0x48..+0x4F are left untouched.
struct Obj575CE0 {
    uint8_t raw[0x70];
    Obj575CE0* AssignFrom(const Obj575CE0* src);
};

// 0x00575D50 (__cdecl): copy the 0x70-byte objects in [src, srcEnd) to `dst`; returns the
// one-past-end destination.
Obj575CE0* Copy575D50(const Obj575CE0* src, const Obj575CE0* srcEnd, Obj575CE0* dst);

// ---------------------------------------------------------------------------------------
// 0x00590080 (__cdecl): the 0x10-byte variant - copy the vector fields of [src, srcEnd)
// into `dst` elements, skipping the vector copy only when source and destination element
// are the same address. Returns the one-past-end destination.
uint8_t* Copy590080(const uint8_t* src, const uint8_t* srcEnd, uint8_t* dst);

// 0x00590B60 (__cdecl): copy the 0x3C-byte elements in [src, srcEnd) to `dst`: vector at
// +0x00 (Assign), the +0x10 vector through 0x00408E20, the three dwords +0x20..+0x28 and
// the +0x2C vector through 0x0089DEB0 (unconditionally). Returns the one-past-end destination.
uint8_t* Copy590B60(const uint8_t* src, const uint8_t* srcEnd, uint8_t* dst);

// ---------------------------------------------------------------------------------------
// 0x007B3BD0 (__thiscall, ret 4). Copy the 88-byte fields from `src`: byte +0x00, dwords
// +0x04..+0x0C, float +0x10, vector +0x14, dwords +0x24..+0x44, byte +0x48, dword +0x4C,
// floats +0x50/+0x54. The vector capacity at +0x1C is never copied.
struct Obj7B3BD0 {
    uint8_t raw[0x58];
    Obj7B3BD0* AssignFrom(const Obj7B3BD0* src);
};

// 0x007B3CD0 (__cdecl): copy the 0x58-byte objects in [src, srcEnd) to `dst`.
Obj7B3BD0* Copy7B3CD0(const Obj7B3BD0* src, const Obj7B3BD0* srcEnd, Obj7B3BD0* dst);

// 0x007F7360 (__thiscall, ret 4). Copy the 64-byte fields from `src`: dword +0x00, vector
// +0x04, dwords +0x18..+0x38 (the +0x38 store is emitted twice, the second one wins) and
// the byte +0x3C. Returns this.
struct Obj7F7360 {
    uint8_t raw[0x40];
    Obj7F7360* AssignFrom(const Obj7F7360* src);
};

// 0x0083BD70 (__cdecl, ret 8 in the original - it is a leaf that returns its dst): reverse
// copy of the 20-byte elements from srcEnd down to src, writing dstEnd-20 downwards; the
// vector at +0x00 is assigned only when source and destination differ, and the dword at
// +0x10 is stored unconditionally *before* the loop's exit test. Returns dst - count.
struct Obj20 {
    uint8_t raw[0x14];
    ByteVector* Vector() { return reinterpret_cast<ByteVector*>(raw); }
};
Obj20* CopyBack83BD70(const Obj20* src, const Obj20* srcEnd, Obj20* dstEnd);

// 0x00579D80 (__cdecl, 2 args, returns char). Normalise the short (<= 5 byte) string in
// `in` through the 0x0114F6B0 translation table and assign the result into `out`; when the
// source and destination are the same object the assign is skipped and the parse still
// runs. Returns 0 for strings longer than 5 bytes or a length/table mismatch, 1 on
// success. (When in == out the assign at the top is skipped, and the parse then reads the
// same vector it will overwrite - the original order is preserved.)
uint8_t NormalizeString579D80(const ByteVector* in, ByteVector* out);

// 0x0058FAD0 (__thiscall, ret 4). Sequence description update: notify the owner (vtable
// slot 33), read the argument count from the Scripting object at +0x04, and either report
// "wrong arg count" through the owner (slot 20) or clear the +0x30 vector of the store at
// +0x30 and append the matching "%x" entries (slots 26). Returns the last result.
struct Obj58FAD0 {
    uint8_t raw[0x34];
    int32_t Update(int32_t a0);
};
TS3_X86_SIZE(Obj58FAD0, 0x34);

// TS3W.exe 0x007D9D40: Fill `out` with the code-version string (__cdecl, char return).
// the compiled-in date against "Local" (the original compares two literals, so that test
// can never be equal - the branch is dead, an EA bug kept verbatim), else the wide
// "code_version.txt" name through a fixed 260-wchar stack string, else "Unknown".
// EA's overflow fault in the original wide append helper is retained.
char CheckVersion7D9D40(ByteVector* out);

// 0x0083EC40 (__thiscall, ret 8): insert one 20-byte element at `pos` into the vector
// {begin,end,capacity} of string elements. Non-full: shift [pos,end) up by one through
// 0x0083BD70, copy the value in. Full: allocate the doubled buffer through 0x00912A90 /
// 0x00912B60 (the copy and destroy primitives) and grow to 20 * max(2*n, 1) bytes.
struct StrVector {
    uint8_t* begin;     // +0x00
    uint8_t* end;       // +0x04
    uint8_t* capacity;  // +0x08
    uintptr_t Insert(Obj20* pos, Obj20* value);
};
TS3_X86_OFFSET(StrVector, begin, 0x00);
TS3_X86_OFFSET(StrVector, end, 0x04);
TS3_X86_OFFSET(StrVector, capacity, 0x08);
TS3_X86_SIZE(StrVector, 0x0C);

// ---------------------------------------------------------------------------------------
// 0x00873A60 (__thiscall, ret 4). Copy-construct from `src`: the pointer at +0x00, the
// vectors at +0x04 and +0x14, the refcounted pointer at +0x24 (addref new, release old -
// the release calls the object's virtual deleting destructor when the count reaches 0) and
// the three pointers at +0x28/+0x2C/+0x30. Returns this.
struct Obj873A60 {
    uint8_t raw[0x34];
    Obj873A60* AssignFrom(const Obj873A60* src);
};

// 0x008780F0 (register argument a1 in EDI, ULONG result): walk the adapter list from
// GetAdaptersInfo and append " %02x-.." for every adapter with a >= 6 byte address; returns
// the last GetAdaptersInfo code or the last append result. Private to 0x00878180.
uint32_t ReportAdapters8780F0(void* a1);

// 0x00878180 (register argument a1 in EAX, ULONG result). Reset the vector to
// "UniqueToken", then if 0x0058E3F0 returns an object append the five strings its +0x174
// sub-object yields, and finally report the adapters. The ABI passes the ByteVector in EAX
// (a game-side register usercall), so the C++ shape takes it as a normal pointer.
uint32_t BuildUniqueToken878180(ByteVector* out);

// ---------------------------------------------------------------------------------------
// 0x00881C70 (__thiscall, ret 4). Assign `src` into the +0x1C vector while the byte +0x08
// is 0; a null `src` clears the vector (the original then reads *NULL because its source
// check is `if (p || *p)` - kept, see the implementation).
struct Obj881C70 {
    uint8_t raw[0x28];
    void Assign88_1C(const char* src);
};

// 0x00881CC0 (__thiscall, ret 4). Assign `src` into +0x0C, then re-derive the two key
// fields at +0x2C/+0x3C from it (wide-string conversion, map clears), copy +0x0C into
// +0x3C and replace '-' with '_' in the copy. Guarded by the byte at +0x08.
struct Obj881CC0 {
    uint8_t raw[0x58];
    void Assign88_2C(const char* src);
};

// 0x00881DE0 (__thiscall, ret 4): convert the wide/char source at `a2` through 0x004F38C0
// and assign it through Assign88_2C; the conversion buffer is released when larger than one
// byte. Returns the buffer base (or the free's result).
struct Obj881DE0 {
    uint8_t raw[0x58];
    void* AssignWide88_2C(const char* a2);
};

// 0x00881E30 (__thiscall, ret 4): same conversion, assigning through Assign88_1C.
struct Obj881E30 {
    uint8_t raw[0x28];
    void* AssignWide88_1C(const char* a2);
};

// 0x00881EB0 (__thiscall, ret 0, returns 1): fill the locale fields from "l"/"en-US" and
// "US"/the live locale API; the same object as Obj881CC0.
struct Obj881EB0 {
    uint8_t raw[0x58];
    uint8_t FillLocale();
};

// ---------------------------------------------------------------------------------------
// 0x008841D0 (__thiscall, ret 4): assign `arg` (a ByteVector*) into this+0x1C; returns
// `arg` when it already is this+0x1C, else the assigned vector's address (the original
// return value is path-dependent).
struct Obj8841D0 {
    uint8_t raw[0x28];
    void* Assign88_41D0(ByteVector* arg);
};

// 0x008842D0 (__thiscall, ret 0xC): assign the three ByteVectors into this+0x38/+0x48/
// +0x58; same path-dependent return on the third one.
struct Obj8842D0 {
    uint8_t raw[0x64];
    void* Assign88_42D0(ByteVector* a2, ByteVector* a3, ByteVector* a4);
};

// ---------------------------------------------------------------------------------------
// 0x008E2060 (__thiscall, ret 4). Read "Swarm/FXString" into the +0xDC vector of the
// object at this+0x0C, set the tick flag (+0x14 bit 4) when the "tick" property exists,
// then the same for the "death" property into +0xEC plus the inherit flag (+0x14 bit 5).
// Returns the last 0x004053F0/0x00405240 result.
struct Obj8E2060 {
    uint8_t raw[0x10];
    int32_t* Target() { return *reinterpret_cast<int32_t**>(raw + 0x0C); }
    uint32_t Init();
};

// ---------------------------------------------------------------------------------------
// 0x0090FEE0 (__thiscall, ret 4). Read the "Swarm/SequenceDesciption/mEffect" property,
// resolve the two float arguments through the owner's vtable slot 36 (+0x90) and append a
// 24-byte {float,float} element to the vector at (this+0x0C)+0x0C. Returns the temporary
// buffer base (or its free result).
struct Obj90FEE0 {
    uint8_t raw[0x10];
    int32_t* Target() { return *reinterpret_cast<int32_t**>(raw + 0x0C); }
    void* Init(int32_t a0);
};

// ---------------------------------------------------------------------------------------
// 0x00912FE0 (__thiscall, ret 0xC, returns char). Look up the 1-byte key `a2` in the map
// at this+0x70 through 0x00C2ABA0 (the header's third dword is the index used to find the
// end node); `a3` is dead in the original. On a hit copy the node's +0x08 vector into `a4`
// and return 1, else 0.
struct Obj912FE0 {
    uint8_t raw[0x7C];
    uint8_t Lookup912FE0(char a2, int32_t a3, ByteVector* a4);
};

// ---------------------------------------------------------------------------------------
// 0x00920F10 (__thiscall, ret 4). Read "Swarm/FXString" into this+0x3C, notify the owner
// (vtable +132), allocate a 40-byte "Swarm/MaterialDesc" record (vtable 0x01041D0C,
// refcount 1), install it at +0x38 releasing the old one, and store 0x00918CA0(14, ...)
// into the record's +0x08/+0x0C. Returns that value.
struct Obj920F10 {
    uint8_t raw[0x48];
    int32_t Init(uint32_t a0);
};

// ---------------------------------------------------------------------------------------
// 0x00943B40 (__cdecl): copy 0x14-byte elements in [src, srcEnd) to `dst`: the dword at
// +0x00 and the vector at +0x04. Returns the one-past-end destination.
uint8_t* Copy943B40(const uint8_t* src, const uint8_t* srcEnd, uint8_t* dst);

// 0x00944160 (__thiscall, ret 8): find the element whose word0 masked with 0x3FFFFFFF
// equals `key` in the vector this+0x20..+0x24; on a hit clear its vector (assign an empty
// string) and remember whether word0 had bit 30. Then: when this+0x4C is nonzero and the
// caller's key has bit 30 clear, delegate to 0x00861230; otherwise 0x00943FB0(key, 1) and
// if the element had bit 30 also 0x00861230. Returns that result.
struct Obj944160 {
    uint8_t raw[0x50];
    uint32_t Touch944160(uint32_t key, uint8_t a3);
};

// ---------------------------------------------------------------------------------------
// 0x009CF440 (__thiscall, ret 4). Fill this+0x1C/+0x2C/+0x3C/+0x4C/+0x6C with the store,
// list, version and key names selected by the 32-bit store id at this+0x18, using the
// interning/assign helper 0x00405B30 or Assign depending on the id. Unlisted ids write
// +0x6C = "0.0".
struct Obj9CF440 {
    uint8_t raw[0x78];
    ByteVector* SelectStoreFields();
};

// 0x009CFE40 (__thiscall, ret 4). Assign the three vectors at +0x00/+0x10/+0x20 of `src`
// into this. Returns this.
struct Obj9CFE40 {
    uint8_t raw[0x30];
    Obj9CFE40* AssignFrom(const Obj9CFE40* src);
};

// ---------------------------------------------------------------------------------------
// 0x009F7180 (__thiscall, ret 4). Ask the object at this+0x28 for its string (vtable slot
// 6) and append it to the global vector (+8 of the global at 0x011D9730, or address 8 when
// the global is null). Returns the string pointer when there is one.
struct Obj9F7180 {
    uint8_t raw[0x2C];
    const char* AppendName9F7180();
};

// 0x009F76C0 (__thiscall, ret 4). Copy the six vectors and the two scalars from `src` and
// build the four derived upload/download URLs at +0x70/+0x80/+0x90/+0xA0 with the
// 0x00590900 formatted-string assignment. Returns the last URL's buffer address.
struct Obj9F76C0 {
    uint8_t raw[0xB0];
    void* CopyAndBuildUrls(const uint8_t* src);
};

// 0x009F77C0 (__thiscall, ret 4, returns bool). One-shot init: when the flag at +0x18 is
// clear, build the URLs on the object at +0x1C, clear the +0x08 vector, set the flag and
// return whether it is now set (always 1 on the first path). Returns 0 when already set.
struct Obj9F77C0 {
    uint8_t raw[0x20];
    bool InitOnce9F77C0(const uint8_t* src);
};

// ---------------------------------------------------------------------------------------
// 0x00F7CCDC (__stdcall import thunk): GetAdaptersInfo from IPHLPAPI.DLL. The original is
// the jump through the import table; on x86 this forwards to that same thunk so the rewrite
// keeps the exact system call, on x64 there is no game image (aborts).
uint32_t __stdcall GetAdaptersInfoThunk(void* adapterInfo, uint32_t* sizePointer);

}  // namespace ts3::swarm