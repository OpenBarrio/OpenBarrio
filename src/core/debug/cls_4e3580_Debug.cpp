// Reimplementation of TS3W.exe 0x004E3580, 0x004E37C0, 0x004E3910, 0x004E3A00,
// 0x004E3A10, 0x004E3B90 and 0x007E94F0 (unit cls_4e3580_Debug: the debug TraceHelper).
#include "ts3/debug/cls_4e3580_Debug.h"

namespace ts3::debug {
namespace {

// vt_TraceHelper: slot[0]=Destruct, [1]=SetId, [6]=GetInfo, [7]=SetEnabled. The game
// always writes this address into the record's +0x00.
constexpr uintptr_t kVtableTraceHelper = 0x00FB8ABC;
// The two labels Construct substitutes for a null / assert name.
constexpr uintptr_t kNameAssert = 0x00FB8AB4;    // "Assert"
constexpr uintptr_t kNameUnknown = 0x00FB8AA8;   // "<Unknown>"

// Still-original dependencies the register/release paths reach through the bridge:
constexpr uint32_t kThreadKey = 0x00931020;      // current thread's slot key (cdecl, none)
constexpr uint32_t kGetSlot = 0x004E3760;        // get-or-create slot for a key (cdecl)
constexpr uint32_t kEaDelete = 0x004E3F70;       // EA operator delete (cdecl)

#if defined(_M_IX86)
// The manager object the global 0x011CB84C points at (its +0x10 cell is the thread slot).
TraceHelperManager* Manager()
{
    return *reinterpret_cast<TraceHelperManager**>(static_cast<uintptr_t>(0x011CB84C));
}
#endif

}  // namespace

// 0x004E37C0. `self` is the manager's +0x10 slot cell, not a TraceHelper. Builds the
// thread's slot (key from the cell, or the game's 0x00931020 when the key is 0), AddRefs
// the new slot, stores it, and Releases the one it replaced. The original reads `self`
// from ECX (thiscall) and ignores EDX, so the x86 entry is __fastcall with a discarded
// second register.
#if defined(_M_IX86)
TraceSlot* __fastcall TraceHelper::SwapThreadSlot(TraceSlotRef* self, void* edxDiscard)
#else
TraceSlot* TraceHelper::SwapThreadSlot(TraceSlotRef* self, void* edxDiscard)
#endif
{
    (void)edxDiscard;
    uint32_t key = self->key;
    if (!key)
        key = static_cast<uint32_t>(CallGameFunction<uintptr_t, kThreadKey>());
    TraceSlot* fresh = CallGameFunction<TraceSlot*, kGetSlot>(key);
    TraceSlot* old = self->slot;
    if (fresh != old) {
        if (fresh)
            CallGameVirtual<void, 0>(fresh);  // AddRef
        self->slot = fresh;
        if (old)
            CallGameVirtual<void, 1>(old);  // Release
    }
    return self->slot;
}

// The current thread's slot from the 0x011CB84C manager, building it through
// SwapThreadSlot when the cache is empty. The original dereferences the manager
// unconditionally (0x004E39B2 / 0x004E3A19), so a null manager faults in both sides
// alike - no guard is added here.
static TraceSlot* CurrentSlot()
{
#if defined(_M_IX86)
    TraceHelperManager* const mgr = Manager();
    TraceSlot* const slot = mgr->cell.slot;  // one read, like the original's the original instruction sequence
    if (slot)
        return slot;
    return TraceHelper::SwapThreadSlot(&mgr->cell, nullptr);
#else
    return nullptr;
#endif
}

// 0x004E3910. Fills the record and hands it to the per-thread trace server (slot [6]);
// when the server is absent it marks the record unregistered instead. Returns this.
TraceHelper* TraceHelper::Construct(int32_t type_, const char* name_, int32_t interval_,
                                    const int32_t infoBlock[3])
{
    name = name_;  // may be overwritten by the type switch below
#if defined(_M_IX86)
    vtable_ = reinterpret_cast<void*>(kVtableTraceHelper);
#endif
    registered = 1;
    flag05 = 1;
    flag06 = 0;
    type = type_;
    level = 0;
    interval = interval_;
    info[0] = infoBlock[0];
    info[1] = infoBlock[1];
    info[2] = infoBlock[2];
    field24 = 0;
    switch (type_) {
        case 0:
            name = reinterpret_cast<const char*>(kNameAssert);
            [[fallthrough]];
        case 1:
            level = 3;
            if (interval_ == 0)
                interval = 150;
            break;
        case 2:
            level = 1;
            if (interval_ == 0)
                interval = 25;
            break;
        case 3:
            level = 3;
            interval = 150;
            break;
        default:
            level = 1;
            break;
    }
    if (!name)
        name = reinterpret_cast<const char*>(kNameUnknown);
#if defined(_M_IX86)
    if (TraceSlot* const slot = CurrentSlot()) {
        CallGameVirtual<void, 6>(slot, this);  // register the record with the server
        return this;
    }
#endif
    registered = 0;
    flag06 = 1;
    return this;
}

// 0x004E3580 (vtable slot 1). Stores the id and resets the two flag bytes; returns id.
int32_t TraceHelper::SetId(int32_t id)
{
    field24 = id;
    flag05 = 1;
    flag06 = 0;
    return id;
}

// 0x004E3A00 (vtable slot 7). Sets the registered flag to the argument's low byte and
// returns that byte.
uint8_t TraceHelper::SetEnabled(uint8_t enabled)
{
    registered = enabled;
    return enabled;
}

// 0x007E94F0 (vtable slot 6). Returns the address of the 3-dword info block.
int32_t* TraceHelper::GetInfo()
{
    return &info[0];
}

// 0x004E3A10. Sets the vtable and releases the record from the server (slot [7]); no
// delete. Returns the slot's release result, or 0 when there is no server: the original
// (0x004E3A32-0x004E3A3D) leaves that in eax and its caller 0x00935FA0 returns it on.
void* TraceHelper::Release()
{
#if defined(_M_IX86)
    vtable_ = reinterpret_cast<void*>(kVtableTraceHelper);
    TraceSlot* const slot = CurrentSlot();
    if (!slot)
        return nullptr;
    return CallGameVirtual<void*, 7>(slot, this);  // release the record from the server
#else
    return nullptr;
#endif
}

// 0x004E3B90 (vtable slot 0). Release's body, then the game's operator delete when the
// low flag bit is set. Returns this (the standard scalar-deleting-destructor contract).
TraceHelper* TraceHelper::Destruct(uint32_t flags)
{
    Release();
    if (flags & 1u)
        CallGameFunction<void, kEaDelete>(static_cast<void*>(this));
    return this;
}

}  // namespace ts3::debug
