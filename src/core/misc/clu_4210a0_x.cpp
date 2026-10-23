// Cluster unit clu_4210a0_x (misc): a grab bag of 40 functions spread over the whole
// image. Originals: 0x004210A0 (5 B), 0x004210B0 (90), 0x004213B0 (203), 0x004215E0
// (158), 0x00421930 (111), 0x0042D5E0 (279), 0x0042D700 (216), 0x0042DDA0 (432),
// 0x004303C0 (27), 0x00436630 (24), 0x004961F0 (151), 0x004DE950 (64), 0x004DE990 (84),
// 0x004E93E0 (167), 0x004FAAD0 (125), 0x006D5390 (73), 0x00C2FD50 (161), the fifteen
// 0x00DA2780..0x00DE3A60 protected script-slot wrappers (~180 B each), 0x00E7A300,
// 0x00E7A330, 0x00EB9040, 0x00EB9400, 0x00F1D6D0, 0x00F4B720 and the two MSVCR80
// import thunks 0x00F7E8A6 (memcpy) and 0x00F7EE92 (_setjmp3).
//
// Two shapes dominate:
//  * plain memory/object helpers, written against raw int32 addresses because the game
//    hands 32-bit object pointers out of its own fields (x86 builds keep them whole);
//  * the fifteen protected slot wrappers: cdecl (a1 unused, a2 = error out, a3 = context
//    whose +0x10 holds the active setjmp frame, a4 = result out). They build a small
//    frame, call the global function pointer at *(global) when 0x011F2D84 <= 0 and under a
//    _setjmp3 frame otherwise, restoring the context slot either way. On a longjmp the
//    original re-reads a prologue-zeroed frame word that the slot can never reach (the
//    slot receives values, never the frame address), so that path always yields 0 - the
//    same simplification clu_de6ed0_Scripting uses. The five DAA260-family wrappers are
//    the exception: there the longjmp path returns a3 verbatim.
#include "ts3/misc/clu_4210a0_x.h"

#include <cstring>

#include "ts3/layout.h"

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

namespace ts3::misc {
namespace {

constexpr int32_t kZero = 0;
// 0x011F2D84: > 0 wraps script calls in a setjmp3 protection frame.
constexpr uint32_t kProtectedFlag = 0x011F2D84;
// 0x00F95654 / 0x00F956D8: the MSVCR80 import slots the two 6-byte thunks jump through.
constexpr uint32_t kMemcpyIat = 0x00F95654;
constexpr uint32_t kSetjmp3Iat = 0x00F956D8;
// 0x00F956E0 / 0x00F956E4: the __imp_malloc / __imp_free slots used by 0x00F4B720.
constexpr uint32_t kMallocIat = 0x00F956E0;
constexpr uint32_t kFreeIat = 0x00F956E4;
// 0x00F951F4..0x00F95838: the kernel32/user32 import slots 0x004E93E0 calls through;
// the test stubs them in the mapped image so both sides run without the OS clipboard.
constexpr uint32_t kClipFreeIat = 0x00F951F4;    // GlobalFree
constexpr uint32_t kClipAllocIat = 0x00F951F8;   // GlobalAlloc
constexpr uint32_t kClipUnlockIat = 0x00F95264;  // GlobalUnlock
constexpr uint32_t kClipLockIat = 0x00F95268;    // GlobalLock
constexpr uint32_t kClipOpenIat = 0x00F9582C;    // OpenClipboard
constexpr uint32_t kClipEmptyIat = 0x00F95830;   // EmptyClipboard
constexpr uint32_t kClipDataIat = 0x00F95834;    // SetClipboardData
constexpr uint32_t kClipCloseIat = 0x00F95838;   // CloseClipboard
// The shared error constructor of the script runtime.
constexpr uint32_t kErrorFn = 0xE770E0;
// jmp_buf storage the original reserves for _setjmp3.
constexpr int kEnvDwords = 16;
// The script-slot wrappers' stack frames: 18 dwords for the three-argument shape
// (P1/P3 protected frames), 11 for the two-argument shape (P2 protected frames).
constexpr int kSlotFrame3 = 18;
constexpr int kSlotFrame2 = 11;

// ---- raw game memory ----------------------------------------------------------------

inline int32_t LoadDword(int32_t address)
{
#if defined(_M_IX86)
    return *reinterpret_cast<const int32_t*>(static_cast<uintptr_t>(static_cast<uint32_t>(address)));
#else
    (void)address;
    AbortNotRewritten("a game dword");
    return kZero;
#endif
}

inline void StoreDword(int32_t address, int32_t value)
{
#if defined(_M_IX86)
    *reinterpret_cast<int32_t*>(static_cast<uintptr_t>(static_cast<uint32_t>(address))) = value;
#else
    (void)address;
    (void)value;
    AbortNotRewritten("a game dword");
#endif
}

inline void StoreByte(int32_t address, uint8_t value)
{
#if defined(_M_IX86)
    *reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(static_cast<uint32_t>(address))) = value;
#else
    (void)address;
    (void)value;
    AbortNotRewritten("a game byte");
#endif
}

inline uint8_t LoadByte(int32_t address)
{
#if defined(_M_IX86)
    return *reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(static_cast<uint32_t>(address)));
#else
    (void)address;
    AbortNotRewritten("a game byte");
    return 0;
#endif
}

inline uint16_t LoadWord(int32_t address)
{
#if defined(_M_IX86)
    return *reinterpret_cast<const uint16_t*>(static_cast<uintptr_t>(static_cast<uint32_t>(address)));
#else
    (void)address;
    AbortNotRewritten("a game word");
    return 0;
#endif
}

inline void StoreWord(int32_t address, uint16_t value)
{
#if defined(_M_IX86)
    *reinterpret_cast<uint16_t*>(static_cast<uintptr_t>(static_cast<uint32_t>(address))) = value;
#else
    (void)address;
    (void)value;
    AbortNotRewritten("a game word");
#endif
}

inline int32_t LoadGlobal(uint32_t address)
{
#if defined(_M_IX86)
    return *reinterpret_cast<const int32_t*>(static_cast<uintptr_t>(address));
#else
    (void)address;
    AbortNotRewritten("a game global");
    return kZero;
#endif
}

inline void* PtrOf(int32_t address)
{
#if defined(_M_IX86)
    return reinterpret_cast<void*>(static_cast<uintptr_t>(static_cast<uint32_t>(address)));
#else
    (void)address;
    AbortNotRewritten("a game pointer");
    return nullptr;
#endif
}

inline int32_t AddressOf(const void* pointer)
{
#if defined(_M_IX86)
    return static_cast<int32_t>(reinterpret_cast<intptr_t>(pointer));
#else
    (void)pointer;
    AbortNotRewritten("a game address");
    return kZero;
#endif
}

