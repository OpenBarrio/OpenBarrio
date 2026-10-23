#pragma once
#include "ts3/layout.h"
#include <cstdint>
namespace ts3::audio::b39impl {
struct System { uint8_t pre[32]; uint8_t* commands; uint8_t middle[148]; uint32_t used; uint8_t gap[8]; float rate; };
struct Node { uint8_t pre[40]; float duration; };
struct Base { void* vtable; System* system; Node* node; void* output; uint32_t words[2]; float duration; uint32_t word; uint8_t channels, meterChannels, bytes[2]; };
using Callback = uintptr_t(__cdecl*)(void*, void*);
struct Capture : Base { uint8_t active, pad[3]; void** begin; void** end; void** capacity; const char* label; void** internal; void* storage[100]; uint32_t length, cursor, rate; Callback callback; void* opaque; };
struct Meter : Base { uint8_t task[24]; int32_t window, cursor; uint32_t spare; uint32_t display[38]; float sum[6], rms[6], peak[6], last[6], maximum[6]; uint8_t processed, live; };
struct Block { uint32_t word; float* data; uint32_t word2; uint16_t offset, stride; };
struct Context { uint8_t pre[0x3000c]; Block* input; Block* output; void* other; float* rateInfo; uint32_t word; uint32_t count; };
struct Start { uint32_t length; Callback callback; void* opaque; uint32_t rate; };
struct Packets { void** data; uint32_t count; };
TS3_X86_OFFSET(Capture, callback, 472);
TS3_X86_OFFSET(Meter, sum, 224);
TS3_X86_OFFSET(Meter, live, 345);
TS3_X86_OFFSET(System, used, 184);
// TS3W.exe 0x008A58C0: Return capture descriptor identity.
uintptr_t CaptureDescriptor();
// TS3W.exe 0x008A58D0: Downmix and deliver clipped signed PCM packets.
int CaptureProcess(Capture*, Context*);
// TS3W.exe 0x008A5B10: Destroy capture vector and optionally free object.
Capture* CaptureDestroy(Capture*, uint32_t);
// TS3W.exe 0x008A5B50: Initialize capture's inline vector.
uint8_t CaptureInit(Capture*);
// TS3W.exe 0x008A5BA0: Start, stop or enqueue capture buffers; preserve unordered removal.
uintptr_t CaptureCommand(Capture*, int, void*);
// TS3W.exe 0x008A5CE0: Append a two-word deferred command.
void* AppendCommand(System*, uintptr_t, uintptr_t);
// TS3W.exe 0x008A5D10: Return meter allocation size.
uint32_t MeterSize();
// TS3W.exe 0x008A5D20: Return meter descriptor identity.
uintptr_t MeterDescriptor();
// TS3W.exe 0x008A5D30: Clear only the even display lanes and published meter values.
void* MeterReset(Meter*);
// TS3W.exe 0x008A5DE0: Cancel a registered meter task; EAX is unspecified when inactive.
void MeterCancel(Meter*);
// TS3W.exe 0x008A5EB0: Publish channel-specific meter values, including fallback six-channel mapping.
uint8_t MeterPublish(Meter*);
// TS3W.exe 0x008A6040: Queue meter update callback.
void* MeterCommand(Meter*, int, int);
// TS3W.exe 0x008A6070: Accumulate scalar/SSE meter statistics and window history.
float* MeterAccumulate(Meter*, Block*, int);
// TS3W.exe 0x008A6360: Process one fixed 256-frame meter block unless disabled.
int MeterProcess(Meter*, Context*);
// TS3W.exe 0x008A63B0: Initialize meter and register its background task.
uint8_t MeterInit(Meter*);
}
// ABI adapters: member entries preserve the game's thiscall convention.
namespace ts3::audio {
struct b39 {
    static uintptr_t CaptureDescriptor() { return b39impl::CaptureDescriptor(); }
    static uintptr_t MeterDescriptor() { return b39impl::MeterDescriptor(); }
    static uint32_t MeterSize() { return b39impl::MeterSize(); }
    static int CaptureProcess(b39impl::Capture* s, b39impl::Context* c) { return b39impl::CaptureProcess(s,c); }
    static uint8_t CaptureInit(b39impl::Capture* s) { return b39impl::CaptureInit(s); }
    static int MeterProcess(b39impl::Meter* s, b39impl::Context* c) { return b39impl::MeterProcess(s,c); }
    static uint8_t MeterInit(b39impl::Meter* s) { return b39impl::MeterInit(s); }
    void* CaptureDestroy(uint32_t flags) { return b39impl::CaptureDestroy(reinterpret_cast<b39impl::Capture*>(this),flags); }
    uint32_t CaptureCommand(uint32_t op, void* arg) { return uint32_t(b39impl::CaptureCommand(reinterpret_cast<b39impl::Capture*>(this),int(op),arg)); }
    void* AppendCommand(uint32_t a,uint32_t b) { return b39impl::AppendCommand(reinterpret_cast<b39impl::System*>(this),a,b); }
    void* MeterReset() { return b39impl::MeterReset(reinterpret_cast<b39impl::Meter*>(this)); }
    void MeterCancel() { b39impl::MeterCancel(reinterpret_cast<b39impl::Meter*>(this)); }
    int8_t MeterPublish() { return int8_t(b39impl::MeterPublish(reinterpret_cast<b39impl::Meter*>(this))); }
    void* MeterCommand(uint32_t a,uint32_t b) { return b39impl::MeterCommand(reinterpret_cast<b39impl::Meter*>(this),int(a),int(b)); }
    void* MeterAccumulate(void* p,uint32_t n) { return b39impl::MeterAccumulate(reinterpret_cast<b39impl::Meter*>(this),static_cast<b39impl::Block*>(p),int(n)); }
};
}
