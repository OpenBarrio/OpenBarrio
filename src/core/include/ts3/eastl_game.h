// EASTL containers laid out like the game's (EA's open-source EASTL, third_party/). The
// game's containers allocate through EASTLICoreAllocator, an adapter holding the owning
// object's ICoreAllocator, allocation flags and name, in that order after the container's
// own pointers: on x86, a GameVector<T> is the game's eastl::vector<T> byte for byte, so
// rewritten code can work on the game's objects directly.
#pragma once

#include <cstddef>
#include <cstdlib>

#include <EASTL/vector.h>

#include "ts3/allocator/core_allocator.h"
#include "ts3/layout.h"

namespace ts3 {

class GameAllocator {
public:
    explicit GameAllocator(const char* name = "openbarrio", allocator::ICoreAllocator* coreAllocator = nullptr,
                           int flags = 0)
        : coreAllocator_(coreAllocator), flags_(flags), name_(name)
    {
    }
    GameAllocator(const GameAllocator& x, const char* name)
        : coreAllocator_(x.coreAllocator_), flags_(x.flags_), name_(name)
    {
    }
    GameAllocator(const GameAllocator&) = default;
    GameAllocator& operator=(const GameAllocator&) = default;

    void* allocate(size_t n, int = 0) { return coreAllocator_->Alloc(n, name_, static_cast<unsigned>(flags_)); }
    void* allocate(size_t n, size_t alignment, size_t offset, int = 0)
    {
        return coreAllocator_->Alloc(n, name_, static_cast<unsigned>(flags_), static_cast<unsigned>(alignment),
                                     static_cast<unsigned>(offset));
    }
    void deallocate(void* p, size_t n) { coreAllocator_->Free(p, n); }

    const char* get_name() const { return name_; }
    void set_name(const char* name) { name_ = name; }
    allocator::ICoreAllocator* core_allocator() const { return coreAllocator_; }

    friend bool operator==(const GameAllocator& a, const GameAllocator& b)
    {
        return a.coreAllocator_ == b.coreAllocator_ && a.flags_ == b.flags_;
    }
    friend bool operator!=(const GameAllocator& a, const GameAllocator& b) { return !(a == b); }

private:
    allocator::ICoreAllocator* coreAllocator_;
    int flags_;
    const char* name_;
};

template <class T>
using GameVector = eastl::vector<T, GameAllocator>;

// EASTL's default allocator as the game has it: only a name. Memory comes from EA's
// operator new[] (TS3W.exe 0x004E3F90, with the source file and line the game's EASTL
// passes) and goes back through operator delete[] (0x004E3F70). x64 builds use the C heap.
class NamedAllocator {
public:
    explicit NamedAllocator(const char* name = "EASTL") : name_(name) {}
    NamedAllocator(const NamedAllocator&, const char* name) : name_(name) {}
    NamedAllocator(const NamedAllocator&) = default;
    NamedAllocator& operator=(const NamedAllocator&) = default;

    void* allocate(size_t n, int flags = 0)
    {
#if defined(_M_IX86)
        return CallGameFunction<void*, 0x004E3F90>(n, name_, flags, 0u, reinterpret_cast<const char*>(0x00F98980), 0xE7);
#else
        (void)flags;
        return std::malloc(n);
#endif
    }
    void* allocate(size_t n, size_t, size_t, int flags = 0) { return allocate(n, flags); }
    void deallocate(void* p, size_t)
    {
#if defined(_M_IX86)
        CallGameFunction<void, 0x004E3F70>(p);
#else
        std::free(p);
#endif
    }

    const char* get_name() const { return name_; }
    void set_name(const char* name) { name_ = name; }

    friend bool operator==(const NamedAllocator&, const NamedAllocator&) { return true; }
    friend bool operator!=(const NamedAllocator&, const NamedAllocator&) { return false; }

private:
    const char* name_;
};

TS3_X86_SIZE(GameAllocator, 12);
TS3_X86_SIZE(NamedAllocator, 4);
TS3_X86_SIZE(GameVector<int>, 0x18);

}  // namespace ts3
