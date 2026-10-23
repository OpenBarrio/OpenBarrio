#pragma once
#include "ts3/anim/m05_layouts.h"
namespace ts3::anim::b10 {
struct DelegateNode { DelegateNode* next; DelegateNode* previous; void* key; uint32_t payload; };
struct DelegateList { void* vtable; ListLink links; uint32_t depth; };
struct Value { Controller* controller; uint32_t tag,flags; float weight,overlap; };
struct Node { Node* next; Node* previous; Value value; };
struct Nodes { Node* next; Node* previous; void* descriptor; };
struct Sequence {
    Controller base; void* secondary28; Controller* transition; Nodes nodes;
    Controller* current; uint32_t value40,value44; float time,weight;
    Controller* pending; uint32_t value54,value58,value5c; float overlap;
};
struct Writer { void* vtable; void* value4; void* stream; uint32_t valueC,order; uint8_t good; };
TS3_X86_OFFSET(Sequence,current,0x3c);
TS3_X86_OFFSET(Sequence,pending,0x50);
TS3_X86_OFFSET(Sequence,overlap,0x60);
TS3_X86_SIZE(Node,0x1c);
TS3_X86_OFFSET(Writer,good,0x14);
// TS3W.exe 0x00566240: Find a controller by ID, preferring current then pending then queued nodes.
Controller* Find(Sequence*,uint32_t);
// TS3W.exe 0x00566320: Accumulate pending and queued durations with per-node positive overlap clamps.
float Duration(Sequence*);
// TS3W.exe 0x00566530: Clear queued bit 2 for matching tags through the +28 secondary interface, retaining the owner during traversal.
uintptr_t ClearFlags(void* secondary,uint32_t tag,uint32_t unused);
// TS3W.exe 0x00566590: Remove the first matching delegate, using tombstones while dispatching; only AL is a boolean result.
bool Remove(DelegateList*,void* key);
// TS3W.exe 0x00566630: Count links starting at a circular list sentinel.
uint32_t Count(Nodes*);
// TS3W.exe 0x00566840: Assign a five-field retained value, retaining before releasing and copying after deletion callbacks.
Value* Assign(Value*,const Value*);
// TS3W.exe 0x005668A0: Remove a delegate from current, pending, and all nonnull queued controllers in order.
void RemoveAll(Sequence*,void* key);
// TS3W.exe 0x00566A80: Allocate a zero-valued queue node and append it before the sentinel.
Node* Append(Nodes*);
// TS3W.exe 0x00566AE0: Copy a half-open source node range before a supplied position, retaining each controller.
void InsertRange(Nodes*,Node* before,Node* first,Node* last,uint32_t unused);
// TS3W.exe 0x005675A0: Return the sequence controller interface ID.
uint32_t Type(void*);
// TS3W.exe 0x005675B0: Query either supported ID, returning the owner eight bytes before the interface.
void* Query(void*,uint32_t);
// TS3W.exe 0x005680A0: Write a word through the external stream helper and AND the success latch with its low byte.
uint32_t Write(Writer*,uint32_t);
}

namespace ts3::anim {
struct b10_hooks {
    uint32_t Find(uint32_t a0) { return static_cast<uint32_t>(reinterpret_cast<uintptr_t>(b10::Find(reinterpret_cast<b10::Sequence*>(this),a0))); }
    float Duration() { return b10::Duration(reinterpret_cast<b10::Sequence*>(this)); }
    uint32_t ClearFlags(uint32_t a0,uint32_t a1) { return static_cast<uint32_t>(b10::ClearFlags(this,a0,a1)); }
    bool Remove(void* a0) { return b10::Remove(reinterpret_cast<b10::DelegateList*>(this),a0); }
    uint32_t Count() { return b10::Count(reinterpret_cast<b10::Nodes*>(this)); }
    uint32_t Assign(void* a0) { return static_cast<uint32_t>(reinterpret_cast<uintptr_t>(b10::Assign(reinterpret_cast<b10::Value*>(this),static_cast<const b10::Value*>(a0)))); }
    void RemoveAll(void* a0) { b10::RemoveAll(reinterpret_cast<b10::Sequence*>(this),a0); }
    uint32_t Append() { return static_cast<uint32_t>(reinterpret_cast<uintptr_t>(b10::Append(reinterpret_cast<b10::Nodes*>(this)))); }
    void InsertRange(void* a0,void* a1,void* a2,uint32_t a3) { b10::InsertRange(reinterpret_cast<b10::Nodes*>(this),static_cast<b10::Node*>(a0),static_cast<b10::Node*>(a1),static_cast<b10::Node*>(a2),a3); }
    uint32_t Type() { return b10::Type(this); }
    uint32_t Query(uint32_t a0) { return static_cast<uint32_t>(reinterpret_cast<uintptr_t>(b10::Query(this,a0))); }
    uint32_t Write(uint32_t a0) { return b10::Write(reinterpret_cast<b10::Writer*>(this),a0); }
};
}