// Calls a game function pointer that is a plain value (stored in an object field or on the
// stack) rather than a fixed image address: the original's indirect calls.
template <class R, class... Args>
R CallFnPtr(int32_t fn, Args... args)
{
    using Fn = R(__cdecl*)(Args...);
    return reinterpret_cast<Fn>(static_cast<uintptr_t>(static_cast<uint32_t>(fn)))(args...);
}

// Calls *(global): the slot globals of the DA-family hold the address of the game
// callback, exactly like the original's the original instruction sequence.
template <class R, class... Args>
R CallSlotAt(uint32_t global, Args... args)
{
    return CallFnPtr<R>(LoadGlobal(global), args...);
}

void* CallMalloc(size_t size)
{
    using Fn = void*(__cdecl*)(size_t);
    return reinterpret_cast<Fn>(static_cast<uintptr_t>(static_cast<uint32_t>(LoadGlobal(kMallocIat))))(size);
}

void CallFree(void* block)
{
    using Fn = void(__cdecl*)(void*);
    reinterpret_cast<Fn>(static_cast<uintptr_t>(static_cast<uint32_t>(LoadGlobal(kFreeIat))))(block);
}

// The Win32 calls of 0x004E93E0 go through the same import slots as the original on x86;
// the x64 build (never a game) keeps the direct imports.
#if defined(_WIN32)
#if defined(_M_IX86)
template <class R, class... Args>
R CallStdcallAt(uint32_t global, Args... args)
{
    using Fn = R(__stdcall*)(Args...);
    return reinterpret_cast<Fn>(static_cast<uintptr_t>(static_cast<uint32_t>(LoadGlobal(global))))(args...);
}
BOOL ClipOpenClipboard(HWND owner) { return CallStdcallAt<BOOL>(kClipOpenIat, owner); }
BOOL ClipEmptyClipboard() { return CallStdcallAt<BOOL>(kClipEmptyIat); }
HGLOBAL ClipGlobalAlloc(UINT flags, SIZE_T size) { return CallStdcallAt<HGLOBAL>(kClipAllocIat, flags, size); }
void* ClipGlobalLock(HGLOBAL block) { return CallStdcallAt<void*>(kClipLockIat, block); }
BOOL ClipGlobalUnlock(HGLOBAL block) { return CallStdcallAt<BOOL>(kClipUnlockIat, block); }
HANDLE ClipSetClipboardData(UINT format, HANDLE data) { return CallStdcallAt<HANDLE>(kClipDataIat, format, data); }
HGLOBAL ClipGlobalFree(HGLOBAL block) { return CallStdcallAt<HGLOBAL>(kClipFreeIat, block); }
BOOL ClipCloseClipboard() { return CallStdcallAt<BOOL>(kClipCloseIat); }
#else
BOOL ClipOpenClipboard(HWND owner) { return OpenClipboard(owner); }
BOOL ClipEmptyClipboard() { return EmptyClipboard(); }
HGLOBAL ClipGlobalAlloc(UINT flags, SIZE_T size) { return GlobalAlloc(flags, size); }
void* ClipGlobalLock(HGLOBAL block) { return GlobalLock(block); }
BOOL ClipGlobalUnlock(HGLOBAL block) { return GlobalUnlock(block); }
HANDLE ClipSetClipboardData(UINT format, HANDLE data) { return SetClipboardData(format, data); }
HGLOBAL ClipGlobalFree(HGLOBAL block) { return GlobalFree(block); }
BOOL ClipCloseClipboard() { return CloseClipboard(); }
#endif
#endif

// ---- the protected slot wrappers ----------------------------------------------------

// P1/P3 tail: the slot gets (frame[0], frame[2], frame[4]); the longjmp path of the
// original returns the prologue-zeroed frame word 6 (always 0), so it is spelled 0 here.
int32_t RunSlot3(uint32_t slotGlobal, int32_t* frame, const int32_t* a2, int32_t* a4, int32_t ctx)
{
    int32_t result = kZero;
    if (LoadGlobal(kProtectedFlag) > kZero) {
        int32_t env[kEnvDwords] = {};
        const int32_t saved = LoadDword(ctx + 0x10);
        volatile int32_t sj = Setjmp3F7EE92(env, kZero);
        if (sj) {
            StoreDword(ctx + 0x10, saved);
            result = kZero;  // the original's frame[6], never written
        } else {
            StoreDword(ctx + 0x10, AddressOf(env));
            result = CallSlotAt<int32_t>(slotGlobal, frame[0], frame[2], frame[4]);
            StoreDword(ctx + 0x10, saved);
        }
    } else {
        result = CallSlotAt<int32_t>(slotGlobal, frame[0], frame[2], frame[4]);
    }
    if (!*a2)
        *a4 = result;
    return result;
}

// P1b/P3 tail: two slot arguments; longjmp yields the zeroed frame word 4 (always 0).
int32_t RunSlot2(uint32_t slotGlobal, int32_t* frame, const int32_t* a2, int32_t* a4, int32_t ctx)
{
    int32_t result = kZero;
    if (LoadGlobal(kProtectedFlag) > kZero) {
        int32_t env[kEnvDwords] = {};
        const int32_t saved = LoadDword(ctx + 0x10);
        volatile int32_t sj = Setjmp3F7EE92(env, kZero);
        if (sj) {
            StoreDword(ctx + 0x10, saved);
            result = kZero;  // the original's frame[4], never written
        } else {
            StoreDword(ctx + 0x10, AddressOf(env));
            result = CallSlotAt<int32_t>(slotGlobal, frame[0], frame[2]);
            StoreDword(ctx + 0x10, saved);
        }
    } else {
        result = CallSlotAt<int32_t>(slotGlobal, frame[0], frame[2]);
    }
    if (!*a2)
        *a4 = result;
    return result;
}

// P2 tail (0x00DAA260/0x00DAF9D0/0x00DBE9E0/0x00DC4320/0x00DE1760): the slot gets a
// pointer to the frame's small value block at frame[2]; the longjmp path returns a3, and
// the nothing-to-report path writes frame[outIndex] through a4 and returns a4 itself.
int32_t RunSlot2Out(uint32_t slotGlobal, int32_t* frame, int outIndex, const int32_t* a2, int32_t* a4,
                    int32_t ctx)
{
    int32_t result = kZero;
    if (LoadGlobal(kProtectedFlag) > kZero) {
        int32_t env[kEnvDwords] = {};
        const int32_t saved = LoadDword(ctx + 0x10);
        volatile int32_t sj = Setjmp3F7EE92(env, kZero);
        if (sj) {
            StoreDword(ctx + 0x10, saved);
            result = ctx;  // EA: the longjmp path leaves the context in EAX
        } else {
            StoreDword(ctx + 0x10, AddressOf(env));
            result = CallSlotAt<int32_t>(slotGlobal, frame[0], frame[2]);
            StoreDword(ctx + 0x10, saved);
        }
    } else {
        result = CallSlotAt<int32_t>(slotGlobal, frame[0], frame[2]);
    }
    if (!*a2) {
        *a4 = frame[outIndex];
        return AddressOf(a4);
    }
    return result;
}

