// Reimplementation of the CASUtils script-bridge cluster TS3W.exe 0x005A4CB0..0x00774E00.
//
// The bridges share one shape: the handle's high byte must be 0x35, the Scripting
// singleton resolves it through a vtable lookup and the bridge reads/writes one field of
// the returned game object. 0x007692B0 is the singleton getter itself, 0x005E0190 is the
// private cache helper of 0x005A5790, and 0x0076A4B0 / 0x0076CCA0 / 0x00774E00 are the
// non-0x005A* entry points of the unit.
#include "ts3/casutils/clu_5a4cb0_CASUtils.h"

#include "ts3/layout.h"

namespace ts3::casutils {
namespace {

// 0x011D4BBC: the Scripting singleton cell that 0x007692B0 loads. On x86 it is the game's
// executable global; x64 keeps an isolated cell because there is no game image (the
// bridges abort inside CallGameVirtual before they could use it).
void** ScriptingCell()
{
#if defined(_M_IX86)
    return reinterpret_cast<void**>(static_cast<uintptr_t>(0x011D4BBC));
#else
    static void* manager = nullptr;
    return &manager;
#endif
}

// The original's the original instruction sequence handle test.
bool IsScriptHandle(uint32_t handle)
{
    return (handle & 0xFF000000u) == 0x35000000u;
}

// What the original leaves in EAX on the setters' tag-mismatch path: the AND result, not 0.
void* MaskedHandleEax(uint32_t handle)
{
    return reinterpret_cast<void*>(static_cast<uintptr_t>(handle & 0xFF000000u));
}

uint32_t& DwordAt(void* object, uintptr_t offset)
{
    return *reinterpret_cast<uint32_t*>(static_cast<char*>(object) + offset);
}

uint8_t& ByteAt(void* object, uintptr_t offset)
{
    return *reinterpret_cast<uint8_t*>(static_cast<char*>(object) + offset);
}

float& FloatAt(void* object, uintptr_t offset)
{
    return *reinterpret_cast<float*>(static_cast<char*>(object) + offset);
}

// The same EAX value as 0x007692B0, as a pointer for the virtual calls.
void* ScriptingObject()
{
    return *ScriptingCell();
}

// The lookup every bridge except 0x005A65A0 uses: manager vtbl slot 51 (offset 0xCC).
void* ResolveObject51(uint32_t handle)
{
    return ts3::CallGameVirtual<void*, 51>(ScriptingObject(), handle);
}

// The original's the original instruction sequence / the original instruction sequence keep EAX's upper 24 bits, so the byte
// getters return the full EAX: low byte = field, upper bits = object pointer (or the
// masked handle on the bad-tag path). 0 when the lookup misses.
uint32_t ByteEax(uint32_t handle, void* object, uintptr_t offset)
{
    if (!IsScriptHandle(handle))
        return handle & 0xFF000000u;
    if (!object)
        return 0;
    return (static_cast<uint32_t>(reinterpret_cast<uintptr_t>(object)) & 0xFFFFFF00u) |
           ByteAt(object, offset);
}

}  // namespace

// TS3W.exe 0x007692B0. the original instruction sequence: the script manager singleton dword.
uint32_t GetScriptingSingleton7692B0()
{
    return static_cast<uint32_t>(reinterpret_cast<uintptr_t>(*ScriptingCell()));
}

// TS3W.exe 0x005A4CB0.
float SimBuilderSetCurlPixelRadius5A4CB0(uint32_t handle)
{
    float result = 0.0f;
    if (IsScriptHandle(handle)) {
        void* object = ResolveObject51(handle);
        if (object)
            result = FloatAt(object, 0x268);
    }
    return result;
}

// TS3W.exe 0x005A4CF0.
float SimBuilderGetFurMap5A4CF0(uint32_t handle)
{
    float result = 0.0f;
    if (IsScriptHandle(handle)) {
        void* object = ResolveObject51(handle);
        if (object)
            result = FloatAt(object, 0x26C);
    }
    return result;
}

// TS3W.exe 0x005A4D30.
float SimOutfitSetMorphWeightD5A4D30(uint32_t handle)
{
    float result = 0.0f;
    if (IsScriptHandle(handle)) {
        void* object = ResolveObject51(handle);
        if (object)
            result = FloatAt(object, 0x7C);
    }
    return result;
}

// TS3W.exe 0x005A4DE0.
float SimOutfitGetFurMap5A4DE0(uint32_t handle)
{
    float result = 0.0f;
    if (IsScriptHandle(handle)) {
        void* object = ResolveObject51(handle);
        if (object)
            result = FloatAt(object, 0x80);
    }
    return result;
}

// TS3W.exe 0x005A4E20.
void* SimOutfitGetCurlPixelRadius5A4E20(uint32_t handle, float value)
{
    if (!IsScriptHandle(handle))
        return MaskedHandleEax(handle);
    void* object = ResolveObject51(handle);
    if (object)
        FloatAt(object, 0x7C) = value;
    return object;
}

// TS3W.exe 0x005A5120. The lookup result is ignored; AL is 1 on every path, including the
// tag-mismatch path (where EAX's upper bits keep the AND result, but callers read AL).
uint8_t CreatePartSearch5A5120(uint32_t handle)
{
    if (IsScriptHandle(handle))
        (void)ResolveObject51(handle);
    return 1;
}

// TS3W.exe 0x005E0190, __thiscall (this in ecx). Private helper of 0x005A5790; IDA
// bundles it into 0x005A5790's chunk, but the body is a separate function.
uint32_t GetCachedScriptObject5E0190(void* self)
{
    uint32_t result = 0;
    if (DwordAt(self, 0x1EC) != 0) {
        if (DwordAt(self, 0x1F0) == 0) {
            void* created = ts3::CallGameVirtual<void*, 50>(ScriptingObject(),
                                                            DwordAt(self, 0x1EC), 0x35u);
            DwordAt(self, 0x1F0) = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(created));
        }
        result = DwordAt(self, 0x1F0);
    }
    return result;
}

