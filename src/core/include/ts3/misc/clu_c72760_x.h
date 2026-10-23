// Cluster unit clu_c72760_x (subsystem misc): the eighteen TS3W.exe functions that
// sort fixed-size FLOAT-keyed records with heapsort/introsort, plus two standalone
// entry points (0x00C7D700 vector copy, 0x00C7F140 ref-element cursor copy). Two record
// families, both keyed by the float the comiss instructions compare:
//   - Rec48: stride 48, key at +0x00. A record MOVE touches only [0,4) and [16,48);
//     bytes +0x04..+0x0F never move (they stay with the SLOT).
//   - Rec80: stride 80, key at +0x44. A record MOVE touches [0,76); bytes
//     +0x4C..+0x4F never move.
// Every compare below is written exactly as the original's comiss branch: `a > b`
// for ja/jg targets and `!(a > b)` for jbe ones, so NaNs (unordered -> jbe taken)
// pick the same arms. ABIs match the originals: the fourteen sort helpers are __cdecl;
// the introsort entries are __cdecl with three stack arguments (call sites push a
// fourth the entries never read); 0x00C7D700 is __thiscall with one stack argument
// (`ret 4`, implemented as a member function); 0x00C7F140 is __cdecl with four.
// EA quirks kept on purpose (see the .cpp file comment): the introsort entries return
// a garbage EAX once a partition loop ends, SortHeap80 returns 48 instead of 1 for
// n >= 2, and MakeHeap80/48 return garbage for n < 2.
#pragma once

#include <cstdint>

#include "ts3/layout.h"