// P3 prologue: run the unit-local lock (0x00E75260) and the 0x00EA3140 lookup that turns
// the raw argument into the handle the slot accepts, then store it back into frame[2].
int32_t LockedHandle(uint32_t objGlobal, int32_t* frame)
{
    const int32_t lock = CallGameFunction<int32_t, 0xE75260>();
    return CallGameFunction<int32_t, 0xEA3140>(lock, LoadGlobal(objGlobal),
                                               AddressOf(&frame[2]));
}

}  // namespace

// 0x004210A0: pure tail thunk to 0x0041E8A0 (clu_41e8a0_x, not rewritten yet).
int32_t TS3_MISC_CDECL Dispatch4210A0(int32_t a1)
{
    return CallGameFunction<int32_t, 0x41E8A0>(a1);
}

// 0x004210B0: script field setter. Index 254 writes the +0xC word of the object's
// 0x1B4-pointer; 224..239 write the +0x1B4 table; anything else pokes error 67 into the
// context and dispatches through the context's vtable[0]. EA quirk (affects the return
// contract): the three paths return the object, the table pointer and the dispatcher's
// result respectively, not the value.
int32_t TS3_MISC_CDECL SetField4210B0(int32_t obj, int32_t index, int32_t value)
{
    if (index == 254) {
        StoreDword(LoadDword(obj + 0x1B4) + 0x0C, value);
        return obj;
    }
    if (static_cast<uint32_t>(index - 224) <= 0xF) {
        const int32_t table = LoadDword(obj + 0x1B4);
        StoreDword(table + 4 * index - 0x370, value);
        return table;
    }
    StoreDword(LoadDword(obj) + 0x14, 67);
    StoreDword(LoadDword(obj) + 0x18, index);
    return CallFnPtr<int32_t>(LoadDword(LoadDword(obj)), obj);
}

// 0x004213B0: one step of the streaming state machine. States 205/206 finish by calling
// the +0x1A0 handler's +4 callback and moving to 210; state 207 just moves to 210; any
// other state raises error 18. Then the +0x1B0 dispatcher is drained until it reports
// completion, after which the +0x14 handler's +0x18 callback and 0x0041E870 run; returns
// 1 on that path and 0 when the dispatcher never completes.
int32_t TS3_MISC_CDECL Step4213B0(int32_t obj)
{
    const int32_t state = LoadDword(obj + 0x10);
    if ((state == 205 || state == 206) && LoadDword(obj + 0x40) == 0) {
        if (static_cast<uint32_t>(LoadDword(obj + 0x8C)) < static_cast<uint32_t>(LoadDword(obj + 0x74))) {
            StoreDword(LoadDword(obj) + 0x14, 66);
            CallFnPtr<void>(LoadDword(LoadDword(obj)), obj);
        }
        CallFnPtr<void>(LoadDword(LoadDword(obj + 0x1A0) + 4), obj);
        StoreDword(obj + 0x10, 210);
    } else if (state == 207) {
        StoreDword(obj + 0x10, 210);
    } else if (state != 210) {
        StoreDword(LoadDword(obj) + 0x14, 18);
        StoreDword(LoadDword(obj) + 0x18, state);
        CallFnPtr<void>(LoadDword(LoadDword(obj)), obj);
    }
    while (LoadDword(LoadDword(obj + 0x1B0) + 0x14) == 0) {
        if (CallFnPtr<int32_t>(LoadDword(LoadDword(obj + 0x1B0)), obj) == 0)
            return 0;
    }
    CallFnPtr<void>(LoadDword(LoadDword(obj + 0x14) + 0x18), obj);
    CallGameFunction<void, 0x41E870>(obj);
    return 1;
}

// 0x004215E0: reads through the object's +0x1A4 callback into a scratch slot, advances
// the +0x8C position by the callback's out value and returns it as a pointer (the caller
// then dereferences the first dword). The overrun path raises error 119 through the
// context's vtable slot 1 and returns null. The +0x8 object gets the window bounds first.
int32_t* TS3_MISC_CDECL ReadBlock4215E0(int32_t obj, int32_t a2, int32_t a3)
{
    if (LoadDword(obj + 0x10) != 205) {
        StoreDword(LoadDword(obj) + 0x14, 18);
        StoreDword(LoadDword(obj) + 0x18, LoadDword(obj + 0x10));
        CallFnPtr<void>(LoadDword(LoadDword(obj)), obj);
    }
    const uint32_t position = static_cast<uint32_t>(LoadDword(obj + 0x8C));
    if (position < static_cast<uint32_t>(LoadDword(obj + 0x74))) {
        const int32_t writer = LoadDword(obj + 8);
        if (writer != 0) {
            StoreDword(writer + 4, static_cast<int32_t>(position));
            StoreDword(LoadDword(obj + 8) + 8, LoadDword(obj + 0x74));
            CallFnPtr<void>(LoadDword(writer), obj);
        }
        int32_t out = 0;
        CallFnPtr<void>(LoadDword(LoadDword(obj + 0x1A4) + 4), obj, a2, AddressOf(&out), a3);
        StoreDword(obj + 0x8C, LoadDword(obj + 0x8C) + out);
        return reinterpret_cast<int32_t*>(PtrOf(out));
    }
    StoreDword(LoadDword(obj) + 0x14, 119);
    CallFnPtr<void>(LoadDword(LoadDword(obj) + 4), obj, -1);
    return nullptr;
}

// 0x00421930: lazily allocates the 0x28-byte sub-object (through the allocator callback at
// *(*(obj+4)) named 0x421860-family handlers) and a 0x1000-byte buffer, then installs the
// six handler pointers. The stored constants are game code addresses, hence the x86-only
// raw store; returns the sub-object.
int32_t TS3_MISC_CDECL InitSub421930(int32_t obj, int32_t a2)
{
    if (LoadDword(obj + 0x14) == 0) {
        const int32_t alloc = LoadDword(LoadDword(obj + 4));
        const int32_t sub = CallFnPtr<int32_t>(alloc, obj, 0, 0x28);
        StoreDword(obj + 0x14, sub);
        const int32_t alloc2 = LoadDword(LoadDword(obj + 4));
        const int32_t buffer = CallFnPtr<int32_t>(alloc2, obj, 0, 0x1000);
        StoreDword(sub + 0x20, buffer);
    }
    const int32_t result = LoadDword(obj + 0x14);
    StoreDword(result + 8, 0x421860);
    StoreDword(result + 0x0C, 0x421870);
    StoreDword(result + 0x10, 0x4218F0);
    StoreDword(result + 0x14, 0x426400);
    StoreDword(result + 0x18, 0x00C0FD60);
    StoreDword(result + 0x1C, a2);
    StoreDword(result + 4, 0);
    StoreDword(result, 0);
    return result;
}

