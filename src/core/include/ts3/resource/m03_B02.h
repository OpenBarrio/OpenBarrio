// Stage 3 m03 batch B02: the shared resource-pool lifetime pair (0x00734000/0x00734030),
// the shadow-primary watcher identity reader (0x007343B0) and the watched-key filtered
// dispatch (0x00733DA0). Bounded used views over the verified m03 accounting views only;
// no complete class or allocation-size claims.
#pragma once
#include "ts3/resource/m03_delivery_accounting.h" // PoolView, WatcherView (verified views)
#include "ts3/resource/compressed_index.h"        // ts3::resource::ResourceKey
#include "ts3/layout.h"

namespace ts3::resource::m03::system {

// ---------------------------------------------------------------------------
// 0x007343B0 - shadow-primary identity reader (vtable 00FFE0D0 slot 2, only xref)
// ---------------------------------------------------------------------------
// TS3W.exe 0x007343B0: the original instruction sequence - thiscall adjustor: return the full
// identity word of the reader subobject at WatcherView+0x24 (reader base +8 plus the
// reader's +0x1C field). No other access, no writes; full EAX.
void* WatcherIdentity(ts3::resource::m03::accounting::WatcherView* watcher);

// ---------------------------------------------------------------------------
// cls_402010_x pool (vtable 00FFDFC8): deleting destructor and slot allocation.
// The pool reuses the verified accounting PoolView (raw critical section at +8,
// 0x80000 bytes of 128-byte slots at +0x28, 128-word allocation bitmap at +0x80028).
// ---------------------------------------------------------------------------
constexpr uintptr_t kPoolVtable = 0x00FFDFC8;     // vt_00FFDFC8 (slot 0 = 0x00734000)
constexpr uintptr_t kPoolBaseVtable = 0x00F97FD4; // base table restamped during destruction

// Only actual external seams, never a provider for the subject algorithm. Null hooks use
// the real seams: DeleteCriticalSection and the verified ts3::scratch::Free4E3F70 (x86;
// native hosts must inject freeBlock because 0x011CB864 is game state).
struct PoolDestroyBoundary {
    void* context = nullptr;
    void* allocatorIdentity = nullptr; // observed by freeBlock (x86: 0x011CB864's pointee)
    void (*deleteSection)(void* context, void* critical) = nullptr;
    void (*freeBlock)(void* context, void* allocator, void* block) = nullptr;
};
// TS3W.exe 0x00734000: thiscall ret 4 returning this. Stamps 0x00FFDFC8 at +0, deletes
// the raw critical section at +8, restamps 0x00F97FD4, then frees this when flag bit 0
// is set (the flag byte only; bit 0 lives in the low byte, so uint32 bit 0 is exact).
void* PoolDeletingDtor(ts3::resource::m03::accounting::PoolView* pool, uint32_t flags,
                       const PoolDestroyBoundary& boundary = {});

// Only actual external seams: raw-section enter/leave and the global-allocator call
// 0x004E6A20(this = *0x011CB864, a0, a3, a4, a2). Null hooks use the real seams on x86;
// native hosts must inject allocate (there is no game allocator behind exehost).
struct PoolAllocBoundary {
    void* context = nullptr;
    void* allocatorIdentity = nullptr; // observed by allocate (x86: 0x011CB864's pointee)
    void (*enter)(void* context, void* critical) = nullptr;
    void (*leave)(void* context, void* critical) = nullptr;
    void* (*allocate)(void* context, void* allocator, uint32_t a0, uint32_t a3, void* a4,
                      void* a2) = nullptr;
};
// TS3W.exe 0x00734030: thiscall, five stack arguments, ret 0x14. a1 is proven unread.
// Out-of-range a0 (> 0x80) or a3 (> 4), or a fully zero 128-word bitmap, tail-calls the
// external allocator and returns its full EAX (leave runs first only on the full-scan
// path; the range guard never locks). Otherwise: under the raw critical section find the
// first nonzero bitmap word, clear its lowest set bit, leave, and return
// pool + 0x28 + (wordIndex*32 + bitIndex)*128. EA's low-word shift chain computes the
// bit index of an isolated power of two, which is exactly its count of trailing zeros.
void* AllocatePool(ts3::resource::m03::accounting::PoolView* pool, uint32_t a0, void* unused1,
                   void* a2, uint32_t a3, void* a4, const PoolAllocBoundary& boundary = {});

// ---------------------------------------------------------------------------
// 0x00733DA0 - filtered delivery of one watched resource (vtable 00FFE2F0 slot 1;
// sole xref is the table word at 0x00FFE2F4, so thiscall/ret 0x14 is the table ABI).
// ---------------------------------------------------------------------------
// +8 embedded adjusted interface subobject: only its table word is read (slot 17,
// byte offset 0x44, the manager key-membership family).
struct DispatchInterfaceView {
    void* table;
};
struct DispatchOwnerView {
    unsigned char unused00[8];
    DispatchInterfaceView interface; // +8
};
TS3_X86_OFFSET(DispatchOwnerView, interface, 8);
// Delivery target: only its table word is read (slot 13, byte offset 0x34).
struct DispatchTargetView {
    void* table;
};
#if defined(_M_IX86)
using DispatchLookupSlot = void* (__thiscall*)(void* self, const void* key);
using DispatchDeliverSlot = void* (__thiscall*)(void* self, const void* key, const void* arg2,
                                                const void* arg3, const void* arg4,
                                                uint32_t one, uint32_t zero);
#else
using DispatchLookupSlot = void* (*)(void* self, const void* key);
using DispatchDeliverSlot = void* (*)(void* self, const void* key, const void* arg2,
                                      const void* arg3, const void* arg4, uint32_t one,
                                      uint32_t zero);
#endif
// Actual external seam only: 0x007D80E0's normalized-key construction (manager lookup +
// 0x7D7580 candidate + 0x0054DD80 difference gate). It is not rewritten as a callable
// unit, so x86 bridges the original; a native host must inject resolveKey. The hook
// copies exactly 16 key bytes into out and returns out.
struct DispatchBoundary {
    void* context = nullptr;
    void* (*resolveKey)(void* context, ResourceKey* out, const ResourceKey* in) = nullptr;
};
// TS3W.exe 0x00733DA0: build a 16-byte key temp with 0x007D80E0 (cdecl 2-arg
// call with caller cleanup; the original is still todo, so x86 bridges it), then:
// - target != null: deliver target->slot13(&temp, a1, a3, a4, 1, 0); return its full EAX.
// - target == null: look up owner+8 slot 17(&temp); when it returns null, clear only AL
//   of that result (it is zero, so the return is exactly null) and return it.
// - otherwise deliver found->slot13(&temp, a1, a3, a4, 1, 0) and return its full EAX.
//   Both convergent delivery paths (direct jump 0x733DBE and lookup jump 0x733DD2) reach
//   the shared block 0x733DDD with ESP = entry-0x14: the real interface slot 17 is
//   0x004AFDA0 (thiscall `ret 4`), so the lookup's one pushed argument is popped by the
//   callee and both paths push the SAME six operands. An earlier revision assumed an
//   ESP discrepancy (misaligned &temp+4, input key, null target on the lookup path);
//   the disassembly and 0x004AFDA0's `ret 4` disprove it.
void* DispatchFiltered(DispatchOwnerView* owner, const ResourceKey* key, void* pass1,
                       DispatchTargetView* target, void* pass3, void* pass4,
                       const DispatchBoundary& boundary = {});

#if defined(_M_IX86)
// ABI-only replacements: fastcall consumes ECX/self and dummy EDX, leaving
// exactly the original thiscall stack and ret-N cleanup contract.
void* __fastcall HookDispatch(DispatchOwnerView*, void*, const ResourceKey*, void*,
                             DispatchTargetView*, void*, void*);
void* __fastcall HookPoolDtor(accounting::PoolView*, void*, uint32_t);
void* __fastcall HookAllocate(accounting::PoolView*, void*, uint32_t, void*, void*, uint32_t, void*);
#endif

} // namespace ts3::resource::m03::system
