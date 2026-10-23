#include "ts3/videorecording/cls_9da5d0_VideoRecording.h"

#if defined(_WIN32)
#include <intrin.h>
#endif

namespace ts3::videorecording {

namespace {
// Game-image addresses the constructor stamps into the object (TS3W.exe, x86).
constexpr uintptr_t kBaseVtable = 0x00FEFEC0u;  // IDA's vt_PhysicsWorld
constexpr uintptr_t kVtable = 0x01051EDCu;  // vt_Class_01051EDC (slot 0 = 0x009DA640)
constexpr uintptr_t kListName = 0x01051EECu;  // "VideoRecording/CompletedAudioBuffers"
}  // namespace

// 0x009DA5D0, __thiscall (this in ecx), plain ret, returns this in eax.
VideoRecording* VideoRecording::Construct()
{
    // The inlined base constructor installs the base vtable first; the derived vtable
    // goes in at 0x009DA600, after the scalar fields are cleared. Only the second value
    // is ever observable and no test can see the first, but the order is kept.
    vtable = reinterpret_cast<void* const*>(kBaseVtable);
    // the original instruction sequence with eax = 0: an atomic store of 0. The old
    // value lands in eax and is dropped.
    _InterlockedExchange(reinterpret_cast<volatile long*>(&lockWord), 0);
    field08 = 0;
    // Three single-byte stores (cl = 0); +0x0F is deliberately left alone.
    flag0c = 0;
    flag0d = 0;
    flag0e = 0;
    field10 = 0;
    field14 = 0;
    field18 = 0;
    field20 = 0;
    field24 = 0;
    vtable = reinterpret_cast<void* const*>(kVtable);
    field1c = 0.0f;  // `movss` from a zeroed xmm0: the +0.0 bit pattern.
    blockBegin = blocks;
    blockEnd = blocks;
    blockLimit = blocks + sizeof blocks;  // the original instruction sequence
    listName = reinterpret_cast<const char*>(kListName);
    listNext = &listNext;  // the original instruction sequence / the original instruction sequence with eax = this+0xBC
    listPrev = &listNext;
    // clu_4e17e0_ResourceSystem (not rewritten): __thiscall on the embedded lock with two
    // stack arguments and `ret 8`. The original pushes 1 first, then the zeroed ecx, so
    // the callee sees (a2 = 0, a3 = 1).
    CallGameMethod<void, 0x004E17E0>(lock, 0, 1);
    return this;
}

// 0x009DA640, __thiscall (this in ecx), one 4-byte stack argument, `ret 4`; vtable slot
// 0 of 0x01051EDC. Returns this in eax even when the object has just been freed.
VideoRecording* VideoRecording::DeletingDtor(uint32_t flags)
{
    // The shared destructor body 0x009DA580 (a function of no scheduled unit) frees the
    // list nodes at +0xBC and destroys the lock through 0x004E1670; the original calls it
    // with this still in ecx and drops its return value.
    CallGameMethod<void, 0x009DA580>(this);
    // the original instruction sequence: the flag argument is examined as a byte, bit 0 only.
    if ((flags & 1u) != 0)
        CallGameFunction<void, 0x004E3F70>(static_cast<void*>(this));  // operator delete
    return this;
}

}  // namespace ts3::videorecording