namespace ts3::misc {

// One 48-byte record of the 48-stride family: float key at +0x00, a 12-byte slot
// region the sorts never copy, then the 32-byte payload that moves with the record.
struct Rec48 {
    float key;         // +0x00: the comiss operand
    uint8_t slot[12];  // +0x04..+0x0F: never moved (stays with the array slot)
    uint8_t tail[32];  // +0x10..+0x2F: moved with the record
};
TS3_X86_OFFSET(Rec48, slot, 0x04);
TS3_X86_OFFSET(Rec48, tail, 0x10);
TS3_X86_SIZE(Rec48, 48);

// One 80-byte record of the 80-stride family: the 68-byte head, the float key at
// +0x44, two words, and a 4-byte slot tail that never moves.
struct Rec80 {
    uint8_t head[0x44];  // +0x00..+0x43: moved (includes the +0x40 companion float)
    float key;           // +0x44: the comiss operand
    uint16_t word48;     // +0x48: moved
    uint16_t word4A;     // +0x4A: moved
    uint8_t slot[4];     // +0x4C..+0x4F: never moved
};
TS3_X86_OFFSET(Rec80, key, 0x44);
TS3_X86_OFFSET(Rec80, word48, 0x48);
TS3_X86_OFFSET(Rec80, word4A, 0x4A);
TS3_X86_SIZE(Rec80, 80);

// The pointer vector 0x00C7D700 re-creates through the game's 0x0069DB60: three
// pointer fields, then a context dword the bridge copies verbatim. eastl layout.
struct KeyVector {
    uint32_t* begin;
    uint32_t* end;
    uint32_t* capacity;
    uint32_t context;  // copied through 0x0069DB60 before the allocation happens
};
TS3_X86_OFFSET(KeyVector, end, 0x04);
TS3_X86_OFFSET(KeyVector, capacity, 0x08);
TS3_X86_OFFSET(KeyVector, context, 0x0C);
TS3_X86_SIZE(KeyVector, 0x10);

// ---------------------------------------------------------------------------
// 48-stride family
// ---------------------------------------------------------------------------

// 0x00C72760: percolate `value` up from heap index `hole` towards `top` (parent =
// (hole-1)>>1, signed); while `hole > top` and `value.key > parent.key`, the parent
// moves down into the hole, then `value` is placed in the hole it stopped at.
// Returns the address of that hole (inside `base`). cdecl.
char* PushUp48(char* base, int top, int hole, const char* value);

// 0x00C74510: sift the hole at index `hole` down a heap of `count` records (children
// 2h+1/2h+2; the left child wins only when strictly greater than the right), take the
// lone left child at count-1 when right == count, then percolate `value` up from
// wherever the hole ended (0x00C72760). Returns that call's hole address. cdecl.
char* SiftDown48(char* base, int top, int count, int hole, const char* value);

// 0x00C756E0: heapify [first, last) bottom-up over every internal node. Returns the
// last SiftDown's hole address; for n < 2 the garbage EAX the original leaves
// (low32(0x2AAAAAAB * byteSpan)) as a pointer (EA quirk, kept). cdecl.
char* MakeHeap48(char* first, char* last);

// 0x00C75770: heapsort an existing heap: swap the root with the last heap slot and
// re-sift, while more than one element remains. Returns 1 for n >= 2, n for n <= 1.
// cdecl.
int SortHeap48(char* first, char* last);

// 0x00C77B30: Hoare-style partition of [first, last) around the VALUE read once from
// *pivot: left scan advances while pivot.key > element.key, right scan retreats while
// element.key > pivot.key, then (i < j, UNSIGNED compare) the two records swap and the
// scans restart; returns the split address i. EA bug kept: both scans are unbounded,
// so a pivot value that appears nowhere in the range runs off the array (the game's
// median-of-three always passes an in-range element). cdecl.
char* Partition48(char* first, char* last, const char* pivot);

// 0x00C77BA0: partial sort: MakeHeap48(first, middle), then every element of
// [middle, end) whose key is strictly below the heap root swaps with the root and the
// root re-sifts (count = (middle-first)/48), then SortHeap48(first, middle).
// Returns SortHeap48's count. cdecl.
int PartialSort48(char* first, char* middle, char* end);

// 0x00C78350: introsort driver ENTRY. Ranges of <= 28 records, or depth <= 0, fall
// out to the tail below (depth == 0 heapsorts the range through 0x00C77BA0, depth < 0
// and depth != 0 just return); otherwise median-of-three + partition, recurse into the
// right half at depth-1 and loop over the left half. Returns the element count for
// n <= 28, the partial sort's count when depth == 0, and for a partition loop that
// ran out the GARBAGE EAX low32(0x2AAAAAAB * (split-first)) (EA quirk, kept). cdecl,
// three stack arguments (a fourth is pushed by call sites and never read).
int IntroSort48(char* first, char* last, int depth);

// ---------------------------------------------------------------------------
// 80-stride family
// ---------------------------------------------------------------------------

// 0x00C72850: PushUp48 over 80-byte records (key at +0x44, whole 76-byte moves).
char* PushUp80(char* base, int top, int hole, const char* value);

// 0x00C72920: swap the first 76 bytes of two 80-byte records (bytes +0x4C..+0x4F stay
// with their slots); returns `a`. cdecl.
char* SwapRecords80(char* a, char* b);

// 0x00C745C0: SiftDown48 over 80-byte records.
char* SiftDown80(char* base, int top, int count, int hole, const char* value);

// 0x00C746C0
// count = (last-first)/80 - 1, then SiftDown80(base, 0, count, 0, saved). Returns that
// call's hole address. Reads [first-80] when last == first (EA: only SortHeap80 calls
// it, always with >= 2 records). cdecl.
char* PopRoot80(char* first, char* last);

// 0x00C75820: MakeHeap48 over 80-byte records; garbage EAX for n < 2 (kept).
char* MakeHeap80(char* first, char* last);

// 0x00C758D0: heapsort an existing 80-heap. Returns 48 for n >= 2 (the original's
// loop leaves low32(0x66666667 * 80) in EAX, the quotient lives in ECX) and n for
// n <= 1 (EA quirk, kept). cdecl.
int SortHeap80(char* first, char* last);

// 0x00C77C40: Partition48 over 80-byte records; swaps run through 0x00C72920.
char* Partition80(char* first, char* last, const char* pivot);

// 0x00C77CA0: PartialSort48 over 80-byte records; the element counter is
// ceil((end-middle)/80), so a non-multiple-of-80 range walks past `end` (EA bug, kept
// — the game always passes aligned ranges). Returns SortHeap80's count. cdecl.
int PartialSort80(char* first, char* middle, char* end);

// 0x00C78430: introsort driver ENTRY over 80-byte records, same shape as
// 0x00C78350; garbage EAX after the loop is low32(0x66666667 * (split-first)), and
// the depth==0 tail returns SortHeap80's 48/n. cdecl, three stack arguments.
int IntroSort80(char* first, char* last, int depth);

// ---------------------------------------------------------------------------
// Standalone entry points
// ---------------------------------------------------------------------------

// 0x00C7D700 (ENTRY, __thiscall, `ret 4`): copies byte00/byte01 and the 0x40-byte
// body (+0x10..+0x4F) from `src`, then re-creates this->vector from src's element
// COUNT through the game's 0x0069DB60 (which copies the context dword and allocates)
// and memmoves src's elements into it; dst.end becomes begin + (span>>2)*4. Bytes
// +0x02..+0x0F are NOT copied (EA: the destination keeps its own). Returns `this`.
// EA bugs kept: a source span of 1..3 bytes makes 0x0069DB60 allocate 0 bytes and the
// memmove write past it (the game always passes multiples of 4). On x64 builds the
// bridge aborts (no game code to call).
struct VecCopyObject {
    VecCopyObject* CopyFrom(const VecCopyObject* src);

    uint8_t byte00;           // +0x00: copied
    uint8_t byte01;           // +0x01: copied
    uint8_t preserved[14];    // +0x02..+0x0F: NOT copied by CopyFrom
    uint8_t body[0x40];       // +0x10..+0x4F: copied as four 16-byte loads
    KeyVector vector;         // +0x50: rebuilt from src->vector
    // the real game object extends past this; 0x60 is the span CopyFrom touches
};
TS3_X86_OFFSET(VecCopyObject, byte01, 0x01);
TS3_X86_OFFSET(VecCopyObject, preserved, 0x02);
TS3_X86_OFFSET(VecCopyObject, body, 0x10);
TS3_X86_OFFSET(VecCopyObject, vector, 0x50);
TS3_X86_SIZE(VecCopyObject, 0x60);

// 0x00C7F140 (ENTRY): `*cursor = init`, then for each 16-byte element of
// [begin, end) — walked while begin != end — copy {p0, p1, data, data} into the cell
// at *cursor when *cursor != 0, calling virtual slot 0 of the game object p0/p1
// (through the bridge) when non-null, and advance *cursor and begin by 16 either way.
// The element fields are dwords (two object pointers + two data words), exactly as
// the original moves them. Returns `cursor`. cdecl.
char* CopyRefElements(char* cursor, const char* begin, const char* end, uint32_t init);

}  // namespace ts3::misc
