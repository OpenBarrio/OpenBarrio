#pragma once
#include "ts3/layout.h"
namespace ts3::audio::b01 {
struct Manager;
struct Block { Block* next; uint32_t slot, size, position, state, reader; };
struct Reader { Reader* next; Manager* manager; uint32_t id, remaining; Block* block; };
struct Request { Request* next; void* a; void* b; uint32_t reader; };
using Producer = int32_t(*)(uint32_t,uint32_t,uint32_t,void*,void*,void*,uint32_t*);
struct Slot {
    uint32_t id, state, unused; Slot* next;
    uint8_t reserved[0x118]; uint32_t position; uint8_t eof, pad[3];
    Producer produce; void* context; uint32_t remaining; uint32_t tail;
};
struct Manager {
    void* vtable; void* unknown; uint8_t lock[0x30];
    Slot* slots; int32_t slotCount;
    Request *requests, *requestTail; uint32_t requestCount;
    Reader *readers, *readerTail; uint32_t readerCount;
    Block *blocks, *blockTail; uint32_t blockCount;
    uint8_t reserved64[12]; uint32_t mode, unused74, signal;
    int32_t threshold; uint8_t below, pad81[3]; uint32_t produced, consumed, cursor, end;
    Slot *head, *current, *active;
    uint8_t reservedA0[0x118]; uint8_t event[0x58]; uint64_t stamp;
};
struct Handle { void* vtable; Manager* manager; };
TS3_X86_SIZE(Slot,0x140);
TS3_X86_OFFSET(Manager,slots,0x38);
TS3_X86_OFFSET(Manager,mode,0x70);
TS3_X86_OFFSET(Manager,event,0x1b8);
TS3_X86_OFFSET(Manager,stamp,0x210);
TS3_X86_SIZE(Manager,0x218);
TS3_X86_SIZE(Reader,0x14);
TS3_X86_SIZE(Block,0x18);
TS3_X86_SIZE(Request,0x10);
TS3_X86_OFFSET(Slot,position,0x128);
TS3_X86_OFFSET(Slot,produce,0x130);
TS3_X86_OFFSET(Slot,remaining,0x138);
// Optional native/test boundary implementations. Null members use original x86 bridges.
struct Boundaries {
    void* (*allocate)(size_t)=nullptr;
    void (*lock)(Manager*)=nullptr;
    int32_t (*unlock)(Manager*)=nullptr;
    uint32_t (*signal)(Manager*,uint32_t)=nullptr;
    void (*retire)(Manager*,Slot*)=nullptr;
    void (*discard)(Manager*,uint32_t)=nullptr;
    Manager* (*construct)(void*,uint32_t,uint32_t,uint32_t,uint32_t,uint32_t,uint32_t)=nullptr;
};
extern thread_local Boundaries boundaries;
// TS3W.exe 0x00402E50: Consume a reader block and return its descriptor, publishing state before advancing.
uint32_t* Consume(Reader*);
// TS3W.exe 0x00402F00: Return remaining bytes for an active generation-checked slot.
uint32_t Remaining(Handle*,uint32_t);
// TS3W.exe 0x00402F40: Return the stream mode.
uint32_t Mode(Handle*);
// TS3W.exe 0x00402F50: Return one only when mode and reader bytes are both zero.
uint32_t Finished(Reader*);
// TS3W.exe 0x00402F70: Find the slot containing the reader position across unsigned wraparound.
uint32_t PositionSlot(Reader*);
// TS3W.exe 0x00402FF0: Return the state of a generation-checked slot.
uint32_t State(Handle*,uint32_t);
// TS3W.exe 0x00403020: Return the full two-word stream timestamp in EDX:EAX.
uint64_t Stamp(Handle*);
// TS3W.exe 0x004033E0: Allocate and construct a stream, returning its first reader; null allocation still dereferences null.
Reader* Create(uint32_t,uint32_t,uint32_t,uint32_t,uint32_t,uint32_t);
// TS3W.exe 0x00403420: Recycle an empty request or allocate one and append it; null head is not supported by EA.
Request* Enqueue(Handle*,uint32_t,void*,void*);
// TS3W.exe 0x004034C0: Change threshold and signal on a below-threshold transition; preserve incidental EAX bits.
uint32_t Threshold(Handle*,int32_t);
// TS3W.exe 0x00403520: Allocate and append a numbered reader, with increment-before-publication ordering.
Reader* AddReader(Handle*);
// TS3W.exe 0x004035A0: Fill queued blocks through producers, retaining callback-visible intermediate state.
int32_t Fill(Manager*);
// TS3W.exe 0x00403BB0: Cancel a slot and discard its unconsumed blocks.
int32_t Cancel(Handle*,uint32_t);
// TS3W.exe 0x00403CD0: Stop all slots and mark remaining blocks discarded.
int32_t Stop(Handle*);
}

// ABI-only free-function adapters: ECX=self, unused EDX; fastcall pops the
// same stack words as each original thiscall entry, including EDX:EAX Stamp.
#if defined(_M_IX86)
namespace ts3::audio::b01::hooks {
inline uint32_t* __fastcall Consume(Reader* p,void*) { return b01::Consume(p); }
inline uint32_t __fastcall Remaining(Handle* p,void*,uint32_t id) { return b01::Remaining(p,id); }
inline uint32_t __fastcall Mode(Handle* p,void*) { return b01::Mode(p); }
inline uint32_t __fastcall Finished(Reader* p,void*) { return b01::Finished(p); }
inline uint32_t __fastcall PositionSlot(Reader* p,void*) { return b01::PositionSlot(p); }
inline uint32_t __fastcall State(Handle* p,void*,uint32_t id) { return b01::State(p,id); }
inline uint64_t __fastcall Stamp(Handle* p,void*) { return b01::Stamp(p); }
inline Request* __fastcall Enqueue(Handle* p,void*,uint32_t id,void* a,void* b) { return b01::Enqueue(p,id,a,b); }
inline uint32_t __fastcall Threshold(Handle* p,void*,int32_t n) { return b01::Threshold(p,n); }
inline Reader* __fastcall AddReader(Handle* p,void*) { return b01::AddReader(p); }
inline int32_t __fastcall Fill(Manager* p,void*) { return b01::Fill(p); }
inline int32_t __fastcall Cancel(Handle* p,void*,uint32_t id) { return b01::Cancel(p,id); }
inline int32_t __fastcall Stop(Handle* p,void*) { return b01::Stop(p); }
}
#endif
