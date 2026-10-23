#include "ts3/world/clu_60f270_World.h"
#include <cstring>
namespace ts3::world {
namespace {
// D3D9 interfaces use COM stdcall, not the game's thiscall virtual ABI.
template<class R, int Slot, class... Args> R Com(void* self, Args... args) {
#if defined(_M_IX86)
    using Fn = R(__stdcall*)(void*, Args...);
    return reinterpret_cast<Fn>((*static_cast<void***>(self))[Slot])(self, args...);
#else
    (void)self; ((void)args, ...);
    AbortNotRewritten("D3D9 COM virtual method");
    return R{};
#endif
}
}
// 0x0060F270
int32_t RenderContext::Clear(uint8_t flags, int32_t color, float depth, int32_t stencil) {
    int32_t mask = 0;
    void* surface;
    int32_t result = Com<int32_t, 40>(device, &surface);
    if (result >= 0) {
        uint32_t desc[8];
        Com<int32_t, 12>(surface, desc);
        if ((desc[0] == 75 || desc[0] == 83) && (flags & 4)) mask = 4;
        // EA ignores the depth flag too when GetDepthStencilSurface fails.
        if (flags & 2) mask |= 2;
        result = Com<int32_t, 2>(surface);
    }
    if (flags & 1) mask |= 1;
    if (mask) {
        const int32_t saved = savedState;
        if (saved) Com<int32_t, 57>(device, 194, 0);
        result = Com<int32_t, 43>(device, 0, 0, mask, color, depth, stencil);
        if (saved) return Com<int32_t, 57>(device, 194, saved);
    }
    return result;
}
// 0x00611620
uint8_t RenderContext::Begin() {
    if (begun) return 1;
    if (blocked) return 0;
    unsigned attempts = 0;
    while (Com<int32_t, 41>(device) < 0) {
        if (CallGameMethod<uint8_t, 0x00610ab0>(this, 0)) {
            if (++attempts < 2) continue;
        }
        return 0;
    }
    begun = 1;
    return 1;
}
// 0x009615E0: load the complete vector before writing, including aliasing inputs.
void* ShaderVectorState::SetVector(const void* source) {
    uint8_t value[16];
    std::memcpy(value, source, 16);
    std::memcpy(vector, value, 16);
    return const_cast<void*>(source);
}
namespace {
template<class T, unsigned Address> T& PickGlobal() {
#if defined(_M_IX86)
    return *reinterpret_cast<T*>(static_cast<uintptr_t>(Address));
#else
    NotRewritten("picking game global");
#endif
}
void SetPickConstant(PickConstants* constants, uint32_t index, const float* value) {
    if (index < constants->count) {
        auto& entry = constants->entries[index];
        ++entry.version; entry.dirty = 1; entry.type = 2; entry.generation = constants->generation; entry.value = value;
    }
}
float PickBits(uint32_t bits) { float value; std::memcpy(&value,&bits,4); return value; }
}
// 0x006DF8E0
int32_t PickRenderer::Draw(PickShader* shader) {
    auto renderer = CallGameFunction<RenderContext*, 0x0060e2c0>();
    alignas(16) uint8_t view[1056];
    CallGameMethod<void, 0x0068d9e0>(view, shader);
    uint32_t mask = ~uint32_t{};
    if ((!alternate) & special) mask = 0xe3ca8205;
    else if ((!alternate) & filtered) mask = 0xefee8205;
    alignas(16) uint8_t request[112];
    CallGameMethod<void, 0x0068df50>(request);
    CallGameMethod<void, 0x0068dee0>(request, view, maskA, mask & maskB, 5, 0);
    const uint32_t requestFlags = ((!alternate) & special) ? 128 : 144;
    std::memcpy(request + 12, &requestFlags, 4);
    if ((!alternate) & special) request[103] = 1;
    request[101] = includeHidden == 0;
    CallGameMethod<void, 0x006e4130>(scene);
    if ((!alternate) & filtered) {
        PickFilter objects{nullptr,nullptr,nullptr,"Renderer/SceneCullFilter/ObjectList",filter,0x02000000,1,{0,0,0},-1,{0,0}};
        CallGameMethod<void, 0x006e6570>(scene, &objects, world, request, 0);
        if (objects.begin) CallGameFunction<void, 0x004e3f70>(objects.begin);
    } else CallGameMethod<void, 0x006e6530>(scene, world, request, 0);
    CallGameMethod<void, 0x0060ec50>(renderer);
    CallGameMethod<void, 0x0060f7e0>(renderer, 0);
    PickDrawContext draw{view,128,shader,shader,{0,0,0,0,0,0,0},0,{0,0,0,0,-1,0,0}};
    CallGameMethod<void, 0x006e18c0>(PickGlobal<void*, 0x011d1530>(), shader);
    CallGameMethod<void, 0x0060f990>(renderer, 2);
    CallGameMethod<void, 0x0060f770>(renderer, 1);
    CallGameMethod<void, 0x0060f790>(renderer, 1);
    CallGameMethod<void, 0x0060f7c0>(renderer, 3);
    CallGameMethod<void, 0x0060f7b0>(renderer, 1);
    CallGameMethod<void, 0x0060f8a0>(renderer, 0);
    CallGameMethod<void, 0x0060f940>(renderer, 0);
    CallGameMethod<void, 0x0060f750>(renderer, 15);
    auto manager = scene->manager;
    CallGameMethod<void, 0x006f16b0>(manager, &world->entries, 1);
    auto constants = PickGlobal<PickConstants*, 0x011d7530>();
    alignas(16) float color[4] = {};
    alignas(16) float projection[4] = {PickBits(0x3f800aed), PickBits(0xbf000aed),0,0};
    SetPickConstant(constants,projectionConstant,projection);
    auto oldMode = PickGlobal<uint32_t, 0x011cf564>();
    PickGlobal<uint32_t, 0x011cf564>() = 5;
    for (auto entry = world->entries.begin; entry != world->entries.end; ++entry) {
        auto item = entry->item; auto object = item->object;
        alignas(16) uint8_t transform[64];
        auto material = CallGameMethod<void*, 0x006faf40>(object, transform);
        CallGameMethod<void, 0x006ea490>(manager, material);
        CallGameMethod<void, 0x006ea780>(manager, object);
        const uint32_t id = object->id + 1;
        // x87 multiplies byte integers by the exact loaded float 0x3B808081.
        const double inverse = static_cast<double>(PickBits(0x3b808081));
        color[0] = static_cast<float>(static_cast<double>(id & 255) * inverse);
        color[1] = static_cast<float>(static_cast<double>((id >> 8) & 255) * inverse);
        SetPickConstant(constants,colorConstant,color);
        if (alternate) {
#if defined(_M_IX86)
            auto token = reinterpret_cast<void*>(uintptr_t{0x017f1d88});
#else
            void* token = nullptr;
#endif
            if (auto interface = CallGameVirtual<void*, 3>(object, token))
                CallGameVirtual<void, 35>(interface, item->data, &draw);
        } else CallGameVirtual<void, 6>(object, item->data, &draw);
    }
    SetPickConstant(constants,colorConstant,nullptr);
    PickGlobal<uint32_t, 0x011cf564>() = oldMode;
    if (auto overlay = CallGameFunction<void*, 0x00712bd0>())
        CallGameVirtual<void, 2>(overlay, world->extras, &draw);
    SetPickConstant(constants,projectionConstant,nullptr);
    return CallGameMethod<int32_t, 0x0060fce0>(renderer);
}
// 0x006DFEC0
uint8_t PickRenderer::Update() {
    auto renderer = CallGameFunction<RenderContext*, 0x0060e2c0>();
    uint8_t result = static_cast<uint8_t>(reinterpret_cast<uintptr_t>(renderer));
    if (pending) {
        auto resources = PickGlobal<void*, 0x011cecdc>();
        auto target = CallGameMethod<void**, 0x00617540>(resources, uint32_t{0x6ee45453});
        auto surface = CallGameMethod<void**, 0x00617500>(resources, uint32_t{0x6ee45453});
        result = static_cast<uint8_t>(reinterpret_cast<uintptr_t>(surface));
        if (target && surface) {
            alignas(16) uint8_t state[48];
            CallGameMethod<void, 0x0060e2e0>(renderer,state);
            CallGameMethod<void, 0x0060ef20>(renderer,0,*surface);
            CallGameMethod<void, 0x0060ef70>(renderer,nullptr);
            completed = CallGameMethod<uint8_t, 0x00610660>(renderer,*target,0,0,0,0,0);
            result = CallGameMethod<uint8_t, 0x0060e410>(renderer,state);
        }
        pending = 0;
        return result;
    }
    if (completed) return result;
    if (requestedX < 0) {
        if (!source->ready) return result;
        result = static_cast<uint8_t>(fallbackX);
        if (fallbackX <= 0) return result;
        requestedX = fallbackX; requestedY = fallbackY;
    }
    result = renderer->Begin();
    if (!result) return result;
    auto resources = PickGlobal<void*, 0x011cecdc>();
    auto target = CallGameMethod<void**, 0x00617540>(resources,uint32_t{0x6ee45453});
    auto surface = CallGameMethod<void**, 0x00617500>(resources,uint32_t{0x6ee45453});
    auto depth = CallGameMethod<void**, 0x00617500>(resources,uint32_t{0x90361c05});
    result = static_cast<uint8_t>(reinterpret_cast<uintptr_t>(depth));
    if (!target || !surface || !depth) return result;
    const int32_t x = requestedX, y = requestedY;
    capturedX = x; capturedY = y;
    std::memcpy(savedMatrix,source->matrix,sizeof savedMatrix);
    requestedX = requestedY = -1;
    alignas(16) uint8_t state[48];
    CallGameMethod<void, 0x0060e2e0>(renderer,state);
    CallGameMethod<void, 0x0060ef20>(renderer,0,*surface);
    CallGameMethod<void, 0x0060ef70>(renderer,*depth);
    PickShader shader;
    CallGameMethod<void, 0x006df560>(&shader,source);
    renderer->Clear(7,0,1.0f,0);
    const float width = source->sizeX, height = source->sizeY;
    CallGameMethod<void, 0x00961570>(&shader, static_cast<float>(scaleX * width) * 16.0f);
    CallGameMethod<void, 0x00962550>(&shader, static_cast<float>(source->extent * worldScale) * scaleX);
    const float sx = static_cast<float>(static_cast<float>(static_cast<float>(x) * scaleX) - 0.5f) * width;
    const float sy = static_cast<float>(0.5f - static_cast<float>(static_cast<float>(y) * scaleY)) * height;
    alignas(16) float vector[4] = {sx,sy,sx,sx}; // EA duplicates x into z AND w.
    shader.state.SetVector(vector);
    CallGameMethod<void, 0x00962940>(&shader,0);
    Draw(&shader);
    if (deferred) pending = 1;
    else {
        CallGameMethod<uint8_t, 0x00610660>(renderer,*target,0,0,0,0,0);
        completed = 1;
    }
    return CallGameMethod<uint8_t, 0x0060e410>(renderer,state);
}
}
