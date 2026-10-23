#include "ts3/lot/clu_60ef20_Lot.h"

#include "ts3/layout.h"

#include <cstddef>
#include <cstdint>
#include <cstring>

namespace ts3::lot {
namespace {

struct LotLayout {
    uint8_t unknown00[0x0C];
    void* interfaceObject;
    uint8_t unknown10[0xD0];
    int32_t cachedValue;
};
TS3_X86_OFFSET(LotLayout, interfaceObject, 0x0C);
TS3_X86_OFFSET(LotLayout, cachedValue, 0xE0);
TS3_X86_SIZE(LotLayout, 0xE4);

void* ReadPointer(const void* object, size_t offset)
{
    void* value = nullptr;
    std::memcpy(&value, static_cast<const uint8_t*>(object) + offset, sizeof value);
    return value;
}

int32_t ReadI32(const void* object, size_t offset)
{
    int32_t value;
    std::memcpy(&value, static_cast<const uint8_t*>(object) + offset, sizeof value);
    return value;
}

template <size_t Slot, class... Args>
int32_t Invoke(void* object, Args... args)
{
    auto** table = *reinterpret_cast<void***>(object);
    using Method = int32_t(__stdcall*)(void*, Args...);
    return reinterpret_cast<Method>(table[Slot])(object, args...);
}

template <size_t Slot, class... Args>
void InvokeVoid(void* object, Args... args)
{
    auto** table = *reinterpret_cast<void***>(object);
    using Method = void(__stdcall*)(void*, Args...);
    reinterpret_cast<Method>(table[Slot])(object, args...);
}

}  // namespace

// TS3W.exe 0x0060EF20. The game forwards two values to virtual slot 37, then
// returns virtual slot 48's result using the interface and cached lot value.
int32_t __fastcall UpdateLot(void* self, void*, void* argument, const void* record)
{
    void* interfaceObject = ReadPointer(self, 0x0C);
    const uint32_t value = record ? static_cast<uint32_t>(ReadI32(record, 0x24)) : 0;
    InvokeVoid<37>(interfaceObject, argument, value);
    const uint32_t cached = static_cast<uint32_t>(ReadI32(self, 0xE0)) + 4;
    return Invoke<48>(interfaceObject, cached);
}

// TS3W.exe 0x0060EF70. The null path deliberately forwards zero to virtual slot 39.
int32_t __fastcall QueryLot(void* self, void*, const void* record)
{
    void* interfaceObject = ReadPointer(self, 0x0C);
    const uint32_t value = record ? static_cast<uint32_t>(ReadI32(record, 0x24)) : 0;
    return Invoke<39>(interfaceObject, value);
}

}  // namespace ts3::lot
