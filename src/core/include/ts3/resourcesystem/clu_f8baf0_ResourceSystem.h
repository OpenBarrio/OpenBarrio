// Unit clu_f8baf0_ResourceSystem: TS3W.exe's seven global lock initializers (the CRT
// dynamic initializers of the project notes). Each constructs one of the game's global
// locks and registers its destructor thunk with the game's CRT atexit.
#pragma once

#include <cstdint>

namespace ts3::resourcesystem {

// The global locks this unit initializes, with the addresses of TS3W.exe 1.67.2 (Steam):
// the lock object in .data, and the "mov ecx, <global>; jmp dtor" thunk the original
// initializers register with atexit. The lock class itself (constructor 0x004E17E0,
// destructor 0x004E1670) belongs to unit clu_4e17e0_ResourceSystem, which is not
// rewritten yet: both are called through the bridge. The locks are 0x30 bytes and
// consecutive in .data; they are the unnamed critical-section flavour (the constructor's
// second argument 1 puts them in InitializeCriticalSection mode, see
// the original disassembly).
struct GlobalLockInit {
    uint32_t lock;       // the game's global lock object
    uint32_t dtorThunk;  // destructor thunk registered with the game's atexit
};

// One initializer per lock, same order as the unit's original entry points
// 0x00F8BAF0 0x00F8BB10 0x00F8BB30 0x00F8BB50 0x00F8C2F0 0x00F8C310 0x00F906C0.
// Each returns the atexit status (0 success, -1 failure), like the originals.
int InitGlobalLock11EAE68();
int InitGlobalLock11EAE98();
int InitGlobalLock11EAEC8();
int InitGlobalLock11EAEF8();
int InitGlobalLock11EB5B0();
int InitGlobalLock11EB5E0();
int InitGlobalLock11FFAE0();

// The shared body: construct `init.lock` and atexit-register `init.dtorThunk`.
int InitGlobalLock(const GlobalLockInit& init);

}  // namespace ts3::resourcesystem
