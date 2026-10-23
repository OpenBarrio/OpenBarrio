#pragma once
#include "ts3/layout.h"
namespace ts3::audio::b05 {
struct Key { uint32_t low,high; };
struct alignas(16) Node {
    Node* next; uint32_t reserved; Key key; uint32_t flags,value;
    uint8_t padding[8]; float vector[4];
};
struct Hash { Node* buckets[53]; Node* sentinel; uint32_t count; };
struct Result { Node* node; Node** bucket; uint8_t inserted; };
struct Group { Key key; void* object; uint32_t reserved; Group* next; };
struct State {
    uint8_t prefix[0x18]; Group** groups; uint32_t groupCount; uint8_t middle[0x14];
    Node* free; uint8_t pool[0xC]; Hash hash; uint32_t reserved; int32_t pending; uint8_t active;
};
TS3_X86_OFFSET(State,hash,0x44); TS3_X86_OFFSET(State,pending,0x124);
TS3_X86_OFFSET(Node,vector,0x20);
// TS3W.exe 0x00862910: Insert by two-word key; duplicate nodes remain unconsumed.
Result* __fastcall Insert(Hash*,void*,Result*,Node*,uint32_t);
// TS3W.exe 0x00862980: Mark a key for removal, incrementing pending even if already marked.
Node* __fastcall Mark(State*,void*,Key);
// TS3W.exe 0x008629D0: Copy all four vector lanes when the key exists.
Node* __fastcall Vector(State*,void*,Key,const float*);
// TS3W.exe 0x00862A10: Clear the mask then OR unmasked supplied flags.
Node* __fastcall Flags(State*,void*,Key,uint32_t,uint32_t);
// TS3W.exe 0x00862A50: Stop every group then submit the stop-all opcode.
uintptr_t __fastcall Stop(State*,void*);
// TS3W.exe 0x00862AC0: Recycle marked nodes and decrement pending once per node.
Node** __fastcall Prune(State*,void*);
// TS3W.exe 0x00862B50: Recycle all nodes and reset pending.
Node** __fastcall Clear(State*,void*);
// TS3W.exe 0x00862BC0: Copy 500-byte elements backward, returning the destination start.
void* __cdecl CopyBackward(const void*,const void*,void*);
// TS3W.exe 0x00862C00: Disable, stop groups, and clear the key table.
Node** __fastcall Disable(State*,void*);
// TS3W.exe 0x00862C20: Update active groups then prune if signed pending is positive.
void __fastcall Tick(State*,void*,float);
// TS3W.exe 0x00862CC0: Consume a pool node, initialize and insert; duplicate insertion leaks that pool node.
void __fastcall Add(State*,void*,Key,uint32_t);
// TS3W.exe 0x00862D30: Query a retained resource and apply it to the selected group; tests only AL.
bool __fastcall Apply(State*,void*,void*,Key);
}
