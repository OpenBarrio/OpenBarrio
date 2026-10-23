// Reimplementation of CAS unit cls_5f7540_CAS (TS3W.exe 0x005F7540..0x005FB510): the
// constructors/destructors of the SkinToneOp (0x00FE6278), Layer (0x00FE62D0) and ClearOp
// (0x00FE6390) CAS resource classes plus three methods of the texture-builder class
// (vtable 0x00FE63E8). Layout evidence in the header. Differentially tested in
// src/exehost/cls_5f7540_CAS_test.cpp.
#include "ts3/cas/cls_5f7540_CAS.h"

#include <cstring>

namespace ts3::cas {
namespace {

// The texture-op manager singleton (unit clu_727290 swaps it at 0x00727290). Vtable slot
// 13 (0x34) queues an op, slot 15 (0x3C) unregisters a deletor callback from an object.
constexpr uint32_t kManagerGlobal = 0x011D3C84;
// The named-service registry object whose method 0x0079B840 finds services by name.
constexpr uint32_t kRegistryGlobal = 0x011D753C;

constexpr uintptr_t kVtableBase = 0x00FE6058;
constexpr uintptr_t kVtableSkinToneOp = 0x00FE6278;
constexpr uintptr_t kVtableLayer = 0x00FE62D0;
constexpr uintptr_t kVtableClearOp = 0x00FE6390;

constexpr uint32_t kRegisterOpId = 0x03420F34;   // 0x005F8170's manager request id
constexpr uint32_t kClearOpRequestId = 0x00B3D7A7;  // 0x005FAFB0's manager request id

#if defined(_M_IX86)
void* GameGlobalPointer(uint32_t address)
{
    return *reinterpret_cast<void* const*>(static_cast<uintptr_t>(address));
}
const char* TbopsServiceName()
{
    return reinterpret_cast<const char*>(0x00FE60AC);  // "tbops" in the exe's rdata
}
void* DeleteCallback()
{
    // 0x00D4B150: cdecl thunk tail-jumping to obj->vtbl[1](obj); passed through opaquely.
    return reinterpret_cast<void*>(0x00D4B150);
}
#else
void* GameGlobalPointer(uint32_t) { return nullptr; }
const char* TbopsServiceName() { return "tbops"; }
void* DeleteCallback() { return nullptr; }
#endif

// cvttss2si + movsx + cvtsi2ss + cvttss2si: truncates to int, sign-extends to 16 bits and
// stores those 16 bits - i.e. exactly (int16)(int)(f * 255).
int16_t ScaledColor(float f) { return static_cast<int16_t>(static_cast<int>(f * 255.0f)); }
// The plain variant: cvttss2si, store the low 16 bits.
uint16_t ScaledColor16(float f) { return static_cast<uint16_t>(static_cast<int>(f * 255.0f)); }

// Copies 16 bytes to an aligned temp and lets the game's packer 0x0060CD10 (thiscall ret 4)
// fold them into one dword at `dest` (magic-constant float-to-byte, 255.0f scale + rounding
// biases; its static state at 0x011CECB0/0x011CECC0 is idempotent).
void PackColor(void* dest, const float* rgba)
{
    alignas(16) float temp[4];
    std::memcpy(temp, rgba, sizeof temp);
    CallGameMethod<void, 0x0060CD10>(dest, temp);
}

// One handle-release block shared by the destructors: with a manager present, unregister
// the slot (vtable slot 15 with the 0x00D4B150 deletor); then, when the (reloaded!) handle
// is non-null and its low two bits are clear, release it through vtable slot 1 and clear
// the slot. Returns the game's eax after the block: 0 for a null handle, the last call's
// return otherwise (a tagged handle leaves the manager call's return in place).
uint32_t ReleaseHandle(void** slot)
{
    uint32_t eax = *reinterpret_cast<uint32_t*>(slot);
    if (!eax)
        return 0;
    void* manager = GameGlobalPointer(kManagerGlobal);
    if (manager)
        eax = CallGameVirtual<uint32_t, 15>(manager, slot, DeleteCallback());
    void* handle = *slot;
    if (!handle)
        return eax;
    if ((reinterpret_cast<uintptr_t>(handle) & 3) != 0)
        return eax;
    eax = CallGameVirtual<uint32_t, 1>(handle);
    *slot = nullptr;
    return eax;
}

// The Layer destructor's FIRST +0x34 block runs the manager unregister BEFORE looking at
// the handle (EA inconsistency: every other block checks the handle first). The game then
// reloads eax from the slot, so the manager call's return never survives: a null handle
// returns 0, a tagged handle returns its own value.
uint32_t ReleaseHandleManagerFirst(void** slot)
{
    void* manager = GameGlobalPointer(kManagerGlobal);
    if (manager)
        CallGameVirtual<uint32_t, 15>(manager, slot, DeleteCallback());
    const uint32_t eax = *reinterpret_cast<uint32_t*>(slot);
    if (!eax)
        return 0;
    void* handle = *slot;
    if ((reinterpret_cast<uintptr_t>(handle) & 3) != 0)
        return eax;
    const uint32_t released = CallGameVirtual<uint32_t, 1>(handle);
    *slot = nullptr;
    return released;
}

}  // namespace

// TS3W.exe 0x005F7E20: base "constructor" (also used to re-init pooled objects). The game
// reads state0B first, so the (x & 0xFA) | 2 fold keeps bits 1 and 3 of the old value.
CasResourceHeader* CasResourceHeader::Construct()
{
    const uint8_t state = state0B;
    field04 = 0;
    refCount = 0;
    flags0A = 0;
    vtable = reinterpret_cast<void*>(kVtableBase);
    state0B = static_cast<uint8_t>((state & 0xFA) | 2);
    return this;
}

// TS3W.exe 0x005FA830.
CasSkinToneOp* CasSkinToneOp::Construct(const float* rgba, uint32_t param, uint8_t flagA,
                                        uint8_t flagB, uint8_t flagC, uint8_t flagD, float scale255,
                                        const float* packColor, const uint32_t* sourceKeys)
{
    const uint8_t state = header.state0B;
    header.flags0A = 0;
    header.field04 = 0;
    header.refCount = 0;
    header.vtable = reinterpret_cast<void*>(kVtableSkinToneOp);
    header.state0B = static_cast<uint8_t>((state & 0xFA) | 2);
    key0C = sourceKeys[0];
    key10 = sourceKeys[1];
    key14 = sourceKeys[3];  // EA: sourceKeys[2] is skipped
    param18 = param;
    flags2E = static_cast<uint8_t>((flagC << 4) | (flagB & 0xF));
    handle24 = nullptr;
    header.flags0A = static_cast<uint8_t>(((flagA & 1) << 5) | (header.flags0A & 0xD0) | (flagD & 0xF));
    param2C = ScaledColor16(scale255);
    PackColor(&packedColor28, packColor);
    color1C[0] = ScaledColor(rgba[0]);
    color1C[1] = ScaledColor(rgba[1]);
    color1C[2] = ScaledColor(rgba[2]);
    color1C[3] = ScaledColor(rgba[3]);
    return this;
}

// TS3W.exe 0x005FA9B0.
uint32_t CasSkinToneOp::Destruct()
{
    header.vtable = reinterpret_cast<void*>(kVtableSkinToneOp);
    const uint32_t result = ReleaseHandle(&handle24);
    header.vtable = reinterpret_cast<void*>(kVtableBase);
    return result;
}

// TS3W.exe 0x005FAA20.
CasLayer* CasLayer::ConstructFromRefs(const uint32_t* refs, const float* colorA, const float* colorB,
                                      float weight0, uint8_t flag0, uint8_t flag1, uint8_t flag2,
                                      uint8_t flag3, uint8_t flag4, float weight1, float weight2,
                                      const float* packColor, const uint32_t* keys, uint8_t id)
{
    const uint8_t state = header.state0B;
    header.flags0A = 0;
    header.field04 = 0;
    header.refCount = 0;
    header.state0B = static_cast<uint8_t>((state & 0xFA) | 2);
    header.vtable = reinterpret_cast<void*>(kVtableLayer);
    keys1C[0] = keys[0];
    keys1C[1] = keys[1];
    keys1C[2] = keys[3];  // EA: keys[2] is skipped
    ref28 = refs[0];
    ref2C = refs[1];
    ref30 = refs[3];  // EA: refs[2] is skipped
    handle34 = nullptr;
    handle38 = nullptr;
    object3C = nullptr;
    id40 = 0;
    flags4E = static_cast<uint8_t>((flag2 << 4) | (flag1 & 0xF));
    id4F = id;
    // The game folds the (flag3:flag0) bits into the old flags0A & 0x90, which is 0 here
    // because flags0A was just cleared.
    header.flags0A = static_cast<uint8_t>(((((flag3 & 1) << 1) | (flag0 & 1)) << 5) | (header.flags0A & 0x90) | (flag4 & 0xF));
    weight48 = ScaledColor16(weight2);
    weight4A = ScaledColor16(weight0);
    weight4C = ScaledColor16(weight1);
    color0C[0] = ScaledColor(colorA[0]);
    color0C[1] = ScaledColor(colorA[1]);
    color0C[2] = ScaledColor(colorA[2]);
    color0C[3] = ScaledColor(colorA[3]);
    color14[0] = ScaledColor(colorB[0]);
    color14[1] = ScaledColor(colorB[1]);
    color14[2] = ScaledColor(colorB[2]);
    color14[3] = ScaledColor(colorB[3]);
    PackColor(&packedColor44, packColor);
    return this;
}

// TS3W.exe 0x005FAC60. The game first stores plain cvttss2si truncations of colorA/colorB
// (no 255 scale) into +0x0C..+0x1B and overwrites every one of them below - dead stores,
// no effect on the object.
CasLayer* CasLayer::ConstructWithId(uint32_t id, const float* colorA, const float* colorB,
                                    float weight0, uint8_t flag0, uint8_t flag1, uint8_t flag2,
                                    uint8_t flag3, uint8_t flag4, float weight1, float weight2,
                                    const float* packColor, const uint32_t* keys, uint8_t idArg)
{
    const uint8_t state = header.state0B;
    header.vtable = reinterpret_cast<void*>(kVtableLayer);
    header.flags0A = 0;
    header.field04 = 0;
    header.refCount = 0;
    header.state0B = static_cast<uint8_t>((state & 0xFA) | 2);
    color0C[0] = ScaledColor(colorA[0]);
    color0C[1] = ScaledColor(colorA[1]);
    color0C[2] = ScaledColor(colorA[2]);
    color0C[3] = ScaledColor(colorA[3]);
    color14[0] = ScaledColor(colorB[0]);
    color14[1] = ScaledColor(colorB[1]);
    color14[2] = ScaledColor(colorB[2]);
    color14[3] = ScaledColor(colorB[3]);
    keys1C[0] = keys[0];
    keys1C[1] = keys[1];
    keys1C[2] = keys[3];
    ref28 = 0;
    ref2C = 0;
    ref30 = 0;
    handle34 = nullptr;
    handle38 = nullptr;
    object3C = nullptr;
    id40 = id;
    flags4E = static_cast<uint8_t>((flag2 << 4) | (flag1 & 0xF));
    id4F = idArg;
    header.flags0A = static_cast<uint8_t>(((((flag3 & 1) << 1) | (flag0 & 1)) << 5) | (header.flags0A & 0x90) | (flag4 & 0xF));
    weight48 = ScaledColor16(weight2);
    weight4A = ScaledColor16(weight0);
    weight4C = ScaledColor16(weight1);
    PackColor(&packedColor44, packColor);
    return this;
}

// TS3W.exe 0x005FAEB0. The order is exactly the game's: +0x34 (manager unregister FIRST,
// then the handle check - EA inconsistency), +0x3C (vtable slot 2, no manager and no tag
// check), +0x38, then +0x34 again - EA: the retry unregisters a still set (tagged) +0x34
// with the manager a second time; after a normal release it reads 0.
uint32_t CasLayer::Destruct()
{
    header.vtable = reinterpret_cast<void*>(kVtableLayer);
    ReleaseHandleManagerFirst(&handle34);
    if (object3C) {
        CallGameVirtual<uint32_t, 2>(object3C);
        object3C = nullptr;
    }
    ReleaseHandle(&handle38);
    const uint32_t result = ReleaseHandle(&handle34);
    header.vtable = reinterpret_cast<void*>(kVtableBase);
    return result;
}

// TS3W.exe 0x005FAFB0.
CasClearOp* CasClearOp::Construct(const uint32_t* keysA, const uint32_t* keysB, const uint32_t* keysC,
                                  const uint32_t* keysD, float weight, uint8_t flag0, uint8_t flag1,
                                  uint8_t flag2, uint8_t flag3, float scale255,
                                  const float* packColor, const uint32_t* keys)
{
    const uint8_t state = header.state0B;
    header.vtable = reinterpret_cast<void*>(kVtableClearOp);
    header.state0B = static_cast<uint8_t>((state & 0xFA) | 2);
    header.field04 = 0;
    header.refCount = 0;
    header.flags0A = 0;
    key0C[0] = keysA[0];  key0C[1] = keysA[1];  key0C[2] = keysA[3];
    key0C[3] = keysB[0];  key0C[4] = keysB[1];  key0C[5] = keysB[3];
    key0C[6] = keysC[0];  key0C[7] = keysC[1];  key0C[8] = keysC[3];
    key0C[9] = keysD[0];  key0C[10] = keysD[1]; key0C[11] = keysD[3];
    key0C[12] = keys[0];  key0C[13] = keys[1];  key0C[14] = keys[3];
    reserved48[0] = 0;
    reserved48[1] = 0;
    reserved48[2] = 0;
    reserved48[3] = 0;
    managerOut58 = 0;
    weight5C = weight;  // bit-exact: the game loads the raw dword into xmm
    PackColor(&packedColor60, packColor);
    param64 = ScaledColor16(scale255);
    flags66 = static_cast<uint8_t>((flag2 << 4) | (flag1 & 0xF));
    header.flags0A = static_cast<uint8_t>(((flag0 & 1) << 5) | (header.flags0A & 0xD0) | (flag3 & 0xF));
    if (keys[2] != 0) {
        void* manager = GameGlobalPointer(kManagerGlobal);
        if (manager)
            CallGameVirtual<uint32_t, 13>(manager, keys, kClearOpRequestId, &managerOut58, 0, 0,
                                          DeleteCallback(), 0, 1);
    }
    return this;
}

// TS3W.exe 0x005F7540. The registry lookup (0x0079B840) and the limit call (0x0079A2D0) are
// plain game methods; there is no null check on the registry global - the original would
// crash on a null registry and so does this.
void* CasTextureBuilder::FetchTextureOpService()
{
    void* service = CallGameMethod<void*, 0x0079B840>(GameGlobalPointer(kRegistryGlobal),
                                                      TbopsServiceName(), nullptr, nullptr);
    if (!service)
        return nullptr;
    const uint32_t limit = CallGameVirtual<uint32_t, 20>(this);
    CallGameMethod<void*, 0x0079A2D0>(service, limit);
    return service;
}

// TS3W.exe 0x005F8170.
uint32_t CasTextureBuilder::RegisterTextureOps(uint32_t a2, uint32_t a3, uint32_t a4, uint32_t a5)
{
    void* manager = GameGlobalPointer(kManagerGlobal);
    if (!manager)
        return 0;
    const uint32_t accepted =
        CallGameVirtual<uint32_t, 13>(manager, a2, kRegisterOpId, static_cast<void*>(this), a3, a4,
                                      DeleteCallback(), a5, 1);
    return (accepted & 0xFF) != 0 ? 1u : 0u;  // test al, al
}

// TS3W.exe 0x005FB510.
uint8_t CasTextureBuilder::SetPatternLevel(uint8_t level)
{
    patternByte ^= static_cast<uint8_t>((level ^ patternByte) & 0xF);
    uint8_t value = patternByte;
    if ((value & 0xF) >= 4)
        patternByte = value = static_cast<uint8_t>((value & 0xF3) | 3);
    return value;
}

}  // namespace ts3::cas
