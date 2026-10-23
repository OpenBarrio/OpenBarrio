#pragma once
#include "ts3/layout.h"
#include <cstdint>
namespace ts3::audio::b37 {
struct Record { uint32_t word[4]; };
struct Vector { Record *begin,*end,*capacity; const char* label; Record* inlineBuffer; uint8_t storage[0x1e4]; };
struct Selector { void* table; uint32_t reserved; Vector entries; Record current; Record* cursor; uint32_t remaining,mode; };
TS3_X86_OFFSET(Selector,current,0x200); TS3_X86_OFFSET(Selector,cursor,0x210);
struct Queue { void* primary; void* secondary; uint32_t refs; const char* label; uint32_t state; uint8_t containers[0x1860]; };
struct Block { uint32_t reserved; float* data; uint8_t pad[6]; uint16_t stride; };
struct Context { uint8_t pad[0x3000c]; Block *input,*output; uint8_t more[12]; int32_t frames; };
struct Crusher { void* table; uint8_t pad[8]; float* parameters; uint8_t more[17]; uint8_t channels; uint8_t tail[6]; float bits,unused,mix; };
TS3_X86_OFFSET(Crusher,bits,0x28); TS3_X86_OFFSET(Context,frames,0x30020);
// TS3W.exe 0x008A3D90: construct the three queue containers through their original interfaces.
Queue* ConstructQueue(Queue*,const char*);
// TS3W.exe 0x008A3E00: destroy the queue and optionally free it, returning self.
Queue* DeleteQueue(Queue*,uint32_t);
// TS3W.exe 0x008A3E20: probe the Windows audio endpoint property, releasing each COM interface.
uint32_t ProbeEndpoint();
// TS3W.exe 0x008A3F40: step circularly; short vectors preserve incoming EAX above AL.
uintptr_t Step(Selector*,uint8_t,uintptr_t residue=0);
// TS3W.exe 0x008A3FD0: retrieve the current entry; empty output clears only three words.
Record* Get(Selector*,Record*);
// TS3W.exe 0x008A4060: reset selector state, retaining the uninitialized fourth-word stack residue.
uint32_t ResetResidue(Selector*,uint32_t);
// TS3W.exe 0x008A40B0: shuffle; return the last swap address or untouched incoming EAX for fewer than two entries.
uintptr_t Shuffle(Record*,Record*,uint32_t(__cdecl**)(uint32_t));
// TS3W.exe 0x008A4140: advance/reshuffle with three-word repeat avoidance; retain branch-dependent incidental EAX.
uintptr_t Advance(Selector*,uint8_t);
// TS3W.exe 0x008A4300: insert a four-word record, with aliased input and inline-buffer growth.
uintptr_t Insert(Vector*,Record*,const Record*);
// TS3W.exe 0x008A4430: append a record to the selector vector.
uintptr_t Append(Selector*,Record);
// TS3W.exe 0x008A4480: return the filter descriptor address (verified constant-return twin).
uintptr_t FilterDescriptor();
// TS3W.exe 0x008A4490: quantize signed low-16 samples and mix; sixteen bits bypasses buffer swapping.
uint32_t Process(Crusher*,Context*);
// TS3W.exe 0x008A45C0: initialize the crusher defaults; AL-only success preserves pointer upper bits.
uintptr_t InitCrusher(Crusher*);
// TS3W.exe 0x008A45F0: return the crusher allocation size.
uint32_t CrusherSize();
// TS3W.exe 0x008A4600: return the crusher descriptor address (verified constant-return twin).
uintptr_t CrusherDescriptor();
inline Queue* __fastcall HookConstruct(Queue* s,void*,const char* l) {return ConstructQueue(s,l);}
inline Queue* __fastcall HookDelete(Queue* s,void*,uint32_t f) {return DeleteQueue(s,f);}
uintptr_t __fastcall HookStep(Selector*,void*,uint8_t);
inline Record* __fastcall HookGet(Selector* s,void*,Record* o) {return Get(s,o);}
uint32_t __fastcall HookReset(Selector*,void*);
inline uintptr_t __fastcall HookAdvance(Selector* s,void*,uint8_t f) {return Advance(s,f);}
inline uintptr_t __fastcall HookInsert(Vector* s,void*,Record* p,const Record* v) {return Insert(s,p,v);}
inline uintptr_t __fastcall HookAppend(Selector* s,void*,Record r) {return Append(s,r);}
}
