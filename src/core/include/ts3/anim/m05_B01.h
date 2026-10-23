#pragma once
#include "ts3/anim/m05_layouts.h"
namespace ts3::anim::b01 {
struct Node {
    void** vtable;
    ListLink link, children;
    Node* parent;
    void* host;
    float weight, effective;
};
TS3_X86_OFFSET(Node,weight,0x1c);
struct Cursor {
    int32_t position;
    uint32_t opaque[2];
    unsigned char* input;
    unsigned char* output;
};
struct Entry { uint32_t key; void* value; Entry* next; };
struct Registry {
    void* opaque; Entry** buckets; uint32_t bucketCount,count;
    unsigned char unknown[12];
    Entry* freeList; void* descriptor; void* poolBegin; void* poolEnd;
    uint32_t stride; Entry* special;
};
TS3_X86_OFFSET(Registry,freeList,0x1c);
TS3_X86_OFFSET(Registry,special,0x30);
// TS3W.exe 0x0054BF30: Read the node's local weight.
float Weight(const Node*);
// TS3W.exe 0x0054BF40: Set weight and propagate through parent, or copy to effective on a root.
uintptr_t SetWeight(Node*,float);
// TS3W.exe 0x0054BF70: Write type id then virtual payload, returning true in AL.
bool WriteNode(Node*,Cursor*);
// TS3W.exe 0x0054BFC0: Read type id, construct node through host and read virtual payload.
bool ReadNode(void*,Node**,Cursor*);
// TS3W.exe 0x0054C070: Load weight, extension and children without clearing existing children.
void Load(Node*,Cursor*);
// TS3W.exe 0x0054C130: Reset only the base vtable.
void Reset(Node*);
// TS3W.exe 0x0054C140: Return evaluator base type identity.
uint32_t Type(Node*);
// TS3W.exe 0x0054C150: Query evaluator base identity.
Node* Query(Node*,uint32_t);
// TS3W.exe 0x0054C170: Append child and propagate; does not detach an old parent.
uintptr_t Attach(Node*,Node*);
// TS3W.exe 0x0054C1A0: Unlink child and propagate; leaves stale child link fields.
uintptr_t Detach(Node*,Node*);
// TS3W.exe 0x0054C1D0: Initialize evaluator base, preserving its own sibling links.
Node* Construct(Node*,void*);
// TS3W.exe 0x0054C210: Reset base vtable and optionally free, returning the freed address.
Node* Destroy(Node*,uint32_t);
// TS3W.exe 0x0054C280: Save weight, extension and recursive children in insertion order.
void Save(Node*,Cursor*);
// TS3W.exe 0x0054C370: Look up factory in the global evaluator registry.
void* Find(uint32_t);
// TS3W.exe 0x0054C480: Erase consecutive matching keys and recycle pool entries.
uint32_t Erase(Registry*,const uint32_t*);
}

#if defined(_M_IX86)
namespace ts3::anim::b01::hooks {
float __fastcall Weight(Node*,void*);
uintptr_t __fastcall SetWeight(Node*,void*,float);
bool __cdecl WriteNode(Node*,Cursor*);
bool __cdecl ReadNode(void*,Node**,Cursor*);
void __fastcall Load(Node*,void*,Cursor*);
void __fastcall Reset(Node*,void*);
uint32_t __fastcall Type(Node*,void*);
Node* __fastcall Query(Node*,void*,uint32_t);
uintptr_t __fastcall Attach(Node*,void*,Node*);
uintptr_t __fastcall Detach(Node*,void*,Node*);
Node* __fastcall Construct(Node*,void*,void*);
Node* __fastcall Destroy(Node*,void*,uint32_t);
void __fastcall Save(Node*,void*,Cursor*);
void* __cdecl Find(uint32_t);
uint32_t __fastcall Erase(Registry*,void*,const uint32_t*);
}
#endif