// TS3W.exe 0x005A5790. Tail-jumps to 0x005E0190 with the resolved object.
uint32_t SimBuilderResetAnimation5A5790(uint32_t handle)
{
    uint32_t result = 0;
    if (IsScriptHandle(handle)) {
        void* object = ResolveObject51(handle);
        if (object)
            result = GetCachedScriptObject5E0190(object);
    }
    return result;
}

// TS3W.exe 0x005A57D0.
uint32_t SimBuilderGetModelHandle5A57D0(uint32_t handle)
{
    uint32_t result = 0;
    if (IsScriptHandle(handle)) {
        void* object = ResolveObject51(handle);
        if (object)
            result = DwordAt(object, 0x64);
    }
    return result;
}

// TS3W.exe 0x005A5960.
float SimBuilderSetMorphWeightD5A5960(uint32_t handle)
{
    float result = 0.0f;
    if (IsScriptHandle(handle)) {
        void* object = ResolveObject51(handle);
        if (object)
            result = FloatAt(object, 0x50);
    }
    return result;
}

// TS3W.exe 0x005A59A0.
float SimBuilderGetMorphWeightA5A59A0(uint32_t handle)
{
    float result = 0.0f;
    if (IsScriptHandle(handle)) {
        void* object = ResolveObject51(handle);
        if (object)
            result = FloatAt(object, 0x54);
    }
    return result;
}

// TS3W.exe 0x005A59E0.
float SimBuilderGetMorphWeightB5A59E0(uint32_t handle)
{
    float result = 0.0f;
    if (IsScriptHandle(handle)) {
        void* object = ResolveObject51(handle);
        if (object)
            result = FloatAt(object, 0x58);
    }
    return result;
}

// TS3W.exe 0x005A5A20.
float SimBuilderGetMorphWeightC5A5A20(uint32_t handle)
{
    float result = 0.0f;
    if (IsScriptHandle(handle)) {
        void* object = ResolveObject51(handle);
        if (object)
            result = FloatAt(object, 0x5C);
    }
    return result;
}

// TS3W.exe 0x005A5EA0.
uint32_t SimBuilderSetSelectionIndex5A5EA0(uint32_t handle)
{
    return ByteEax(handle, IsScriptHandle(handle) ? ResolveObject51(handle) : nullptr, 0x69);
}

// TS3W.exe 0x005A5F30.
void* SimBuilderSetUseCompression5A5F30(uint32_t handle, uint8_t value)
{
    if (!IsScriptHandle(handle))
        return MaskedHandleEax(handle);
    void* object = ResolveObject51(handle);
    if (object)
        ByteAt(object, 0x6B) = value;
    return object;
}

