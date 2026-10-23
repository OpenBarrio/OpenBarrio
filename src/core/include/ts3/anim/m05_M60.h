#pragma once
#include "ts3/layout.h"
namespace ts3::anim {
struct M60 {
    void* vtable;
    // TS3W.exe 0x005710C0: Allocate a 0x30-byte controller and forward its host to the original constructor, or return null.
    void* Create30(void* host);
    // TS3W.exe 0x005710F0: Allocate a 0x48-byte controller and forward its host to the original constructor, or return null.
    void* Create48(void* host);
    // TS3W.exe 0x00571120: Allocate a 0x38-byte controller and invoke constructor 0056F790, or return null.
    void* Create38A(void* host);
    // TS3W.exe 0x00571150: Allocate a 0x38-byte controller and invoke constructor 0056BDA0, or return null.
    void* Create38B(void* host);
    // TS3W.exe 0x00571180: Restore the registry vtable, unregister key 13EA6F90, optionally free, and return this even after free.
    void* DeleteA(uint32_t flags);
    // TS3W.exe 0x005711B0: Restore the registry vtable, unregister key F40282A6, optionally free, and return this even after free.
    void* DeleteB(uint32_t flags);
};
}
