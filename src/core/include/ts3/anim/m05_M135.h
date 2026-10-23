#pragma once
#include "ts3/layout.h"
#include <cstdint>
namespace ts3::anim::m135 {
struct Link { Link* next; Link* previous; };
struct Progress { void* begin; void* end; void* capacity; void* descriptor; void* inlineBegin; uint32_t opaque[2]; };
struct Body {
    void* vtable; void* interface4; void* base8[3]; void* host;
    void* secondary; Link controllers; void* controllerDescriptor;
    unsigned char style[0x7c];
    Link effects; void* effectsDescriptor; uint32_t b0;
    void** slaves; void** slavesEnd; void** slavesCapacity; void* slaveDescriptor; void* master;
    uint32_t c8; uint8_t cc[4];
    unsigned char stateD0[0x20],stateF0[0x24];
    void* brainTable; uint32_t brain118[2]; float maxTime; uint32_t argument,reset128;
    float values12c[4]; void* values13c[2]; float weights[6]; uint32_t untouched15c[2];
    float speed,speed168; uint32_t counters[3],value178; uint8_t flags17c[10],padding186[2];
    Progress *begin,*end,*capacity; void* descriptor; Progress* inlineBegin;
    Progress storage[6]; uint32_t value244,count,target; uint8_t flags[5];
};
TS3_X86_OFFSET(Body,controllers,0x1c);
TS3_X86_OFFSET(Body,speed,0x164);
TS3_X86_OFFSET(Body,begin,0x188);
TS3_X86_OFFSET(Body,flags,0x250);
struct State { Link controllers; void* descriptor; float speed; uint32_t counters[3],count,target; uint8_t flags[8]; };
TS3_X86_OFFSET(State,flags,0x24);
// TS3W.exe 0x0084E900: Detach controllers and export motion state, leaving byte 40 untouched.
uint8_t Export(Body*,State*);
// TS3W.exe 0x0084EA00: Restore motion state and reconcile owned controllers and route state.
uint8_t Restore(Body*,State*);
// TS3W.exe 0x0084EEF0: Reset motion and destroy progression payloads, returning the retained begin pointer.
Progress* Reset(Body*);
// TS3W.exe 0x0084EFF0: Construct an animated Sim body and attach its three brain interfaces.
Body* Construct(Body*,void*,uint32_t);
// TS3W.exe 0x0084F230: Return the AnimatedSimBody type ID (adapted from verified FileType twin).
uint32_t Type(void*);
// TS3W.exe 0x0084F240: Return this for either supported ID (adapted from verified PackedFileRecordQueryInterfaceById).
void* Query(void*,uint32_t);
}
namespace ts3::anim::m135::hooks {
uint8_t __fastcall Export(Body*,void*,State*);
uint8_t __fastcall Restore(Body*,void*,State*);
Progress* __fastcall Reset(Body*,void*);
Body* __fastcall Construct(Body*,void*,void*,uint32_t);
uint32_t __fastcall Type(void*,void*);
void* __fastcall Query(void*,void*,uint32_t);
}
