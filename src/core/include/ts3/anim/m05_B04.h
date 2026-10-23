#pragma once
#include "ts3/anim/m05_layouts.h"
namespace ts3::anim {
// Comparator payload consists of five integers, not pointers.
struct B04Record { int32_t key, id, payload, parent, tag; };
static_assert(sizeof(B04Record)==20);
// TS3W.exe 0x0054F400: Stable bounded insertion sort with hierarchy ordering.
void B04Insertion(B04Record*, B04Record*);
// TS3W.exe 0x0054F4E0: Unguarded insertion sort, requiring a preceding sentinel.
void B04Unguarded(B04Record*, B04Record*);
// TS3W.exe 0x0054F5A0: Push a saved record up the heap and return its destination.
B04Record* B04Push(B04Record*, int32_t, int32_t, const B04Record*);
// TS3W.exe 0x0054F6B0: Adjust a heap hole and return the saved record destination.
B04Record* B04Adjust(B04Record*, int32_t, int32_t, int32_t, const B04Record*);
// TS3W.exe 0x0054F7B0: Pop the heap root to the final record.
B04Record* B04Pop(B04Record*, B04Record*);
// TS3W.exe 0x00550020: Make a heap; short ranges return a multiply residue.
uintptr_t B04MakeHeap(B04Record*, B04Record*);
// TS3W.exe 0x005500A0: Sort a heap; nontrivial ranges return the multiply residue 12.
int32_t B04SortHeap(B04Record*, B04Record*);
// TS3W.exe 0x005500F0: Partition using a snapshot of the pivot's comparison fields.
B04Record* B04Partition(B04Record*, B04Record*, const B04Record*);
// TS3W.exe 0x00550210: Partial heap sort preserving the replaced root in the tail.
int32_t B04PartialSort(B04Record*, B04Record*, B04Record*);
// TS3W.exe 0x00550320: Median-of-three introsort partitioning, leaving small ranges unsorted.
int32_t B04Intro(B04Record*, B04Record*, int32_t);
// TS3W.exe 0x00550580: Sort evaluator records with a 28-element insertion threshold.
void B04Sort(B04Record*, B04Record*);
struct B04Node { B04Node* next; B04Node* previous; uint32_t key, value; };
struct B04List { unsigned char opaque[0x24]; B04Node* first; B04Node* last; B04Node* free; };
TS3_X86_OFFSET(B04List,first,0x24);
TS3_X86_OFFSET(B04List,free,0x2c);
// TS3W.exe 0x00554DB0: Find the descending unsigned upper-bound list insertion position.
B04Node** B04UpperBound(B04Node**, B04Node*, B04Node*, const uint32_t*);
// TS3W.exe 0x00554E20: Return the class identifier.
uint32_t B04ClassId(void*);
// TS3W.exe 0x00554E30: Return this for the derived or base identifier, otherwise null.
void* B04Query(void*, uint32_t);
// TS3W.exe 0x00554EE0: Consume a free-list node and insert after equal keys; null pool faults.
B04Node* B04Insert(B04List*, uint32_t, uint32_t);
}
namespace ts3::anim {
// ABI-only adapters: EDX is a dummy, matching x86 thiscall register/stack cleanup.
uint32_t __fastcall B04ClassHook(void*, void*);
void* __fastcall B04QueryHook(void*, void*, uint32_t);
B04Node* __fastcall B04InsertHook(B04List*, void*, uint32_t, uint32_t);
}
