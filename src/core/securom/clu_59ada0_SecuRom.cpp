// Unit clu_59ada0_SecuRom (SecuRom): the LocalizedStringService getter, its keyed
// lookup and the five SIGS "FailToServer" job reporters. See the header for the shape
// the reporters share and for the EA quirks kept here.
#include "ts3/securom/clu_59ada0_SecuRom.h"

namespace ts3::securom {
namespace {

// The game's own string data these functions hand to the game's own functions: TS3W.exe
// addresses, never dereferenced by this unit. Only the x86 build can reach them (every
// consumer below goes through the bridge, which aborts on x64).
constexpr uintptr_t kFailToServerKey = 0x0102ECA8u;    // L"SecuRom/ErrorMessage:FailToServer"
constexpr uintptr_t kParametersFormat = 0x00FB9684u;   // the miss path's format string
constexpr uintptr_t kMsgPoolName = 0x0102EB44u;        // "SIGS/pMsg"
constexpr uintptr_t kPassword = 0x00F97F7Au;           // szPassword (an empty string)
constexpr uintptr_t kTagFsiParams = 0x0102EC84u;       // "SIGS/DownloadFSIParamsJob/errString"
constexpr uintptr_t kTagSimPoints = 0x0102EF1Cu;       // "SIGS/DownloadStoreSimPointsJob/errString"
constexpr uintptr_t kTagPng = 0x0102EF98u;             // "SIGS/DownloadPngJob/errString"
constexpr uintptr_t kTagIgsPurchase = 0x0102F03Cu;     // "SIGS/IgsPurchaseJob/errString"

// The service-pointer global the getter returns (0x011CE2D0).
constexpr uintptr_t kServiceGlobal = 0x011CE2D0u;

// Message ids of the SIGS dispatcher's slot 7 (the value it matches the message type on).
constexpr uint32_t kMsgIdDownloadParams = 0x526701D1u;  // FSI params and StoreSimPoints
constexpr uint32_t kMsgIdDownloadPng = 0x49AFD6A0u;
constexpr uint32_t kMsgIdIgsPurchase = 0x0CCFB69Du;

template <class T>
T* At(uintptr_t address)
{
    return reinterpret_cast<T*>(static_cast<uintptr_t>(address));
}

}  // namespace

SecuRomString SecuRomString::Empty(const void* tag)
{
    SecuRomString message;
    message.begin = At<void>(kStaticEmptyString);
    message.cursor = message.begin;
    message.capacityEnd = At<void>(kStaticEmptyStringEnd);
    message.tag = tag;
    return message;
}

bool SecuRomString::IsHeapBuffer() const
{
    // the original instruction sequence:
    // pointer-width unsigned difference with the low bit cleared, compared SIGNED - so a
    // capacityEnd below begin wraps negative and is not freed.
    const uintptr_t span =
        reinterpret_cast<uintptr_t>(capacityEnd) - reinterpret_cast<uintptr_t>(begin);
    return static_cast<intptr_t>(span & ~static_cast<uintptr_t>(1)) > 2;
}

void* SecuRomString::Release()
{
    void* result = begin;
    if (IsHeapBuffer() && begin != nullptr)
        return CallGameFunction<void*, 0x004E3F70>(begin);
    return result;
}

LocalizedStringService* __cdecl GetLocalizedStringService0059ADA0()
{
#if defined(_M_IX86)
    return *At<LocalizedStringService*>(kServiceGlobal);
#else
    return nullptr;
#endif
}

uint8_t LocalizedStringService::GetErrorString0059CD10(const wchar_t* key,
                                                      SecuRomString* out) const
{
    // 0x007CFE40 is the game's FNV-64 over the lowercased wide key (cdecl, EDX:EAX).
    const uint64_t hash = CallGameFunction<uint64_t, 0x007CFE40>(key);
    if (CallGameMethod<uint8_t, 0x0059CB00>(
            this, static_cast<uint32_t>(hash & 0xFFFFFFFFu), static_cast<uint32_t>(hash >> 32), out))
        return 1;
    // The miss path: 0x0040E920 clears the string (its cursor = begin) and reports.
    CallGameFunction<void, 0x0040E920>(out, At<const void>(kParametersFormat));
    return 0;
}

int32_t __cdecl Forward59C5B000743790(uint32_t a1, int32_t a2)
{
    return CallGameMethod<int32_t, 0x0059C5B0>(GetLocalizedStringService0059ADA0(), a1, a2);
}

void* __cdecl ReportFsiParamsFailToServer0088ED80()
{
#if defined(_M_IX86)
    // The original presets these before anything else, so they are written even when the
    // lookup misses and no message is sent.
    *At<uint32_t>(0x01169378u) = 100;
    *At<uint32_t>(0x0116937Cu) = 5;
    *At<uint32_t>(0x01169380u) = 1000;
    *At<uint32_t>(0x01169384u) = 20;
    *At<uint8_t>(0x011D985Cu) = 1;
#endif
    // No null check on the service: with the global unset the original faults inside
    // 0x0059CB00, and so does this.
    LocalizedStringService* service = GetLocalizedStringService0059ADA0();
    SecuRomString message = SecuRomString::Empty(At<const void>(kTagFsiParams));
    if (service->GetErrorString0059CD10(At<const wchar_t>(kFailToServerKey), &message)) {
        void* dispatcher = CallGameFunction<void*, 0x007692C0>();
        if (dispatcher != nullptr) {
            void* block = CallGameFunction<void*, 0x004E3F90>(
                44, At<const char>(kMsgPoolName), 0, 0, 0, 0);
            void* msg = block != nullptr
                            ? CallGameMethod<void*, 0x0088ED20>(
                                  block, 29, message.begin, At<const char>(kPassword))
                            : nullptr;
            if (msg != nullptr)
                CallGameVirtual<void, 1>(msg);  // AddRef
            CallGameVirtual<void, 7>(dispatcher, kMsgIdDownloadParams, msg, nullptr, nullptr);
            if (msg != nullptr)
                CallGameVirtual<void, 2>(msg);  // Release
        }
    }
    return message.Release();
}

void* __cdecl ReportStoreSimPointsFailToServer0088F720()
{
    LocalizedStringService* service = GetLocalizedStringService0059ADA0();
    SecuRomString message = SecuRomString::Empty(At<const void>(kTagSimPoints));
    if (service->GetErrorString0059CD10(At<const wchar_t>(kFailToServerKey), &message)) {
        void* dispatcher = CallGameFunction<void*, 0x007692C0>();
        if (dispatcher != nullptr) {
            void* block = CallGameFunction<void*, 0x004E3F90>(
                44, At<const char>(kMsgPoolName), 0, 0, 0, 0);
            void* msg = block != nullptr
                            ? CallGameMethod<void*, 0x0088ED20>(
                                  block, 38, message.begin, At<const char>(kPassword))
                            : nullptr;
            if (msg != nullptr)
                CallGameVirtual<void, 1>(msg);
            CallGameVirtual<void, 7>(dispatcher, kMsgIdDownloadParams, msg, nullptr, nullptr);
            if (msg != nullptr)
                CallGameVirtual<void, 2>(msg);
        }
    }
    return message.Release();
}

void* __cdecl ReportPngDownloadFailToServer0088F940()
{
    LocalizedStringService* service = GetLocalizedStringService0059ADA0();
    SecuRomString message = SecuRomString::Empty(At<const void>(kTagPng));
    if (service->GetErrorString0059CD10(At<const wchar_t>(kFailToServerKey), &message)) {
        void* dispatcher = CallGameFunction<void*, 0x007692C0>();
        if (dispatcher != nullptr) {
            void* block = CallGameFunction<void*, 0x004E3F90>(
                28, At<const char>(kMsgPoolName), 0, 0, 0, 0);
            void* msg = block != nullptr
                            ? CallGameMethod<void*, 0x00884320>(block, 27, message.begin)
                            : nullptr;
            if (msg != nullptr)
                CallGameVirtual<void, 1>(msg);
            CallGameVirtual<void, 7>(dispatcher, kMsgIdDownloadPng, msg, nullptr, nullptr);
            if (msg != nullptr)
                CallGameVirtual<void, 2>(msg);
        }
    }
    return message.Release();
}

void* IgsPurchaseJob::ReportFailToServer0088FCE0()
{
    LocalizedStringService* service = GetLocalizedStringService0059ADA0();
    SecuRomString message = SecuRomString::Empty(At<const void>(kTagIgsPurchase));
    if (service->GetErrorString0059CD10(At<const wchar_t>(kFailToServerKey), &message)) {
        void* dispatcher = CallGameFunction<void*, 0x007692C0>();
        if (dispatcher != nullptr) {
            void* block = CallGameFunction<void*, 0x004E3F90>(
                48, At<const char>(kMsgPoolName), 0, 0, 0, 0);
            void* msg = block != nullptr
                            ? CallGameMethod<void*, 0x0088FC10>(block, 25, message.begin,
                                                                d18, 4)
                            : nullptr;
            if (msg != nullptr)
                CallGameVirtual<void, 1>(msg);
            CallGameVirtual<void, 7>(dispatcher, kMsgIdIgsPurchase, msg, nullptr, nullptr);
            if (msg != nullptr)
                CallGameVirtual<void, 2>(msg);
        }
    }
    return message.Release();
}

void* IgsPurchaseJob::ReportFailToServerTo00890A30(uint32_t dispatcher)
{
    LocalizedStringService* service = GetLocalizedStringService0059ADA0();
    SecuRomString message = SecuRomString::Empty(At<const void>(kTagIgsPurchase));
    // EA quirk kept: the lookup's AL is never tested - 0x0088FDB0 is called on both paths
    // (and it gates on the dispatcher argument itself).
    service->GetErrorString0059CD10(At<const wchar_t>(kFailToServerKey), &message);
    // 0x0088FDB0 takes (dispatcher, &string, 6) and reads the string's `begin` field from
    // [arg2] (arg2 is the struct base, not a cursor pointer), so `message` itself is passed.
    CallGameMethod<void, 0x0088FDB0>(this, dispatcher, &message, 6);
    return message.Release();
}

}  // namespace ts3::securom
