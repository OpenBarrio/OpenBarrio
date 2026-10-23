#pragma once
#include "ts3/anim/m05_layouts.h"
namespace ts3::anim::b09impl {
struct Sequence {
    Controller base;
    void* interface28;
    Controller* transition;
    ListLink nodes;
    void* descriptor;
    Controller* first;
    uint32_t value40, value44;
    float time, weight;
    Controller* second;
    uint32_t value54, flags58, value5c, value60, value64, mode, flags;
};
struct Node { ListLink links; Controller* controller; uint32_t value, flags; };
struct State { Controller* controller; uint32_t a,b; float x,y; };
struct Delegates { void* vtable; ListLink list; };
struct Delegate { ListLink links; void* callback; uint32_t key; };
struct Reader { void* vtable; uint32_t value4; void* source; uint32_t valuec; uint32_t context; uint8_t good; };
TS3_X86_OFFSET(Sequence,first,0x3c);
TS3_X86_OFFSET(Sequence,second,0x50);
TS3_X86_OFFSET(Sequence,flags58,0x58);
TS3_X86_OFFSET(Node,flags,0x10);
TS3_X86_SIZE(State,0x14);
TS3_X86_OFFSET(Reader,good,0x14);
// TS3W.exe 0x00565B10: Forward to virtual slot 7.
float Forward7(Controller* self);
// TS3W.exe 0x00565B20: Forward to virtual slot 8.
float Forward8(Controller* self);
// TS3W.exe 0x00565B70: Look up a factory in the global registry.
void* FindFactory(uint32_t key);
// TS3W.exe 0x00565C80: Read a versioned controller through its factory and load it.
Controller* ReadController(void* argument, Reader* reader);
// TS3W.exe 0x00565D50: Insert a factory; preserve low-byte-only true result.
uint32_t RegisterFactory(uint32_t key, void* factory);
// TS3W.exe 0x00565EA0: Write one float through the stream status helper.
uintptr_t WriteFloat(Reader* self, float value);
// TS3W.exe 0x00565F30: Return FLT_MAX; normalized twin 00ADB0E0 uses a different constant.
float MinusOne(Sequence* self);
// TS3W.exe 0x00565F70: Prefer the second controller, falling back to the first.
Controller* Active(Sequence* self);
// TS3W.exe 0x00565FA0: Change bit zero on the last node and optional second state.
uintptr_t SetFlag(Sequence* self, uint32_t enabled);
// TS3W.exe 0x00565FE0: Read bit zero, retaining the sentinel's high bytes on empty.
uintptr_t GetFlag(Sequence* self);
// TS3W.exe 0x00566010: Short-circuit child slot 11 queries (only AL is defined).
bool Query11(Sequence* self);
// TS3W.exe 0x00566040: Short-circuit child slot 12 queries (only AL is defined).
bool Query12(Sequence* self);
// TS3W.exe 0x00566170: Allocate and append a delegate, returning success in AL.
bool AddDelegate(Delegates* self, void* callback, uint32_t key);
// TS3W.exe 0x005661D0: Clear state before releasing its non-atomic controller reference.
uintptr_t Reset(State* self);
// TS3W.exe 0x00566210: Copy state, retaining the controller before copying scalars.
State* Copy(State* self, const State* source);
}

// Native member facade for hookgen's thiscall adapters; no extra object state.
namespace ts3::anim {
class b09 {
public:
    float Forward7() { return b09impl::Forward7(reinterpret_cast<Controller*>(this)); }
    float Forward8() { return b09impl::Forward8(reinterpret_cast<Controller*>(this)); }
    static void* FindFactory(uint32_t key) { return b09impl::FindFactory(key); }
    static Controller* ReadController(void* argument, void* reader) { return b09impl::ReadController(argument,static_cast<b09impl::Reader*>(reader)); }
    static uint32_t RegisterFactory(uint32_t key, void* factory) { return b09impl::RegisterFactory(key,factory); }
    uintptr_t WriteFloat(float value) { return b09impl::WriteFloat(reinterpret_cast<b09impl::Reader*>(this),value); }
    float MinusOne() { return b09impl::MinusOne(reinterpret_cast<b09impl::Sequence*>(this)); }
    Controller* Active() { return b09impl::Active(reinterpret_cast<b09impl::Sequence*>(this)); }
    uintptr_t SetFlag(uint32_t enabled) { return b09impl::SetFlag(reinterpret_cast<b09impl::Sequence*>(this),enabled); }
    uintptr_t GetFlag() { return b09impl::GetFlag(reinterpret_cast<b09impl::Sequence*>(this)); }
    bool Query11() { return b09impl::Query11(reinterpret_cast<b09impl::Sequence*>(this)); }
    bool Query12() { return b09impl::Query12(reinterpret_cast<b09impl::Sequence*>(this)); }
    bool AddDelegate(void* callback,uint32_t key) { return b09impl::AddDelegate(reinterpret_cast<b09impl::Delegates*>(this),callback,key); }
    uintptr_t Reset() { return b09impl::Reset(reinterpret_cast<b09impl::State*>(this)); }
    b09impl::State* Copy(void* source) { return b09impl::Copy(reinterpret_cast<b09impl::State*>(this),static_cast<b09impl::State*>(source)); }
};
}
