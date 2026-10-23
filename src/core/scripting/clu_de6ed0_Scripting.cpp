// Cluster unit clu_de6ed0_Scripting (scripting): protected-call wrappers around the
// script (Lua-like) runtime's per-handle value callbacks, plus the shared error
// dispatcher. Originals: TS3W.exe 0x00DE6ED0 (490 B), 0x00DE9D10 (266), 0x00DE9EB0
// (243), 0x00DEA2B0 (265), 0x00DEA450 (258), 0x00DEA5F0 (243), 0x00DEA780 (265),
// 0x00DEA920 (266), 0x00DEB530 (254), 0x00DEC4E0 (263), 0x00DEC620 (247), 0x00DEC810
// (243), 0x00DEC940 (263), 0x00DED790 (266), 0x00DF5580 (265), 0x00DF6E60 (267),
// 0x00DF8F10 (266), 0x00DFA520 (243), 0x00DFB3A0 (257), 0x00DFBB00 (246), 0x00DFC0C0
// (266), 0x00DFC260 (257).
//
// Every wrapper is cdecl and reads the same argument slots: a1 is never read, a2 is
// the int* error out, a3 is the scripting state object (errJmp slot at +0x10), a4 (or
// a5 for the getter families) is the null-tested handle whose +4 holds the callback-
// table slot value, and the remaining argument is the value to store or the out-pointer
// to fill. The callback runs either directly or under a setjmp3 protection frame
// (global 0x11F2D84 <= 0 -> direct, > 0 -> protected); a longjmp out of script code
// resumes in the wrapper's setjmp branch, and because the result slot the original
// reads there was only zeroed in the prologue and is never written before the jump,
// the longjmp path always yields result 0. A negative result is routed to error
// formatting: +0x28 of the unit's error object decides between the shared dispatcher
// 0x00DE6ED0
// 0x00DF8F10 returns the zero-extended u16 value where its siblings return the
// out-pointer. The routing callees (0x00D8A8E0, 0x00E770E0, 0x00EA36F0) are stubbed
// by the differential test; 0x00E75260 (process lock) runs for real (TlsGetValue).
#include "ts3/scripting/clu_de6ed0_Scripting.h"

