#pragma once
// Cluster unit clu_de6ed0_Scripting (scripting subsystem): TS3W.exe 0x00DE6ED0 (490 B),
// 0x00DE9D10 (266), 0x00DE9EB0 (243), 0x00DEA2B0 (265), 0x00DEA450 (258), 0x00DEA5F0
// (243), 0x00DEA780 (265), 0x00DEA920 (266), 0x00DEB530 (254), 0x00DEC4E0 (263),
// 0x00DEC620 (247), 0x00DEC810 (243), 0x00DEC940 (263), 0x00DED790 (266), 0x00DF5580
// (265), 0x00DF6E60 (267), 0x00DF8F10 (266), 0x00DFA520 (243), 0x00DFB3A0 (257),
// 0x00DFBB00 (246), 0x00DFC0C0 (266), 0x00DFC260 (257) — 22 functions, 5899 bytes.
//
// Protected-call wrappers around the script (Lua-like) runtime's per-handle value
// callbacks. Every wrapper is cdecl and shares the same shape: a1 (never read), a2 =
// int* error out, a3 = scripting state object (its +0x10 slot is the running wrapper's
// setjmp buffer), then a handle (null-tested; +4 is the callback-table index) and a
// value or out-pointer. Flow: null handle -> route through 0x00E770E0 (a generic "bad
// argument" error); otherwise the per-handle callback at the unit's global slot runs,
// either directly or under a setjmp3 protection frame (flag global 0x11F2D84 <= 0 ->
// direct, > 0 -> protected, so a longjmp out of script code lands back in the wrapper
// with a zeroed result slot). A negative callback result is routed to error formatting:
// if the unit's error object has an entry at +0x28 the shared dispatcher 0x00DE6ED0
// handles it, else the record is handed straight to 0x00D8A8E0.
//
// DE6ED0 itself is the dispatcher for the script runtime's error codes -7..-1: each
// case chain-reads the error id from one of the fixed globals at 0x11F0C74..0x11F0C9C,
// looks the id up under the process lock (0x00E75260 / 0x00EA36F0) and formats the
// record via 0x00D8A8E0; any other code returns code+7. EA quirk kept: 0x00DF8F10's
// out-write stores and RETURNS the zero-extended u16 value, not the out-pointer like
// its siblings 0x00DF5580/0x00DF6E60. On x64 every entry aborts (the scripting runtime
// is never reached by datatests).
#include <cstdint>

#include "ts3/layout.h"