// 0x0042D5E0: this-in-ECX (the source pixel) plus one stack out-buffer, caller-cleaned
// stack. Per channel t = a + (v-255)*a/255 (signed truncating division, x86 magic
// sequence), clamped to 255; writes out[0..2] only, returns the blue channel's value.
int32_t TS3_MISC_FASTCALL Shade42D5E0(const uint8_t* src, [[maybe_unused]] void* edx, uint8_t* out)
{
    const int32_t alpha = static_cast<int32_t>(src[3]);
    int32_t result = kZero;
    for (int channel = 0; channel < 3; ++channel) {
        const int32_t v = static_cast<int32_t>(src[channel]) - 255;
        int32_t t = alpha + v * alpha / 255;
        if (t >= 255)
            t = 255;
        out[channel] = static_cast<uint8_t>(t);
        result = t;
    }
    return result;
}

// 0x0042D700: private packed-pixel converter; the original keeps dst in EAX and src in
// EDX with size, mode, flag on the stack and a plain ret (the caller drops the arguments).
// flag != 0 reverses every 4-byte group; mode 1 copies; mode 4 converts through
// 0x0042D5E0 replacing the fourth byte with 0xFF; anything else spreads 3-byte pixels
// into 4-byte ones with 0xFF. The EAX tails below mirror the original's even where the
// value is only reachable through mode-4's scratch-dword address (the test compares that
// byte): the original leaves EAX = mode-4 on the empty-loop paths and dst+2+4*size or
// src+2+4*size after the copy loops.
int32_t TS3_MISC_CDECL ConvertPixels42D700(uint8_t* dst, const char* src, int32_t size,
                                          int32_t mode, int32_t flag)
{
    int32_t result = AddressOf(dst);
    if (flag != 0) {
        if (size > 0) {
            for (int32_t i = 0; i < size; ++i) {
                dst[4 * i + 3] = static_cast<uint8_t>(src[4 * i + 0]);
                dst[4 * i + 2] = static_cast<uint8_t>(src[4 * i + 1]);
                dst[4 * i + 1] = static_cast<uint8_t>(src[4 * i + 2]);
                dst[4 * i + 0] = static_cast<uint8_t>(src[4 * i + 3]);
            }
            result = AddressOf(src + 2 + 4 * size);
        }
    } else if (mode == 1) {
        result = AddressOf(std::memcpy(dst, src, static_cast<size_t>(static_cast<uint32_t>(size))));
    } else if (mode == 4) {
        result = mode - 4;  // the original leaves mode-4 in EAX when the loop does not run
        if (size > 0) {
            for (int32_t i = 0; i < size; ++i) {
                int32_t packed = 0;
                Shade42D5E0(reinterpret_cast<const uint8_t*>(src + 4 * i), nullptr,
                            reinterpret_cast<uint8_t*>(&packed));
                const uint8_t b0 = static_cast<uint8_t>(packed);
                const uint8_t b1 = static_cast<uint8_t>(static_cast<uint32_t>(packed) >> 8);
                const uint8_t b2 = static_cast<uint8_t>(static_cast<uint32_t>(packed) >> 16);
                dst[4 * i + 3] = 0xFF;
                dst[4 * i + 2] = b0;
                dst[4 * i + 1] = b1;
                dst[4 * i + 0] = b2;
                // The original's EAX keeps its scratch dword's address and only replaces
                // the low byte with the third channel, exactly this shape.
                result = (AddressOf(&packed) & ~0xFF) | static_cast<int32_t>(b2);
            }
        }
    } else {
        result = mode - 4;  // the original leaves mode-4 in EAX when the loop does not run
        if (size > 0) {
            for (int32_t i = 0; i < size; ++i) {
                const char* pixel = src + 3 * i;
                dst[4 * i + 3] = 0xFF;
                dst[4 * i + 2] = static_cast<uint8_t>(pixel[0]);
                dst[4 * i + 1] = static_cast<uint8_t>(pixel[1]);
                dst[4 * i + 0] = static_cast<uint8_t>(pixel[2]);
            }
            result = AddressOf(dst + 2 + 4 * size);
        }
    }
    return result;
}

// 0x0042DDA0 (stdcall, ret 0x10): builds the 0x1D0-byte scripting context in a stack
// frame, protects the whole setup with _setjmp3 (longjmp -> 0x004210A0 cleanup and return
// 0), asks its allocator for v13*v15 bytes and converts v14 rows of 4-byte pixels through
// 0x0042D700. The setjmp handler word 0x0042D8E0 and the row callback 0x0042D7E0 are
// other units' code addresses, stored as raw dwords like the original does.
int32_t TS3_MISC_STDCALL ConvertRegion42DDA0(int32_t a1, int32_t a2, uint8_t* a3, int32_t a4)
{
    int32_t env[33] = {};        // original [ebp-0x238]: setjmp handler block
    int32_t context[0x1D0 / 4] = {};  // original [ebp-0x174]: the 0x1D0-byte context
    int32_t jmpbuf[kEnvDwords] = {};
    context[0] = CallGameFunction<int32_t, 0x428C40>(AddressOf(env));
    env[0] = 0x42D8E0;
    volatile int32_t sj = Setjmp3F7EE92(jmpbuf, kZero);
    if (sj) {
        Dispatch4210A0(AddressOf(context));
        return 0;
    }
    CallGameFunction<void, 0x420FB0>(context, 61, 464);
    SetField4210B0(AddressOf(context), 237, 0x42D7E0);
    CallGameFunction<void, 0x446220>(LoadDword(a1 + 16), 0, 0);
    InitSub421930(AddressOf(context), LoadDword(a1 + 16));
    StoreDword(0x011B02A4, 0);
    CallGameFunction<void, 0x421480>(context, 1);
    const int32_t v17 = LoadDword(0x011B02A4);
    int32_t mode = context[0x28 / 4];
    if (context[0x24 / 4] == 3)
        mode = 2;
    else if (context[0x24 / 4] == 5)
        mode = 4;
    CallGameFunction<void, 0x421680>(context);
    const int32_t v16 = context[0x70 / 4];
    const int32_t strideFn = LoadDword(LoadDword(AddressOf(context) + 4) + 8);
    const int32_t rowDesc = CallFnPtr<int32_t>(strideFn, AddressOf(context), 1,
                                               context[0x70 / 4] * context[0x7C / 4], 1);
    for (int32_t i = 0; i < LoadDword(AddressOf(context) + 0x74); ++i) {
        ReadBlock4215E0(AddressOf(context), rowDesc, 1);
        if (i < LoadDword(a2 + 20)) {
            ConvertPixels42D700(a3, reinterpret_cast<const char*>(PtrOf(LoadDword(rowDesc))), v16,
                                mode, v17);
            a3 += a4;
        }
    }
    Step4213B0(AddressOf(context));
    Dispatch4210A0(AddressOf(context));
    return 1;
}

