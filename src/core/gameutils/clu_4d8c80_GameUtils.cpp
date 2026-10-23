// Rewritten cluster unit clu_4d8c80_GameUtils. The asm in
// the original disassembly is the source of truth; each function keeps its
// original address in the comment above it.
#include "ts3/gameutils/clu_4d8c80_GameUtils.h"

#include <cstdint>
#include <cstring>

namespace ts3::gameutils {

// The structs are views of the game's objects, checked on x86 where the game runs.
TS3_X86_OFFSET(RendererExtentSource, extentA, 0x2C);
TS3_X86_OFFSET(RendererExtentSource, extentB, 0x30);
TS3_X86_OFFSET(RendererExtentSource, extentC, 0x34);
TS3_X86_SIZE(RendererExtentSource, 0x38);
TS3_X86_OFFSET(LauncherHost, launcherId, 0x3CC);
TS3_X86_SIZE(LauncherHost, 0x3D0);

namespace {

constexpr uintptr_t kGameUtilsSingletonGlobal = 0x011C7980;  // 0x004D8C80 reads it
constexpr unsigned kScriptClassA = 0x01165908;               // 0x007EAEC0 pushes it
constexpr unsigned kScriptClassB = 0x0116590C;               // 0x007EAF10 pushes it
constexpr unsigned kUIManagerGetter = 0x0050AB70;            // global 0x11CD080
constexpr unsigned kRendererGetter = 0x0060E2C0;             // global 0x11CECC8
constexpr unsigned kLauncherHostGetter = 0x007EFE00;         // global 0x11D8C44

// vtable byte offsets the original hard-codes; slot = offset / 4.
constexpr int kSlotRegisterUiManager = 0x04 / 4;  // UIManager::slot1()
constexpr int kSlotCreateLauncher = 0x20 / 4;     // GameUtils::slot8(0x034B248B, ...)
constexpr int kSlotAddLauncher = 0xDC / 4;        // UIManager::slot55(launcher)
constexpr int kSlotShowLauncher = 0xF0 / 4;       // UIManager::slot60(launcher)
constexpr int kSlotSetRect = 0x64 / 4;            // launcher::slot25(const float[4])
constexpr int kSlotSetId = 0x54 / 4;              // launcher::slot21(0x034B248B)
constexpr int kSlotAttachHost = 0x108 / 4;        // launcher::slot66(hostId)

constexpr uint32_t kLauncherIdValue = 0x034B248B;  // the window/control id, pushed thrice
constexpr uint32_t kLauncherArg2 = 0xEEEE8218;     // second argument of the create call
constexpr int kRectFloats = 4;                     // the float[4] handed to the launcher

int32_t ReadI32(const void* object, size_t offset)
{
    int32_t value = 0;
    std::memcpy(&value, static_cast<const uint8_t*>(object) + offset, sizeof value);
    return value;
}

void WriteI32(void* out, int32_t value)
{
    // The original stores through the caller's pointer unconditionally (no null check).
    std::memcpy(out, &value, sizeof value);
}

}  // namespace

// 0x004D8C80 - the original instruction sequence. The global is uninitialised (0xFFFFFFFF) in the
// image and holds the singleton pointer once the engine has started. The read is written
// without an x64 branch on purpose: a dead `#else` would leave mutation sites that never
// compile, and only the autotest/exehost runner calls this - the game itself is x86.
void* GetGameUtilsInstance()
{
    void* instance = nullptr;
    std::memcpy(&instance, reinterpret_cast<const void*>(kGameUtilsSingletonGlobal), sizeof instance);
    return instance;
}

// 0x0060E6A0 - __thiscall, the original instruction sequence. Read/store pairs in the original's order, so an out
// pointer that aliases this object reads exactly like the original does.
int32_t RendererExtentSource::GetExtent(void* outA, void* outB, void* outC)
{
    const int32_t a = ReadI32(this, 0x2C);
    WriteI32(outA, a);
    const int32_t b = ReadI32(this, 0x30);
    WriteI32(outB, b);
    const int32_t c = ReadI32(this, 0x34);  // still in EAX when the original returns
    WriteI32(outC, c);
    return c;
}

// 0x007EFE60 - __thiscall, plain ret.
int32_t LauncherHost::GetLauncherId() const
{
    return ReadI32(this, 0x3CC);
}

namespace {

// The two thunks differ only in the descriptor they push.
void* RegisterScriptClass(uint32_t descriptor)
{
    void* gameUtils = GetGameUtilsInstance();
    // the original instruction sequence.
    // No null check: the original faults when the singleton has not been created yet.
    return ts3::CallGameVirtual<void*, kSlotRegisterUiManager>(
        gameUtils, reinterpret_cast<void*>(static_cast<uintptr_t>(descriptor)),
        static_cast<int32_t>(0));
}

}  // namespace

// 0x007EAEC0.
void* RegisterScriptClassA()
{
    return RegisterScriptClass(kScriptClassA);
}

// 0x007EAF10.
void* RegisterScriptClassB()
{
    return RegisterScriptClass(kScriptClassB);
}

// 0x00EC1600 - "GameUtils_StartLauncherImpl". Every callee address is called through the
// bridge, so on x64 the build traps at the first unrewritten function instead of guessing.
// The original's two the original instruction sequence early exits both leave EAX = 0, so the body is a
// single exit over `result` - a nested early `return 0` would only add mutation sites that
// `return {}` leaves identical.
uint32_t StartLauncherImpl()
{
    void* uiManager = ts3::CallGameFunction<void*, kUIManagerGetter>();          // 0x0050AB70
    void* uiManagerTarget = ts3::CallGameVirtual<void*, kSlotRegisterUiManager>(uiManager);  // +0x04

    void* gameUtils = GetGameUtilsInstance();
    void* launcher = ts3::CallGameVirtual<void*, kSlotCreateLauncher>(
        gameUtils, kLauncherIdValue, kLauncherArg2, 0, 0);                       // +0x20

    uint32_t result = 0;  // what EAX holds on the the original instruction sequence exits below
    if (launcher) {
        ts3::CallGameVirtual<void, kSlotAddLauncher>(uiManagerTarget, launcher);  // +0xDC
        ts3::CallGameVirtual<void, kSlotShowLauncher>(uiManagerTarget, launcher); // +0xF0

        void* renderer = ts3::CallGameFunction<void*, kRendererGetter>();         // 0x0060E2C0
        int32_t extentA = {};
        int32_t extentB = {};
        int32_t extentC = {};  // written by the callee, never used (the original's v11)
        static_cast<RendererExtentSource*>(renderer)->GetExtent(&extentA, &extentB, &extentC);

        // The original zeroes the first two floats and converts the +0x2C and +0x30 dwords
        // as signed integers; the +0x34 read is dead.
        float rect[kRectFloats] = {0.0f, 0.0f, static_cast<float>(extentA),
                                   static_cast<float>(extentB)};
        ts3::CallGameVirtual<void, kSlotSetRect>(launcher, rect);                 // +0x64
        ts3::CallGameVirtual<void, kSlotSetId>(launcher, kLauncherIdValue);       // +0x54

        void* host = ts3::CallGameFunction<void*, kLauncherHostGetter>();         // 0x007EFE00
        if (host) {
            const int32_t hostId = static_cast<LauncherHost*>(host)->GetLauncherId();
            result = ts3::CallGameVirtual<uint32_t, kSlotAttachHost>(launcher, hostId);  // +0x108
        }
    }
    return result;
}

}  // namespace ts3::gameutils