// Layout checks for structures shared with the original 32-bit game. On x86 they must
// match TS3W.exe's objects exactly (the harness hands the game's objects to rewritten
// code); x64 builds get their own layout, as pointer-sized fields grow.
#pragma once

#include <cstddef>
#include <cstdint>

#define TS3_X86_OFFSET(type, field, offset) \
    static_assert(sizeof(void*) != 4 || offsetof(type, field) == (offset), #type "::" #field " is at " #offset " on x86")
#define TS3_X86_SIZE(type, size) \
    static_assert(sizeof(void*) != 4 || sizeof(type) == (size), "sizeof(" #type ") is " #size " on x86")

namespace ts3 {

// For interface methods the game has but nothing rewritten implements yet: reports the
// call and stops, rather than returning a made-up result.
[[noreturn]] void NotRewritten(const char* what);
// The same, for generic code that must still look like it returns (keeps callers free of
// unreachable-code warnings).
void AbortNotRewritten(const char* what);

// Calls a virtual method of a game object by vtable slot, for methods that are not
// rewritten yet (x86 __thiscall). In x64 builds there is no game code to call.
template <class R, int kSlot, class Self, class... Args>
R CallGameVirtual(Self* self, Args... args)
{
#if defined(_M_IX86)
    using Method = R(__thiscall*)(Self*, Args...);
    return reinterpret_cast<Method>((*reinterpret_cast<void* const* const*>(self))[kSlot])(self, args...);
#else
    (void)self;
    ((void)args, ...);
    AbortNotRewritten("a game virtual method");
    return R{};
#endif
}

// Calls a plain (__cdecl) function of the game at its TS3W.exe address, for functions not
// rewritten yet. In x64 builds there is no game code to call.
template <class R, unsigned kAddress, class... Args>
R CallGameFunction(Args... args)
{
#if defined(_M_IX86)
    using Function = R(__cdecl*)(Args...);
    return reinterpret_cast<Function>(static_cast<uintptr_t>(kAddress))(args...);
#else
    ((void)args, ...);
    AbortNotRewritten("a game function");
    return R{};
#endif
}

// Calls a non-virtual method of the game at its TS3W.exe address, for methods that are not
// rewritten yet (x86 __thiscall). In x64 builds there is no game code to call.
template <class R, unsigned kAddress, class Self, class... Args>
R CallGameMethod(Self* self, Args... args)
{
#if defined(_M_IX86)
    using Method = R(__thiscall*)(Self*, Args...);
    return reinterpret_cast<Method>(static_cast<uintptr_t>(kAddress))(self, args...);
#else
    (void)self;
    ((void)args, ...);
    AbortNotRewritten("a game method");
    return R{};
#endif
}

}  // namespace ts3
