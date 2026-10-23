#pragma once
#include "ts3/layout.h"
#include <cstdint>
#include <cstddef>

namespace ts3::resource::m03::a12 {
struct Value { uint32_t words[4]; };
struct Iterator { Value* cur; Value* begin; Value* end; Value** node; };
struct KeyDeque { Value** map; uint32_t capacity; Iterator first,last; const char* tag; };
struct PointerIterator { void** cur; void** begin; void** end; void*** node; };
struct PointerDeque { void*** map; uint32_t capacity; PointerIterator first,last; const char* tag; };
struct Preloader {
    void* vtable; int32_t requested; uint32_t reserved[2];
    void* slots[8]; PointerDeque pending, auxiliary;
};
TS3_X86_SIZE(Iterator,0x10);
TS3_X86_OFFSET(KeyDeque,first,8);
TS3_X86_OFFSET(KeyDeque,last,0x18);
TS3_X86_OFFSET(KeyDeque,tag,0x28);
TS3_X86_OFFSET(Preloader,slots,0x10);
TS3_X86_OFFSET(Preloader,pending,0x30);
TS3_X86_OFFSET(Preloader,auxiliary,0x5c);
struct Boundaries {
    void* (*allocate)(size_t,const char*);
    void (*free)(void*);
    void (*grow)(KeyDeque*,uint32_t,bool);
    uintptr_t (*clear)(PointerDeque*);
    void* (*loader)();
    void (*cancel)(void*,void**);
    void (*release)(void*);
};
const Boundaries& OriginalBoundaries();
// TS3W.exe 0x0072F830: copies a flat value range to a segmented iterator and returns out.
Iterator* CopyFlat(Iterator* out,const Value* first,const Value* last,Iterator dst);
// TS3W.exe 0x0072FA70: copies then offsets an iterator; EA's biased signed division is retained.
Iterator* Offset(const Iterator* self,Iterator* out,int32_t delta);
// TS3W.exe 0x0072FAC0: reserves front/back value pages and returns the proposed iterator, without publishing it.
Iterator* Reserve(KeyDeque*,Iterator* out,uint32_t count,uint32_t back,const Boundaries& = OriginalBoundaries());
// TS3W.exe 0x0072FC20: constructs repeated values in a segmented range; returns last.cur.
Value* ConstructFill(Iterator first,Iterator last,const Value* value);
// TS3W.exe 0x0072FC80: constructs a segmented source range into a destination; returns out.
Iterator* ConstructCopy(Iterator* out,Iterator first,Iterator last,Iterator dst);
// TS3W.exe 0x0072FD00: assigns repeated values across a segmented range; returns last.cur.
Value* AssignFill(Iterator first,Iterator last,const Value* value);
// TS3W.exe 0x0072FD50: constructs the source prefix and fills the remainder of the destination.
Value* CopyThenFill(Iterator first,Iterator last,Iterator dst,Iterator dstLast,const Value* value);
// TS3W.exe 0x0072FE10: fills a prefix then constructs the source suffix; returns out.
Iterator* FillThenCopy(Iterator* out,Iterator first,Iterator middle,const Value* value,Iterator src,Iterator srcLast);
// TS3W.exe 0x0072FFE0: atomically exchanges requested with one; ignores all three stack arguments.
int32_t Request(Preloader*,int32_t,int32_t,int32_t);
// TS3W.exe 0x00730090: allocates a centered pointer deque; returns its final current pointer.
void** Initialize(PointerDeque*,uint32_t count,const Boundaries& = OriginalBoundaries());
// TS3W.exe 0x00730270: cancels eight slots, clears auxiliary storage, then pops/releases pending objects.
uintptr_t Drain(Preloader*,const Boundaries& = OriginalBoundaries());
// TS3W.exe 0x00730340: forwards flat-range copying, returning out.
Iterator* CopyFlatWrapper(Iterator* out,const Value* first,const Value* last,Iterator dst);
// TS3W.exe 0x00730380: forwards flat-source copying; source page metadata is deliberately ignored.
Iterator* CopyRangeWrapper(Iterator* out,Iterator first,Iterator last,Iterator dst);
#if defined(_M_IX86)
inline Iterator* __fastcall HookOffset(const Iterator* self,void*,Iterator* out,int32_t delta) { return Offset(self,out,delta); }
inline Iterator* __fastcall HookReserve(KeyDeque* self,void*,Iterator* out,uint32_t count,uint32_t back) { return Reserve(self,out,count,back); }
inline int32_t __fastcall HookRequest(Preloader* self,void*,int32_t a,int32_t b,int32_t c) { return Request(self,a,b,c); }
inline void** __fastcall HookInitialize(PointerDeque* self,void*,uint32_t count) { return Initialize(self,count); }
inline uintptr_t __fastcall HookDrain(Preloader* self,void*) { return Drain(self); }
#endif
}