// 0x004303C0: memcpy(dst, src, count << 7); the shift is the original's 32-bit shl.
void* TS3_MISC_CDECL CopyShifted4303C0(const void* src, void* dst, int32_t count)
{
    return std::memcpy(dst, src, static_cast<size_t>(static_cast<uint32_t>(count) << 7));
}

// 0x00436630: memcpy(dst, src, size); the first argument is dead in the original too.
void* TS3_MISC_CDECL CopyPacked436630([[maybe_unused]] int32_t unused, void* dst, const void* src, size_t size)
{
    return std::memcpy(dst, src, size);
}

// 0x004961F0: allocates the 572-byte stream object (through 0x0049AFC0) and initializes
// it; when a4 is non-null the allocation runs under a _setjmp3 frame at a1+0x10 whose
// failure path calls 0x0049AF70 and returns 0. The byte at +0x238 and the byte at +0x228
// (=4) are the object's mode fields.
int32_t TS3_MISC_CDECL CreateStream4961F0(int32_t a1, int32_t a2, int32_t a3, int32_t* a4)
{
    if (a4 != nullptr) {
        const int32_t sj = Setjmp3F7EE92(PtrOf(a1 + 0x10), kZero);
        *a4 = sj;
        if (sj != 0) {
            CallGameFunction<void, 0x49AF70>(a1);
            return 0;
        }
    }
    const int32_t object = CallGameFunction<int32_t, 0x49AFC0>(a1, 572);
    StoreDword(object + 0x22C, a1);
    StoreDword(object + 4, 0);
    StoreDword(object + 8, 0);
    StoreDword(object + 0x214, 0);
    StoreDword(object + 0x218, 0);
    StoreDword(object + 0x21C, 0);
    StoreDword(object + 0x224, 0);
    StoreDword(object + 0x230, 0);
    StoreDword(object + 0x234, 0);
    StoreByte(object + 0x238, 0);
    StoreDword(object, a2);
    StoreDword(object + 0x220, a3);
    StoreByte(object + 0x228, 4);
    return object;
}

// 0x004DE950: this-in-ECX (window) + two stack args (ret 8). Copies min(len,
// end - position) bytes out of base+position, advances position and returns the count;
// len 0 or an exhausted window returns 0.
int32_t TS3_MISC_FASTCALL ReadWindow4DE950(void* self, [[maybe_unused]] void* edx, void* dst, int32_t len)
{
    if (len == 0)
        return 0;
    const int32_t base = AddressOf(self);
    const uint32_t position = static_cast<uint32_t>(LoadDword(base + 0x14));
    const uint32_t available = static_cast<uint32_t>(LoadDword(base + 0x0C)) - position;
    if (available == 0)
        return 0;
    uint32_t count = static_cast<uint32_t>(len);
    if (count > available)
        count = available;
    std::memcpy(dst, PtrOf(LoadDword(base + 4) + static_cast<int32_t>(position)), count);
    StoreDword(base + 0x14, static_cast<int32_t>(position + count));
    return static_cast<int32_t>(count);
}

// 0x004DE990: this-in-ECX (window) + two stack args (ret 8). len 0 returns 1; when
// position+len fits inside the window the +0xC high-water mark grows; otherwise only the
// remaining room is copied. Returns whether all len bytes were copied.
int32_t TS3_MISC_FASTCALL WriteWindow4DE990(void* self, [[maybe_unused]] void* edx, const void* src, int32_t len)
{
    if (len == 0)
        return 1;
    const int32_t base = AddressOf(self);
    const uint32_t position = static_cast<uint32_t>(LoadDword(base + 0x14));
    const uint32_t end = position + static_cast<uint32_t>(len);
    uint32_t count = static_cast<uint32_t>(len);
    if (end <= static_cast<uint32_t>(LoadDword(base + 0x10))) {
        if (static_cast<uint32_t>(LoadDword(base + 0x0C)) < end)
            StoreDword(base + 0x0C, static_cast<int32_t>(end));
    } else {
        count = static_cast<uint32_t>(LoadDword(base + 0x0C)) - position;
    }
    std::memcpy(PtrOf(LoadDword(base + 4) + static_cast<int32_t>(position)), src, count);
    StoreDword(base + 0x14, static_cast<int32_t>(position + count));
    return count == static_cast<uint32_t>(len);
}

// 0x004E93E0: this-in-ECX (a one-byte enabled flag, a busy dword at +4, a nesting counter
// at +8) + two stack args (ret 8). Puts the source as a NUL-terminated UTF-16 (CF_UNICODETEXT)
// clipboard string; the enabled/busy check and every Win32 failure return 0. The nesting
// counter grows for the duration.
uint8_t TS3_MISC_FASTCALL SetClipboardText4E93E0(void* self, [[maybe_unused]] void* edx, const void* src, int32_t len)
{
#if !defined(_WIN32)
    AbortNotRewritten("the Win32 clipboard");
    return 0;
#else
    const int32_t base = AddressOf(self);
    if (LoadByte(base) == 0 || LoadDword(base + 4) != 0)
        return 0;
    StoreDword(base + 8, LoadDword(base + 8) + 1);
    uint8_t result = 0;
    if (ClipOpenClipboard(nullptr)) {
        ClipEmptyClipboard();
        const size_t bytes = 2 * static_cast<size_t>(static_cast<uint32_t>(len));
        HGLOBAL block = ClipGlobalAlloc(GMEM_MOVEABLE, bytes + 2);
        if (block != nullptr) {
            char* locked = static_cast<char*>(ClipGlobalLock(block));
            if (locked != nullptr) {
                std::memcpy(locked, src, bytes);
                *reinterpret_cast<uint16_t*>(locked + bytes) = 0;
                ClipGlobalUnlock(block);
                if (ClipSetClipboardData(CF_UNICODETEXT, block) != nullptr)
                    result = 1;
                else
                    ClipGlobalFree(block);
            }
        }
        ClipCloseClipboard();
    }
    StoreDword(base + 8, LoadDword(base + 8) - 1);
    return result;
#endif
}