// TS3W.exe 0x005A5F70.
uint32_t SimBuilderSetCacheAtlases5A5F70(uint32_t handle)
{
    return ByteEax(handle, IsScriptHandle(handle) ? ResolveObject51(handle) : nullptr, 0x6A);
}

// TS3W.exe 0x005A5FB0.
void* SimBuilderGetWaitForTextures5A5FB0(uint32_t handle, uint8_t value)
{
    if (!IsScriptHandle(handle))
        return MaskedHandleEax(handle);
    void* object = ResolveObject51(handle);
    if (object)
        ByteAt(object, 0x6A) = value;
    return object;
}

// TS3W.exe 0x005A6030.
uint32_t SimBuilderGetIsDirty5A6030(uint32_t handle)
{
    return ByteEax(handle, IsScriptHandle(handle) ? ResolveObject51(handle) : nullptr, 0x204);
}

// TS3W.exe 0x005A6220. Tail-calls the game method 0x005E10B0 (game unit clu_56dc40_CAS,
// not rewritten) with the resolved object in ECX and no stack arguments. The mismatch and
// lookup-miss paths share the epilogue, so a bad tag returns the masked handle and a miss
// returns 0 (the AND result is EAX).
uint32_t SimBuilderGetPickDataFromBodyTypeAndRegion5A6220(uint32_t handle)
{
    uint32_t result = 0;
    if (IsScriptHandle(handle)) {
        void* object = ResolveObject51(handle);
        if (object)
            result = ts3::CallGameMethod<uint32_t, 0x005E10B0>(object);
    } else {
        result = handle & 0xFF000000u;
    }
    return result;
}

// TS3W.exe 0x005A62B0.
uint32_t FacialBlendGetName5A62B0(uint32_t handle)
{
    uint32_t result = 0;
    if (IsScriptHandle(handle)) {
        void* object = ResolveObject51(handle);
        if (object)
            result = DwordAt(object, 0x38);
    }
    return result;
}

// TS3W.exe 0x005A63B0.
uint32_t BlendUnitGetLocalizationKey5A63B0(uint32_t handle)
{
    return ByteEax(handle, IsScriptHandle(handle) ? ResolveObject51(handle) : nullptr, 0x50);
}

// TS3W.exe 0x005A63F0.
uint32_t BlendUnitIsBidirectional5A63F0(uint32_t handle)
{
    uint32_t result = 0;
    if (IsScriptHandle(handle)) {
        void* object = ResolveObject51(handle);
        if (object)
            result = DwordAt(object, 0x60);
    }
    return result;
}

// TS3W.exe 0x005A64A0.
uint32_t BlendUnitGetBlendKeyAtIndex5A64A0(uint32_t handle)
{
    uint32_t result = 0;
    if (IsScriptHandle(handle)) {
        void* object = ResolveObject51(handle);
        if (object)
            result = DwordAt(object, 0x54);
    }
    return result;
}

// TS3W.exe 0x005A64E0.
uint32_t BlendUnitGetRegion5A64E0(uint32_t handle)
{
    uint32_t result = 0;
    if (IsScriptHandle(handle)) {
        void* object = ResolveObject51(handle);
        if (object)
            result = DwordAt(object, 0x58);
    }
    return result;
}

// TS3W.exe 0x005A6520.
uint32_t BlendUnitGetCategory5A6520(uint32_t handle)
{
    uint32_t result = 0;
    if (IsScriptHandle(handle)) {
        void* object = ResolveObject51(handle);
        if (object)
            result = DwordAt(object, 0x5C);
    }
    return result;
}

// TS3W.exe 0x005A6560.
uint32_t CompoundBlendGetAgeGender5A6560(uint32_t handle)
{
    uint32_t result = 0;
    if (IsScriptHandle(handle)) {
        void* object = ResolveObject51(handle);
        if (object)
            result = DwordAt(object, 0x3C);
    }
    return result;
}

// TS3W.exe 0x005A65A0. The only bridge that resolves through vtable slot 54 (offset 0xD8)
// and then tail-calls slot 1 (offset 0x4) of the result. The mismatch and miss paths share
// the epilogue: bad tag returns the masked handle, a miss returns 0.
uint32_t FacialBlendGetRegionFlags5A65A0(uint32_t handle)
{
    uint32_t result = 0;
    if (IsScriptHandle(handle)) {
        void* object = ts3::CallGameVirtual<void*, 54>(ScriptingObject(), handle);
        if (object)
            result = ts3::CallGameVirtual<uint32_t, 1>(object);
    } else {
        result = handle & 0xFF000000u;
    }
    return result;
}

