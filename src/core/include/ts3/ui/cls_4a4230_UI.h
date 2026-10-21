// cls_4a4230_UI: the row list behind EA's UI grid control (RTTI name
// EA::UTFWinControls::MatrixRowList). Ten entry points from 0x004A4230 to 0x005319F0:
// construction, hit testing, keyboard navigation, a coalescing stopwatch and the
// selection bookkeeping. Everything below is the original 32-bit layout (checked on x86);
// pointer-holding fields are pointer-sized so x64 builds keep working.
#pragma once

#include <cstdint>

#include "ts3/layout.h"

namespace ts3::ui {

// 0x004A4230: a wide-string vector as the game's containers keep it: begin/end/capacity
// and the allocator it was built with at +0x0C. Elements are 2 bytes (UTF-16).
struct Utf16Vector {
    uint8_t* begin;     // +0x00
    uint8_t* end;       // +0x04
    uint8_t* capacity;  // +0x08
    void* allocator;    // +0x0C (copied out of the caller's allocator slot)

    // Replaces the contents with `count` UTF-16 code units from src plus a terminator.
    // The original grows through the game's vector helper 0x00408830 with
    // (((src + count*2) - src) >> 1) + 1 elements; for counts 0 and 1 that helper keeps
    // the buffer in its static sentinel (0x011C7E54) instead of allocating, so the
    // terminator write below lands in that shared sentinel (EA bug kept for fidelity).
    Utf16Vector* Assign4A4230(const void* src, int32_t count, void** allocatorSlot);
};
TS3_X86_OFFSET(Utf16Vector, begin, 0x00);
TS3_X86_OFFSET(Utf16Vector, end, 0x04);
TS3_X86_OFFSET(Utf16Vector, capacity, 0x08);
TS3_X86_OFFSET(Utf16Vector, allocator, 0x0C);
TS3_X86_SIZE(Utf16Vector, 0x10);

// A UI event the grid sends through slot 0x118 of the +0x04 interface. Only the fields
// listed here are written by the original; it leaves pad_00/pad_04 (and, per message,
// payload_a or payload_b) uninitialized. The rewrite zero-fills them (no output change:
// the receiver is a game function that reads type/source/payload only).
struct GridMessage {
    uint32_t pad_00;        // +0x00 uninitialized in the original
    uint32_t pad_04;        // +0x04 uninitialized in the original
    uint32_t type;          // +0x08
    void* source;           // +0x0C f_03C, or f_038 when f_03C is null
    uint32_t arg_10;        // +0x10
    uint32_t arg_14;        // +0x14
    const void* payload_a;  // +0x18
    const void* payload_b;  // +0x1C
};
TS3_X86_SIZE(GridMessage, 0x20);

class MatrixRowList {
public:
    // ---- entry points, in address order ---------------------------------------------
    // 0x00523710: zero the header (dwords 1..8 and 10/11, byte +0x24 = 0xFF, bytes
    // +0x25/+0x26 = 0) and set +0x30/+0x34 to the game's -1.0f (0x00FAA210). Byte +0x27
    // is deliberately left untouched, exactly like the original.
    MatrixRowList* Init523710();
    // 0x00523760: same header writes, then the original copies 0x30 bytes over itself
    // (so only the two -1.0f floats survive) - i.e. a copy constructor of the 0x30-byte
    // header plus that sentinel pair.
    MatrixRowList* InitCopy523760(const void* src);
    // 0x00523800: forwards a3 / -100 (the original's magic multiply, truncated toward
    // zero) to slot 0xAC of the +0x250 object. a0/a1/a2 are unused.
    int32_t ScrollRows523800(int32_t a0, int32_t a1, int32_t a2, int32_t a3);
    // 0x005241D0: lazily creates the row list through slot 0x74 of the +0x04 interface
    // (8, 1) and remembers it in f_258, then tail-calls 0x00514340.
    int32_t EnsureRowList5241D0();
    // 0x00528F20: asks the +0x250 object's slot 0x1D4 for a cell under (a2, a3); the
    // method writes the (possibly snapped) y back through a3 and the cell through a
    // 2-int cell. When the hit cell differs from f_408/f_40C/f_410 the original sends
    // event 0x9A1552D6 (payload: y bits, cell x, cell y) through slot 0x118 of +0x04.
    // a4 is passed by the original but never read.
    uint8_t HitTest528F20(float a2, float a3, int32_t a4);
    // 0x00529050: mode change notification: a0 == 0 clears the cached hit when bit 0 of
    // f_254 is set, a0 == 1 clears it when a1 is not the +0x04 interface, a0 == 2 (or
    // anything else) does nothing. Always reports success in AL.
    uint8_t SetMode529050(int32_t a0, const void* a1);
    // 0x005297F0: keyboard navigation, only when a0 == 0 and f_254 has bit 26 set.
    // '!'/'"' call 0x00524510/0x00524560 (or 0x00524540/0x00524590 when flags has bit 2);
    // '#' moves the selection to the neighbouring row/column; '$' clears it; '%', '&',
    // '\'' and '(' go to 0x00528C20(key, flags); '-' toggles bit 7 of f_254. AL
    // reports success, the upper EAX bytes are whatever the last call left there.
    uint8_t OnKey5297F0(int32_t a0, int32_t key, uint32_t flags);
    // 0x00529A20: the per-frame update: restart the +0x2F0 stopwatch (mode 1 = rdtsc,
    // mode 0 = the game's timestamp function at 0x00F953B8) and, when it had been running
    // for 500+ ticks, remember the timeout; then ask slot 0x1D4 of +0x250 for the cell
    // under (x, y) and act on the command it returns: 2 = reselect the row range, 3 =
    // the column range, 5 = selection click/drag, anything else just updates the cache.
    // flags bit 0/1 select the cell/node paths (only when f_254 has bit 27). Finally the
    // cache f_3FC/f_400/f_404 is updated, the child object (f_030) is notified and up to
    // three events (0x9A1552D4 timeout, 0x9A1552D3 updated, 0x9A1552D2 selection) are
    // sent through slot 0x118 of +0x04. AL = (command != 0).
    uint8_t Update529A20(float x, float y, int32_t a3, uint32_t flags);
    // 0x005319F0: when the row list exists, refresh it (0x005315E0) and invalidate
    // through slot 0x90 of +0x04. Always reports success in AL.
    uint8_t Refresh5319F0();

