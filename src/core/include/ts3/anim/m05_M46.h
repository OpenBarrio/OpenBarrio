#pragma once
#include "ts3/anim/m05_layouts.h"
namespace ts3::anim {
struct ScratchM46 {
    Controller base;
    void* interface28;
    void* source;
    void* bindings;
    uint32_t channel;
    float time;
    uint32_t value3c, value40;
    uint8_t flags[3];
    uint8_t untouched47;
};
TS3_X86_OFFSET(ScratchM46,source,0x2c);
TS3_X86_OFFSET(ScratchM46,bindings,0x30);
TS3_X86_OFFSET(ScratchM46,channel,0x34);
TS3_X86_SIZE(ScratchM46,0x48);
struct M46Vector { void* begin; void* end; void* capacity; void* descriptor; };
struct M46Sync {
    unsigned char opaque00[0x38];
    M46Vector resolved, keys, cumulative, weights;
    uint32_t value78, value7c, index;
    uint8_t latch;
};
TS3_X86_OFFSET(M46Sync,keys,0x48);
TS3_X86_OFFSET(M46Sync,index,0x80);
// TS3W.exe 0x0056E010: replace synchronization keys, accumulate durations and prime two entries.
uintptr_t M46SetKeys(M46Sync*, const void* keys, const float* durations, const float* weights, uint32_t count);
// TS3W.exe 0x0056E1E0: sample the optional source or zero the aligned output; return output.
Float4Value* M46Sample(ScratchM46*, Float4Value*, uint32_t, uint32_t, float);
// TS3W.exe 0x0056E360: resolve a read binding and propagate the stream status byte.
uint8_t M46Read(ScratchM46*, void* stream);
// TS3W.exe 0x0056E3B0: resolve a write binding, accepting only unsigned type codes zero and one.
uint8_t M46Write(ScratchM46*, void* stream);
// TS3W.exe 0x0056E420: construct the scratch controller, preserving padding bytes.
ScratchM46* M46Construct(ScratchM46*, void* host);
// TS3W.exe 0x0056E4B0: adjust the secondary interface by twelve x86 bytes and tail-call deletion.
void* M46DeleteSecondary(void*, uint32_t flags);
#if defined(_M_IX86)
inline uintptr_t __fastcall HookM46SetKeys(M46Sync* s, void*, const void* k, const float* d, const float* w, uint32_t n) { return M46SetKeys(s,k,d,w,n); }
inline Float4Value* __fastcall HookM46Sample(ScratchM46* s, void*, Float4Value* o, uint32_t a, uint32_t b, float t) { return M46Sample(s,o,a,b,t); }
inline uint8_t __fastcall HookM46Read(ScratchM46* s, void*, void* stream) { return M46Read(s,stream); }
inline uint8_t __fastcall HookM46Write(ScratchM46* s, void*, void* stream) { return M46Write(s,stream); }
inline ScratchM46* __fastcall HookM46Construct(ScratchM46* s, void*, void* h) { return M46Construct(s,h); }
inline void* __fastcall HookM46DeleteSecondary(void* s, void*, uint32_t f) { return M46DeleteSecondary(s,f); }
#endif
}
