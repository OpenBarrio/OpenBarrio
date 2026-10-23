#pragma once

#include <cstdint>

#include "ts3/layout.h"

namespace ts3::world {

// TS3W.exe 0x004EC520: compares the wide-character ranges [firstBegin, firstEnd) and
// [secondBegin, secondEnd) character by character, treating both '\' and the mode
// dependent separator ('/' for modes 3..6, '\' otherwise) as equal to each other via a
// 0xFFFF placeholder. For modes 2, 3, 5 and 6 characters below 0x100 are folded through
// the game's case table (identity except 'A'..'Z' -> 'a'..'z'). Returns 0 when the ranges
// compare equal, else the zero-extended 16-bit difference second - first of the first
// mismatching pair (a placeholder 0xFFFF counts as 65535). The range length is
// max(len1, len2) and the comparison reads both ranges for that many characters, so
// unequal-length ranges read to the longer one's end. EA quirk kept: end pointers are not
// clamped to a NUL, the caller supplies them.
int32_t ComparePathRanges(const wchar_t* firstBegin, const wchar_t* firstEnd,
                          const wchar_t* secondBegin, const wchar_t* secondEnd, int32_t mode);

// TS3W.exe 0x004EC8D0: splits a wide path into caller-supplied slots. *outBase receives
// the path start or, for a drive prefix ("X:") / UNC root ("\\\\host"), the first
// character after it; *outFile receives the first character after the last separator
// ('\', '/' or ':') seen anywhere in the path, or *outBase when there is none; *outExt
// receives the last '.' after the last separator, else (UNC: from the end of the scanned
// host) the string end. Scan quirks kept: a dot is forgotten at every separator, the UNC
// host scan stops at the second backslash, '/', ':' or NUL, and the returned EAX is the
// outExt slot address on the dot path but the pre-scan end pointer otherwise.
const wchar_t* SplitPath(const wchar_t* path, const wchar_t** outBase,
                         const wchar_t** outFile, const wchar_t** outExt);

// TS3W.exe 0x004ED740 (originally __thiscall with the path in ECX): copies the file-name
// part of path (from the last separator to the last dot) into dst, NUL-terminating it,
// when its length is below dstCount characters, and returns that length either way. EA
// quirk kept: when the length does not fit nothing is copied and dst is untouched.
uint32_t CopyFileNamePart(const wchar_t* path, wchar_t* dst, uint32_t dstCount);

// TS3W.exe 0x004ED7A0 (originally __thiscall with the path in ECX): copies the part of
// path before its file name (the drive/UNC/directory prefix) into dst, NUL-terminating
// it, when its length is below dstCount characters, and returns that length either way.
// The same "does not fit -> nothing copied" quirk as 0x4ED740.
uint32_t CopyDirectoryPart(const wchar_t* path, wchar_t* dst, uint32_t dstCount);

// TS3W.exe 0x004ED840: splits both paths at 0x4EC8D0 into the five pointers
// [whole, base, file, ext, 0] and compares the range selected by [from, to) through
// 0x4EC520. With foldFlag == 0 the two pointers selected by from/to are the range ends
// (to == 4 passes the zeroed fifth slot); otherwise only from is used and the comparison
// of the single part [from, from+1) is repeated until it reports a difference. EA quirk
// kept: that retry loop never terminates when the parts compare equal; and an index of 4
// passes a null range end, which makes 0x4EC520 read past the strings.
int32_t ComparePathParts(int32_t from, int32_t to, const wchar_t* lhs, const wchar_t* rhs,
                         int32_t mode, char foldFlag);

// TS3W.exe 0x004ED940 (originally __thiscall with lhs in ECX): compares two paths with
// 0x4ED840(0, 4, ...) -- the whole-path form, i.e. ranges [path, null). foldFlag selects
// the repeat-until-different loop of 0x4ED840 and mode the 0x4EC520 comparison mode.
int32_t CompareWorldPaths(const wchar_t* lhs, const wchar_t* rhs, char foldFlag, int32_t mode);

// TS3W.exe 0x004F22E0: copies the element pointers [begin, end) to consecutive slots of a
// client array whose cursor lives in *cursor, starting at first, adding a reference to
// each non-null element through vtable slot 1. *cursor is advanced by four bytes per
// element even when it was null (nothing is written then), so a null first acts as a
// counting pass. Returns the cursor variable's address.
void** CopyRefElements(void** cursor, void** begin, void** end, void* first);

// TS3W.exe 0x006F5970 (__stdcall): calls vtable slot 2 (release) on every non-null element
// of [begin, end) (pointer-compared, end < begin runs away), and returns the EAX of the
// last such call -- there is no defined value when every element is null.
int32_t ReleaseRefElements(void** begin, void** end);

// TS3W.exe 0x0069DC20 (thiscall, ret 4): copy constructor of a 0x38-byte object. Copies
// the 0x30-byte header, rebuilds the vector at +0x30 through the game's allocation helper
// 0x0069DB60 with the source's context dword at +0x3C, then copies the source element
// pointers [src+0x30, src+0x34) into that vector with 0x4F22E0 and stores the resulting
// end cursor at +0x34. Returns this.
struct WorldResourceCopy {
    uint8_t header[0x30];
    void** elements;      // +0x30, begin of the pointer vector
    void** elementsEnd;   // +0x34, end of the pointer vector