// 0x004FAAD0: bounded UTF-8 aware append into the state object (base at +0, position at
// +4, end at +8, overrun byte at +0xC). A size that does not fit is trimmed at a UTF-8
// sequence boundary (0xC2/0xE0/0xF0 lead-byte widths, 0xF0+ aborts the scan); the overrun
// flag latches and later calls return 0.
uint32_t TS3_MISC_CDECL WriteUtf8Bounded4FAAD0(const uint8_t* src, uint32_t size, int32_t state)
{
    if (LoadByte(state + 0x0C) != 0)
        return 0;
    const uint32_t position = static_cast<uint32_t>(LoadDword(state + 4));
    const uint32_t space = static_cast<uint32_t>(LoadDword(state + 8)) - position;
    uint32_t count = size;
    if (size > space) {
        uint32_t i = 0;
        StoreByte(state + 0x0C, 1);
        if (size != 0) {
            do {
                const uint8_t lead = src[i];
                uint32_t step;
                if (lead >= 0xC2) {
                    if (lead >= 0xE0) {
                        if (lead >= 0xF0)
                            break;
                        step = 3;
                    } else {
                        step = 2;
                    }
                } else {
                    step = 1;
                }
                const uint32_t next = i + step;
                if (next > space)
                    break;
                i = next;
            } while (i < size);
        }
        count = i;
    }
    std::memcpy(PtrOf(LoadDword(state) + static_cast<int32_t>(position)), src, count);
    StoreDword(state + 4, static_cast<int32_t>(position + count));
    return count;
}

// 0x006D5390 (stdcall, ret 8): appends the source dword vector to the destination vector
// after asking the destination's allocator method 0x00AEF830 for the grown capacity;
// returns the memcpy destination.
void* TS3_MISC_STDCALL AppendVector6D5390(void* dstVec, const void* srcVec)
{
    const int32_t dstBase = AddressOf(dstVec);
    const int32_t srcBase = AddressOf(srcVec);
    const int32_t dstCount = (LoadDword(dstBase + 4) - LoadDword(dstBase)) >> 2;
    const int32_t srcCount = (LoadDword(srcBase + 4) - LoadDword(srcBase)) >> 2;
    CallGameMethod<void, 0xAEF830>(dstVec, srcCount + dstCount);
    return std::memcpy(PtrOf(LoadDword(dstBase) + 4 * dstCount), PtrOf(LoadDword(srcBase)),
                       static_cast<size_t>(static_cast<uint32_t>(4 * srcCount)));
}

// 0x00C2FD50 (stdcall, ret 8): looks the MRTR record `index` up in the renderer global
// 0x011D7530 (16-byte records at owner+4); a tag word that is neither 0 nor the owner's
// +0x10 falls back to the static record 0x1015860. Writes 16 zeroed bytes then either
// copies min(record+8, 4) dwords from the record's +0xC pointer (kind 1) or the four
// dwords it points at (kind 2), and returns the out buffer.
void* TS3_MISC_STDCALL ReadRecordC2FD50(void* out, int32_t index)
{
    const int32_t owner = LoadGlobal(0x011D7530);
    int32_t record = LoadDword(owner + 4) + 16 * index;
    const uint16_t tag = LoadWord(record + 4);
    if (tag != 0 && tag != LoadWord(owner + 0x10))
        record = 0x1015860;
    std::memset(out, 0, 16);
    if (record != 0) {
        const uint16_t kind = LoadWord(record + 6);
        if (kind == 1) {
            uint32_t count = static_cast<uint32_t>(LoadDword(record + 8));
            if (count > 4)
                count = 4;
            std::memcpy(out, PtrOf(LoadDword(record + 0x0C)), 4 * static_cast<size_t>(count));
        } else if (kind == 2 && LoadDword(record + 8) != 0) {
            const int32_t source = LoadDword(record + 0x0C);
            int32_t* dst = static_cast<int32_t*>(out);
            dst[0] = LoadDword(source);
            dst[1] = LoadDword(source + 4);
            dst[2] = LoadDword(source + 8);
            dst[3] = LoadDword(source + 0x0C);
        }
    }
    return out;
}

// 0x00E7A300: copies one element (stride from 0x00E6B690 of the object's first field)
// out of the object's inline array starting at +0x10 into the caller's buffer.
void* TS3_MISC_CDECL CopyElementE7A300(int32_t obj, int32_t count, void* src)
{
    const int32_t stride = CallGameFunction<int32_t, 0xE6B690>(LoadDword(LoadDword(obj)));
    return std::memcpy(src, PtrOf(obj + 0x10 + count * stride),
                       static_cast<size_t>(static_cast<uint32_t>(stride)));
}

// 0x00E7A330: copies (count at a1+8) * stride bytes from the pointer at a2+0x10 into the
// object's inline array at a1+0x10.
void* TS3_MISC_CDECL CopyRowE7A330(int32_t a1, int32_t a2)
{
    const int32_t stride = CallGameFunction<int32_t, 0xE6B690>(LoadDword(LoadDword(a1)));
    return std::memcpy(PtrOf(a1 + 0x10), PtrOf(LoadDword(a2 + 0x10)),
                       static_cast<size_t>(static_cast<uint32_t>(stride * LoadDword(a1 + 8))));
}

// 0x00EB9040: memcpy(a3 + 2*a4 + 16, a1 + 2*a2 + 8, 2*a5).
void* TS3_MISC_CDECL CopySpanEB9040(int32_t a1, int32_t a2, int32_t a3, int32_t a4, int32_t a5)
{
    return std::memcpy(PtrOf(a3 + 2 * a4 + 0x10), PtrOf(a1 + 2 * a2 + 8),
                       static_cast<size_t>(static_cast<uint32_t>(2 * a5)));
}

// 0x00EB9400: memcpy(dst, src, 2*count) for u16 elements.
void* TS3_MISC_CDECL CopyU16EB9400(const void* src, void* dst, int32_t count)
{
    return std::memcpy(dst, src, static_cast<size_t>(static_cast<uint32_t>(2 * count)));
}

// 0x00F1D6D0: parses a 16-byte chunk header plus an optional u16-length payload from the
// cursor srcPtr/avail. False when the cursor has less than `size` bytes or size < 0x10;
// the 16-byte header is copied out and the u16 is zeroed when size < 0x12; otherwise the
// u16 length is capped to `cap` and memcpy'd from the cursor, after which the cursor
// skips the remaining size-0x12 bytes. The fifth argument (a5) is the payload buffer and
// is dead in the original, which the signature keeps.
uint8_t TS3_MISC_CDECL ReadChunkF1D6D0(int32_t* srcPtr, int32_t* avail, int32_t size, void* headerOut,
                                       void* dst, int32_t cap)
{
    if (static_cast<uint32_t>(*avail) < static_cast<uint32_t>(size))
        return 0;
    if (static_cast<uint32_t>(size) < 0x10)
        return 0;
    std::memcpy(headerOut, PtrOf(*srcPtr), 16);
    *srcPtr += 0x10;
    *avail -= 0x10;
    StoreWord(AddressOf(static_cast<uint8_t*>(headerOut) + 0x10), 0);
    if (static_cast<uint32_t>(size) < 0x12)
        return 1;
    const uint16_t length = *reinterpret_cast<const uint16_t*>(PtrOf(*srcPtr));
    StoreWord(AddressOf(static_cast<uint8_t*>(headerOut) + 0x10), length);
    *srcPtr += 2;
    *avail -= 2;
    uint32_t copy = length;
    if (copy > static_cast<uint32_t>(cap))
        copy = static_cast<uint32_t>(cap);
    std::memcpy(dst, PtrOf(*srcPtr), copy);
    const int32_t rest = size - 0x12;
    *srcPtr += rest;
    *avail -= rest;
    return 1;
}

