#include "ts3/renderer/clu_58aad0_Renderer.h"
#include <atomic>
#include <cstring>
#include <cmath>
#include "ts3/locomotion/clu_58abf0_Locomotion.h"
#include "ts3/misc/clu_961480_x.h"
#include "ts3/routing/clu_5a00a0_Routing.h"
#if defined(_M_IX86)
#include <intrin.h>
#endif

namespace ts3::renderer {
namespace {
const char* Label(uintptr_t address, const char* portable)
{
#if defined(_M_IX86)
    (void)portable;
    return reinterpret_cast<const char*>(address);
#else
    (void)address;
    return portable;
#endif
}
void InitArray(RenderArray& a, const char* name)
{
    a.data = nullptr;
    a.size = 0;
    a.capacity = 0;
    a.name = name;
}
float X87Copy(float value)
{
    // FLD/FSTP quiets signaling NaNs, unlike a plain SSE MOVSS copy.
    uint32_t bits;
    std::memcpy(&bits, &value, sizeof bits);
    if ((bits & 0x7f800000u) == 0x7f800000u && (bits & 0x007fffffu)) bits |= 0x00400000u;
    std::memcpy(&value, &bits, sizeof bits);
    return value;
}
}

// 0x006E0540
RenderArray* RenderArray::InitRenderSet()
{
    InitArray(*this, Label(0x00ff7680, "Renderer/RenderSet/Set"));
    return this;
}

// 0x0068E540
DisplayList* DisplayList::Init()
{
    for (auto& bin : bins) InitArray(bin, Label(0x00ff097c, "Renderer/DisplayList/BinList"));
    for (auto& set : sets) set.InitRenderSet();
    InitArray(activeMask, Label(0x00ff09c0, "Renderer/DisplayList/ActiveObjectMask"));
    InitArray(transforms, Label(0x00ff099c, "Renderer/DisplayList/TransformCache"));
    return this;
}

// 0x006E7550: only AL is defined; the upper EAX bytes retain the argument's bits.
uint8_t LodSchedulerLink::SetScene(Scene* value)
{
    scene = value;
    return 1;
}

// 0x0096BA10: preserve the original interleaved load/store order, including aliased outputs.
float* CameraParameters::Get(CameraVector* a, CameraVector* b, CameraVector* c,
                            float* nearOut, float* farOut, float* limitOut)
{
    *a = vectors[0];
    *b = vectors[1];
    *c = vectors[2];
    *nearOut = X87Copy(nearPlane);
    *farOut = X87Copy(farPlane);
    *limitOut = X87Copy(limit);
    return farOut;
}

// 0x006E3FA0
uint8_t Scene::CreateNode(SceneNode* parent, SceneNode** output)
{
    if (!parent) parent = root;
    void* memory = CallGameFunction<void*, 0x004e3f90>(static_cast<uint32_t>(sizeof(SceneNode)),
        Label(0x00ff7e70, "Renderer/Scene/SceneNode"), 0u, 0u, 0u, 0u);
    *output = memory ? CallGameMethod<SceneNode*, 0x006fa880>(static_cast<SceneNode*>(memory)) : nullptr;
    if (!*output) return 0;
    CallGameVirtual<void, 1>(*output);
    if (parent) {
        (*output)->sibling.prev = parent->children.prev;
        (*output)->sibling.next = &parent->children;
        parent->children.prev = &(*output)->sibling;
        (*output)->sibling.prev->next = &(*output)->sibling;
        CallGameVirtual<void, 1>(*output);
    }
    (*output)->parent = parent;
    return 1;
}

// 0x006E4530
Scene* Scene::Init(void* ownerValue)
{
    root = nullptr;
    InitArray(objects, Label(0x00ff7ed8, "Renderer/Scene/ObjectTable"));
    unknown20 = unknown24 = 0;
    owner = ownerValue;
    links.next = links.prev = &links;
    // The game uses XCHG here. There is no live C++ atomic object in incoming game storage.
#if defined(_M_IX86)
    _InterlockedExchange(reinterpret_cast<volatile long*>(&lock), 0);
#else
    lock = 0;
    std::atomic_thread_fence(std::memory_order_seq_cst);
#endif
    unknown44 = 0;
    InitArray(frustum, Label(0x00ff7eb8, "Renderer/Scene/ObjectsInFrustum"));
    InitArray(visible, Label(0x00ff7e8c, "Renderer/Scene/VisibleObjectsForLighting"));
    unknown80 = unknown84 = unknown88 = unknown92 = 0;
    enabled = 1;
    unknown100 = 0;
    CreateNode(nullptr, &root); // Allocation failure is deliberately not propagated.
    return this;
}

// 0x006E8480
uint8_t RendererSceneHost::CreateNode(SceneNode* parent, SceneNode** output)
{
    return scene ? scene->CreateNode(parent, output) : 0;
}

// 0x006EB540
uint8_t RendererSceneHost::EnsureDisplayList(DisplayList** output)
{
    if (*output) return 1;
    void* memory = CallGameFunction<void*, 0x004e3fb0>(static_cast<uint32_t>(sizeof(DisplayList)),
        16u, 0u, Label(0x00ff85d8, "Renderer/DisplayList"), 0u, 0u, 0u, 0u);
    *output = memory ? static_cast<DisplayList*>(memory)->Init() : nullptr;
    if (*output) return 1;
    CallGameMethod<void, 0x006e97f0>(this);
    return 0;
}

namespace {
template<class T, uintptr_t Address> T& Global()
{
#if defined(_M_IX86)
    return *reinterpret_cast<T*>(Address);
#else
    AbortNotRewritten("renderer game global");
    static T unreachable{};
    return unreachable;
#endif
}
const wchar_t* Section() { return Global<const wchar_t*, 0x0114fab0>(); }
const wchar_t* Wide(uintptr_t address, const wchar_t* portable)
{
#if defined(_M_IX86)
    (void)portable; return reinterpret_cast<const wchar_t*>(address);
#else
    (void)address; return portable;
#endif
}
int CompareWide(const wchar_t* a, const wchar_t* b)
{
#if defined(_M_IX86)
    using Fn = int(__cdecl*)(const wchar_t*, const wchar_t*);
    return Global<Fn, 0x00f95758>()(a, b);
#else
    (void)a; (void)b; AbortNotRewritten("game wcsicmp"); return 0;
#endif
}
float ConfigFloat(const wchar_t* section, const wchar_t* key, float fallback)
{
    locomotion::ConfigDoubleLookup lookup;
    return static_cast<float>(lookup.ConfigDouble(section, key, fallback));
}
int32_t ConfigInt(const wchar_t* key, int32_t fallback)
{ return CallGameFunction<int32_t, 0x0058aba0>(Section(), key, fallback); }
uint8_t ReadBool(const wchar_t* key, uint8_t fallback)
{ return ConfigBool(Section(), key, fallback); }
void StoreWord(uint8_t* p, uint32_t value) { std::memcpy(p, &value, 4); }
void StoreFloat(uint8_t* p, float value) { std::memcpy(p, &value, 4); }
float LoadFloat(const uint8_t* p) { float value; std::memcpy(&value, p, 4); return value; }
uint32_t FloatBits(float value) { uint32_t bits; std::memcpy(&bits, &value, 4); return bits; }
float FromBits(uint32_t bits) { float value; std::memcpy(&value, &bits, 4); return value; }
}

// 0x0058AAD0. Empty strings count as true; unrecognized strings retain the fallback.
uint8_t ConfigBool(const wchar_t* section, const wchar_t* key, uint8_t fallback)
{
    void* registry = CallGameFunction<void*, 0x0058b880>();
    if (!registry) return fallback;
    const wchar_t* value;
    if (!CallGameMethod<uint8_t, 0x0058c380>(registry, section, key, &value)) return fallback;
    uint8_t result = fallback;
    if (!CompareWide(value, L"true") || !CompareWide(value, L"1") || !CompareWide(value, L"")) result = 1;
    if (!CompareWide(value, L"false") || !CompareWide(value, L"0")) result = 0;
    void* table;
#if defined(_M_IX86)
    table = reinterpret_cast<void*>(uintptr_t{0x0114fab4});
#else
    AbortNotRewritten("renderer config override table"); table = nullptr;
#endif
    auto* entry = CallGameFunction<uint8_t*, 0x0058a9a0>(table, 1, section, key);
    return entry ? entry[4] : result;
}

// 0x0068D8A0. The callee is ordinary thiscall with no stack arguments (IDA's
// guessed four-argument prototype was wrong). It fills vectors 18..43.
void Frustum::Init()
{
    const float initial[12][4] = {
        {1,0,0,1},{-1,0,0,-1},{0,1,0,0},{0,-1,0,0},{0,0,1,0},{0,0,-1,0},
        {1,0,0,1},{1,0,0,1},{0,1,0,0},{0,1,0,0},{0,0,1,0},{0,0,1,0}};
    std::memcpy(vectors, initial, sizeof initial);
    std::memset(vectors + 12, 0, 6 * sizeof vectors[0]);
    CallGameMethod<void, 0x0068c360>(this);
}

// 0x006E7850
LodScheduler* LodScheduler::Init()
{
    limits[0] = 400; limits[1] = 500; limits[2] = limits[3] = limits[4] = 0;
    flags[0] = 0; flags[1] = 1; unknown28 = 0; scene = nullptr; unknown36 = 0;
    factor = 0.03f; unknown48[0] = unknown48[1] = 0;
    InitArray(activeMask, Label(0x00ff80d4, "Renderer/SceneLODScheduler/LastActiveMask"));
    InitArray(objectStates, Label(0x00ff80ac, "Renderer/SceneLODScheduler/ObjectStates"));
    if (CallGameFunction<uint8_t, 0x0058b100>("ForceHighLODObjects", 0)) {
        adjust = 100.0f; nearDistance = farDistance = 100000.0f;
        for (unsigned i = 0; i < 5; ++i) { distances[i] = 100000.0f; squared[i] = 1.0e10f; }
    } else {
        adjust = 0; nearDistance = 10; farDistance = 80;
        alignas(16) float vector[4];
        float simDistance = 150;
        if (CallGameFunction<uint8_t, 0x0058b040>(Section(), Wide(0x00fcc25c, L"RenderSimLODDistances"), vector)) simDistance = vector[3];
        distances[0] = 1000; distances[1] = 2500; distances[2] = distances[3] = 500; distances[4] = simDistance;
        squared[0] = 1000000; squared[1] = 6250000; squared[2] = squared[3] = 250000; squared[4] = simDistance * simDistance;
    }
    flags[2] = ReadBool(Wide(0x00ff8060, L"ForceActiveLotObjectsToHighLOD"), 1);
    if (CallGameFunction<uint8_t, 0x0058b100>("ForceMinspecObjectLOD", 0)) flags[2] = 0;
    CallGameFunction<void*, 0x006e8330>();
    Global<float, 0x0115b148>() = Global<uint8_t, 0x0115b178>() ? 2000.0f :
        ConfigFloat(Section(), Wide(0x00ff801c, L"ObjectSizeCullFactor"), 100.0f);
    return this;
}

// 0x006E9270. The returned EAX is the selector after its actual subtract chain.
int32_t RendererSceneHost::ReloadConfig()
{
    auto byte = [&](size_t offset) -> uint8_t& { return config[offset - 56]; };
    auto word = [&](size_t offset, int32_t value) { StoreWord(config + offset - 56, static_cast<uint32_t>(value)); };
    if (this == Global<RendererSceneHost*, 0x011d1860>()) {
        byte(613) = ReadBool(Wide(0x00ff8590, L"RenderPostProcessEnabled"), 1);
        byte(616) = ReadBool(Wide(0x00ff855c, L"RenderReflectionsEnabled"), 1);
        byte(614) = ReadBool(Wide(0x00ff8528, L"ComplexPostProcessEnabled"), 1);
        word(660, ConfigInt(Wide(0x00fcc054, L"WaterReflectionDetail"), 0));
        word(664, ConfigInt(Wide(0x00fcc018, L"WaterReflectionAreaThreshold"), 1000));
    }
    word(644, ConfigInt(Wide(0x00fcc13c, L"CASLightMapWidth"), 1024));
    byte(634) = ReadBool(Wide(0x00ff84f0, L"CASLightMapFilteringEnabled"), 1);
    byte(636) = ReadBool(Wide(0x00ff84b8, L"CASSimpleHairShaderEnabled"), 0);
    uint32_t mode; std::memcpy(&mode, Global<uint8_t*, 0x011d753c>() + 64, 4);
    uint32_t result = mode;
    if (!mode) word(640, 1);
    else if (!(--result) || !(result -= 2)) word(640, 2);
    else word(640, 0);
    return static_cast<int32_t>(result);
}

// 0x00713430
uint8_t TreeLodConfig::Reload()
{
    for (auto& r : ranges) { r[0] = 30; r[1] = 170; r[3] = 3000; r[4] = 3300; }
    for (unsigned i = 0; i < 4; ++i) {
        alignas(16) float value[4];
        const wchar_t* key;
#if defined(_M_IX86)
        key = reinterpret_cast<const wchar_t* const*>(uintptr_t{0x0115b4dc})[i];
#else
        AbortNotRewritten("tree LOD key table"); key = nullptr;
#endif
        if (CallGameFunction<uint8_t, 0x0058b040>(Section(), key, value)) {
            ranges[i][0] = value[0]; ranges[i][1] = value[1]; ranges[i][3] = value[2]; ranges[i][4] = value[3];
        }
        float width = ranges[i][1] - ranges[i][0];
        ranges[i][2] = width > 0 ? 1.0f / width : 0.0f;
    }
    return alphaEdges = ReadBool(Wide(0x00ffb81c, L"RenderTreeAlphaEdges"), 1);
}

// 0x0076B3C0
uint8_t MinimizedConfig::Reload()
{ return simulate = ReadBool(L"SimWhileMinimized", 0); }

// 0x00962F80
Camera* Camera::Init()
{
#if defined(_M_IX86)
    vtable = reinterpret_cast<void**>(uintptr_t{0x0107396c});
    _InterlockedExchange(reinterpret_cast<volatile long*>(&refs), 0);
#else
    vtable = nullptr; refs = 0; std::atomic_thread_fence(std::memory_order_seq_cst);
#endif
    auto word = [&](size_t offset, uint32_t value) { StoreWord(body + offset - 8, value); };
    auto scalar = [&](size_t offset, float value) { StoreFloat(body + offset - 8, value); };
    auto byte = [&](size_t offset, uint8_t value) {
        if (offset >= 1788) tail[offset - 1788] = value;
        else body[offset - 8] = value;
    };
    auto vector = [&](size_t offset, const float* value) { std::memcpy(body + offset - 8, value, 16); };
    scalar(608,0); scalar(612,1); scalar(616,1); scalar(620,0); byte(1756,0);
    for (auto& cell : reserved) cell = nullptr;
    index = -1; byte(1824,0);
    reinterpret_cast<Frustum*>(body + 640 - 8)->Init();
    scalar(24, FromBits(0x3faaaaab)); scalar(624,10); word(8,0);
    const float zero[4] = {0,0,0,0}, x[4] = {1,0,0,0}, y[4] = {0,1,0,0}, z[4] = {0,0,1,0}, w[4] = {0,0,0,1};
    vector(496,zero); scalar(12,2); scalar(16,1.5f);
    float angle = static_cast<float>(std::atan2(1.0,1.0)); scalar(20,angle * 2.0f);
    vector(528,y);
    auto* parameters = CallGameFunction<CameraParameters*, 0x0096b390>();
    alignas(16) CameraVector a,b,c; float nearValue, farValue, limit;
    parameters->Get(&a,&b,&c,&nearValue,&farValue,&limit);
    std::memcpy(body + 1696 - 8, &a, 16); std::memcpy(body + 1712 - 8, &b, 16); std::memcpy(body + 1728 - 8, &c, 16);
    scalar(1744,nearValue); scalar(1748,farValue);
    float delta = farValue - limit;
    // MAXPS(-delta,delta) returns its second operand on NaN and ties.
    float negative = -1.0f * delta;
    float absolute = negative > delta ? negative : delta;
    scalar(1752,absolute <= FromBits(0x37800000) ? 1.0f : 1.0f / (limit - farValue));
    word(28,b.bits[0]); word(32,b.bits[1]);
    vector(48,x); vector(64,y); vector(80,z); vector(96,w);
    vector(432,x); vector(448,y); vector(464,z); vector(480,w);
    CallGameMethod<void, 0x00962f10>(this, body + 48 - 8);
    byte(1757,0); byte(1758,0); byte(1789,0); byte(1788,1);
    return this;
}

Camera* Camera::InitForCompare()
{
    // The constructor stores no self-relative pointers and its math bridges do not
    // retain this. A byte-identical aligned scratch object therefore preserves all
    // outputs while accommodating the generic compare engine's byte-vector shadow.
    alignas(16) Camera aligned;
    std::memcpy(&aligned, this, sizeof aligned);
    aligned.Init();
    std::memcpy(this, &aligned, sizeof aligned);
    return this;
}

// 0x006EB5A0
uint8_t RendererSceneHost::EnsureCamera(Camera** output)
{
    if (*output) return 1;
    void* memory = CallGameFunction<void*, 0x004e3fb0>(static_cast<uint32_t>(sizeof(Camera)), 16u, 0u,
        Label(0x00ff85f0, "Renderer/SceneManagerCamera"), 0u,0u,0u,0u);
    *output = memory ? static_cast<Camera*>(memory)->Init() : nullptr;
    if (!*output) { CallGameMethod<void, 0x006e97f0>(this); return 0; }
#if defined(_M_IX86)
    _InterlockedExchangeAdd(reinterpret_cast<volatile long*>(&(*output)->refs), 1);
#else
    ++(*output)->refs; std::atomic_thread_fence(std::memory_order_seq_cst);
#endif
    return 1;
}

// 0x006EB610
uint8_t RendererSceneHost::Initialize()
{
    ReloadConfig();
    if (!scene) {
        auto* memory = CallGameFunction<Scene*, 0x004e3fb0>(static_cast<uint32_t>(sizeof(Scene)),4u,0u,
            Label(0x00ff8628,"Renderer/Scene"),0u,0u,0u,0u);
        scene = memory ? memory->Init(this) : nullptr;
        if (!scene) { CallGameMethod<void, 0x006e97f0>(this); return 0; }
    }
    if (!scheduler) {
        auto* memory = CallGameFunction<LodScheduler*, 0x004e3fb0>(static_cast<uint32_t>(sizeof(LodScheduler)),8u,0u,
            Label(0x00ff860c,"Renderer/SceneLODScheduler"),0u,0u,0u,0u);
        scheduler = memory ? memory->Init() : nullptr;
        if (!scheduler) { CallGameMethod<void, 0x006e97f0>(this); return 0; }
        if (this == Global<RendererSceneHost*, 0x011d1868>()) routing::RegisterLodSettings(reinterpret_cast<routing::LodSettingsHost*>(scheduler));
    }
    for (auto& list : lists) if (!EnsureDisplayList(&list)) return 0;
    // Same private helper as 0x006E7550, but the actual scheduler layout grows with pointers.
    reinterpret_cast<LodSchedulerLink*>(scheduler)->SetScene(scene);
    for (auto& camera : cameras) if (!EnsureCamera(&camera)) return 0;
    return 1;
}

// 0x0097C2F0. Only its stores are consumed by callers; false-path EAX in the game
// is undefined (the already-verified ChainSetWeight deliberately returns idx).
int RotationWeights::SetVertical(uint8_t value)
{
    int result = 0;
    for (auto& chain : chains) result = misc::ChainSetWeight(chain + 64, 4, value != 0);
    return result;
}

// 0x0096B730
uint8_t RotationConfig::Reload()
{
    invertX = ReadBool(L"InvertHorizontalRotation",0);
    invertY = ReadBool(L"InvertVerticalRotation",0);
    static_cast<uint8_t*>(horizontal)[2252] = ReadBool(Wide(0x010451a8,L"EdgeScrolling"),1);
    misc::SetChainWeights(*static_cast<misc::ChainMotionState*>(horizontal),invertX != 0,invertY != 0);
    vertical->SetVertical(invertY);
    misc::SetChainWeights(*static_cast<misc::ChainMotionState*>(secondary),invertX != 0,invertY != 0);
    return squarePixels = ReadBool(Wide(0x01045184,L"ForceSquarePixels"),0);
}

// 0x00B25D10
uint8_t CullGroup::SetCull(uint8_t value)
{
    uint8_t result = value;
    if (enabled != value) {
        enabled = value;
        int32_t newMask = value ? 0 : -1;
        result = static_cast<uint8_t>(newMask);
        if (newMask != mask) {
            mask = newMask;
            auto* query = static_cast<uint8_t*>(queryOwner) + 2 * sizeof(void*);
            int token = CallGameVirtual<int,0>(query,newMask);
            void* manager = CallGameFunction<void*,0x006e8330>();
            result = static_cast<uint8_t>(reinterpret_cast<uintptr_t>(firstEnd));
            for (auto p = firstBegin; p != firstEnd; ++p) {
                void* object = *p;
                CallGameMethod<void,0x006e84c0>(manager,object);
                result = CallGameMethod<uint8_t,0x006e84b0>(manager,object,token);
            }
            for (auto p = secondBegin; p != secondEnd; ++p) {
                void* object = *p;
                CallGameMethod<void,0x006e84c0>(manager,object);
                result = CallGameMethod<uint8_t,0x006e84b0>(manager,object,token);
            }
        }
    }
    return result;
}

// 0x00AE3870
int ObjectHidingConfig::Reload()
{
    hiding = ReadBool(L"ObjectHiding",1);
    unload = ReadBool(L"AllowImpostorUnload",0);
    uint8_t value = ReadBool(L"CullUndergroundBuildBuyStructures",0);
    if (value != cull) { cull = value; if (group) group->SetCull(value); }
    return CallGameMethod<int,0x00ae1b80>(this);
}

// 0x00C0E140
uint8_t UndergroundObject::Reload()
{ return cull = ReadBool(L"CullUndergroundBuildBuyStructures",0); }

// 0x00C0B4C0
uint8_t UndergroundConfig::Reload()
{
    for (auto p = begin; p != end; ++p) (*p)->Reload();
    return cull = ReadBool(L"CullUndergroundBuildBuyStructures",0);
}

// 0x00EC5480. Preserve the arithmetic sign-bit selects rather than replacing them
// with min/max: signed zeros and NaN payloads follow the original bit operations.
double MovieVolume(const wchar_t* movie)
{
    if (ReadBool(Wide(0x01028518,L"MusicMute"),0)) return 0.0;
    float level = ConfigFloat(Section(),Wide(0x01028588,L"MusicLevel"),255.0f) * FromBits(0x3b808081);
    uint32_t mask = 0u - (FloatBits(-0.0f - level) >> 31);
    level = FromBits(FloatBits(level) & mask);
    mask = 0u - (FloatBits(1.0f - level) >> 31);
    level = FromBits((0x3f800000u & mask) | (FloatBits(level) & ~mask));
    // The gain remains in x87 double precision through the final multiply.
    locomotion::ConfigDoubleLookup lookup;
    return lookup.ConfigDouble(Wide(0x010f3df8,L"IntroMovies"),movie,1.0f) * static_cast<double>(level);
}
}