    WorldResourceCopy* CopyFrom(const void* source);
};

// TS3W.exe 0x00A7EB80 (thiscall, ret 8): inserts the element pointed to by value before
// pos in a pointer vector. With spare capacity the tail is shifted right through the
// game's 0x006B05F0 after duplicating the last element, then the value is assigned with
// add/release (vtable slots 1 and 2). EA quirks kept: when value points inside the vector
// at or after pos it is advanced by one element; the growth path (end == capacity) sizes
// the new capacity to max(2*count, 1) but rebuilds through the counting form of 0x4F22E0
// with a null base, i.e. it copies at byte offsets 4*n instead of an allocated buffer, and
// then stores begin = null with end = 4*elementCount, capacity = 4*max(2*count, 1).
struct WorldRefVector {
    void** begin;         // +0x00
    void** end;           // +0x04
    void** capacity;      // +0x08

    int32_t Insert(void** pos, void** value);
};

// TS3W.exe 0x00C71560 (thiscall, ret 0x10): builds an object name string. The four
// non-null arguments are the source path, a part appended to the output string, a part
// appended behind the generated suffix, and the output string object. The 64-bit value at
// this+0x48 is formatted as hex into the output as well, and this+0x50 / +0x54 hold an
// optional replacement path part. The body drives the game's wide-string helpers
// 0x408E20 (assign range), 0x408B90 (append range), 0x4EDD80, 0x4F1F50 (append literal),
// 0x4AEFE0 (sprintf-like) and 0x7304F0 (width-limited form), freeing the temporary heap
// buffers through 0x004E3F70. Returns nothing; the original leaves EAX as the last
// callee's value.
struct WorldObjNameSource {
    uint8_t pad00[0x48];
    uint32_t valueLow;    // +0x48
    uint32_t valueHigh;   // +0x4C
    void* partBegin;      // +0x50, optional path part (begin/end); empty means use the split result
    void* partEnd;        // +0x54

    void BuildObjName(const wchar_t* path, const wchar_t* appendPart, const wchar_t* suffixPart,
                      void* outString);
};

// Layout of the game's wide string objects passed to 0x408E20 / 0x408B90 / 0x4F1F50 /
// 0x7304F0 (pointer-sized fields; on x86 TS3W.exe has the four dwords below).
struct WorldWideString {
    void* begin;          // +0x00
    void* end;            // +0x04
    void* capacity;       // +0x08
    void* allocator;      // +0x0C
};

}  // namespace ts3::world