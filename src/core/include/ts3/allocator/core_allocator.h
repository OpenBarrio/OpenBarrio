// EA::Allocator::ICoreAllocator, the allocator interface engine objects keep a pointer to.
// MSVC groups overloaded virtual functions and places them in reverse declaration order,
// so (on x86, as in the game) the 5-argument Alloc is slot 1 (+0x4), the 3-argument one
// slot 2 (+0x8) and Free slot 3 (+0xC).
#pragma once

#include <cstddef>

namespace ts3::allocator {

class ICoreAllocator {
public:
    virtual ~ICoreAllocator() = default;
    virtual void* Alloc(size_t size, const char* name, unsigned flags) = 0;
    virtual void* Alloc(size_t size, const char* name, unsigned flags, unsigned align, unsigned alignOffset = 0) = 0;
    virtual void Free(void* block, size_t size = 0) = 0;
};

}  // namespace ts3::allocator
