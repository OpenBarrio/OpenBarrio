#pragma once
#include "ts3/layout.h"
namespace ts3::audio::b28 {
struct Allocator { void* vtable; void* heap; };
struct BlockView {
    uint8_t* block; uint8_t* end; uint8_t* data;
    uint32_t tag, bytes, count;
};
struct Queue {
    uint8_t* begin; uint8_t* end; uint8_t* reserved; uint32_t word0c;
    BlockView view; uint32_t word28;
    uint8_t* cursor; uint8_t* readable; uint8_t* writing;
    uint32_t* length; uint8_t* start;
    uint32_t total, word44, word48, consumed;
};
TS3_X86_OFFSET(Queue,view,0x10);
TS3_X86_OFFSET(Queue,cursor,0x2c);
TS3_X86_OFFSET(Queue,total,0x40);
TS3_X86_OFFSET(Queue,consumed,0x4c);
// TS3W.exe 0x0089F450: Allocate from the heap, ignoring the name argument.
void* Allocate(Allocator*, uint32_t, const char*, uint32_t);
// TS3W.exe 0x0089F470: Forward aligned allocation, ignoring the name argument.
void* AllocateAligned(Allocator*, uint32_t, const char*, uint32_t, uint32_t, uint32_t);
// TS3W.exe 0x0089F4D0: Install allocator vtable and clear heap (adapted from verified UiWorldInner::Construct).
Allocator* Construct(Allocator*);
// TS3W.exe 0x0089F4E0: Create and configure the audio heap; buffer failure leaves the heap installed.
uint8_t Initialize(Allocator*, uint32_t);
// TS3W.exe 0x0089F5A0: Drain and delete the owned heap.
void Shutdown(Allocator*);
// TS3W.exe 0x0089F5E0: Destroy the allocator and install the base vtable.
void Destruct(Allocator*);
// TS3W.exe 0x0089F620: Destroy and optionally free the allocator, returning its address.
Allocator* Delete(Allocator*, uint32_t);
// TS3W.exe 0x0089F680: Load a block header, returning its end; null input leaves the view untouched.
uint8_t* LoadBlock(BlockView*, uint8_t*);
// TS3W.exe 0x0089F6D0: Publish an unfinished block and clear its write state.
uint8_t* Publish(Queue*);
// TS3W.exe 0x0089F6F0: Test total bytes for zero (verified IndexModifiable::IsEmpty twin).
uint32_t Empty(const Queue*);
// TS3W.exe 0x0089F700: Return total minus consumed with unsigned wrap (MemoryStream::GetAvailable twin).
uint32_t Available(const Queue*);
// TS3W.exe 0x0089F710: Finalize block length and load the read view, returning the view address.
BlockView* Finish(Queue*);
// TS3W.exe 0x0089F770: Read next record and advance the offset by its padded length.
uint8_t* Next(BlockView*, uint32_t*, uint32_t*, uint32_t*, uint32_t*);
// TS3W.exe 0x0089F7D0: Search record count for a tag and return its payload.
uint8_t* Find(BlockView*, uint32_t, uint32_t*, uint32_t*);
// TS3W.exe 0x0089F830: Reset consumed count and select begin unless begin equals end.
uint8_t* Rewind(Queue*);
#if defined(_M_IX86)
void* __fastcall HookAllocate(Allocator* a0, void*, uint32_t a1, const char* a2, uint32_t a3);
void* __fastcall HookAllocateAligned(Allocator* a0, void*, uint32_t a1, const char* a2, uint32_t a3, uint32_t a4, uint32_t a5);
Allocator* __fastcall HookConstruct(Allocator* a0, void*);
uint8_t __fastcall HookInitialize(Allocator* a0, void*, uint32_t a1);
void __fastcall HookShutdown(Allocator* a0, void*);
void __fastcall HookDestruct(Allocator* a0, void*);
Allocator* __fastcall HookDelete(Allocator* a0, void*, uint32_t a1);
uint8_t* __fastcall HookLoadBlock(BlockView* a0, void*, uint8_t* a1);
uint8_t* __fastcall HookPublish(Queue* a0, void*);
uint32_t __fastcall HookEmpty(const Queue* a0, void*);
uint32_t __fastcall HookAvailable(const Queue* a0, void*);
BlockView* __fastcall HookFinish(Queue* a0, void*);
uint8_t* __fastcall HookNext(BlockView* a0, void*, uint32_t* a1, uint32_t* a2, uint32_t* a3, uint32_t* a4);
uint8_t* __fastcall HookFind(BlockView* a0, void*, uint32_t a1, uint32_t* a2, uint32_t* a3);
uint8_t* __fastcall HookRewind(Queue* a0, void*);
#endif
}
