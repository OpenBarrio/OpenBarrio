// Unit clu_f8baf0_ResourceSystem (the project notes): the seven dynamic initializers
// TS3W.exe's CRT runs at startup for its global locks. All seven originals are the same
// 26-byte pattern, e.g. 0x00F8BAF0:
//
//   push 1                                  ; useCriticalSection = true
//   push 0                                  ; name = none
//   mov ecx, 0x11EAE68                      ; the game's global lock
//   call 0x4E17E0                           ; lock ctor (clu_4e17e0_ResourceSystem, ret 8)
//   push 0xF940C0                           ; dtor thunk "mov ecx, <lock>; jmp 0x4E1670"
//   call 0xF7E9A1                           ; the CRT's atexit wrapper (returns 0 / -1)
//   pop ecx
//   ret
//
// The lock ctor and dtor are unit clu_4e17e0_ResourceSystem (not rewritten yet) and the
// atexit wrapper is TS3W's own CRT code, so all three are called through the bridge: the
// registered handler must land in the game's exit list, and the lock must be constructed
// by exactly the game's ctor. On x64 there is no game code; the bridge reports the call
// and returns 0.
#include "ts3/resourcesystem/clu_f8baf0_ResourceSystem.h"

#include "ts3/layout.h"

namespace ts3::resourcesystem {

namespace {

// Dependency addresses in TS3W.exe 1.67.2 (Steam).
constexpr unsigned kLockCtor = 0x004E17E0;  // __thiscall (lock, const void* name, bool useSection), ret 8
constexpr unsigned kAtexit = 0x00F7E9A1;    // __cdecl atexit(void* handler) -> 0 / -1

}  // namespace

int InitGlobalLock(const GlobalLockInit& init)
{
    CallGameMethod<void, kLockCtor>(reinterpret_cast<void*>(init.lock), static_cast<const void*>(nullptr), true);
    return CallGameFunction<int, kAtexit>(reinterpret_cast<void*>(init.dtorThunk));
}

// The seven entry points, in the original unit's order. Each is __cdecl int(void),
// like its original (EAX = the atexit status when the caller looks).
int InitGlobalLock11EAE68() { return InitGlobalLock({0x011EAE68, 0x00F940C0}); }
int InitGlobalLock11EAE98() { return InitGlobalLock({0x011EAE98, 0x00F940D0}); }
int InitGlobalLock11EAEC8() { return InitGlobalLock({0x011EAEC8, 0x00F940E0}); }
int InitGlobalLock11EAEF8() { return InitGlobalLock({0x011EAEF8, 0x00F940F0}); }
int InitGlobalLock11EB5B0() { return InitGlobalLock({0x011EB5B0, 0x00F94150}); }
int InitGlobalLock11EB5E0() { return InitGlobalLock({0x011EB5E0, 0x00F94170}); }
int InitGlobalLock11FFAE0() { return InitGlobalLock({0x011FFAE0, 0x00F94800}); }

}  // namespace ts3::resourcesystem