// TS3W.exe 0x005A69D0.
float SimOutfitGetSkinToneKey5A69D0(uint32_t handle)
{
    float result = 0.0f;
    if (IsScriptHandle(handle)) {
        void* object = ResolveObject51(handle);
        if (object)
            result = FloatAt(object, 0x50);
    }
    return result;
}

// TS3W.exe 0x005A6AC0.
float SimOutfitGetRigKey5A6AC0(uint32_t handle)
{
    float result = 0.0f;
    if (IsScriptHandle(handle)) {
        void* object = ResolveObject51(handle);
        if (object)
            result = FloatAt(object, 0x68);
    }
    return result;
}

// TS3W.exe 0x005A6B00.
void* SimOutfitGetMorphWeightD5A6B00(uint32_t handle, float value)
{
    if (!IsScriptHandle(handle))
        return MaskedHandleEax(handle);
    void* object = ResolveObject51(handle);
    if (object)
        FloatAt(object, 0x68) = value;
    return object;
}

// TS3W.exe 0x005A6B40.
float SimOutfitGetMorphWeightA5A6B40(uint32_t handle)
{
    float result = 0.0f;
    if (IsScriptHandle(handle)) {
        void* object = ResolveObject51(handle);
        if (object)
            result = FloatAt(object, 0x6C);
    }
    return result;
}

// TS3W.exe 0x005A6B80.
void* SimOutfitSetMorphWeightA5A6B80(uint32_t handle, float value)
{
    if (!IsScriptHandle(handle))
        return MaskedHandleEax(handle);
    void* object = ResolveObject51(handle);
    if (object)
        FloatAt(object, 0x6C) = value;
    return object;
}

// TS3W.exe 0x005A6BC0.
float SimOutfitGetMorphWeightB5A6BC0(uint32_t handle)
{
    float result = 0.0f;
    if (IsScriptHandle(handle)) {
        void* object = ResolveObject51(handle);
        if (object)
            result = FloatAt(object, 0x70);
    }
    return result;
}

// TS3W.exe 0x005A6C00.
void* SimOutfitSetMorphWeightB5A6C00(uint32_t handle, float value)
{
    if (!IsScriptHandle(handle))
        return MaskedHandleEax(handle);
    void* object = ResolveObject51(handle);
    if (object)
        FloatAt(object, 0x70) = value;
    return object;
}

// TS3W.exe 0x005A6C40.
float SimOutfitGetMorphWeightC5A6C40(uint32_t handle)
{
    float result = 0.0f;
    if (IsScriptHandle(handle)) {
        void* object = ResolveObject51(handle);
        if (object)
            result = FloatAt(object, 0x74);
    }
    return result;
}

// TS3W.exe 0x005A6C80.
void* SimOutfitSetMorphWeightC5A6C80(uint32_t handle, float value)
{
    if (!IsScriptHandle(handle))
        return MaskedHandleEax(handle);
    void* object = ResolveObject51(handle);
    if (object)
        FloatAt(object, 0x74) = value;
    return object;
}

// TS3W.exe 0x0076A4B0, __thiscall (this in ecx, `ret 8`). Calls the singleton getter and
// forces AL to 1; the upper 24 bits of EAX keep the manager pointer, which the original
// also leaves there.
uint32_t __fastcall ScriptingReturnsTrue76A4B0(void*, void*, int32_t, int32_t)
{
    const uintptr_t manager = reinterpret_cast<uintptr_t>(ScriptingObject());
    return static_cast<uint32_t>(manager & 0xFFFFFF00u) | 1u;
}

// TS3W.exe 0x0076CCA0. No null check, like the original.
uint32_t CheckPickData76CCA0()
{
    void* object = ts3::CallGameVirtual<void*, 95>(ScriptingObject());
    return ts3::CallGameVirtual<uint32_t, 7>(object);
}

// TS3W.exe 0x00774E00.
void* CreateAndNotify774E00(void* arg)
{
    void* created = ts3::CallGameVirtual<void*, 50>(ScriptingObject(), arg, 0x60u);
    if (created)
        (void)ts3::CallGameVirtual<void*, 1>(arg);
    return created;
}

}  // namespace ts3::casutils