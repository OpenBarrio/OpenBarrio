#pragma once
#include "ts3/anim/m05_B12.h"
namespace ts3::anim {
struct M79Slot {
    int32_t index;
    uint32_t opaque04;
    B12Record *begin, *end, *capacity;
    void* descriptor;
    B12Record storage[10];
};
TS3_X86_SIZE(M79Slot,0xab8);
// TS3W.exe 0x00577DC0: Assign a record range backwards and return the destination start.
B12Record* Backward7DC0(const B12Record*, const B12Record*, B12Record*);
// TS3W.exe 0x00577E00: Copy construct count records, skipping construction at address zero.
void Construct7E00(B12Record*, uint32_t, const B12Record*);
// TS3W.exe 0x00577E30: Initialize two inline strings and nine words, preserving trailing fields and descriptors.
B12Record* __fastcall Init7E30(B12Record*, void*);
// TS3W.exe 0x00577EA0: Resolve a resource's parent joint and update the name, joint index and hash; return AL only.
uint8_t __fastcall Resolve7EA0(B12Record*, void*, void*);
// TS3W.exe 0x00577F50: Assign one record to every element of a range; empty EAX is undefined.
void Fill7F50(B12Record*, B12Record*, const B12Record*);
// TS3W.exe 0x00577F80: Reset slot records, namespace/hash ranges and rotation; preserve empty-string contents.
uint8_t __fastcall Reset7F80(ClipIkInfo*, void*);
}
