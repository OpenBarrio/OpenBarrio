#include "ts3/misc/cls_4a6c80_x.h"
#include "ts3/io/memory_stream.h"
#include "ts3/animation/cls_7f1620_Animation.h"
#include <cstring>
#include <xmmintrin.h>
#include "ts3/resource/packed_file.h"

namespace ts3::misc {
resource::PackedFileRecord* RecordReadStream::Owner()
{
    return reinterpret_cast<resource::PackedFileRecord*>(reinterpret_cast<uint8_t*>(this) -
        offsetof(resource::PackedFileRecord, streamVtable));
}
uint32_t RecordReadStream::Type() // 004AD670
{
    return 0x12e4a891;
}
uint32_t RecordReadStream::Size() // 004AD680
{
    auto* p = Owner();
    if (p->offset) return p->size;
    return CallGameVirtual<uint32_t, 7>(p->Inner());
}
int32_t RecordReadStream::Position(int32_t origin) // 004AD6A0
{
    auto* p = Owner();
    if (!p->offset) return CallGameVirtual<int32_t, 9>(p->Inner(), int32_t{0});
    if (origin == 0) return static_cast<int32_t>(p->position);
    if (origin == 2) return static_cast<int32_t>(p->position - p->size);
    return 0;
}
bool RecordReadStream::Seek(int32_t distance, int32_t origin) // 004AD6E0
{
    auto* p = Owner();
    if (!p->accessFlags) return false;
    if (!p->offset) return CallGameVirtual<bool, 10>(p->Inner(), distance, origin);
    const uint32_t d = static_cast<uint32_t>(distance);
    if (origin == 0) {
        p->position = distance < 0 ? 0 : (d > p->size ? p->size : d);
    } else if (origin == 1) {
        if (distance < 0 && (0u - d) > p->position) p->position = 0;
        else if (distance > 0 && (p->position > p->size || d > p->size - p->position)) p->position = p->size;
        else p->position += d;
    } else if (origin == 2) {
        // EA does not clamp negative end-relative seeks at the start.
        p->position = distance < 0 ? p->size + d : p->size;
    }
    // Unknown origins succeed without changing the position.
    return true;
}
uint32_t RecordReadStream::Available() // 004AD790
{
    auto* p = Owner();
    if (p->offset) return p->size - p->position;
    return CallGameVirtual<uint32_t, 11>(p->Inner());
}
uint32_t RecordReadStream::Read(void* dst, uint32_t count) // 004AD7B0
{
    auto* p = Owner();
    if (!p->offset) return CallGameVirtual<uint32_t, 12>(p->Inner(), dst, count);
    if (!p->accessFlags) return UINT32_MAX;
    const uint32_t end = p->position + count;
    if (end > p->size || end < p->position) count = p->size - p->position;
    if (!p->owner->ReadRaw(dst, p->offset + p->position, count)) count = 0;
    p->position += count;
    return count;
}
void* RecordReadStream::Delete(uint32_t flags) // 004ADEF0
{
    return reinterpret_cast<ReadRecordDeleter*>(Owner())->Delete(flags);
}
bool RecordReadStream::Close() // 004ADF00
{
    return reinterpret_cast<resource::PackedFileWriteRecord*>(Owner())->CloseThroughOwner();
}
int32_t RecordReadStream::Release() // 004ADF10
{
    return CallGameMethod<int32_t, 0x005044c0>(Owner());
}
int32_t RecordReadStream::AddRef() // 004ADF20
{
    return CallGameMethod<int32_t, 0x00ec17a0>(Owner());
}
uint32_t RecordReadStream::Access() // 0093E770
{
    return Owner()->accessFlags;
}
void* ReadRecordDeleter::Delete(uint32_t flags) // 004AE420
{
    CallGameMethod<void, 0x004ae080>(this);
    if (flags & 1) io::EaDelete(this);
    return this;
}
uint32_t* RecordKeyView::Key() // 00580430
{
    return key;
}
uint32_t RecordKeyView::SetKey(const void* data) // 00739A60
{
    const auto* source = static_cast<const uint32_t*>(data);
    // Deliberately sequential loads/stores: overlapping source can observe prior stores.
    key[0] = source[0];
    key[1] = source[1];
    key[2] = source[2];
    key[3] = source[3];
    return key[3];
}
int32_t RectangleLightEntries::UpdateTransform() // 006BCD70
{
    for (unsigned i = 0; i < 4; ++i)
        worldB.lane[i] = (localB.lane[1] * basis[1].lane[i] + localB.lane[0] * basis[0].lane[i]) + localB.lane[2] * basis[2].lane[i];
    for (unsigned i = 0; i < 4; ++i)
        worldA.lane[i] = (localA.lane[1] * basis[1].lane[i] + localA.lane[0] * basis[0].lane[i]) + localA.lane[2] * basis[2].lane[i];
    for (unsigned i = 0; i < 4; ++i)
        worldPosition.lane[i] = ((localPosition.lane[0] * basis[0].lane[i] + basis[3].lane[i]) + localPosition.lane[1] * basis[1].lane[i]) + localPosition.lane[2] * basis[2].lane[i];
    const unsigned shuffle18[4] = {2, 0, 1, 0}, shuffle09[4] = {1, 2, 0, 0};
    for (unsigned i = 0; i < 4; ++i)
        normal.lane[i] = worldB.lane[shuffle18[i]] * worldA.lane[shuffle09[i]] - worldB.lane[shuffle09[i]] * worldA.lane[shuffle18[i]];
    area = extent * 3.141590118408203125f; // 0x00FF429C, bits 40490FD0
    return CallGameVirtual<int32_t, 14>(this);
}
uint8_t RectangleLightEntries::Flip(uint32_t enabled) // 006BF2A0
{
    const uint8_t old = (flags >> 3) & 1;
    if (static_cast<uint8_t>(enabled) == old) return old;
    for (unsigned i = 0; i < 4; ++i) localB.lane[i] = 0.0f - localB.lane[i];
    for (unsigned i = 0; i < 4; ++i) localA.lane[i] = 0.0f - localA.lane[i];
    localPosition.lane[2] -= 0.100000001490116119384765625f; // 0106E33C
    // This method does not update bit 3 itself. A repeated call can flip again.
    return CallGameVirtual<uint8_t, 23>(this);
}
namespace {
template <class T, uint32_t Address> T& Global()
{
#if defined(_M_IX86)
    return *reinterpret_cast<T*>(static_cast<uintptr_t>(Address));
#else
    NotRewritten("animation component game global");
#endif
}
void SetComponentTables(AnimationComponentEntries& p)
{
#if defined(_M_IX86)
    p.vtable = reinterpret_cast<void* const*>(0x0104341c);
    p.secondary[0] = reinterpret_cast<void* const*>(0x010433ec);
    p.secondary[1] = reinterpret_cast<void* const*>(0x010433d4);
    p.secondary[2] = reinterpret_cast<void* const*>(0x01043388);
#else
    p.vtable = nullptr;
    p.secondary[0] = p.secondary[1] = p.secondary[2] = nullptr;
#endif
}
}
AnimationComponentEntries* AnimationComponentEntries::Construct(void* newOwner) // 0093E5C0
{
    owner = newOwner;
    unknown14 = 0;
    SetComponentTables(*this);
    listener = runtime = binding = nullptr;
    flags = 8;
    unknown30 = unknown34 = 0;
    status = 0;
    unknown1c = unknown18 = 0;
    const auto ownerFlags = *reinterpret_cast<const uint32_t*>(static_cast<const uint8_t*>(owner) + 0x2c);
    if (Global<uint32_t, 0x011e90fc>() & ownerFlags)
        CallGameMethod<void, 0x009357e0>(owner, Global<void*, 0x011dc47c>());
    return this;
}
bool AnimationComponentEntries::EnsureRuntime() // 0093F580
{
    if (runtime) return true;
    void* allocation = CallGameFunction<void*, 0x004e3f90>(uint32_t{192}, "Animation/ComponentRuntime", 0u, 0u, 0u, 0u);
    runtime = allocation ? CallGameMethod<void*, 0x0093e550>(allocation, &secondary[0]) : nullptr;
    if (!runtime) return false;
    if (owner) {
        auto* source = CallGameMethod<animation::ClipBinding*, 0x00935770>(owner, Global<void*, 0x011e9158>());
        if (source) {
            std::memcpy(runtime, source->GetMatrix(), 16);
            const auto* key = reinterpret_cast<RecordKeyView*>(source)->Key();
            std::memcpy(static_cast<uint8_t*>(runtime) + 16, key, 16);
            std::memcpy(static_cast<uint8_t*>(runtime) + 32, static_cast<uint8_t*>(runtime) + 16, 16);
        }
    }
    return true;
}
int32_t AnimationComponentEntries::Cleanup() // 00941CD0
{
    flags |= 0x800;
    SetComponentTables(*this);
    int32_t result = CallGameMethod<int32_t, 0x00940310>(this);
    if (binding) {
        CallGameMethod<void, 0x00581100>(binding);
        result = CallGameFunction<int32_t, 0x004e3f70>(binding);
        binding = nullptr;
    }
    if (listener) result = CallGameVirtual<int32_t, 1>(listener);
#if defined(_M_IX86)
    secondary[2] = reinterpret_cast<void* const*>(0x01043290);
    vtable = reinterpret_cast<void* const*>(0x01043260);
#else
    secondary[2] = vtable = nullptr;
#endif
    return result;
}
namespace {
__m128 Load(const LightVector& v) { return _mm_loadu_ps(v.lane); }
void Store(LightVector& v, __m128 x) { _mm_storeu_ps(v.lane, x); }
__m128 Sum3(__m128 v)
{
    return _mm_shuffle_ps(_mm_add_ps(_mm_add_ps(_mm_shuffle_ps(v,v,2), _mm_shuffle_ps(v,v,1)),v),
        _mm_add_ps(_mm_add_ps(_mm_shuffle_ps(v,v,2), _mm_shuffle_ps(v,v,1)),v),0);
}
__m128 Dot3(__m128 a, __m128 b) { return Sum3(_mm_mul_ps(a,b)); }
__m128 NormalizeEA(__m128 v)
{
    auto n = Dot3(v,v), r = _mm_rsqrt_ps(n);
    const auto one = _mm_set1_ps(1.0f), half = _mm_set1_ps(0.5f);
    r = _mm_add_ps(_mm_mul_ps(_mm_sub_ps(one,_mm_mul_ps(_mm_mul_ps(r,r),n)),_mm_mul_ps(half,r)),r);
    r = _mm_add_ps(_mm_mul_ps(_mm_sub_ps(one,_mm_mul_ps(_mm_mul_ps(r,r),n)),_mm_mul_ps(half,r)),r);
    return _mm_mul_ps(r,v);
}
float Property(uint32_t address)
{
#if defined(_M_IX86)
    return *CallGameMethod<const float*, 0x00f626f0>(reinterpret_cast<void*>(static_cast<uintptr_t>(address)));
#else
    (void)address;
    NotRewritten("rectangle light game property");
#endif
}
LightVector Project(RectangleLightEntries& light, __m128 difference)
{
    LightVector out{}; // W is an unused reserved lane in the original's stack temporary.
    out.lane[0] = _mm_cvtss_f32(Dot3(Load(light.worldA),difference));
    out.lane[1] = _mm_cvtss_f32(Dot3(Load(light.normal),difference));
    out.lane[2] = _mm_cvtss_f32(Dot3(Load(light.worldB),difference));
    return out;
}
}
double RectangleLightEntries::Intensity() // 006BCB00; ST0, not EAX
{
    const float factor = Property(0x011d0a78);
    auto weighted = _mm_mul_ps(Load(diffuse),_mm_set1_ps(Property(0x011d0ab0)));
    weighted = _mm_mul_ps(weighted,_mm_set1_ps(1.0f)); // 0107A538
    const float mean = _mm_cvtss_f32(_mm_mul_ps(Sum3(weighted),_mm_set1_ps(0.3333333432674408f))); // FAD54C
    // Both operands are floats, so this product is exact in double (as in x87 ST0).
    return static_cast<double>(mean) * static_cast<double>(factor);
}
bool RectangleLightEntries::RefreshEnvironment() // 006BF320
{
    const auto* environment = CallGameFunction<const uint8_t*, 0x006e97b0>();
    std::memcpy(&direction,environment+0x140,16);
    localColor.lane[0] = _mm_cvtss_f32(Dot3(Load(worldA),Load(direction)));
    localColor.lane[1] = _mm_cvtss_f32(Dot3(Load(normal),Load(direction)));
    localColor.lane[2] = _mm_cvtss_f32(Dot3(Load(worldB),Load(direction)));
    std::memcpy(&diffuse,environment+0x130,16);
    Store(diffuse,_mm_mul_ps(Load(multiplier),Load(diffuse)));
    highlight = {{0.1f,0.30000001192092896f,1.0f,0.1f}};
    frontFacing = !(localColor.lane[2] < 0.0f); // COMISS/JBE treats unordered as the positive branch.
    if (void* manager = Global<void*,0x011ecbc4>()) {
        struct PaletteManager { uint8_t prefix[0x74]; RecordKeyView* palette; };
        auto* palette = static_cast<PaletteManager*>(manager)->palette;
        LightVector color{}; color.lane[0] = localColor.lane[2];
        CallGameMethod<void,0x00c175e0>(palette->Key(), &color, frontFacing ? 3 : 2);
        highlight = color;
    }
    Store(highlight,_mm_mul_ps(Load(highlight),Load(multiplier)));
    Store(localColor,_mm_sub_ps(_mm_setzero_ps(),Load(localColor)));
    Store(localColor,_mm_mul_ps(_mm_set1_ps(64.0f),Load(localColor))); // FF8688
    return true;
}
LightVector* RectangleLightEntries::Intersect(LightVector* color, const LightVector* ray, LightVector* directionOut, float* weight) // 006BF540
{
    Store(*color,_mm_setzero_ps()); Store(*directionOut,_mm_setzero_ps());
    // Written in instruction order: NaN ray Z passes the first JAE test.
    if (ray->lane[2] <= 0.0f || !frontFacing) return color;
    auto z = _mm_set1_ps(localColor.lane[2]);
    const auto minus = _mm_set1_ps(-1.0f), two = _mm_set1_ps(2.0f), one = _mm_set1_ps(1.0f), zero = _mm_setzero_ps();
    auto absZ = _mm_max_ps(_mm_mul_ps(minus,z),z);
    if (1.0e-6f > _mm_cvtss_f32(absZ)) return color; // FBAA24=358637BD
    auto r = _mm_rcp_ps(z);
    r = _mm_mul_ps(_mm_sub_ps(two,_mm_mul_ps(r,z)),r);
    r = _mm_mul_ps(_mm_sub_ps(two,_mm_mul_ps(r,z)),r);
    auto t = _mm_mul_ps(r,_mm_sub_ps(zero,_mm_set1_ps(ray->lane[2])));
    const float time = _mm_cvtss_f32(t);
    if (0.0f > time || time > 1.0f) return color;
    auto point = _mm_add_ps(_mm_mul_ps(Load(localColor),t),Load(*ray));
    auto absolute = _mm_max_ps(_mm_mul_ps(minus,point),point);
    auto& initialized = Global<uint32_t,0x011d10f0>();
    if (!(initialized & 1)) { initialized |= 1; Store(Global<LightVector,0x011d10e0>(),_mm_set1_ps(0.25f)); }
    if (!(initialized & 2)) { initialized |= 2; Store(Global<LightVector,0x011d10d0>(),two); }
    auto falloff = _mm_min_ps(_mm_max_ps(_mm_sub_ps(one,_mm_mul_ps(
        _mm_add_ps(_mm_sub_ps(absolute,Load(bounds)),Load(Global<LightVector,0x011d10e0>())),
        Load(Global<LightVector,0x011d10d0>()))),zero),one);
    auto product = _mm_mul_ps(_mm_shuffle_ps(falloff,falloff,0x55),_mm_shuffle_ps(falloff,falloff,0));
    const float coverage = _mm_cvtss_f32(product);
    if (!(coverage > 0.0f)) return color;
    auto emitted = _mm_mul_ps(_mm_mul_ps(Load(diffuse),_mm_set1_ps(Property(0x011d0ab0))),product);
    *weight = coverage;
    Store(*color,_mm_add_ps(Load(*color),emitted));
    *directionOut = direction;
    return color;
}
LightVector* RectangleLightEntries::Attenuate(void* output, const void* input) // 006BF780
{
    auto* out = static_cast<LightVector*>(output);
    const auto* ray = static_cast<const LightVector*>(input);
    // COMISS zero,Z / JB enters on unordered too.
    if (ray->lane[2] <= 0.0f) { Store(*out,_mm_setzero_ps()); return out; }
    auto d = _mm_add_ps(Dot3(Load(*ray),Load(*ray)),_mm_set1_ps(1.0f));
    auto r = _mm_rcp_ps(d);
    r = _mm_mul_ps(_mm_sub_ps(_mm_set1_ps(2.0f),_mm_mul_ps(d,r)),r);
    const float threshold = Property(0x011d0a80);
    if (threshold > ray->lane[2]) {
        auto p = _mm_set1_ps(threshold), reciprocal = _mm_rcp_ps(p);
        reciprocal = _mm_mul_ps(_mm_sub_ps(_mm_set1_ps(2.0f),_mm_mul_ps(reciprocal,p)),reciprocal);
        r = _mm_mul_ps(_mm_mul_ps(reciprocal,_mm_set1_ps(ray->lane[2])),r);
    }
    Store(*out,_mm_mul_ps(Load(attenuation),r));
    return out;
}
RectangleLightEntries* RectangleLightEntries::Construct(const float* parameters, void* contextArg) // 006C0E60
{
    CallGameMethod<void,0x006c0170>(this,0u,8u,contextArg);
    flags &= ~8u;
#if defined(_M_IX86)
    vtable = reinterpret_cast<void* const*>(0x00ff4408);
#endif
    if (parameters) {
        Store(scale,_mm_set1_ps(1.0f));
        Store(activeColor, flags & 0x20 ? _mm_mul_ps(Load(tint),Load(scale)) : _mm_setzero_ps());
        multiplier = {{parameters[4],parameters[5],parameters[6],parameters[4]}};
        localPosition = {{parameters[1],parameters[2],parameters[3],parameters[1]}};
        localB = {{parameters[8],parameters[9],parameters[10],parameters[8]}};
        localA = {{parameters[11],parameters[12],parameters[13],parameters[11]}};
        extent = parameters[14] * parameters[14];
    }
    Store(attenuation,_mm_set1_ps(0.699999988079071f)); // FBA7D8=3F333333
    return this;
}
uintptr_t RectangleLightEntries::Evaluate(const LightVector* position, const LightVector* frame, LightVector* out) // 006BF880
{
    const uint32_t skip = CallGameVirtual<uint32_t,7>(this,1);
    if (static_cast<uint8_t>(skip) || !(flags & 0x20)) { Store(*out,_mm_setzero_ps()); return skip; }
    auto delta = _mm_sub_ps(Load(worldPosition),Load(*position));
    LightVector local = Project(*this,delta), normalized;
    Store(normalized,NormalizeEA(delta));
    LightVector windowColor{}, windowDirection{}, ambient{}, windowDot{}, ambientDot{};
    float coverage = 0;
    CallGameVirtual<void,21>(this,&windowColor,&local,&windowDirection,&coverage);
    CallGameMethod<void,0x006be440>(this,&ambient,&local);
    auto* dot = CallGameFunction<LightVector*,0x006bdba0>(&windowDot,frame,&windowDirection);
    Store(windowDot,_mm_mul_ps(Load(*dot),Load(windowColor)));
    dot = CallGameFunction<LightVector*,0x006bdba0>(&ambientDot,frame,&normalized);
    Store(*out,_mm_mul_ps(_mm_add_ps(_mm_mul_ps(Load(ambient),Load(*dot)),Load(windowDot)),Load(tint)));
    return reinterpret_cast<uintptr_t>(out);
}
void* RectangleLightEntries::Sample(const LightVector* position, void* output) // 006C0FE0
{
    auto* counters = CallGameFunction<void*,0x006c3550>();
    CallGameMethod<void,0x006c2920>(counters,context);
    auto* ownerField = reinterpret_cast<RectangleLightEntries**>(static_cast<uint8_t*>(output)+32);
    if (CallGameVirtual<uint8_t,7>(this,1) || !(flags & 0x20)) {
        LightVector zero{};
        *ownerField = this;
        return CallGameMethod<void*,0x006bdb00>(output,&zero,&zero,&zero,this);
    }
    auto delta = _mm_sub_ps(Load(worldPosition),Load(*position));
    LightVector local = Project(*this,delta), normalized;
    Store(normalized,NormalizeEA(delta));
    LightVector windowColor{}, windowDirection{}, ambient{};
    float coverage = 0;
    CallGameVirtual<void,21>(this,&windowColor,&local,&windowDirection,&coverage);
    CallGameMethod<void,0x006be440>(this,&ambient,&local);
    auto t = _mm_set1_ps(coverage);
    Store(normalized,_mm_add_ps(_mm_mul_ps(_mm_sub_ps(Load(windowDirection),Load(normalized)),t),Load(normalized)));
    Store(ambient,_mm_add_ps(_mm_mul_ps(_mm_sub_ps(Load(windowColor),Load(ambient)),t),Load(ambient)));
    *ownerField = this;
    return CallGameMethod<void*,0x006bdb00>(output,&normalized,&ambient,&tint,this);
}
bool PackedIndexWriter::WriteIndex() // 004A8680
{
    if (!(openFlags & 2) || !indexChanged) return true;
    const uint32_t count = index->Count();
    void* data = nullptr; uint32_t bytes = 0;
    if (!index->Save(&data,&bytes,1)) return false;
    headerDirty = true;
    bool ok;
    if (!bytes) {
        ok = FreeRegion(indexOffset,indexSize,false);
        indexCount = indexOffset = indexSize = 0;
    } else {
        const uint32_t oldSize = indexSize;
        indexCount = count;
        indexSize = bytes;
        ok = true;
        if (bytes != oldSize || (writeFlags & 0x10)) {
            ok = FreeRegion(indexOffset,oldSize,false);
            if (ok) {
                indexOffset = 0;
                uint32_t offset = 0;
                ok = AllocateRegion(&offset,indexSize,true);
                indexOffset = offset;
                if (!ok) indexCount = indexOffset = indexSize = 0;
            }
        }
        if (ok) {
            ok = WriteRaw(data,indexOffset,bytes);
            if (ok && indexObserver) ok = CallGameVirtual<bool,1>(indexObserver,this,data,bytes);
            if (!ok && indexOffset) {
                FreeRegion(indexOffset,indexSize,false);
                indexCount = indexOffset = indexSize = 0;
            }
        }
    }
    indexChanged = false;
    allocator->Free(data,0);
    return ok;
}

void PackedIndexWriter::DestroyIndex(void* object) // 009EED90
{
    if (object) CallGameVirtual<void,0>(object,1);
}
void PackedIndexWriter::MakeMutable() // 004A6C80
{
    resource::DatabasePackedFile::MakeIndexModifiable();
}
bool PackedIndexWriter::HeaderValid(const resource::PackageHeader* header) // 004A7AD0
{
    return resource::IsValidPackageHeader(*header,GetFileSize());
}
bool PackedIndexWriter::CheckIndex() // 004A7D80
{
    return resource::DatabasePackedFile::ValidateIndex();
}
void PackedIndexWriter::RebuildSpace() // 004A8810
{
    resource::DatabasePackedFile::RebuildFreeSpace();
}
bool PackedIndexWriter::ReadIndex() // 004A9950
{
    return resource::DatabasePackedFile::LoadIndex();
}
} // namespace ts3::misc