namespace ts3::scripting {

// The scripting state object handed to every wrapper as a3. Its +0x10 field holds the
// setjmp buffer of the wrapper currently running a protected callback (or null).
struct ScriptState {
    uint8_t padToErrJmp[0x10];
    uint32_t errJmp;  // +0x10: setjmp3 buffer pointer of the active wrapper
};
TS3_X86_OFFSET(ScriptState, errJmp, 0x10);

// The per-script handle passed as the wrapper's a4/a5. +4 indexes the value-callback
// table stored in the unit's per-wrapper global (e.g. 0x11F0B3C for 0x00DE9D10).
struct ScriptHandle {
    uint8_t padToFnstate[0x4];
    int32_t fnstate;  // +0x04: callback-table slot value
};
TS3_X86_OFFSET(ScriptHandle, fnstate, 0x04);

// The error object stored in each wrapper's errObj global (e.g. 0x11F0B40). +0x28 is
// the script-runtime dispatcher entry: set -> route through 0x00DE6ED0, clear -> the
// wrapper formats via 0x00D8A8E0 itself.
struct ScriptErrorObject {
    uint8_t padToDispatcher[0x28];
    int32_t dispatcher;  // +0x28: entry of the shared error dispatcher
};
TS3_X86_OFFSET(ScriptErrorObject, dispatcher, 0x28);

// 0x00DE6ED0 (490 B): script error dispatcher for codes -7..-1; anything else returns
// code+7. a1 (the caller's error object) is never read.
int32_t ScriptErrorDispatch(int32_t a1, int32_t* a2, int32_t a3, int32_t a4);

// Value setters (family A: return the callback/routing result). Tag, when pushed, is
// the value's size in bytes (1/2/4).
int32_t ScriptSetU16(int32_t a1, int32_t* a2, int32_t a3, int32_t a4, uint16_t a5);   // 0x00DE9D10 (266 B) slot 0x11F0B3C/err 0x11F0B40, tag 2
int32_t ScriptSetI32(int32_t a1, int32_t* a2, int32_t a3, int32_t a4, int32_t a5);    // 0x00DEA2B0 (265 B) slot 0x11F0B1C/err 0x11F0B20, tag 4
int32_t ScriptSetF64(int32_t a1, int32_t* a2, int32_t a3, int32_t a4, double a5);    // 0x00DEA450 (258 B) slot 0x11F0B10/err 0x11F0B14, no tag
int32_t CDEA780(int32_t a1, int32_t* a2, int32_t a3, int32_t a4, int32_t a5);        // 0x00DEA780 (265 B) slot 0x11F0AF8/err 0x11F0AFC, tag 4 (dup of DEA2B0)
int32_t ScriptSetU8(int32_t a1, int32_t* a2, int32_t a3, int32_t a4, uint8_t a5);    // 0x00DEA920 (266 B) slot 0x11F0AEC/err 0x11F0AF0, tag 1 (movzx)
int32_t ScriptSetF32(int32_t a1, int32_t* a2, int32_t a3, int32_t a4, float a5);     // 0x00DEB530 (254 B) slot 0x11F0A88/err 0x11F0A8C, no tag
int32_t ScriptSetPair(int32_t a1, int32_t* a2, int32_t a3, int32_t a4, int32_t a5,
                      int32_t a6);                                                   // 0x00DEC4E0 (263 B) slot 0x11F0F04/err 0x11F0F08, no tag
int32_t ScriptSetU16NoTag(int32_t a1, int32_t* a2, int32_t a3, int32_t a4,
                          uint16_t a5);                                              // 0x00DEC620 (247 B) slot 0x11F0EF8/err 0x11F0EFC, no tag (movzx)
int32_t CDEC940(int32_t a1, int32_t* a2, int32_t a3, int32_t a4, int32_t a5,
                int32_t a6);                                                         // 0x00DEC940 (263 B) slot 0x11F0EB8/err 0x11F0EBC, no tag (dup of DEC4E0)
int32_t ScriptSetChar(int32_t a1, int32_t* a2, int32_t a3, int32_t a4, int8_t a5);   // 0x00DED790 (266 B) slot 0x11F0DC8/err 0x11F0DCC, tag 1 (movsx)
int32_t CDFB3A0(int32_t a1, int32_t* a2, int32_t a3, int32_t a4, int32_t a5,
                int32_t a6);                                                         // 0x00DFB3A0 (257 B) slot 0x11F0FFC/err 0x11F1000, no tag (dup of DEC4E0)
int32_t ScriptSetI32NoTag(int32_t a1, int32_t* a2, int32_t a3, int32_t a4,
                          int32_t a5);                                               // 0x00DFBB00 (246 B) slot 0x11F0FDC/err 0x11F0FE0, no tag
int32_t ScriptSetI16(int32_t a1, int32_t* a2, int32_t a3, int32_t a4, int16_t a5);   // 0x00DFC0C0 (266 B) slot 0x11F0FBC/err 0x11F0FC0, tag 2 (movsx)
int32_t CDFC260(int32_t a1, int32_t* a2, int32_t a3, int32_t a4, int32_t a5,
                int32_t a6);                                                         // 0x00DFC260 (257 B) slot 0x11F0FB0/err 0x11F0FB4, no tag (dup of DEC4E0)

// Result getters (family B: the callback takes the handle only and its result is the
// value; it is written through a4, and the wrapper returns the a4 pointer).
int32_t ScriptGetResult(int32_t a1, int32_t* a2, int32_t a3, int32_t* a4,
                        int32_t a5);                                                 // 0x00DE9EB0 (243 B) slot 0x11F0B30/err 0x11F0B34
int32_t CDEA5F0(int32_t a1, int32_t* a2, int32_t a3, int32_t* a4, int32_t a5);       // 0x00DEA5F0 (243 B) slot 0x11F0B04/err 0x11F0B08 (dup of DE9EB0)
int32_t CDEC810(int32_t a1, int32_t* a2, int32_t a3, int32_t* a4, int32_t a5);       // 0x00DEC810 (243 B) slot 0x11F0EDC/err 0x11F0EE0 (dup of DE9EB0)
int32_t CDFA520(int32_t a1, int32_t* a2, int32_t a3, int32_t* a4, int32_t a5);       // 0x00DFA520 (243 B) slot 0x11F1074/err 0x11F1078 (dup of DE9EB0)

// Out-param getters (family C: the callback writes its value through a pointer the
// wrapper owns; the written result is then copied to *a4).
int32_t ScriptGetInt(int32_t a1, int32_t* a2, int32_t a3, int32_t* a4,
                     int32_t a5);                                                    // 0x00DF5580 (265 B) slot 0x11F1544/err 0x11F1548, returns the a4 pointer
int32_t ScriptGetF64(int32_t a1, int32_t* a2, int32_t a3, double* a4,
                     int32_t a5);                                                    // 0x00DF6E60 (267 B) slot 0x11F1360/err 0x11F1364, returns the a4 pointer
// 0x00DF8F10 (266 B) slot 0x11F1114/err 0x11F1118: EA quirk - the out-write AND the
// return value are the zero-extended u16 callback result, not the a4 pointer.
int32_t ScriptGetU16(int32_t a1, int32_t* a2, int32_t a3, int32_t* a4, int32_t a5);

}  // namespace ts3::scripting
