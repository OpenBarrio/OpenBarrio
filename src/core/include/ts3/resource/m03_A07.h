#pragma once
#include "ts3/layout.h"
#include <cstdint>
namespace ts3::resource::m03::a07 {
struct Node { Node* next; uint32_t reserved; uint32_t key[4]; };
struct Table { Node* buckets[191]; Node* sentinel; uint32_t count; };
struct Position { Node* node; Node** bucket; };
struct InsertResult { Position pos; uint8_t inserted; };
struct Entry { void* descriptor; uint32_t priority; };
struct Iterator { Entry* current; Entry* begin; Entry* end; Entry** block; };
struct Client { Client* next; Client* prev; void* receiver; };
struct Request {
 uint32_t unknown00,status; uint8_t done,active,pad0A[2]; uint32_t control;
 void* driver; void* descriptor; void* callback;
 uint32_t unknown1C,offsetLow,offsetHigh,sizeLow,sizeHigh; void* buffer;
 uint32_t unknown34,transferredLow,transferredHigh; void* enqueueReceiver; void* internalCallback;
};
struct LoadInfo {
 void* vtable; uint32_t unknown04[3]; uint32_t key[4]; uint32_t unknown20; void* record;
 uint32_t unknown28; void* service; uint32_t unknown30,offset,size,accountedBytes;
 uint8_t pad40[8]; Request request; int32_t refs; uint32_t unknown94;
 uint8_t flags[9]; uint8_t padA1[3]; uint32_t unknownA4; void* job; uint32_t unknownAC; int64_t timestamp;
};
struct Descriptor { uint32_t unknown[4]; LoadInfo* load; };
struct Loader {
 void* vtable; void* unknown04; void* unknown08;
 Entry* current; Entry* begin; Entry* end; Entry** block; Entry* finish;
 uint8_t pad20[16]; uint32_t bytes,limit,budget,unknown3C;
 uint8_t mutex[0x30]; uint8_t pad70[12]; int32_t activeCount;
 Table table; uint32_t unknown384; void* activeJob; Client clients;
};
struct Growth { void* unknown[3]; uint32_t capacity; };
// Optional deterministic/native clock boundary; null selects the x86 import.
using Clock = int32_t (*)(int64_t*);
extern thread_local Clock clockProvider;
TS3_X86_OFFSET(LoadInfo,refs,0x90);
TS3_X86_OFFSET(LoadInfo,flags,0x98);
TS3_X86_OFFSET(LoadInfo,timestamp,0xB0);
TS3_X86_OFFSET(Loader,mutex,0x40);
TS3_X86_OFFSET(Loader,table,0x80);
TS3_X86_OFFSET(Loader,clients,0x38C);
TS3_X86_OFFSET(Node,key,8);
// TS3W.exe 0x007272E0: grow the tagged node pool, clamping automatic growth to 454.
uintptr_t __fastcall Grow(Growth*,void*);
// TS3W.exe 0x00727440: store the QPC timestamp and return the imported BOOL.
int32_t __fastcall Stamp(LoadInfo*,void*);
// TS3W.exe 0x00727470: return signed modular elapsed ticks times scale(4), or zero for no timestamp.
double __fastcall Elapsed(LoadInfo*,void*);
// TS3W.exe 0x00727670: interlocked increment of the LoadInfo refcount at +90.
int32_t __fastcall AddRef(LoadInfo*,void*);
// TS3W.exe 0x00727690: interlocked decrement and slot2 destruction at zero, returning the count.
int32_t __fastcall Release(LoadInfo*,void*);
// TS3W.exe 0x007276C0: unlink every matching key from one bucket without freeing nodes.
uint32_t __fastcall Erase(Table*,void*,const Node*);
// TS3W.exe 0x00727780: copy a four-pointer deque iterator and advance it by a signed delta.
Iterator* __fastcall AdvanceCopy(const Iterator*,void*,Iterator*,int32_t);
// TS3W.exe 0x007277D0: find a key in a node chain, retaining EA's word comparison order.
Node* __stdcall FindNode(Node*,const Node*);
// TS3W.exe 0x00727810: find a raw four-word key in a node chain; the third argument is unused.
Node* __stdcall FindKey(Node*,const uint32_t*,uintptr_t);
// TS3W.exe 0x00727C50: account and reopen a load, then initialize its async request on AL success.
uintptr_t __fastcall Produce(Loader*,void*,Descriptor*,uint32_t);
// TS3W.exe 0x00727D20: under lock, pop one pending request unless both budget limits are exceeded.
uintptr_t __fastcall Pop(Loader*,void*,uint8_t);
// TS3W.exe 0x00727DD0: notify the live client list; EAX is undefined for an empty list.
uintptr_t __fastcall Notify(Loader*,void*,uint32_t,uint32_t,float);
// TS3W.exe 0x00727E80: insert a unique key or return the existing node and a false insertion byte.
InsertResult* __fastcall Insert(Table*,void*,InsertResult*,Node*,uintptr_t);
// TS3W.exe 0x00727EF0: find a key, returning the sentinel position on a miss.
Position* __fastcall Lookup(Table*,void*,Position*,const uint32_t*,uintptr_t,uintptr_t);
// TS3W.exe 0x00727F60: descending-priority upper bound over 32-entry deque blocks.
Iterator* __cdecl UpperBound(Iterator*,Iterator,Iterator,const Entry*);
}
