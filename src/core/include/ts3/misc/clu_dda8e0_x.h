#pragma once
// Cluster unit clu_dda8e0_x (misc subsystem): TS3W.exe 0x00DDA8E0 (41 B), 0x00DDAC00
// (95), 0x00DDAFB0 (41), 0x00DDAFE0 (81), 0x00DE5E10 (115) — the scripting bridge's
// value-copy helpers. All five are cdecl. The three table-dispatched ones
// (DDA8E0/DDAC00/DDAFE0, entries of the callback table around 0x00E43E38) share one
// argument object: +8 out-handle slot, +0x0C value pointer, +0x14 record pointer,
// +0x20 error/result slot (cleared on entry, set to the raised error otherwise).
// DDAFB0/DE5E10 instead follow the scripting-wrapper shape (error out-pointer a2)
// from the neighbouring clu_de6ed0_Scripting unit.
//
// On bad input each helper raises one of the runtime's three canned exceptions through
// zero-argument cdecl raisers that this unit does NOT rewrite: 0x00E770E0
// (NullReferenceException), 0x00E76F00 (DivideByZeroException) and 0x00E772A0
// (IndexOutOfRangeException); their returned error code is stored to the error slot /
// out-parameter and returned. In-unit calls: none. On x64 every entry aborts in the
// bridge (there is no game code to raise the exceptions with).
#include <cstdint>

#include "ts3/layout.h"

namespace ts3::misc {

// The argument object of the three table-dispatched helpers (0x24 bytes observed: the
// largest access is the +0x20 error slot; the type may be larger in the game).
struct BridgeArgObject {
    uint8_t padToOutHandle[0x08];
    int32_t outHandle;  // +0x08: pointer to the destination pointer (DDA8E0/DDAC00)
    int32_t valuePtr;   // +0x0C: value object (+4 = the dword) / destination (DDAFE0)
    uint8_t padToRecord[0x04];
    int32_t recordPtr;  // +0x14: divide record (DDAC00) / pointer chain (DDAFE0)
    uint8_t padToError[0x08];
    int32_t error;      // +0x20: error/result slot
};
TS3_X86_OFFSET(BridgeArgObject, outHandle, 0x08);
TS3_X86_OFFSET(BridgeArgObject, valuePtr, 0x0C);
TS3_X86_OFFSET(BridgeArgObject, recordPtr, 0x14);
TS3_X86_OFFSET(BridgeArgObject, error, 0x20);
TS3_X86_SIZE(BridgeArgObject, 0x24);

// The 64-bit divide record behind BridgeArgObject::recordPtr (0x14 bytes).
struct BridgeDivideRecord {
    int32_t numLo;    // +0x00: numerator, low dword
    int32_t numHi;    // +0x04: numerator, high dword
    int32_t denLo;    // +0x08: divisor, low dword
    int32_t denHi;    // +0x0C: divisor, high dword
    int32_t remOut;   // +0x10: int64_t* remainder destination
};
TS3_X86_SIZE(BridgeDivideRecord, 0x14);

// 0x00DDA8E0 (41 B): unbox a dword. Writes the value object's second dword (+4, the
// object at +0x0C) through the pointer stored at *outHandle (+8). Raises
// NullReferenceException when the value pointer is null. Returns the stored value, or
// the raised error code (also stored to +0x20).
int32_t BridgeUnboxDword(BridgeArgObject* args);

// 0x00DDAC00 (95 B): signed 64-bit divide. The record at +0x14 supplies the operands;
// the quotient is stored to *outHandle (+8), the remainder to the record's +0x10
// pointer. Raises DivideByZeroException on a zero divisor (both destination reads and
// the record deref still happen first, exactly like the original). Returns the
// quotient's low dword (error code on the raise path, also stored to +0x20).
int32_t BridgeUnboxDivide(BridgeArgObject* args);

// 0x00DDAFB0 (41 B): box three words. a4 is the destination (three dwords a5..a7 are
// written and a4 returned); when a4 is null the a5..a7 words are dropped and
// NullReferenceException is raised into *a2 and returned. a1/a3 are unused.
int32_t BridgeBoxTriple(int32_t a1, int32_t* a2, int32_t a3, int32_t* a4, int32_t a5,
                        int32_t a6, int32_t a7);

// 0x00DDAFE0 (81 B): unbox three words. Follows +0x14 -> *+0x14 to the boxed value and
// copies its three words to the destination at +0x0C (returned). Raises
// NullReferenceException on a null destination (the box is still read). Returns the
// destination pointer, or the raised error code (also stored to +0x20).
int32_t BridgeUnboxTriple(BridgeArgObject* args);

// 0x00DE5E10 (115 B): two-level member lookup. a6's high byte indexes the table at
// container+8, its low byte indexes the chosen sub-table; the found dword is stored to
// *a4 (returned). Raises NullReferenceException for a null container/table/entry and
// IndexOutOfRangeException for an out-of-range byte. a1/a3 are unused.
int32_t BridgeLookupMember(int32_t a1, int32_t* a2, int32_t a3, int32_t* a4,
                           const uint32_t* container, int32_t index);

}  // namespace ts3::misc
