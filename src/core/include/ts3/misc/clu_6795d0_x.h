#pragma once
#include "ts3/layout.h"

namespace ts3::misc::clu6795 {
struct Point { float x, y; };
struct Points { Point *begin, *end, *capacity; uint32_t allocator[3]; };
struct Record { float x, y; double payload; };
using Less = uint8_t (__cdecl*)(const Record*, const Record*);
struct Item { uint8_t prefix[44]; uint32_t flags; uint8_t middle[12]; uint32_t index; };
struct Holder { Item* value; };
struct Adapter { void* prefix[2]; Holder* holder; };
// Intrusive links point eight bytes into each child, not to the child itself.
struct Tree { void* prefix[7]; Item** begin; Item** end; };
TS3_X86_SIZE(Point, 8);
TS3_X86_SIZE(Record, 16);
TS3_X86_SIZE(Points, 24);
TS3_X86_OFFSET(Points, end, 4);
TS3_X86_OFFSET(Item, flags, 44);
TS3_X86_OFFSET(Item, index, 60);
TS3_X86_OFFSET(Adapter, holder, 8);
TS3_X86_OFFSET(Tree, begin, 28);
TS3_X86_OFFSET(Tree, end, 32);
float __stdcall Nearest(const Point*, const Points*, Point*);
Record* __cdecl Partition(Record*, Record*, const Record*, Less);
bool __stdcall Simplify(const Points*, Points*);
Item* __fastcall SetFlag(Holder*, void*, Item*, uint8_t);
Item* __fastcall AdapterSetFlag(Adapter*, void*, Item*, uint8_t);
bool __fastcall FindOwner(Tree*, void*, Item*, Tree**);
bool __cdecl Overlap(const void*, const void*, const void*, const void*);
}