namespace ts3::scripting {
namespace {

constexpr int32_t kZero = 0;
constexpr int32_t kCodeBase = -7;   // payload record's third dword in every DE6ED0 case
constexpr int32_t kCodeSpan = 7;    // out-of-range codes return code+7
constexpr int kEnvDwords = 16;      // setjmp3 buffer, the [ebp-0x64]..[ebp-4] frame slot

// A plain dword read from game memory (the unit's globals: the flag, the per-wrapper
// callback-slot and error-object pointers, DE6ED0's error-id chain).
inline int32_t GameDword(int32_t address)
{
#if defined(_M_IX86)
    return *reinterpret_cast<const int32_t*>(static_cast<uintptr_t>(address));
#else
    (void)address;
    AbortNotRewritten("a script-global dword");
    return kZero;
#endif
}

// The handle's callback-table slot value (+4), loaded before the protected frame is
// entered so that a null handle can still be reported as an argument error.
inline int32_t FnstateOf(int32_t handle)
{
#if defined(_M_IX86)
    return reinterpret_cast<const ScriptHandle*>(static_cast<uintptr_t>(handle))->fnstate;
#else
    (void)handle;
    AbortNotRewritten("a script handle");
    return kZero;
#endif
}

// The scripting state object (a3): +0x10 holds the active wrapper's setjmp buffer.
inline ScriptState* StateAt(int32_t address)
{
#if defined(_M_IX86)
    return reinterpret_cast<ScriptState*>(static_cast<uintptr_t>(address));
#else
    (void)address;
    AbortNotRewritten("the script state object");
    return nullptr;
#endif
}

// Calls the value callback stored in one of the unit's per-wrapper globals (e.g.
// 0x11F0B3C for 0x00DE9D10): a function pointer the test plants, the game fills.
template <class R, unsigned kGlobal, class... Args>
R CallSlot(Args... args)
{
#if defined(_M_IX86)
    using Fn = R(__cdecl*)(Args...);
    const uintptr_t fn = *reinterpret_cast<const uint32_t*>(kGlobal);
    return reinterpret_cast<Fn>(fn)(args...);
#else
    ((void)args, ...);
    AbortNotRewritten("a script value callback");
    return R{};
#endif
}

// The wrappers' second error route: build the {code, 0, 0} record the original
// assembles on its stack and hand it straight to 0x00D8A8E0 (used when the error
// object carries no dispatcher entry at +0x28).
int32_t RouteViaD8A8E0(int32_t errObj, int32_t* a2, int32_t state, int32_t code)
{
    int32_t record[3] = {code, kZero, kZero};
    return CallGameFunction<int32_t, 0xD8A8E0>(errObj, kZero, record, kZero, a2, state);
}

// One 0x00DE6ED0 case (-7..-1): chain-read the error id from the case's global
// (*( *(global+8) +8 )), take the process lock, look the id up and format via
// 0x00D8A8E0. payload0Addr == 0 (codes -7..-5) passes a null arg3 like the original;
// otherwise the 6-dword record's first dword is that global's value (codes -4..-1).
int32_t RunErrorCase(int32_t errGlobalAddr, int32_t payload0Addr, int32_t* a2,
                     int32_t state, int32_t* lookup)
{
    const int32_t errGlobal = GameDword(errGlobalAddr);
    const int32_t key = GameDword(GameDword(errGlobal + 8) + 8);
    const int32_t lock = CallGameFunction<int32_t, 0xE75260>();
    *lookup = CallGameFunction<int32_t, 0xEA36F0>(lock, key);
    int32_t record[6] = {};
    int32_t* arg3 = nullptr;
    if (payload0Addr) {
        record[0] = GameDword(payload0Addr);
        record[2] = kCodeBase;
        arg3 = record;
    }
    return CallGameFunction<int32_t, 0xD8A8E0>(errGlobal, *lookup, arg3, kZero, a2, state);
}

}  // namespace

// 0x00DE6ED0 (490 B): script error dispatcher. Codes -7..-1 each format one of the
// fixed error globals at 0x11F0C74..0x11F0C9C through the id lookup + 0x00D8A8E0;
// anything else returns code+7. a1 (the caller's error object) is never read.
int32_t ScriptErrorDispatch(int32_t a1, int32_t* a2, int32_t a3, int32_t a4)
{
    int32_t result = kZero;
    int32_t lookup = kZero;
    switch (a4) {
    case -7:
        result = RunErrorCase(0x11F0C74, kZero, a2, a3, &lookup);
        break;
    case -6:
        result = RunErrorCase(0x11F0C78, kZero, a2, a3, &lookup);
        break;
    case -5:
        result = RunErrorCase(0x11F0C7C, kZero, a2, a3, &lookup);
        break;
    case -4:
        result = RunErrorCase(0x11F0C84, 0x11F0C80, a2, a3, &lookup);
        break;
    case -3:
        result = RunErrorCase(0x11F0C8C, 0x11F0C88, a2, a3, &lookup);
        break;
    case -2:
        result = RunErrorCase(0x11F0C94, 0x11F0C90, a2, a3, &lookup);
        break;
    case -1:
        result = RunErrorCase(0x11F0C9C, 0x11F0C98, a2, a3, &lookup);
        break;
    default:
        return a4 + kCodeSpan;
    }
    // LABEL_11: the case's id lookup fills the error out-param when the route left it
    // empty (a zero lookup falls back to the generic 0x00E770E0 error).
    if (!*a2) {
        if (lookup)
            *a2 = lookup;
        else {
            result = CallGameFunction<int32_t, 0xE770E0>();
            *a2 = result;
        }
    }
    return result;
}

// 0x00DE9D10 (266 B): store a u16 (size tag 2) through the handle's callback.
int32_t ScriptSetU16(int32_t a1, int32_t* a2, int32_t a3, int32_t a4, uint16_t a5)
{
    if (!a4) {
        const int32_t err = CallGameFunction<int32_t, 0xE770E0>();
        *a2 = err;
        return err;
    }
    const int32_t fnstate = FnstateOf(a4);
    int32_t result;
    if (*reinterpret_cast<const int32_t*>(0x11F2D84) > kZero) {
        int32_t env[kEnvDwords] = {};
        ScriptState* state = StateAt(a3);
        const int32_t saved = (int32_t)state->errJmp;
        volatile int32_t sj = CallGameFunction<int32_t, 0xF7EE92>(env, kZero);
        if (sj) {
            result = kZero;  // longjmp path: the prologue-zeroed result slot
            state->errJmp = (uint32_t)saved;
        } else {
            state->errJmp = (uint32_t)(uintptr_t)env;
            result = CallSlot<int32_t, 0x11F0B3C>(fnstate, (int32_t)a5, 2);
            state->errJmp = (uint32_t)saved;
        }
    } else {
        result = CallSlot<int32_t, 0x11F0B3C>(fnstate, (int32_t)a5, 2);
    }
    if (*a2)
        return result;
    if (result < kZero) {
        const int32_t errObj = GameDword(0x11F0B40);
        if (*reinterpret_cast<const int32_t*>(static_cast<uintptr_t>(errObj) + 0x28) != kZero)
            return ScriptErrorDispatch(errObj, a2, a3, result);
        return RouteViaD8A8E0(errObj, a2, a3, result);
    }
    return result;
}

// 0x00DE9EB0 (243 B): the handle's no-argument callback IS the value; write it to
// *a4 and return the a4 pointer.
int32_t ScriptGetResult(int32_t a1, int32_t* a2, int32_t a3, int32_t* a4, int32_t a5)
{
    if (!a5) {
        const int32_t err = CallGameFunction<int32_t, 0xE770E0>();
        *a2 = err;
        return err;
    }
    const int32_t fnstate = FnstateOf(a5);
    int32_t result;
    if (*reinterpret_cast<const int32_t*>(0x11F2D84) > kZero) {
        int32_t env[kEnvDwords] = {};
        ScriptState* state = StateAt(a3);
        const int32_t saved = (int32_t)state->errJmp;
        volatile int32_t sj = CallGameFunction<int32_t, 0xF7EE92>(env, kZero);
        if (sj) {
            result = kZero;  // longjmp path: the prologue-zeroed result slot
            state->errJmp = (uint32_t)saved;
        } else {
            state->errJmp = (uint32_t)(uintptr_t)env;
            result = CallSlot<int32_t, 0x11F0B30>(fnstate);
            state->errJmp = (uint32_t)saved;
        }
    } else {
        result = CallSlot<int32_t, 0x11F0B30>(fnstate);
    }
    if (*a2)
        return result;
    const int32_t outValue = result;  // written to *a4 even when the route runs
    if (result < kZero) {
        const int32_t errObj = GameDword(0x11F0B34);
        if (*reinterpret_cast<const int32_t*>(static_cast<uintptr_t>(errObj) + 0x28) != kZero)
            result = ScriptErrorDispatch(errObj, a2, a3, result);
        else
            result = RouteViaD8A8E0(errObj, a2, a3, result);
        if (*a2)
            return result;
    }
    *a4 = outValue;
    return (int32_t)(intptr_t)a4;
}

// 0x00DEA2B0 (265 B): store an i32 (size tag 4) through the handle's callback.
int32_t ScriptSetI32(int32_t a1, int32_t* a2, int32_t a3, int32_t a4, int32_t a5)
{
    if (!a4) {
        const int32_t err = CallGameFunction<int32_t, 0xE770E0>();
        *a2 = err;
        return err;
    }
    const int32_t fnstate = FnstateOf(a4);
    int32_t result;
    if (*reinterpret_cast<const int32_t*>(0x11F2D84) > kZero) {
        int32_t env[kEnvDwords] = {};
        ScriptState* state = StateAt(a3);
        const int32_t saved = (int32_t)state->errJmp;
        volatile int32_t sj = CallGameFunction<int32_t, 0xF7EE92>(env, kZero);
        if (sj) {
            result = kZero;  // longjmp path: the prologue-zeroed result slot
            state->errJmp = (uint32_t)saved;
        } else {
            state->errJmp = (uint32_t)(uintptr_t)env;
            result = CallSlot<int32_t, 0x11F0B1C>(fnstate, a5, 4);
            state->errJmp = (uint32_t)saved;
        }
    } else {
        result = CallSlot<int32_t, 0x11F0B1C>(fnstate, a5, 4);
    }
    if (*a2)
        return result;
    if (result < kZero) {
        const int32_t errObj = GameDword(0x11F0B20);
        if (*reinterpret_cast<const int32_t*>(static_cast<uintptr_t>(errObj) + 0x28) != kZero)
            return ScriptErrorDispatch(errObj, a2, a3, result);
        return RouteViaD8A8E0(errObj, a2, a3, result);
    }
    return result;
}

// 0x00DEA450 (258 B): store a double through the handle's callback (no size tag; the
// original round-trips the value through an x87 qword temp, which is exact).
int32_t ScriptSetF64(int32_t a1, int32_t* a2, int32_t a3, int32_t a4, double a5)
{
    if (!a4) {
        const int32_t err = CallGameFunction<int32_t, 0xE770E0>();
        *a2 = err;
        return err;
    }
    const int32_t fnstate = FnstateOf(a4);
    int32_t result;
    if (*reinterpret_cast<const int32_t*>(0x11F2D84) > kZero) {
        int32_t env[kEnvDwords] = {};
        ScriptState* state = StateAt(a3);
        const int32_t saved = (int32_t)state->errJmp;
        volatile int32_t sj = CallGameFunction<int32_t, 0xF7EE92>(env, kZero);
        if (sj) {
            result = kZero;  // longjmp path: the prologue-zeroed result slot
            state->errJmp = (uint32_t)saved;
        } else {
            state->errJmp = (uint32_t)(uintptr_t)env;
            result = CallSlot<int32_t, 0x11F0B10>(fnstate, a5);
            state->errJmp = (uint32_t)saved;
        }
    } else {
        result = CallSlot<int32_t, 0x11F0B10>(fnstate, a5);
    }
    if (*a2)
        return result;
    if (result < kZero) {
        const int32_t errObj = GameDword(0x11F0B14);
        if (*reinterpret_cast<const int32_t*>(static_cast<uintptr_t>(errObj) + 0x28) != kZero)
            return ScriptErrorDispatch(errObj, a2, a3, result);
        return RouteViaD8A8E0(errObj, a2, a3, result);
    }
    return result;
}

// 0x00DEA5F0 (243 B): duplicate of 0x00DE9EB0 (globals 0x11F0B04/0x11F0B08).
int32_t CDEA5F0(int32_t a1, int32_t* a2, int32_t a3, int32_t* a4, int32_t a5)
{
    if (!a5) {
        const int32_t err = CallGameFunction<int32_t, 0xE770E0>();
        *a2 = err;
        return err;
    }
    const int32_t fnstate = FnstateOf(a5);
    int32_t result;
    if (*reinterpret_cast<const int32_t*>(0x11F2D84) > kZero) {
        int32_t env[kEnvDwords] = {};
        ScriptState* state = StateAt(a3);
        const int32_t saved = (int32_t)state->errJmp;
        volatile int32_t sj = CallGameFunction<int32_t, 0xF7EE92>(env, kZero);
        if (sj) {
            result = kZero;  // longjmp path: the prologue-zeroed result slot
            state->errJmp = (uint32_t)saved;
        } else {
            state->errJmp = (uint32_t)(uintptr_t)env;
            result = CallSlot<int32_t, 0x11F0B04>(fnstate);
            state->errJmp = (uint32_t)saved;
        }
    } else {
        result = CallSlot<int32_t, 0x11F0B04>(fnstate);
    }
    if (*a2)
        return result;
    const int32_t outValue = result;  // written to *a4 even when the route runs
    if (result < kZero) {
        const int32_t errObj = GameDword(0x11F0B08);
        if (*reinterpret_cast<const int32_t*>(static_cast<uintptr_t>(errObj) + 0x28) != kZero)
            result = ScriptErrorDispatch(errObj, a2, a3, result);
        else
            result = RouteViaD8A8E0(errObj, a2, a3, result);
        if (*a2)
            return result;
    }
    *a4 = outValue;
    return (int32_t)(intptr_t)a4;
}

// 0x00DEA780 (265 B): duplicate of 0x00DEA2B0 (globals 0x11F0AF8/0x11F0AFC).
int32_t CDEA780(int32_t a1, int32_t* a2, int32_t a3, int32_t a4, int32_t a5)
{
    if (!a4) {
        const int32_t err = CallGameFunction<int32_t, 0xE770E0>();
        *a2 = err;
        return err;
    }
    const int32_t fnstate = FnstateOf(a4);
    int32_t result;
    if (*reinterpret_cast<const int32_t*>(0x11F2D84) > kZero) {
        int32_t env[kEnvDwords] = {};
        ScriptState* state = StateAt(a3);
        const int32_t saved = (int32_t)state->errJmp;
        volatile int32_t sj = CallGameFunction<int32_t, 0xF7EE92>(env, kZero);
        if (sj) {
            result = kZero;  // longjmp path: the prologue-zeroed result slot
            state->errJmp = (uint32_t)saved;
        } else {
            state->errJmp = (uint32_t)(uintptr_t)env;
            result = CallSlot<int32_t, 0x11F0AF8>(fnstate, a5, 4);
            state->errJmp = (uint32_t)saved;
        }
    } else {
        result = CallSlot<int32_t, 0x11F0AF8>(fnstate, a5, 4);
    }
    if (*a2)
        return result;
    if (result < kZero) {
        const int32_t errObj = GameDword(0x11F0AFC);
        if (*reinterpret_cast<const int32_t*>(static_cast<uintptr_t>(errObj) + 0x28) != kZero)
            return ScriptErrorDispatch(errObj, a2, a3, result);
        return RouteViaD8A8E0(errObj, a2, a3, result);
    }
    return result;
}

// 0x00DEA920 (266 B): store a u8 (size tag 1, zero-extended like the original's movzx).
int32_t ScriptSetU8(int32_t a1, int32_t* a2, int32_t a3, int32_t a4, uint8_t a5)
{
    if (!a4) {
        const int32_t err = CallGameFunction<int32_t, 0xE770E0>();
        *a2 = err;
        return err;
    }
    const int32_t fnstate = FnstateOf(a4);
    int32_t result;
    if (*reinterpret_cast<const int32_t*>(0x11F2D84) > kZero) {
        int32_t env[kEnvDwords] = {};
        ScriptState* state = StateAt(a3);
        const int32_t saved = (int32_t)state->errJmp;
        volatile int32_t sj = CallGameFunction<int32_t, 0xF7EE92>(env, kZero);
        if (sj) {
            result = kZero;  // longjmp path: the prologue-zeroed result slot
            state->errJmp = (uint32_t)saved;
        } else {
            state->errJmp = (uint32_t)(uintptr_t)env;
            result = CallSlot<int32_t, 0x11F0AEC>(fnstate, (int32_t)a5, 1);
            state->errJmp = (uint32_t)saved;
        }
    } else {
        result = CallSlot<int32_t, 0x11F0AEC>(fnstate, (int32_t)a5, 1);
    }
    if (*a2)
        return result;
    if (result < kZero) {
        const int32_t errObj = GameDword(0x11F0AF0);
        if (*reinterpret_cast<const int32_t*>(static_cast<uintptr_t>(errObj) + 0x28) != kZero)
            return ScriptErrorDispatch(errObj, a2, a3, result);
        return RouteViaD8A8E0(errObj, a2, a3, result);
    }
    return result;
}

// 0x00DEB530 (254 B): store a float through the handle's callback (no size tag; the
// original round-trips the value through an x87 qword temp, which is exact).
int32_t ScriptSetF32(int32_t a1, int32_t* a2, int32_t a3, int32_t a4, float a5)
{
    if (!a4) {
        const int32_t err = CallGameFunction<int32_t, 0xE770E0>();
        *a2 = err;
        return err;
    }
    const int32_t fnstate = FnstateOf(a4);
    int32_t result;
    if (*reinterpret_cast<const int32_t*>(0x11F2D84) > kZero) {
        int32_t env[kEnvDwords] = {};
        ScriptState* state = StateAt(a3);
        const int32_t saved = (int32_t)state->errJmp;
        volatile int32_t sj = CallGameFunction<int32_t, 0xF7EE92>(env, kZero);
        if (sj) {
            result = kZero;  // longjmp path: the prologue-zeroed result slot
            state->errJmp = (uint32_t)saved;
        } else {
            state->errJmp = (uint32_t)(uintptr_t)env;
            result = CallSlot<int32_t, 0x11F0A88>(fnstate, a5);
            state->errJmp = (uint32_t)saved;
        }
    } else {
        result = CallSlot<int32_t, 0x11F0A88>(fnstate, a5);
    }
    if (*a2)
        return result;
    if (result < kZero) {
        const int32_t errObj = GameDword(0x11F0A8C);
        if (*reinterpret_cast<const int32_t*>(static_cast<uintptr_t>(errObj) + 0x28) != kZero)
            return ScriptErrorDispatch(errObj, a2, a3, result);
        return RouteViaD8A8E0(errObj, a2, a3, result);
    }
    return result;
}

// 0x00DEC4E0 (263 B): store two i32s through the handle's callback (no size tag).
int32_t ScriptSetPair(int32_t a1, int32_t* a2, int32_t a3, int32_t a4, int32_t a5,
                      int32_t a6)
{
    if (!a4) {
        const int32_t err = CallGameFunction<int32_t, 0xE770E0>();
        *a2 = err;
        return err;
    }
    const int32_t fnstate = FnstateOf(a4);
    int32_t result;
    if (*reinterpret_cast<const int32_t*>(0x11F2D84) > kZero) {
        int32_t env[kEnvDwords] = {};
        ScriptState* state = StateAt(a3);
        const int32_t saved = (int32_t)state->errJmp;
        volatile int32_t sj = CallGameFunction<int32_t, 0xF7EE92>(env, kZero);
        if (sj) {
            result = kZero;  // longjmp path: the prologue-zeroed result slot
            state->errJmp = (uint32_t)saved;
        } else {
            state->errJmp = (uint32_t)(uintptr_t)env;
            result = CallSlot<int32_t, 0x11F0F04>(fnstate, a5, a6);
            state->errJmp = (uint32_t)saved;
        }
    } else {
        result = CallSlot<int32_t, 0x11F0F04>(fnstate, a5, a6);
    }
    if (*a2)
        return result;
    if (result < kZero) {
        const int32_t errObj = GameDword(0x11F0F08);
        if (*reinterpret_cast<const int32_t*>(static_cast<uintptr_t>(errObj) + 0x28) != kZero)
            return ScriptErrorDispatch(errObj, a2, a3, result);
        return RouteViaD8A8E0(errObj, a2, a3, result);
    }
    return result;
}

// 0x00DEC620 (247 B): store a u16 through the handle's callback (movzx, no size tag).
int32_t ScriptSetU16NoTag(int32_t a1, int32_t* a2, int32_t a3, int32_t a4, uint16_t a5)
{
    if (!a4) {
        const int32_t err = CallGameFunction<int32_t, 0xE770E0>();
        *a2 = err;
        return err;
    }
    const int32_t fnstate = FnstateOf(a4);
    int32_t result;
    if (*reinterpret_cast<const int32_t*>(0x11F2D84) > kZero) {
        int32_t env[kEnvDwords] = {};
        ScriptState* state = StateAt(a3);
        const int32_t saved = (int32_t)state->errJmp;
        volatile int32_t sj = CallGameFunction<int32_t, 0xF7EE92>(env, kZero);
        if (sj) {
            result = kZero;  // longjmp path: the prologue-zeroed result slot
            state->errJmp = (uint32_t)saved;
        } else {
            state->errJmp = (uint32_t)(uintptr_t)env;
            result = CallSlot<int32_t, 0x11F0EF8>(fnstate, (int32_t)a5);
            state->errJmp = (uint32_t)saved;
        }
    } else {
        result = CallSlot<int32_t, 0x11F0EF8>(fnstate, (int32_t)a5);
    }
    if (*a2)
        return result;
    if (result < kZero) {
        const int32_t errObj = GameDword(0x11F0EFC);
        if (*reinterpret_cast<const int32_t*>(static_cast<uintptr_t>(errObj) + 0x28) != kZero)
            return ScriptErrorDispatch(errObj, a2, a3, result);
        return RouteViaD8A8E0(errObj, a2, a3, result);
    }
    return result;
}

// 0x00DEC810 (243 B): duplicate of 0x00DE9EB0 (globals 0x11F0EDC/0x11F0EE0).
int32_t CDEC810(int32_t a1, int32_t* a2, int32_t a3, int32_t* a4, int32_t a5)
{
    if (!a5) {
        const int32_t err = CallGameFunction<int32_t, 0xE770E0>();
        *a2 = err;
        return err;
    }
    const int32_t fnstate = FnstateOf(a5);
    int32_t result;
    if (*reinterpret_cast<const int32_t*>(0x11F2D84) > kZero) {
        int32_t env[kEnvDwords] = {};
        ScriptState* state = StateAt(a3);
        const int32_t saved = (int32_t)state->errJmp;
        volatile int32_t sj = CallGameFunction<int32_t, 0xF7EE92>(env, kZero);
        if (sj) {
            result = kZero;  // longjmp path: the prologue-zeroed result slot
            state->errJmp = (uint32_t)saved;
        } else {
            state->errJmp = (uint32_t)(uintptr_t)env;
            result = CallSlot<int32_t, 0x11F0EDC>(fnstate);
            state->errJmp = (uint32_t)saved;
        }
    } else {
        result = CallSlot<int32_t, 0x11F0EDC>(fnstate);
    }
    if (*a2)
        return result;
    const int32_t outValue = result;  // written to *a4 even when the route runs
    if (result < kZero) {
        const int32_t errObj = GameDword(0x11F0EE0);
        if (*reinterpret_cast<const int32_t*>(static_cast<uintptr_t>(errObj) + 0x28) != kZero)
            result = ScriptErrorDispatch(errObj, a2, a3, result);
        else
            result = RouteViaD8A8E0(errObj, a2, a3, result);
        if (*a2)
            return result;
    }
    *a4 = outValue;
    return (int32_t)(intptr_t)a4;
}

// 0x00DEC940 (263 B): duplicate of 0x00DEC4E0 (globals 0x11F0EB8/0x11F0EBC).
int32_t CDEC940(int32_t a1, int32_t* a2, int32_t a3, int32_t a4, int32_t a5, int32_t a6)
{
    if (!a4) {
        const int32_t err = CallGameFunction<int32_t, 0xE770E0>();
        *a2 = err;
        return err;
    }
    const int32_t fnstate = FnstateOf(a4);
    int32_t result;
    if (*reinterpret_cast<const int32_t*>(0x11F2D84) > kZero) {
        int32_t env[kEnvDwords] = {};
        ScriptState* state = StateAt(a3);
        const int32_t saved = (int32_t)state->errJmp;
        volatile int32_t sj = CallGameFunction<int32_t, 0xF7EE92>(env, kZero);
        if (sj) {
            result = kZero;  // longjmp path: the prologue-zeroed result slot
            state->errJmp = (uint32_t)saved;
        } else {
            state->errJmp = (uint32_t)(uintptr_t)env;
            result = CallSlot<int32_t, 0x11F0EB8>(fnstate, a5, a6);
            state->errJmp = (uint32_t)saved;
        }
    } else {
        result = CallSlot<int32_t, 0x11F0EB8>(fnstate, a5, a6);
    }
    if (*a2)
        return result;
    if (result < kZero) {
        const int32_t errObj = GameDword(0x11F0EBC);
        if (*reinterpret_cast<const int32_t*>(static_cast<uintptr_t>(errObj) + 0x28) != kZero)
            return ScriptErrorDispatch(errObj, a2, a3, result);
        return RouteViaD8A8E0(errObj, a2, a3, result);
    }
    return result;
}

// 0x00DED790 (266 B): store a signed byte (size tag 1, sign-extended like the
// original's movsx).
int32_t ScriptSetChar(int32_t a1, int32_t* a2, int32_t a3, int32_t a4, int8_t a5)
{
    if (!a4) {
        const int32_t err = CallGameFunction<int32_t, 0xE770E0>();
        *a2 = err;
        return err;
    }
    const int32_t fnstate = FnstateOf(a4);
    int32_t result;
    if (*reinterpret_cast<const int32_t*>(0x11F2D84) > kZero) {
        int32_t env[kEnvDwords] = {};
        ScriptState* state = StateAt(a3);
        const int32_t saved = (int32_t)state->errJmp;
        volatile int32_t sj = CallGameFunction<int32_t, 0xF7EE92>(env, kZero);
        if (sj) {
            result = kZero;  // longjmp path: the prologue-zeroed result slot
            state->errJmp = (uint32_t)saved;
        } else {
            state->errJmp = (uint32_t)(uintptr_t)env;
            result = CallSlot<int32_t, 0x11F0DC8>(fnstate, (int32_t)a5, 1);
            state->errJmp = (uint32_t)saved;
        }
    } else {
        result = CallSlot<int32_t, 0x11F0DC8>(fnstate, (int32_t)a5, 1);
    }
    if (*a2)
        return result;
    if (result < kZero) {
        const int32_t errObj = GameDword(0x11F0DCC);
        if (*reinterpret_cast<const int32_t*>(static_cast<uintptr_t>(errObj) + 0x28) != kZero)
            return ScriptErrorDispatch(errObj, a2, a3, result);
        return RouteViaD8A8E0(errObj, a2, a3, result);
    }
    return result;
}

// 0x00DF5580 (265 B): the callback writes an i32 through a wrapper-owned slot; copy
// it to *a4 and return the a4 pointer.
int32_t ScriptGetInt(int32_t a1, int32_t* a2, int32_t a3, int32_t* a4, int32_t a5)
{
    if (!a5) {
        const int32_t err = CallGameFunction<int32_t, 0xE770E0>();
        *a2 = err;
        return err;
    }
    int32_t outLocal{};
    const int32_t fnstate = FnstateOf(a5);
    // Unlike the families that return the result, this family only routes on it, so
    // the longjmp zero (the original's prologue-zeroed slot) lives in the initializer
    // where it cannot be dropped-and-survive mutation testing.
    int32_t result = kZero;
    if (*reinterpret_cast<const int32_t*>(0x11F2D84) > kZero) {
        int32_t env[kEnvDwords] = {};
        ScriptState* state = StateAt(a3);
        const int32_t saved = (int32_t)state->errJmp;
        volatile int32_t sj = CallGameFunction<int32_t, 0xF7EE92>(env, kZero);
        if (sj)
            state->errJmp = (uint32_t)saved;  // longjmp landed here; result stays 0
        else {
            state->errJmp = (uint32_t)(uintptr_t)env;
            result = CallSlot<int32_t, 0x11F1544>(fnstate, &outLocal);
            state->errJmp = (uint32_t)saved;
        }
    } else {
        result = CallSlot<int32_t, 0x11F1544>(fnstate, &outLocal);
    }
    if (*a2)
        return result;
    if (result < kZero) {
        const int32_t errObj = GameDword(0x11F1548);
        if (*reinterpret_cast<const int32_t*>(static_cast<uintptr_t>(errObj) + 0x28) != kZero)
            result = ScriptErrorDispatch(errObj, a2, a3, result);
        else
            result = RouteViaD8A8E0(errObj, a2, a3, result);
        if (*a2)
            return result;
    }
    *a4 = outLocal;
    return (int32_t)(intptr_t)a4;
}

// 0x00DF6E60 (267 B): the callback writes a double through a wrapper-owned slot;
// copy it to *a4 and return the a4 pointer.
int32_t ScriptGetF64(int32_t a1, int32_t* a2, int32_t a3, double* a4, int32_t a5)
{
    if (!a5) {
        const int32_t err = CallGameFunction<int32_t, 0xE770E0>();
        *a2 = err;
        return err;
    }
    double outLocal{};  // the original's fldz/fstp qword zero
    const int32_t fnstate = FnstateOf(a5);
    int32_t result = kZero;  // longjmp path: the original's prologue-zeroed slot
    if (*reinterpret_cast<const int32_t*>(0x11F2D84) > kZero) {
        int32_t env[kEnvDwords] = {};
        ScriptState* state = StateAt(a3);
        const int32_t saved = (int32_t)state->errJmp;
        volatile int32_t sj = CallGameFunction<int32_t, 0xF7EE92>(env, kZero);
        if (sj)
            state->errJmp = (uint32_t)saved;  // longjmp landed here; result stays 0
        else {
            state->errJmp = (uint32_t)(uintptr_t)env;
            result = CallSlot<int32_t, 0x11F1360>(fnstate, &outLocal);
            state->errJmp = (uint32_t)saved;
        }
    } else {
        result = CallSlot<int32_t, 0x11F1360>(fnstate, &outLocal);
    }
    if (*a2)
        return result;
    if (result < kZero) {
        const int32_t errObj = GameDword(0x11F1364);
        if (*reinterpret_cast<const int32_t*>(static_cast<uintptr_t>(errObj) + 0x28) != kZero)
            result = ScriptErrorDispatch(errObj, a2, a3, result);
        else
            result = RouteViaD8A8E0(errObj, a2, a3, result);
        if (*a2)
            return result;
    }
    *a4 = outLocal;
    return (int32_t)(intptr_t)a4;
}

// 0x00DF8F10 (266 B): like 0x00DF5580 but EA: the out-write AND the return value are
// the zero-extended u16 callback result (movzx of the slot), not the a4 pointer.
int32_t ScriptGetU16(int32_t a1, int32_t* a2, int32_t a3, int32_t* a4, int32_t a5)
{
    if (!a5) {
        const int32_t err = CallGameFunction<int32_t, 0xE770E0>();
        *a2 = err;
        return err;
    }
    int32_t outLocal{};
    const int32_t fnstate = FnstateOf(a5);
    int32_t result = kZero;  // longjmp path: the original's prologue-zeroed slot
    if (*reinterpret_cast<const int32_t*>(0x11F2D84) > kZero) {
        int32_t env[kEnvDwords] = {};
        ScriptState* state = StateAt(a3);
        const int32_t saved = (int32_t)state->errJmp;
        volatile int32_t sj = CallGameFunction<int32_t, 0xF7EE92>(env, kZero);
        if (sj)
            state->errJmp = (uint32_t)saved;  // longjmp landed here; result stays 0
        else {
            state->errJmp = (uint32_t)(uintptr_t)env;
            result = CallSlot<int32_t, 0x11F1114>(fnstate, &outLocal);
            state->errJmp = (uint32_t)saved;
        }
    } else {
        result = CallSlot<int32_t, 0x11F1114>(fnstate, &outLocal);
    }
    if (*a2)
        return result;
    if (result < kZero) {
        const int32_t errObj = GameDword(0x11F1118);
        if (*reinterpret_cast<const int32_t*>(static_cast<uintptr_t>(errObj) + 0x28) != kZero)
            result = ScriptErrorDispatch(errObj, a2, a3, result);
        else
            result = RouteViaD8A8E0(errObj, a2, a3, result);
        if (*a2)
            return result;
    }
    const int32_t value = (int32_t)(uint16_t)outLocal;
    *a4 = value;
    return value;
}

// 0x00DFA520 (243 B): duplicate of 0x00DE9EB0 (globals 0x11F1074/0x11F1078).
int32_t CDFA520(int32_t a1, int32_t* a2, int32_t a3, int32_t* a4, int32_t a5)
{
    if (!a5) {
        const int32_t err = CallGameFunction<int32_t, 0xE770E0>();
        *a2 = err;
        return err;
    }
    const int32_t fnstate = FnstateOf(a5);
    int32_t result;
    if (*reinterpret_cast<const int32_t*>(0x11F2D84) > kZero) {
        int32_t env[kEnvDwords] = {};
        ScriptState* state = StateAt(a3);
        const int32_t saved = (int32_t)state->errJmp;
        volatile int32_t sj = CallGameFunction<int32_t, 0xF7EE92>(env, kZero);
        if (sj) {
            result = kZero;  // longjmp path: the prologue-zeroed result slot
            state->errJmp = (uint32_t)saved;
        } else {
            state->errJmp = (uint32_t)(uintptr_t)env;
            result = CallSlot<int32_t, 0x11F1074>(fnstate);
            state->errJmp = (uint32_t)saved;
        }
    } else {
        result = CallSlot<int32_t, 0x11F1074>(fnstate);
    }
    if (*a2)
        return result;
    const int32_t outValue = result;  // written to *a4 even when the route runs
    if (result < kZero) {
        const int32_t errObj = GameDword(0x11F1078);
        if (*reinterpret_cast<const int32_t*>(static_cast<uintptr_t>(errObj) + 0x28) != kZero)
            result = ScriptErrorDispatch(errObj, a2, a3, result);
        else
            result = RouteViaD8A8E0(errObj, a2, a3, result);
        if (*a2)
            return result;
    }
    *a4 = outValue;
    return (int32_t)(intptr_t)a4;
}

// 0x00DFB3A0 (257 B): duplicate of 0x00DEC4E0 (globals 0x11F0FFC/0x11F1000).
int32_t CDFB3A0(int32_t a1, int32_t* a2, int32_t a3, int32_t a4, int32_t a5, int32_t a6)
{
    if (!a4) {
        const int32_t err = CallGameFunction<int32_t, 0xE770E0>();
        *a2 = err;
        return err;
    }
    const int32_t fnstate = FnstateOf(a4);
    int32_t result;
    if (*reinterpret_cast<const int32_t*>(0x11F2D84) > kZero) {
        int32_t env[kEnvDwords] = {};
        ScriptState* state = StateAt(a3);
        const int32_t saved = (int32_t)state->errJmp;
        volatile int32_t sj = CallGameFunction<int32_t, 0xF7EE92>(env, kZero);
        if (sj) {
            result = kZero;  // longjmp path: the prologue-zeroed result slot
            state->errJmp = (uint32_t)saved;
        } else {
            state->errJmp = (uint32_t)(uintptr_t)env;
            result = CallSlot<int32_t, 0x11F0FFC>(fnstate, a5, a6);
            state->errJmp = (uint32_t)saved;
        }
    } else {
        result = CallSlot<int32_t, 0x11F0FFC>(fnstate, a5, a6);
    }
    if (*a2)
        return result;
    if (result < kZero) {
        const int32_t errObj = GameDword(0x11F1000);
        if (*reinterpret_cast<const int32_t*>(static_cast<uintptr_t>(errObj) + 0x28) != kZero)
            return ScriptErrorDispatch(errObj, a2, a3, result);
        return RouteViaD8A8E0(errObj, a2, a3, result);
    }
    return result;
}

// 0x00DFBB00 (246 B): store an i32 through the handle's callback (no size tag).
int32_t ScriptSetI32NoTag(int32_t a1, int32_t* a2, int32_t a3, int32_t a4, int32_t a5)
{
    if (!a4) {
        const int32_t err = CallGameFunction<int32_t, 0xE770E0>();
        *a2 = err;
        return err;
    }
    const int32_t fnstate = FnstateOf(a4);
    int32_t result;
    if (*reinterpret_cast<const int32_t*>(0x11F2D84) > kZero) {
        int32_t env[kEnvDwords] = {};
        ScriptState* state = StateAt(a3);
        const int32_t saved = (int32_t)state->errJmp;
        volatile int32_t sj = CallGameFunction<int32_t, 0xF7EE92>(env, kZero);
        if (sj) {
            result = kZero;  // longjmp path: the prologue-zeroed result slot
            state->errJmp = (uint32_t)saved;
        } else {
            state->errJmp = (uint32_t)(uintptr_t)env;
            result = CallSlot<int32_t, 0x11F0FDC>(fnstate, a5);
            state->errJmp = (uint32_t)saved;
        }
    } else {
        result = CallSlot<int32_t, 0x11F0FDC>(fnstate, a5);
    }
    if (*a2)
        return result;
    if (result < kZero) {
        const int32_t errObj = GameDword(0x11F0FE0);
        if (*reinterpret_cast<const int32_t*>(static_cast<uintptr_t>(errObj) + 0x28) != kZero)
            return ScriptErrorDispatch(errObj, a2, a3, result);
        return RouteViaD8A8E0(errObj, a2, a3, result);
    }
    return result;
}

// 0x00DFC0C0 (266 B): store a signed i16 (size tag 2, sign-extended like the
// original's movsx).
int32_t ScriptSetI16(int32_t a1, int32_t* a2, int32_t a3, int32_t a4, int16_t a5)
{
    if (!a4) {
        const int32_t err = CallGameFunction<int32_t, 0xE770E0>();
        *a2 = err;
        return err;
    }
    const int32_t fnstate = FnstateOf(a4);
    int32_t result;
    if (*reinterpret_cast<const int32_t*>(0x11F2D84) > kZero) {
        int32_t env[kEnvDwords] = {};
        ScriptState* state = StateAt(a3);
        const int32_t saved = (int32_t)state->errJmp;
        volatile int32_t sj = CallGameFunction<int32_t, 0xF7EE92>(env, kZero);
        if (sj) {
            result = kZero;  // longjmp path: the prologue-zeroed result slot
            state->errJmp = (uint32_t)saved;
        } else {
            state->errJmp = (uint32_t)(uintptr_t)env;
            result = CallSlot<int32_t, 0x11F0FBC>(fnstate, (int32_t)a5, 2);
            state->errJmp = (uint32_t)saved;
        }
    } else {
        result = CallSlot<int32_t, 0x11F0FBC>(fnstate, (int32_t)a5, 2);
    }
    if (*a2)
        return result;
    if (result < kZero) {
        const int32_t errObj = GameDword(0x11F0FC0);
        if (*reinterpret_cast<const int32_t*>(static_cast<uintptr_t>(errObj) + 0x28) != kZero)
            return ScriptErrorDispatch(errObj, a2, a3, result);
        return RouteViaD8A8E0(errObj, a2, a3, result);
    }
    return result;
}

// 0x00DFC260 (257 B): duplicate of 0x00DEC4E0 (globals 0x11F0FB0/0x11F0FB4).
int32_t CDFC260(int32_t a1, int32_t* a2, int32_t a3, int32_t a4, int32_t a5, int32_t a6)
{
    if (!a4) {
        const int32_t err = CallGameFunction<int32_t, 0xE770E0>();
        *a2 = err;
        return err;
    }
    const int32_t fnstate = FnstateOf(a4);
    int32_t result;
    if (*reinterpret_cast<const int32_t*>(0x11F2D84) > kZero) {
        int32_t env[kEnvDwords] = {};
        ScriptState* state = StateAt(a3);
        const int32_t saved = (int32_t)state->errJmp;
        volatile int32_t sj = CallGameFunction<int32_t, 0xF7EE92>(env, kZero);
        if (sj) {
            result = kZero;  // longjmp path: the prologue-zeroed result slot
            state->errJmp = (uint32_t)saved;
        } else {
            state->errJmp = (uint32_t)(uintptr_t)env;
            result = CallSlot<int32_t, 0x11F0FB0>(fnstate, a5, a6);
            state->errJmp = (uint32_t)saved;
        }
    } else {
        result = CallSlot<int32_t, 0x11F0FB0>(fnstate, a5, a6);
    }
    if (*a2)
        return result;
    if (result < kZero) {
        const int32_t errObj = GameDword(0x11F0FB4);
        if (*reinterpret_cast<const int32_t*>(static_cast<uintptr_t>(errObj) + 0x28) != kZero)
            return ScriptErrorDispatch(errObj, a2, a3, result);
        return RouteViaD8A8E0(errObj, a2, a3, result);
    }
    return result;
}

}  // namespace ts3::scripting
