// Reimplementation of TS3W.exe 0x004E38C0, 0x004E3C50, 0x004E3CB0 and 0x00F83E60.
#include "ts3/debug/clu_4e38c0_Debug.h"

namespace ts3::debug {
namespace {

constexpr uint32_t kLockCtor = 0x004E17E0;  // clu_4e17e0_ResourceSystem; still original
constexpr uint32_t kGameAtexit = 0x00F7E9A1;

template <uint32_t Address, class T>
T* GameGlobal()
{
#if defined(_M_IX86)
    return reinterpret_cast<T*>(static_cast<uintptr_t>(Address));
#else
    static T fallback{};
    return &fallback;
#endif
}

}  // namespace

// TS3W.exe 0x004E3C50. Every field write is unconditional; the lock object is constructed
// by the still-original dependency at 0x004E17E0.
TempTraceHelperMap* TempTraceHelperMap::Construct()
{
    CallGameMethod<void, kLockCtor>(lock, static_cast<const void*>(nullptr), true);
    field34 = 0x0114C900;
    field38 = 1;
    field3C = 0;
    field40 = 0x3F800000;
    field44 = 0x40000000;
    field48 = 0;
    name4C = 0x00FB8AF8;
    flag50 = 1;
    return this;
}

// TS3W.exe 0x004E38C0. Keep its non-atomic one-time guard and register the game's dtor
// thunk in the game's CRT exit table.
uint32_t* InitLogTraceHelpers()
{
    uint32_t* const guard = GameGlobal<0x011CB7E8, uint32_t>();
    uint32_t* const fields = GameGlobal<0x011CB7A8, uint32_t>();
    if ((*guard & 1u) == 0) {
        *guard |= 1u;
        fields[0] = 0;
        fields[1] = 0;
        fields[2] = 0;
        fields[3] = 0x00FB8A70;
    CallGameMethod<void, kLockCtor>(GameGlobal<0x011CB7B8, uint8_t>(), static_cast<const void*>(nullptr), true);
        CallGameFunction<int, kGameAtexit>(reinterpret_cast<void*>(0x00F90CD0));
    }
    return fields;
}

// TS3W.exe 0x004E3CB0.
void* InitAssertTraceHelpers()
{
    uint32_t* const guard = GameGlobal<0x011CB840, uint32_t>();
    uint32_t* const fields = GameGlobal<0x011CB7F0, uint32_t>();
    if ((*guard & 1u) == 0) {
        *guard |= 1u;
        fields[1] = 0x0114C900;
        fields[2] = 1;
        fields[3] = 0;
        fields[4] = 0x3F800000;
        fields[5] = 0x40000000;
        fields[6] = 0;
        fields[7] = 0x00FB8B1C;
        CallGameMethod<void, kLockCtor>(GameGlobal<0x011CB810, uint8_t>(), static_cast<const void*>(nullptr), true);
        CallGameFunction<int, kGameAtexit>(reinterpret_cast<void*>(0x00F90D40));
    }
    return fields;
}

// TS3W.exe 0x00F83E60. The constructor sets +0x50, then this thunk clears that byte and
// registers its separate cleanup handler. The atexit return is the thunk's EAX result.
int InitTempTraceHelperMap()
{
    auto* const object = GameGlobal<0x011D3F20, TempTraceHelperMap>();
    object->Construct();
    GameGlobal<0x011D3F70, uint8_t>()[0] = 0;
    return CallGameFunction<int, kGameAtexit>(reinterpret_cast<void*>(0x00F926C0));
}

}  // namespace ts3::debug