    // ---- layout (sizeof 0x500) --------------------------------------------------------
    uint8_t* Sub4() { return reinterpret_cast<uint8_t*>(this) + 4; }
    uint8_t* Sub250() { return reinterpret_cast<uint8_t*>(this) + 0x250; }

    void* vtbl;        // +0x000
    void* vtbl_sec;    // +0x004 the +0x04 interface's vtable
    uint32_t f_008;    // +0x008
    uint32_t f_00C;    // +0x00C
    uint32_t f_010;    // +0x010
    uint32_t f_014;    // +0x014
    uint32_t f_018;    // +0x018
    uint32_t f_01C;    // +0x01C
    uint32_t f_020;    // +0x020
    uint8_t f_024;     // +0x024 (0xFF after construction)
    uint8_t f_025;     // +0x025
    uint8_t f_026;     // +0x026
    uint8_t f_027;     // +0x027 untouched by construction
    uint32_t f_028;    // +0x028
    uint32_t f_02C;    // +0x02C
    void* f_030;       // +0x030 the child object 0x00529A20 notifies; the constructors
                       //        store -1.0f here as a "none" sentinel (non-null, EA quirk)
    float f_034;       // +0x034 (-1.0f after construction)
    void* f_038;       // +0x038 event source fallback
    void* f_03C;       // +0x03C event source (preferred)
    uint8_t pad_040[0x210];
    void* f_250;       // +0x250 the child grid object (its vtable is called by slot)
    uint32_t f_254;    // +0x254 flags (bit 0, 7, 23, 26, 27 are read by this unit)
    uint8_t f_258;     // +0x258 "row list created" flag
    uint8_t pad_259[3];
    int32_t f_25C;     // +0x25C
    int32_t f_260;     // +0x260
    uint8_t pad_264[0x5C];
    int32_t f_2C0;     // +0x2C0 selected row (or -1)
    int32_t f_2C4;     // +0x2C4 selected column (or -1)
    uint8_t pad_2C8[0x28];
    uint32_t f_2F0[5]; // +0x2F0 stopwatch: start lo/hi, accumulated lo/hi, clock mode
    int32_t f_304;     // +0x304 stopwatch scale (not touched by this unit)
    int32_t f_308;     // +0x308
    int32_t f_30C;     // +0x30C
    int32_t f_310;     // +0x310
    void* f_314;       // +0x314 selection list head (points at itself when empty)
    void* f_318;       // +0x318 first selection node
    uint32_t f_31C;    // +0x31C
    uint32_t f_320;    // +0x320
    uint32_t f_324;    // +0x324
    int32_t f_328;     // +0x328
    int32_t f_32C;     // +0x32C
    uint8_t f_330;     // +0x330
    uint8_t pad_331[3];
    int32_t f_334;     // +0x334
    int32_t f_338;     // +0x338
    int32_t f_33C;     // +0x33C
    uint8_t pad_340[0x8C];
    int32_t f_3CC;     // +0x3CC
    int32_t f_3D0;     // +0x3D0
    uint8_t f_3D4;     // +0x3D4
    uint8_t f_3D5;     // +0x3D5
    uint8_t f_3D6;     // +0x3D6
    uint8_t pad_3D7[1];
    int32_t f_3D8;     // +0x3D8
    int32_t f_3DC;     // +0x3DC
    uint8_t pad_3E0[8];
    int32_t f_3E8;     // +0x3E8
    int32_t f_3EC;     // +0x3EC
    int32_t f_3F0;     // +0x3F0
    int32_t f_3F4;     // +0x3F4
    int32_t f_3F8;     // +0x3F8
    int32_t f_3FC;     // +0x3FC last command
    int32_t f_400;     // +0x400 last hit cell x
    int32_t f_404;     // +0x404 last hit cell y
    int32_t f_408;     // +0x408 last y (as bits)
    int32_t f_40C;     // +0x40C last cell x
    int32_t f_410;     // +0x410 last cell y
    uint8_t pad_414[0xEC];
};
TS3_X86_OFFSET(MatrixRowList, vtbl, 0x000);
TS3_X86_OFFSET(MatrixRowList, vtbl_sec, 0x004);
TS3_X86_OFFSET(MatrixRowList, f_024, 0x024);
TS3_X86_OFFSET(MatrixRowList, f_030, 0x030);
TS3_X86_OFFSET(MatrixRowList, f_034, 0x034);
TS3_X86_OFFSET(MatrixRowList, f_038, 0x038);
TS3_X86_OFFSET(MatrixRowList, f_03C, 0x03C);
TS3_X86_OFFSET(MatrixRowList, f_250, 0x250);
TS3_X86_OFFSET(MatrixRowList, f_254, 0x254);
TS3_X86_OFFSET(MatrixRowList, f_258, 0x258);
TS3_X86_OFFSET(MatrixRowList, f_2C0, 0x2C0);
TS3_X86_OFFSET(MatrixRowList, f_2C4, 0x2C4);
TS3_X86_OFFSET(MatrixRowList, f_2F0, 0x2F0);
TS3_X86_OFFSET(MatrixRowList, f_304, 0x304);
TS3_X86_OFFSET(MatrixRowList, f_310, 0x310);
TS3_X86_OFFSET(MatrixRowList, f_314, 0x314);
TS3_X86_OFFSET(MatrixRowList, f_318, 0x318);
TS3_X86_OFFSET(MatrixRowList, f_330, 0x330);
TS3_X86_OFFSET(MatrixRowList, f_3D4, 0x3D4);
TS3_X86_OFFSET(MatrixRowList, f_3E8, 0x3E8);
TS3_X86_OFFSET(MatrixRowList, f_3FC, 0x3FC);
TS3_X86_OFFSET(MatrixRowList, f_400, 0x400);
TS3_X86_OFFSET(MatrixRowList, f_404, 0x404);
TS3_X86_OFFSET(MatrixRowList, f_408, 0x408);
TS3_X86_OFFSET(MatrixRowList, f_410, 0x410);
TS3_X86_SIZE(MatrixRowList, 0x500);

}  // namespace ts3::ui