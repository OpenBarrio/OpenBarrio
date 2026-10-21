// cls_4a4230_UI: portable C++ for the ten MatrixRowList entry points
// (0x004A4230, 0x00523710, 0x00523760, 0x00523800, 0x005241D0,
// 0x00528F20, 0x00529050, 0x005297F0, 0x00529A20, 0x005319F0). Functions of other units
// are reached through the ts3::CallGame* bridges from ts3/layout.h; already rewritten
// dependencies (0x004F3220) are called directly. Behaviour quirks of the original that
// can show up in outputs are kept and marked "EA bug".
#include "ts3/ui/cls_4a4230_UI.h"

#include <cstring>
#include <intrin.h>

#include "ts3/layout.h"
#include "ts3/objects/clu_4f3220_Objects.h"

namespace ts3::ui {

namespace {

constexpr uintptr_t kNegOneFloatAddress = 0x00FAA210;  // float -1.0f
constexpr uintptr_t kNowFunctionAddress = 0x00F953B8;  // void __stdcall(uint64_t*)

// Event types the original sends through slot 0x118 of the +0x04 interface.
constexpr uint32_t kEventSelectionChanged = 0x9A1552D2u;
constexpr uint32_t kEventUpdated = 0x9A1552D3u;
constexpr uint32_t kEventUpdateTimedOut = 0x9A1552D4u;
constexpr uint32_t kEventHitChanged = 0x9A1552D6u;

// The construction sentinels are the game's -1.0f constant, loaded from its global in the
// original (a test may patch that global, so keep reading it on x86).
float NegOneFloat()
{
#if defined(_M_IX86)
    return *reinterpret_cast<const float*>(kNegOneFloatAddress);
#else
    return -1.0f;
#endif
}

using NowFunction = void(__stdcall*)(uint64_t* now);

// Mode 0 of the stopwatch reads the time through the game's function pointer at
// 0x00F953B8 (the original calls through the pointer stored there); reading the pointer at run
// time keeps it fakeable.
void ReadCurrentTime(uint64_t* now)
{
#if defined(_M_IX86)
    (*reinterpret_cast<NowFunction const*>(kNowFunctionAddress))(now);
#else
    (void)now;
    AbortNotRewritten("the game's current-time function");
#endif
}

// One event through slot 0x118 of the +0x04 interface. The original leaves pad_00/pad_04
// (and one payload per message type) uninitialized; the rewrite zero-fills them.
void SendEvent(uint8_t* iface, void* source, uint32_t type, uint32_t arg10, uint32_t arg14,
               const void* payloadA, const void* payloadB)
{
    GridMessage message = {};
    message.type = type;
    message.source = source;
    message.arg_10 = arg10;
    message.arg_14 = arg14;
    message.payload_a = payloadA;
    message.payload_b = payloadB;
    ts3::CallGameVirtual<void, 0x118 / 4>(iface, &message);
}

}  // namespace

// 0x004A4230
Utf16Vector* Utf16Vector::Assign4A4230(const void* src, int32_t count, void** allocatorSlot)
{
    begin = nullptr;
    end = nullptr;
    capacity = nullptr;
    allocator = *allocatorSlot;
    // The original computes the element count as (((src + count * 2) - src) >> 1) + 1,
    // i.e. count + 1 (wrapping); 0x00408830 keeps counts <= 1 in its static sentinel
    // (0x011C7E54) instead of allocating, so with count 1 the terminator write below
    // lands in that sentinel - EA bug, kept because it is observable.
    const int32_t elements = static_cast<int32_t>(static_cast<uint32_t>(count) + 1u);
    ts3::CallGameMethod<void, 0x00408830>(this, elements);
    uint8_t* const data = begin;
    const size_t bytes = static_cast<size_t>(static_cast<uint32_t>(count)) * 2u;
    std::memmove(data, src, bytes);
    end = data + bytes;
    *reinterpret_cast<uint16_t*>(data + bytes) = 0;
    return this;
}

// 0x00523710
MatrixRowList* MatrixRowList::Init523710()
{
    const float minusOne = NegOneFloat();
    vtbl = nullptr;
    f_024 = 0xFF;
    f_025 = 0;
    f_026 = 0;
    f_028 = 0;
    f_02C = 0;
    vtbl_sec = nullptr;
    f_008 = 0;
    f_00C = 0;
    f_010 = 0;
    f_014 = 0;
    f_018 = 0;
    f_01C = 0;
    f_020 = 0;
    *reinterpret_cast<float*>(&f_030) = minusOne;
    f_034 = minusOne;
    return this;
}

// 0x00523760
MatrixRowList* MatrixRowList::InitCopy523760(const void* src)
{
    const float minusOne = NegOneFloat();
    // The original writes the same header as 0x00523710 and then copies 0x30 bytes over
    // it, so byte +0x24 and every other header write is dead; only the sentinel floats
    // at +0x30/+0x34 survive. (Kept in that order for the record.)
    vtbl = nullptr;
    f_025 = 0;
    f_026 = 0;
    f_028 = 0;
    f_02C = 0;
    vtbl_sec = nullptr;
    f_008 = 0;
    f_00C = 0;
    f_010 = 0;
    f_014 = 0;
    f_018 = 0;
    f_01C = 0;
    f_020 = 0;
    f_024 = 0xFF;
    *reinterpret_cast<float*>(&f_030) = minusOne;
    f_034 = minusOne;
    std::memcpy(this, src, 0x30);
    return this;
}

// 0x00523800
int32_t MatrixRowList::ScrollRows523800(int32_t a0, int32_t a1, int32_t a2, int32_t a3)
{
    (void)a0;
    (void)a1;
    (void)a2;
    // The original multiplies by 0xAE147AE1 and shifts right by 5, i.e. divides by -100
    // with truncation toward zero (C++ integer division behaves the same).
    return ts3::CallGameVirtual<int32_t, 0x0AC / 4>(Sub250(), a3 / -100);
}

// 0x005241D0
int32_t MatrixRowList::EnsureRowList5241D0()
{
    if (f_258 == 0) {
        ts3::CallGameVirtual<void, 0x074 / 4>(Sub4(), 8, 1);
        f_258 = 1;
    }
    return ts3::CallGameMethod<int32_t, 0x00514340>(this);
}

// 0x00528F20
uint8_t MatrixRowList::HitTest528F20(float a2, float a3, int32_t a4)
{
    (void)a4;  // passed by the original, never read
    uint8_t* const sub = Sub250();
    uint8_t* const iface = Sub4();
    int32_t cell[2] = {0, 0};
    float hitY = a3;  // the +0x250 method may snap this value through the 3rd argument
    if (ts3::CallGameVirtual<uint8_t, 0x1D4 / 4>(sub, a2, a3, &hitY, cell) == 0) {
        ts3::CallGameMethod<void, 0x00527AB0>(this, cell);
        f_410 = cell[1];
        std::memcpy(&f_408, &hitY, sizeof(f_408));
        f_40C = cell[0];
        (void)ts3::CallGameVirtual<int32_t, 0x090 / 4>(iface);
        return 0;
    }
    uint32_t hitBits;
    std::memcpy(&hitBits, &hitY, sizeof(hitBits));
    if (hitBits != static_cast<uint32_t>(f_408) || cell[0] != f_40C || cell[1] != f_410) {
        // Payload: the (snapped) y bits and the cell; the message type is 0x9A1552D6.
        uint32_t payload[3];
        payload[0] = hitBits;
        payload[1] = static_cast<uint32_t>(cell[0]);
        payload[2] = static_cast<uint32_t>(cell[1]);
        void* const source = f_03C != nullptr ? f_03C : f_038;
        SendEvent(iface, source, kEventHitChanged, 0, 0, payload, nullptr);
        ts3::CallGameMethod<void, 0x00527AB0>(this, cell);
        f_40C = cell[0];
        f_408 = static_cast<int32_t>(hitBits);
        f_410 = cell[1];
        (void)ts3::CallGameVirtual<int32_t, 0x090 / 4>(iface);
    }
    // The original returns the y bits in EAX and only forces AL = (EAX != 0).
    return hitBits != 0 ? 1 : 0;
}

// 0x00529050
uint8_t MatrixRowList::SetMode529050(int32_t a0, const void* a1)
{
    if (a0 != 0) {
        if (a0 == 1) {
            // The original tests its this pointer here; a member call cannot be null, but
            // the comparison value stays the address of the +0x04 interface.
            const uintptr_t child = this != nullptr ? reinterpret_cast<uintptr_t>(this) + 4 : 0;
            if (reinterpret_cast<uintptr_t>(a1) != child) {
                int32_t cell[2] = {-1, -1};
                f_408 = 0;
                ts3::CallGameMethod<void, 0x00527AB0>(this, cell);
                (void)ts3::CallGameVirtual<int32_t, 0x090 / 4>(Sub4());
            }
        }
    } else if ((f_254 & 1u) != 0) {
        (void)ts3::CallGameVirtual<int32_t, 0x090 / 4>(Sub4());
    }
    return 1;
}

// 0x005297F0
uint8_t MatrixRowList::OnKey5297F0(int32_t a0, int32_t key, uint32_t flags)
{
    uint8_t* const sub = Sub250();
    const uint32_t entryFlags = f_254;
    if (a0 != 0 || (entryFlags & 0x04000000u) == 0)
        return 0;
    switch (key) {
    case '!':  // 0x21
        if ((flags & 4u) != 0)
            ts3::CallGameMethod<void, 0x00524560>(this);
        else
            ts3::CallGameMethod<void, 0x00524510>(this);
        break;
    case '"':  // 0x22
        if ((flags & 4u) != 0)
            ts3::CallGameMethod<void, 0x00524590>(this);
        else
            ts3::CallGameMethod<void, 0x00524540>(this);
        break;
    case '#': {  // 0x23: move the selection one row/column up or down
        uint32_t box[4] = {0, 0, 0, 0};
        if ((entryFlags & 0x00010000u) == 0 || (entryFlags & 0x00008000u) == 0)
            ts3::CallGameVirtual<void, 0x0F8 / 4>(sub, box);
        if ((entryFlags & 0x00008000u) != 0) {
            const int32_t row = f_2C0;
            if (row != -1 && row != 0)
                ts3::CallGameVirtual<void, 0x0BC / 4>(sub, row - 1);
        } else {
            ts3::CallGameVirtual<void, 0x0BC / 4>(sub, static_cast<int32_t>(box[2]));
        }
        if ((entryFlags & 0x00010000u) != 0) {
            const int32_t column = f_2C4;
            if (column != -1 && column != 0)
                (void)ts3::CallGameVirtual<int32_t, 0x0C0 / 4>(sub, column - 1);
        } else {
            (void)ts3::CallGameVirtual<int32_t, 0x0C0 / 4>(sub, static_cast<int32_t>(box[3]));
        }
        break;
    }
    case '$':  // 0x24: clear the selection
        (void)ts3::CallGameVirtual<int32_t, 0x0C4 / 4>(sub, 0, 0);
        break;
    case '%':  // 0x25..0x28 are forwarded to another unit (the handler returns its
    case '&':  // eax unchanged, so the key result is the callee's)
    case '\'':
    case '(':
        return static_cast<uint8_t>(
            ts3::CallGameMethod<int32_t, 0x00528C20>(this, key, static_cast<int32_t>(flags)));
    case '-':  // 0x2D: toggle bit 7
        f_254 = (entryFlags & 0x80u) == 0 ? (entryFlags | 0x80u) : (entryFlags & ~0x80u);
        break;
    default:
        return 0;
    }
    return 1;
}

// 0x00529A20
uint8_t MatrixRowList::Update529A20(float x, float y, int32_t a3, uint32_t flags)
{
    uint8_t* const sub = Sub250();
    uint8_t* const iface = Sub4();
    const bool bit27 = ((f_254 >> 27) & 1u) != 0;
    const bool flagCell = bit27 && ((flags & 1u) != 0);
    const bool flagRow = bit27 && ((flags & 2u) != 0);
    bool timedOut = false;

    // Stopwatch at +0x2F0: stop it when it has been running for 500+ ticks, then restart
    // it (mode 1 = rdtsc, mode 0 = the game's timestamp function).
    {
        uint32_t* const stopwatch = f_2F0;
        if (stopwatch[0] != 0 || stopwatch[1] != 0) {
            const uint64_t elapsed = ts3::CallGameMethod<uint64_t, 0x004F32C0>(stopwatch);
            if (elapsed < 500u) {
                timedOut = true;
                (void)ts3::objects::StopwatchStop4F3220(
                    reinterpret_cast<ts3::objects::Stopwatch4F3220*>(stopwatch), nullptr);
            }
        }
        const bool useRdtsc = stopwatch[4] == 1;
        stopwatch[0] = 0;
        stopwatch[1] = 0;
        stopwatch[2] = 0;
        stopwatch[3] = 0;
        uint64_t now = 0;
        if (useRdtsc)
            now = __rdtsc();
        else
            ReadCurrentTime(&now);
        stopwatch[0] = static_cast<uint32_t>(now);
        stopwatch[1] = static_cast<uint32_t>(now >> 32);
    }

    // Hit test. The original passes an uninitialized stack rect to slot 0x0F8 (EA bug);
    // this rewrite passes zero-filled memory, so the call is deterministic. The command
    // and the cell are both written by slot 0x1D4 (the command is an int there, the same
    // slot 0x00528F20 reads as the snapped y).
    uint32_t command = 0;
    int32_t cell[2] = {0, 0};
    uint32_t box[5] = {0, 0, 0, 0, 0};
    (void)ts3::CallGameVirtual<uint8_t, 0x1D4 / 4>(sub, x, y, &command, cell);

    // Calls shared by the "5" cases: press the cell, then select it unless the press
    // call failed (its AL decides).
    const auto pressCell = [&]() {
        const uint8_t pressed = ts3::CallGameVirtual<uint8_t, 0x0E4 / 4>(
            sub, cell[0], cell[1]);
        ts3::CallGameVirtual<void, 0x0E0 / 4>(sub, cell[0], cell[1], pressed == 0 ? 1 : 0);
    };
    const auto selectCell = [&]() {
        (void)ts3::CallGameVirtual<void, 0x0E8 / 4>(sub);
        ts3::CallGameVirtual<void, 0x0E0 / 4>(sub, cell[0], cell[1], 1);
    };

    switch (command) {
    case 2: {  // 0x00529CED: reselect the row range
        ts3::CallGameMethod<void, 0x005279D0>(this);
        ts3::CallGameMethod<void, 0x005263E0>(reinterpret_cast<void*>(&f_314));
        uint32_t range[5] = {0, 0, 0, 0, 0};
        ts3::CallGameVirtual<void, 0x0F8 / 4>(sub, range);
        int32_t end = f_2C0;
        if (end == -1)
            end = static_cast<int32_t>(range[2]);
        // the original compares with jbe (unsigned)
        for (uint32_t i = 0; i <= static_cast<uint32_t>(end); ++i)
            ts3::CallGameVirtual<void, 0x0E0 / 4>(sub, static_cast<int32_t>(i), cell[1], 1);
        break;
    }
    case 3: {  // 0x00529C85: reselect the column range
        ts3::CallGameMethod<void, 0x005279D0>(this);
        ts3::CallGameMethod<void, 0x005263E0>(reinterpret_cast<void*>(&f_314));
        uint32_t range[5] = {0, 0, 0, 0, 0};
        ts3::CallGameVirtual<void, 0x0F8 / 4>(sub, range);
        int32_t end = f_2C4;
        if (end == -1)
            end = static_cast<int32_t>(range[3]);
        for (uint32_t i = 0; i <= static_cast<uint32_t>(end); ++i)
            ts3::CallGameVirtual<void, 0x0E0 / 4>(sub, cell[0], static_cast<int32_t>(i), 1);
        break;
    }
    case 5:  // 0x00529B47: click/drag on a cell
        if (flagCell) {
            if (!flagRow) {
                // 0x00529B5E: grow/limit the selection list over the hit cell
                if (f_314 == reinterpret_cast<void*>(&f_314)) {
                    int32_t cellCopy[2] = {cell[0], cell[1]};
                    ts3::CallGameMethod<void, 0x00528880>(reinterpret_cast<void*>(&f_314), cellCopy);
                    ts3::CallGameMethod<void, 0x00527960>(this, cellCopy[0], cellCopy[1]);
                } else {
                    int32_t rect[4] = {cell[0], cell[1], 0, 0};
                    if (f_318 != nullptr) {
                        const uint8_t* const node = static_cast<const uint8_t*>(f_318);
                        std::memcpy(&rect[2], node + 8, sizeof(rect[2]));
                        std::memcpy(&rect[3], node + 0xC, sizeof(rect[3]));
                    }
                    ts3::CallGameMethod<void, 0x005240B0>(rect);
                    for (int32_t row = rect[0]; row <= rect[2]; ++row)
                        for (int32_t column = rect[1]; column <= rect[3]; ++column)
                            ts3::CallGameVirtual<void, 0x0E0 / 4>(sub, row, column, 1);
                }
            } else if ((f_254 & 0x00800000u) != 0 && f_310 > 1) {
                pressCell();
            }
        } else if (flagRow) {
            pressCell();
        } else if ((f_254 & 0x00800000u) != 0 && f_310 > 1) {
            pressCell();
        } else {
            selectCell();
        }
        break;
    default:
        break;
    }

    // Remember the hit; the child object is notified first, then the events go out.
    bool changed;
    if (static_cast<uint32_t>(f_3FC) == command &&
        static_cast<uint32_t>(f_400) == static_cast<uint32_t>(cell[0]) &&
        static_cast<uint32_t>(f_404) == static_cast<uint32_t>(cell[1])) {
        changed = false;
    } else {
        timedOut = false;
        changed = true;
    }
    f_404 = cell[1];
    void* const child = f_030;
    f_3FC = static_cast<int32_t>(command);
    f_400 = cell[0];
    if (child != nullptr) {
        if (ts3::CallGameVirtual<uint8_t, 0x088 / 4>(child, iface) != 0) {
            if (command == 5 || command == 0)
                ts3::CallGameVirtual<void, 0x084 / 4>(child, iface, 0);
            else
                return 1;  // the original jumps straight to AL = 1 here
        }
    }
    void* const source = f_03C != nullptr ? f_03C : f_038;
    // Both messages carry &f_3FC in payload_a (+0x18); the original never writes +0x1C
    // for them (the rewrite zero-fills that word).
    if (timedOut) {
        SendEvent(iface, source, kEventUpdateTimedOut, static_cast<uint32_t>(a3), flags, &f_3FC,
                  nullptr);
    } else {
        SendEvent(iface, source, kEventUpdated, static_cast<uint32_t>(a3), flags, &f_3FC, nullptr);
        if (changed) {
            SendEvent(iface, source, kEventSelectionChanged, 0, 0, cell, nullptr);
            (void)ts3::CallGameVirtual<int32_t, 0x090 / 4>(iface);
        }
    }
    return command != 0 ? 1 : 0;
}

// 0x005319F0
uint8_t MatrixRowList::Refresh5319F0()
{
    if (f_258 != 0) {
        ts3::CallGameMethod<void, 0x005315E0>(this);
        (void)ts3::CallGameVirtual<int32_t, 0x090 / 4>(Sub4());
    }
    return 1;  // the original returns with only AL = 1 defined
}

}  // namespace ts3::ui