// 0x00F4B720: allocation hook for the object's +0x4B4 (alloc) and +0x4B8 (free)
// callbacks, falling back to the MSVCR80 malloc/free import slots. copySize 0 means
// "free src and return 0"; otherwise alloc `size`, copy min(copySize, size) from src and
// free src, returning the new block.
int32_t TS3_MISC_CDECL AllocHookF4B720(int32_t obj, void* src, int32_t size, int32_t copySize)
{
    if (copySize == 0) {
        const int32_t freeFn = LoadDword(obj + 0x4B8);
        if (freeFn != 0)
            CallFnPtr<void>(freeFn, src);
        else
            CallFree(src);
        return 0;
    }
    // The original asks its allocator for the copy size, not the source size.
    const int32_t allocFn = LoadDword(obj + 0x4B4);
    const int32_t block =
        (allocFn != 0) ? CallFnPtr<int32_t>(allocFn, copySize)
                      : AddressOf(CallMalloc(static_cast<size_t>(static_cast<uint32_t>(copySize))));
    if (copySize > 0) {
        uint32_t count = static_cast<uint32_t>(copySize);
        if (count > static_cast<uint32_t>(size))
            count = static_cast<uint32_t>(size);
        std::memcpy(PtrOf(block), src, count);
        const int32_t freeFn = LoadDword(obj + 0x4B8);
        if (freeFn != 0)
            CallFnPtr<void>(freeFn, src);
        else
            CallFree(src);
    }
    return block;
}

// 0x00F7E8A6: the MSVCR80 memcpy import thunk. The original is a 6-byte indirect jump
// that this forwards through the same IAT slot, so the harness can replace it without
// changing what runs.
void* TS3_MISC_CDECL MemcpyF7E8A6(void* dst, const void* src, size_t size)
{
    using Fn = void*(__cdecl*)(void*, const void*, size_t);
    return reinterpret_cast<Fn>(
        static_cast<uintptr_t>(static_cast<uint32_t>(LoadGlobal(kMemcpyIat))))(dst, src, size);
}

// 0x00F7EE92: the MSVCR80 _setjmp3 import thunk, forwarded through the same IAT slot.
int32_t TS3_MISC_CDECL Setjmp3F7EE92(void* env, int32_t count)
{
    using Fn = int32_t(__cdecl*)(void*, int32_t);
    return reinterpret_cast<Fn>(
        static_cast<uintptr_t>(static_cast<uint32_t>(LoadGlobal(kSetjmp3Iat))))(env, count);
}

// ---- 0x00DA2780 .. 0x00DE3A60: the protected slot wrappers ---------------------------

// 0x00DA2780: slot 0x011EE830 with (a5, 1, 1).
int32_t TS3_MISC_CDECL SlotValueDA2780([[maybe_unused]] int32_t a1, int32_t* a2, int32_t ctx, int32_t* a4, int32_t a5)
{
    int32_t frame[kSlotFrame3] = {};
    frame[0] = a5;
    frame[2] = 1;
    frame[4] = 1;
    if (a5 == 0) {
        const int32_t error = CallGameFunction<int32_t, kErrorFn>();
        *a2 = error;
        return error;
    }
    return RunSlot3(0x011EE830, frame, a2, a4, ctx);
}

// 0x00DA3110: slot 0x011EE7DC with (a5, a6, a5).
int32_t TS3_MISC_CDECL SlotValueDA3110([[maybe_unused]] int32_t a1, int32_t* a2, int32_t ctx, int32_t* a4, int32_t a5,
                                       int32_t a6)
{
    int32_t frame[kSlotFrame3] = {};
    frame[0] = a5;
    frame[2] = a6;
    frame[4] = a5;
    if (a5 == 0) {
        const int32_t error = CallGameFunction<int32_t, kErrorFn>();
        *a2 = error;
        return error;
    }
    return RunSlot3(0x011EE7DC, frame, a2, a4, ctx);
}

// 0x00DAA260: slot 0x011EF3A4 with (handle->+4, &frame[6]); the out block is frame[10].
int32_t TS3_MISC_CDECL SlotValueDAA260([[maybe_unused]] int32_t a1, int32_t* a2, int32_t ctx, int32_t* a4, int32_t a5)
{
    int32_t frame[kSlotFrame2] = {};
    frame[0] = a5;
    if (a5 == 0) {
        const int32_t error = CallGameFunction<int32_t, kErrorFn>();
        *a2 = error;
        return error;
    }
    frame[0] = LoadDword(a5 + 4);
    frame[2] = AddressOf(&frame[6]);
    return RunSlot2Out(0x011EF3A4, frame, 10, a2, a4, ctx);
}

// 0x00DAF9D0: slot 0x011EF0EC; the out block is frame[7].
int32_t TS3_MISC_CDECL SlotValueDAF9D0([[maybe_unused]] int32_t a1, int32_t* a2, int32_t ctx, int32_t* a4, int32_t a5)
{
    int32_t frame[kSlotFrame2] = {};
    frame[0] = a5;
    if (a5 == 0) {
        const int32_t error = CallGameFunction<int32_t, kErrorFn>();
        *a2 = error;
        return error;
    }
    frame[0] = LoadDword(a5 + 4);
    frame[2] = AddressOf(&frame[6]);
    return RunSlot2Out(0x011EF0EC, frame, 7, a2, a4, ctx);
}

// 0x00DAFD60: slot 0x011EF0D0 with (a5, a6, a5).
int32_t TS3_MISC_CDECL SlotValueDAFD60([[maybe_unused]] int32_t a1, int32_t* a2, int32_t ctx, int32_t* a4, int32_t a5,
                                       int32_t a6)
{
    int32_t frame[kSlotFrame3] = {};
    frame[0] = a5;
    frame[2] = a6;
    frame[4] = a5;
    if (a5 == 0) {
        const int32_t error = CallGameFunction<int32_t, kErrorFn>();
        *a2 = error;
        return error;
    }
    return RunSlot3(0x011EF0D0, frame, a2, a4, ctx);
}

