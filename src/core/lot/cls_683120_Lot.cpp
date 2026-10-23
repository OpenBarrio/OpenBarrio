// Rewritten from TS3W.exe unit cls_683120_Lot - the LotCollisionShape /
// LotFloorCollisionShape / HeightFieldShape constructors, copy paths, the height
// sampler and the clone. Original addresses are in the header; every call into the
// not-yet-rewritten units (clu_682890_Lot, clu_67f4a0_Physics, clu_4e3fb0_Renderer)
// goes through the game bridge.

#include "ts3/lot/cls_683120_Lot.h"

#include <cstddef>
#include <cstdint>
#include <cstring>

namespace ts3::lot {
namespace {

// Game globals (x86 addresses, meaningful only inside TS3W.exe):
//   0x00FEFD10  vtable of LotFloorCollisionShape (slot 1 = 0x00683130)
//   0x0107690C  vtable of HeightFieldShape (slot 1 = 0x00683130, slot 2 = 0x00C2AAF0)
//   0x010769D8  allocation tag string "Terrain/ClonedHFShape" (verified in rdata)
constexpr uintptr_t kVtableLotFloor = 0x00FEFD10u;
constexpr uintptr_t kVtableHeightField = 0x0107690Cu;
constexpr uintptr_t kAllocTagAddress = 0x010769D8u;

constexpr uint32_t kShapeCode = 0x065BAD16u;           // returned by 0x00683130

constexpr uint32_t kCloneSize = 0xA0u;                 // sizeof(HeightFieldShape)
constexpr uint32_t kCloneAlign = 0x10u;

void* VtableAt(uintptr_t address)
{
    return reinterpret_cast<void*>(address);
}

const char* AllocTagInGame()
{
    return reinterpret_cast<const char*>(kAllocTagAddress);
}

}  // namespace

LotFloorCollisionShape* LotFloorCollisionShape::Construct()
{
    // 0x00683120
    //
    // 0x00682890 builds its constants on its own 16-aligned stack frame and only
    // WRITES the object (verified instruction by instruction: it never reads [this]),
    // using movaps for the stores at +0x10/+0x20. The generic compare engine hands
    // the rewrite an 8-byte-aligned shadow copy of the object (verified empirically
    // with MSVC x86), where the movaps would #GP - so on a misaligned this we run the
    // base ctor into a 16-aligned scratch buffer and copy its 0x5C bytes back. Same
    // stores, same values; the aligned fast path keeps the common case identical.
    auto* base = static_cast<LotCollisionShape*>(this);
    if (reinterpret_cast<uintptr_t>(this) % 16 == 0) {
        CallGameMethod<void, 0x00682890>(base);
    } else {
        // Pre-fill the scratch with this object's bytes: 0x00682890 leaves
        // +0x04..+0x0F (and any other unwritten bytes) untouched, so a zeroed
        // scratch would differ from an in-place run exactly there.
        alignas(16) uint8_t scratch[0x5C];
        std::memcpy(scratch, this, sizeof(scratch));
        CallGameMethod<void, 0x00682890>(reinterpret_cast<LotCollisionShape*>(scratch));
        std::memcpy(base, scratch, sizeof(scratch));
    }
    vtable = VtableAt(kVtableLotFloor);
    return this;
}

uint32_t LotFloorCollisionShape::ShapeCode()
{
    // 0x00683130
    return kShapeCode;
}

LotFloorCollisionShape* LotFloorCollisionShape::ConstructFrom(
    const LotFloorCollisionShape* src)
{
    // 0x00A8CA70
    // the stack); [esi] = 0x00FEFD10; then movaps from src+0x60 and src+0x70 plus the
    // four dwords at +0x80..+0x8C - i.e. exactly +0x60..+0x8F; return esi.
    CallGameMethod<void, 0x00682950>(this, src);
    vtable = VtableAt(kVtableLotFloor);
    std::memcpy(data60, src->data60, sizeof(data60));
    return this;
}

HeightFieldShape* HeightFieldShape::Construct(uint32_t samplesAddr, int32_t stride,
                                              int32_t width, int32_t height)
{
    // 0x00C29B80
    // [esi] = 0x0107690C, a3 -> +0x94, a4 -> +0x98; return esi. The EA code stores the
    // fields in a different order; the four dwords are independent, the final object
    // state is identical.
    LotFloorCollisionShape::Construct();
    samples = reinterpret_cast<void*>(static_cast<uintptr_t>(samplesAddr));
    this->stride = stride;
    this->width = width;
    this->height = height;
    vtable = VtableAt(kVtableHeightField);
    return this;
}

void* HeightFieldShape::Sample(void* out, int32_t x, int32_t y)
{
    // 0x00C29C60: unsigned compare of x against +0x98 and y against +0x9C; when the
    // index is out of range the result is sar(~index, 31) & (bound - 1) - an all-ones
    // mask for a non-negative index (=> bound - 1) and zero for a negative one; then
    // v = (float)*(uint16*)(+0x90 + y*(+0x94) + 2*x) broadcast to all four float lanes
    // of *(__m128*)out (shufps xmm0, xmm0, 0 + movaps); return out.
    uint32_t cx = static_cast<uint32_t>(x);
    const uint32_t boundX = static_cast<uint32_t>(width);
    if (cx >= boundX)
        cx = ((static_cast<uint32_t>(~x) >> 31) != 0) ? boundX - 1u : 0u;
    uint32_t cy = static_cast<uint32_t>(y);
    const uint32_t boundY = static_cast<uint32_t>(height);
    if (cy >= boundY)
        cy = ((static_cast<uint32_t>(~y) >> 31) != 0) ? boundY - 1u : 0u;

    const uint32_t offset = cy * static_cast<uint32_t>(stride) + 2u * cx;
    const auto* word = reinterpret_cast<const uint16_t*>(
        static_cast<const uint8_t*>(samples) + offset);
    const float value = static_cast<float>(*word);
    auto* lanes = static_cast<float*>(out);
    lanes[0] = value;
    lanes[1] = value;
    lanes[2] = value;
    lanes[3] = value;
    return out;
}

HeightFieldShape* HeightFieldShape::ConstructFrom(const HeightFieldShape* src)
{
    // 0x00C2A240
    // from src; return esi.
    LotFloorCollisionShape::ConstructFrom(src);
    samples = src->samples;
    stride = src->stride;
    width = src->width;
    height = src->height;
    vtable = VtableAt(kVtableHeightField);
    return this;
}

uint32_t HeightFieldShape::Clone(void** out)
{
    // 0x00C2AAF0: p = 0x004E3FB0(0xA0, 0x10, 0, "Terrain/ClonedHFShape", 0, 0, 0, 0)
    // (cdecl, caller cleans 0x20); if p: p->ConstructFrom(this); *out = p; on success
    // 0x00682750(p) bumps the refcount through the bridge, then only AL is set to 1;
    // on failure *out = nullptr and the function returns 0 (full eax zeroed).
    void* p = CallGameFunction<void*, 0x004E3FB0>(
        kCloneSize, kCloneAlign, static_cast<uint32_t>(0), AllocTagInGame(),
        static_cast<uint32_t>(0), static_cast<uint32_t>(0), static_cast<uint32_t>(0),
        static_cast<uint32_t>(0));
    if (p != nullptr)
        static_cast<HeightFieldShape*>(p)->ConstructFrom(this);
    *out = p;
    if (p == nullptr)
        return 0u;
    const uint32_t refcount =
        CallGameMethod<uint32_t, 0x00682750>(static_cast<LotCollisionShape*>(p));
    // EA quirk: mov al, 1 - the refcount call's high bytes survive the |= 1.
    return (refcount & 0xFFFFFF00u) | 1u;
}

}  // namespace ts3::lot
