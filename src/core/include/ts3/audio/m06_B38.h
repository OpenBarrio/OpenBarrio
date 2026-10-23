#pragma once
#include <cstdint>
namespace ts3::audio::b38 {
#pragma pack(push,4)
struct Header { void* vtable; uint8_t reserved4[8]; void* data; uint8_t reserved10[17]; uint8_t channels; uint8_t reserved22[2]; };
struct LongEntry { void* allocation; uint32_t words[4]; };
struct ShortEntry { void* allocation; uint32_t words[2]; };
struct LongEffect { Header header; LongEntry entries[6]; uint32_t gap; uint32_t parameters[60]; };
struct Effect { Header header; ShortEntry entries[6]; uint32_t gap; uint32_t parameters[28]; uint32_t phase; uint32_t tail; };
struct Block { uint32_t tag; float* samples; uint32_t reserved; uint16_t offset,stride; };
struct Rate { uint32_t reserved[3]; float rate; };
struct Context { uint8_t reserved[0x3000c]; Block* input; Block* output; uint32_t reserved1; Rate* rate; uint32_t reservedRate; int32_t frames; uint32_t reserved2; float scale; };
#pragma pack(pop)
// TS3W.exe 0x008A4C90: Reconstruct effect state or callback, retaining partial stores and incidental returns.
LongEffect* LongConstruct(LongEffect* self);
// TS3W.exe 0x008A4D10: Reconstruct effect state or callback, retaining partial stores and incidental returns.
uintptr_t LongDestroy(LongEffect* self);
// TS3W.exe 0x008A4D50: Reconstruct effect state or callback, retaining partial stores and incidental returns.
LongEffect* LongDelete(LongEffect* self, uint32_t flags);
// TS3W.exe 0x008A4D70: Reconstruct effect state or callback, retaining partial stores and incidental returns.
uintptr_t LongInitialize(LongEffect* self);
// TS3W.exe 0x008A4DB0: Reconstruct effect state or callback, retaining partial stores and incidental returns.
uint32_t LongSize();
// TS3W.exe 0x008A4DC0: Reconstruct effect state or callback, retaining partial stores and incidental returns.
void* LongDescriptor();
// TS3W.exe 0x008A5430: Reconstruct effect state or callback, retaining partial stores and incidental returns.
uintptr_t Destroy(Effect* self);
// TS3W.exe 0x008A5470: Reconstruct effect state or callback, retaining partial stores and incidental returns.
Effect* Delete(Effect* self, uint32_t flags);
// TS3W.exe 0x008A5490: Reconstruct effect state or callback, retaining partial stores and incidental returns.
Effect* Construct(Effect* self, uint32_t offset);
// TS3W.exe 0x008A54F0: Reconstruct effect state or callback, retaining partial stores and incidental returns.
uintptr_t Initialize(Effect* self);
// TS3W.exe 0x008A5520: Reconstruct effect state or callback, retaining partial stores and incidental returns.
uint32_t Size();
// TS3W.exe 0x008A5530: Reconstruct effect state or callback, retaining partial stores and incidental returns.
void* Descriptor();
// TS3W.exe 0x008A5540: Reconstruct effect state or callback, retaining partial stores and incidental returns.
uint32_t Process(Effect* self, Context* context);
// TS3W.exe 0x008A5860: Reconstruct effect state or callback, retaining partial stores and incidental returns.
uintptr_t InitializeModulator(Effect* self);
// TS3W.exe 0x008A58B0: Reconstruct effect state or callback, retaining partial stores and incidental returns.
uint32_t ModulatorSize();
inline LongEffect* __fastcall LongConstructHook(LongEffect* self, void*) {return LongConstruct(self);}
inline uintptr_t __fastcall LongDestroyHook(LongEffect* self, void*) {return LongDestroy(self);}
inline LongEffect* __fastcall LongDeleteHook(LongEffect* self, void*, uint32_t flags) {return LongDelete(self, flags);}
inline uintptr_t __fastcall DestroyHook(Effect* self, void*) {return Destroy(self);}
inline Effect* __fastcall DeleteHook(Effect* self, void*, uint32_t flags) {return Delete(self, flags);}
}
