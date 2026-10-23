// Cluster unit clu_4ff680_SACS (subsystem SACS): the twelve TS3W.exe functions that
// sort fixed-size records by a string key — case-insensitive compare (0x004FF680),
// guarded/unguarded insertion sorts, median-of-three, heap push/adjust/make/sort,
// Hoare-style partition, partial sort, the introsort driver and the sort entry
// (0x00982DF0). All twelve are __cdecl and take one extra trailing context argument
// that the unit never reads (every call site pushes one); it is kept for ABI
// compatibility and threaded through untouched.
#pragma once

#include <cstdint>

#include "ts3/layout.h"

namespace ts3::sacs {

// One sorted record: 0xE08 bytes; the sort key is the NUL-terminated string at
// +0x208. An empty key (first byte 0) sorts before everything.
struct SortRecord {
    uint8_t prefix[0x208];  // +0x000 .. +0x207, untouched by this unit
    char key[0xC00];        // +0x208 .. +0xE07, NUL-terminated name
};
TS3_X86_OFFSET(SortRecord, key, 0x208);
TS3_X86_SIZE(SortRecord, 0xE08);

// 0x004FF680: case-insensitive compare of two NUL-terminated strings over the fold
// table at 0x0114CB30 (signed char index): returns fold(*a) - fold(*b); equal bytes
// advance; a fold of 0 (both NULs) ends the string. cdecl, int result in EAX.
int CompareStrings(const char* a, const char* b);

// 0x009821F0: guarded insertion sort over [first, last): the saved element shifts
// left while empty-keyed or strictly smaller than its predecessor, bounded by
// `first`. Returns `last`.
char* InsertionSort(char* first, char* last, int context);

// 0x009822E0: unguarded tail insertion sort over [first, last): same order, but the
// shift loop has no `first` bound and skips the predecessor test when the saved key
// is empty (EA liveness bug — an empty key in the range walks off the front of the
// array). Every range handed to it must be non-empty-keyed and have a stop before
// `first` (an empty-keyed predecessor, or the introsort ordering invariant).
// Returns `last`.
char* InsertionSortTail(char* first, char* last, int context);

// 0x009823A0: median-of-three over element pointers a, b, c using the unit's
// empty-key order; returns one of the three element pointers.
char* MedianOfThree(char* a, char* b, char* c, int context);

// 0x009824A0: percolate `value` up from heap index `hole` towards `top` (parent =
// (hole-1) >> 1), moving parents down one step at a time, then place `value` in the
// hole it stopped at. Heap indexes, element stride.
void PushHeap(char* base, int top, int hole, char* value, int context);

// 0x00982730: sift the hole at index `hole` down through a heap of `count` records
// rooted at `base` (choosing the larger child per the empty-key order), then
// percolate `value` up from the bottom hole back towards `top`.
void AdjustHeap(char* base, int top, int count, int hole, char* value, int context);

// 0x009827F0: heapify [first, last) bottom-up.
void MakeHeap(char* first, char* last, int context);

// 0x00982890: heapsort: repeatedly swap the root with the last heap slot and re-sift.
// Returns the final element count (1 for a non-trivial range; n when n <= 1).
int SortHeap(char* first, char* last, int context);

// 0x00982980: Hoare-style partition of [first, last) around the VALUE copied from
// *pivot: left scan stops at an element >= the pivot value, right scan at one <= it,
// then the two are swapped and the scans restart. Returns the split element pointer.
// EA bug: an empty-keyed pivot makes the right scan decrement forever (pure hang).
char* Partition(char* first, char* last, char* pivot, int context);

// 0x00982A70: partial sort: heapify [first, middle), swap every element of
// (middle, end) that is below the heap root down into the heap, then SortHeap
// [first, middle). Returns SortHeap's count.
int PartialSort(char* first, char* middle, char* end, int context);

// 0x00982CB0: introsort driver: while the range holds more than 28 records and
// depth remains, median-of-three + partition and recurse into the right half (the
// left half is looped over); at <= 28 records or depth == 0 it falls back to
// PartialSort over [first, split). Returns the last computed element count.
int IntroSortPass(char* first, char* last, int depth, int context);

// 0x00982DF0: the sort entry point: IntroSortPass with depth 2*bitlen-2, then
// InsertionSort over the first 28 records plus InsertionSortTail over the rest
// (n > 28), or a single InsertionSort (n <= 28).
void SortRecords(char* first, char* last, int context);

}  // namespace ts3::sacs
