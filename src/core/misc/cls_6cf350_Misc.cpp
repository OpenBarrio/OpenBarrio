#include "ts3/misc/cls_6cf350_Misc.h"

#include <cstddef>
#include <cstring>
#include <initializer_list>

namespace ts3::misc {
namespace {
uint32_t Load(const void* p, size_t off) { uint32_t v; std::memcpy(&v, static_cast<const uint8_t*>(p) + off, 4); return v; }
void Store(void* p, size_t off, uint32_t v) { std::memcpy(static_cast<uint8_t*>(p) + off, &v, 4); }
void* GamePointer(uint32_t p) { return reinterpret_cast<void*>(static_cast<uintptr_t>(p)); }
void* Address(uint32_t p) { return reinterpret_cast<void*>(static_cast<uintptr_t>(p)); }
}

// TS3W 0x006CF350. Preserve the helper's EAX result as the method result.
int32_t MiscRenderer::Reset()
{
    Store(this, 0x2c, Load(this, 0x2c) + 1);
    Store(this, 0x24, 0);
    Store(this, 0x28, 0);
    ts3::CallGameMethod<void, 0x0061A220>(GamePointer(Load(this, 0x10)));
    const int32_t result = ts3::CallGameMethod<int32_t, 0x00616E30>(GamePointer(Load(this, 0x14)));
    this->bytes[0x4c] = 1;
    return result;
}

// TS3W 0x006CF380. This keeps the original byte-sized tests of both helper results.
int32_t MiscRenderer::Update()
{
    const uint32_t count = Load(this, 0x18) * Load(this, 0x1c);
    const int32_t poolReady = ts3::CallGameMethod<int32_t, 0x0061A1C0>(
        GamePointer(Load(this, 0x10)), count, 1,
        reinterpret_cast<const char*>(static_cast<uintptr_t>(0x00FF5F10)));
    const int32_t rendererReady = ts3::CallGameMethod<int32_t, 0x00616DF0>(
        GamePointer(Load(this, 0x14)), GamePointer(Load(this, 0x20)), 0, 1);
    if (static_cast<uint8_t>(poolReady)) {
        if (static_cast<uint8_t>(rendererReady)) {
            this->bytes[0x4c] = 0;
            return rendererReady;
        }
        return ts3::CallGameMethod<int32_t, 0x0061A220>(GamePointer(Load(this, 0x10)));
    }
    if (static_cast<uint8_t>(rendererReady))
        return ts3::CallGameMethod<int32_t, 0x00616E30>(GamePointer(Load(this, 0x14)));
    return rendererReady;
}

// TS3W 0x006CF860. Release order and clear-after-release stores match the original.
int32_t MiscRenderer::Destroy()
{
    Store(this, 0, 0x00FF5FA4);
    int32_t result = 0;
    void* manager = ts3::CallGameFunction<void*, 0x005A00A0>();
    if (manager)
        result = ts3::CallGameVirtual<int32_t, 18>(manager, Address(0x00FF5F78));
    for (size_t offset : {size_t(0x14), size_t(0x10), size_t(0x0c)}) {
        void* object = GamePointer(Load(this, offset));
        if (object) {
            result = ts3::CallGameVirtual<int32_t, 2>(object);
            Store(this, offset, 0);
        }
    }
    const uint32_t heap = Load(this, 0x3c);
    if (heap)
        result = ts3::CallGameFunction<int32_t, 0x004E3F70>(GamePointer(heap));
    return result;
}

// TS3W 0x006CFCA0. The unusual allocator and manager side effects are retained.
MiscRenderer* MiscRenderer::Construct()
{
    Store(this, 0x34, 60);
    Store(this, 0, 0x00FF5FA4);
    for (size_t offset : {size_t(0x0c), size_t(0x18), size_t(0x1c), size_t(0x20),
                          size_t(0x24), size_t(0x28), size_t(0x3c), size_t(0x40), size_t(0x44)})
        Store(this, offset, 0);
    Store(this, 0x48, 0x00FF5FF0);
    this->bytes[0x4c] = 0;
    // 0x00AEF830 is the embedded vector's resize method (this = this+0x3c, count=2).
    ts3::CallGameMethod<void, 0x00AEF830>(this->bytes + 0x3c, (60 + 31) >> 5);

    void* manager = ts3::CallGameFunction<void*, 0x005A00A0>();
    if (manager)
        ts3::CallGameVirtual<void, 6>(manager, Address(0x00FF5FDC), 4000);
    manager = ts3::CallGameFunction<void*, 0x005A00A0>();
    if (manager)
        ts3::CallGameVirtual<void, 13>(manager, reinterpret_cast<uint32_t*>(this->bytes + 0x34),
                                      Address(0x00FF5F78), 1, 60, 1);
    manager = ts3::CallGameFunction<void*, 0x005A00A0>();
    if (manager)
        ts3::CallGameVirtual<void, 6>(manager, nullptr, 0x7fffffff);
    return this;
}

} // namespace ts3::misc

// Harness replacement entry points preserve the original x86 thiscall ABI.
extern "C" int32_t __fastcall TS3MiscRendererReset(void* self, void*)
{ return static_cast<ts3::misc::MiscRenderer*>(self)->Reset(); }
extern "C" int32_t __fastcall TS3MiscRendererUpdate(void* self, void*)
{ return static_cast<ts3::misc::MiscRenderer*>(self)->Update(); }
extern "C" int32_t __fastcall TS3MiscRendererDestroy(void* self, void*)
{ return static_cast<ts3::misc::MiscRenderer*>(self)->Destroy(); }
extern "C" void* __fastcall TS3MiscRendererConstruct(void* self, void*)
{ return static_cast<ts3::misc::MiscRenderer*>(self)->Construct(); }
