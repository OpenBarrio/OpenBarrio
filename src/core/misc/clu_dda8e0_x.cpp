// Cluster unit clu_dda8e0_x (misc): the scripting bridge's value-copy helpers.
// Originals: TS3W.exe 0x00DDA8E0 (41 B), 0x00DDAC00 (95), 0x00DDAFB0 (41), 0x00DDAFE0
// (81), 0x00DE5E10 (115). All cdecl; see the header for the shared layouts. The three
// exception raisers (0x00E770E0 NullReference, 0x00E76F00 DivideByZero, 0x00E772A0
// IndexOutOfRange) are not rewritten and run through the layout.h bridge. EA quirk kept
// (affects outputs): BridgeUnboxDivide returns the quotient's LOW dword in the success
// path (the low half of the __alldvrm quotient left in EAX) but the error code on the
// raise path. The originals read the divide record and both destination pointers
// BEFORE testing the divisor, so a zero-divisor record with null destination pointers
// still reads them; replicated here.
#include "ts3/misc/clu_dda8e0_x.h"

namespace ts3::misc {
namespace {

// Pointer-typed view of a dword that the game stores as a 32-bit address. The game
// objects this unit runs against exist only in the 32-bit process; on x64 these casts
// merely keep the code portable (the exception raisers abort in the bridge anyway).
template <class T>
T* At(int32_t address)
{
    return reinterpret_cast<T*>(static_cast<uintptr_t>(static_cast<uint32_t>(address)));
}

}  // namespace

// 0x00DDA8E0: value object at +0x0C, its dword at +4 written through *outHandle.
int32_t BridgeUnboxDword(BridgeArgObject* args)
{
    const int32_t valueObj = args->valuePtr;
    const int32_t outHandle = args->outHandle;
    args->error = 0;
    if (!valueObj) {
        const int32_t err = CallGameFunction<int32_t, 0xE770E0>();
        args->error = err;
        return err;
    }
    const int32_t value = *At<const int32_t>(valueObj + 4);
    int32_t* dst = *At<int32_t*>(outHandle);
    *dst = value;
    return value;
}

// 0x00DDAC00: signed 64-bit divide of the record at +0x14; quotient to *outHandle,
// remainder to the record's remOut pointer.
int32_t BridgeUnboxDivide(BridgeArgObject* args)
{
    args->error = 0;
    const BridgeDivideRecord* rec = At<const BridgeDivideRecord>(args->recordPtr);
    // Assemble both operands as 64-bit: a uint32_t shifted left by 32 is undefined
    // behaviour (the compiler folds it to 0), which silently dropped the high half.
    const uint64_t numBits = (static_cast<uint64_t>(static_cast<uint32_t>(rec->numHi)) << 32) |
                             static_cast<uint32_t>(rec->numLo);
    const uint64_t denBits = (static_cast<uint64_t>(static_cast<uint32_t>(rec->denHi)) << 32) |
                             static_cast<uint32_t>(rec->denLo);
    int32_t* quoOut = At<int32_t>(args->outHandle);
    int64_t* remOut = At<int64_t>(rec->remOut);
    if (denBits == 0) {
        const int32_t err = CallGameFunction<int32_t, 0xE76F00>();
        args->error = err;
        return err;
    }
    const int64_t num = static_cast<int64_t>(numBits);
    const int64_t den = static_cast<int64_t>(denBits);
    int64_t quo, rem;
    if (den == -1) {
        // The CRT's __alldvrm special-cases the divisor -1 with a plain negation:
        // INT64_MIN / -1 wraps to INT64_MIN (no #DE), remainder 0. Negating through
        // unsigned keeps that wrap well-defined C++.
        quo = static_cast<int64_t>(~static_cast<uint64_t>(num) + 1u);
        rem = 0;
    } else {
        // Signed division truncating toward zero, exactly like __alldvrm.
        quo = num / den;
        rem = num % den;
    }
    *remOut = rem;
    quoOut[0] = static_cast<int32_t>(static_cast<uint32_t>(quo));
    quoOut[1] = static_cast<int32_t>(static_cast<uint32_t>(static_cast<uint64_t>(quo) >> 32));
    return static_cast<int32_t>(static_cast<uint32_t>(quo));
}

// 0x00DDAFB0: box three words through a4 (or NullReferenceException into *a2).
int32_t BridgeBoxTriple(int32_t /*a1 - unused*/, int32_t* a2, int32_t /*a3 - unused*/,
                        int32_t* a4, int32_t a5, int32_t a6, int32_t a7)
{
    if (!a4) {
        const int32_t err = CallGameFunction<int32_t, 0xE770E0>();
        *a2 = err;
        return err;
    }
    a4[0] = a5;
    a4[1] = a6;
    a4[2] = a7;
    return static_cast<int32_t>(static_cast<uint32_t>(reinterpret_cast<uintptr_t>(a4)));
}

// 0x00DDAFE0: +0x14 -> *+0x14 -> three words copied to the destination at +0x0C.
int32_t BridgeUnboxTriple(BridgeArgObject* args)
{
    const int32_t chain = args->recordPtr;
    args->error = 0;
    const int32_t box = *At<const int32_t>(chain);
    const int32_t w0 = At<const int32_t>(box)[0];
    const int32_t w1 = At<const int32_t>(box)[1];
    const int32_t w2 = At<const int32_t>(box)[2];
    const int32_t out = args->valuePtr;
    if (!out) {
        const int32_t err = CallGameFunction<int32_t, 0xE770E0>();
        args->error = err;
        return err;
    }
    int32_t* dst = At<int32_t>(out);
    dst[0] = w0;
    dst[1] = w1;
    dst[2] = w2;
    return out;
}

// 0x00DE5E10: (index>>8, index&0xFF) member lookup; *a4 = the found dword.
int32_t BridgeLookupMember(int32_t /*a1 - unused*/, int32_t* a2,
                           int32_t /*a3 - unused*/, int32_t* a4,
                           const uint32_t* container, int32_t index)
{
    int32_t table;
    if (!container) {
        const int32_t err = CallGameFunction<int32_t, 0xE770E0>();
        *a2 = err;
        return err;
    }
    table = static_cast<int32_t>(container[2]);  // +0x08
    const int32_t hi = index >> 8;
    if (!table) {
        const int32_t err = CallGameFunction<int32_t, 0xE770E0>();
        *a2 = err;
        return err;
    }
    if (hi < 0 || static_cast<uint32_t>(hi) >= static_cast<uint32_t>(At<const int32_t>(table)[2])) {
        const int32_t err = CallGameFunction<int32_t, 0xE772A0>();
        *a2 = err;
        return err;
    }
    const int32_t entry = At<const int32_t>(table)[4 + hi];  // +0x10 + 4*hi
    const int32_t lo = index & 0xFF;
    if (!entry) {
        const int32_t err = CallGameFunction<int32_t, 0xE770E0>();
        *a2 = err;
        return err;
    }
    // The original also tests lo < 0 (dead code: lo = index & 0xFF is never negative).
    if (lo < 0 || static_cast<uint32_t>(lo) >= static_cast<uint32_t>(At<const int32_t>(entry)[2])) {
        const int32_t err = CallGameFunction<int32_t, 0xE772A0>();
        *a2 = err;
        return err;
    }
    *a4 = At<const int32_t>(entry)[4 + lo];
    return static_cast<int32_t>(static_cast<uint32_t>(reinterpret_cast<uintptr_t>(a4)));
}

}  // namespace ts3::misc