// 0x00DB2BB0: slot 0x011EEF30 with (a5, 0, 0).
int32_t TS3_MISC_CDECL SlotValueDB2BB0([[maybe_unused]] int32_t a1, int32_t* a2, int32_t ctx, int32_t* a4, int32_t a5)
{
    int32_t frame[kSlotFrame3] = {};
    frame[0] = a5;
    frame[2] = 0;
    frame[4] = 0;
    if (a5 == 0) {
        const int32_t error = CallGameFunction<int32_t, kErrorFn>();
        *a2 = error;
        return error;
    }
    return RunSlot3(0x011EEF30, frame, a2, a4, ctx);
}

// 0x00DB5880: slot 0x011EEDEC with (a5, a6).
int32_t TS3_MISC_CDECL SlotValueDB5880([[maybe_unused]] int32_t a1, int32_t* a2, int32_t ctx, int32_t* a4, int32_t a5,
                                       int32_t a6)
{
    int32_t frame[kSlotFrame3] = {};
    frame[0] = a5;
    frame[2] = a6;
    if (a5 == 0) {
        const int32_t error = CallGameFunction<int32_t, kErrorFn>();
        *a2 = error;
        return error;
    }
    return RunSlot2(0x011EEDEC, frame, a2, a4, ctx);
}

// 0x00DB73D0: lock/lookup prologue on global 0x011EECC0, slot 0x011EECC8 with
// (a5, resolved(a6)); a7 sits at frame[4] but the slot only reads two arguments. Unlike
// the other wrappers this one has no null-handle early exit.
int32_t TS3_MISC_CDECL SlotValueDB73D0([[maybe_unused]] int32_t a1, int32_t* a2, int32_t ctx, int32_t* a4, int32_t a5,
                                       int32_t a6, int32_t a7)
{
    int32_t frame[kSlotFrame3] = {};
    frame[0] = a5;
    frame[2] = a6;
    frame[4] = a7;
    frame[2] = LockedHandle(0x011EECC0, frame);
    return RunSlot2(0x011EECC8, frame, a2, a4, ctx);
}

// 0x00DBB270: lock/lookup prologue on global 0x011EFAB4, slot 0x011EFABC; a6 reaches the
// slot sign-extended from 16 bits.
int32_t TS3_MISC_CDECL SlotValueDBB270([[maybe_unused]] int32_t a1, int32_t* a2, int32_t ctx, int32_t* a4, int32_t a5,
                                       int32_t a6)
{
    int32_t frame[kSlotFrame3] = {};
    frame[0] = a5;
    frame[2] = static_cast<int16_t>(a6);
    frame[2] = LockedHandle(0x011EFAB4, frame);
    return RunSlot2(0x011EFABC, frame, a2, a4, ctx);
}

// 0x00DBE9E0: slot 0x011EF958; the out block is frame[8].
int32_t TS3_MISC_CDECL SlotValueDBE9E0([[maybe_unused]] int32_t a1, int32_t* a2, int32_t ctx, int32_t* a4, int32_t a5)
{
    int32_t frame[kSlotFrame2] = {};
    frame[0] = a5;
    if (a5 == 0) {
        const int32_t error = CallGameFunction<int32_t, kErrorFn>();
        *a2 = error;
        return error;
    }
    frame[0] = LoadDword(a5 + 4);
    frame[2] = AddressOf(&frame[6]);
    return RunSlot2Out(0x011EF958, frame, 8, a2, a4, ctx);
}

// 0x00DC2380: lock/lookup prologue on global 0x011EF63C, slot 0x011EF644.
int32_t TS3_MISC_CDECL SlotValueDC2380([[maybe_unused]] int32_t a1, int32_t* a2, int32_t ctx, int32_t* a4, int32_t a5,
                                       int32_t a6)
{
    int32_t frame[kSlotFrame3] = {};
    frame[0] = a5;
    frame[2] = a6;
    frame[2] = LockedHandle(0x011EF63C, frame);
    return RunSlot2(0x011EF644, frame, a2, a4, ctx);
}

// 0x00DC4320: slot 0x011EF52C; the out block is frame[9].
int32_t TS3_MISC_CDECL SlotValueDC4320([[maybe_unused]] int32_t a1, int32_t* a2, int32_t ctx, int32_t* a4, int32_t a5)
{
    int32_t frame[kSlotFrame2] = {};
    frame[0] = a5;
    if (a5 == 0) {
        const int32_t error = CallGameFunction<int32_t, kErrorFn>();
        *a2 = error;
        return error;
    }
    frame[0] = LoadDword(a5 + 4);
    frame[2] = AddressOf(&frame[6]);
    return RunSlot2Out(0x011EF52C, frame, 9, a2, a4, ctx);
}

// 0x00DD3190: lock/lookup prologue on global 0x011EFD88, slot 0x011EFD90; a6 reaches the
// slot zero-extended from 8 bits.
int32_t TS3_MISC_CDECL SlotValueDD3190([[maybe_unused]] int32_t a1, int32_t* a2, int32_t ctx, int32_t* a4, int32_t a5,
                                       int32_t a6)
{
    int32_t frame[kSlotFrame3] = {};
    frame[0] = a5;
    frame[2] = static_cast<uint8_t>(a6);
    frame[2] = LockedHandle(0x011EFD88, frame);
    return RunSlot2(0x011EFD90, frame, a2, a4, ctx);
}

// 0x00DE1760: slot 0x011F0530; the out block is frame[6].
int32_t TS3_MISC_CDECL SlotValueDE1760([[maybe_unused]] int32_t a1, int32_t* a2, int32_t ctx, int32_t* a4, int32_t a5)
{
    int32_t frame[kSlotFrame2] = {};
    frame[0] = a5;
    if (a5 == 0) {
        const int32_t error = CallGameFunction<int32_t, kErrorFn>();
        *a2 = error;
        return error;
    }
    frame[0] = LoadDword(a5 + 4);
    frame[2] = AddressOf(&frame[6]);
    return RunSlot2Out(0x011F0530, frame, 6, a2, a4, ctx);
}

// 0x00DE3A60: slot 0x011F03D4 with (a5, a6).
int32_t TS3_MISC_CDECL SlotValueDE3A60([[maybe_unused]] int32_t a1, int32_t* a2, int32_t ctx, int32_t* a4, int32_t a5,
                                       int32_t a6)
{
    int32_t frame[kSlotFrame3] = {};
    frame[0] = a5;
    frame[2] = a6;
    if (a5 == 0) {
        const int32_t error = CallGameFunction<int32_t, kErrorFn>();
        *a2 = error;
        return error;
    }
    return RunSlot2(0x011F03D4, frame, a2, a4, ctx);
}

}  // namespace ts3::misc