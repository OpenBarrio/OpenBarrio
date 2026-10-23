#include "ts3/misc/clu_745b70_Misc.h"
#include <cstring>
#if defined(_M_IX86)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#endif
namespace ts3::misc {
namespace {
template<unsigned Address> const char* Tag(const char* fallback) {
#if defined(_M_IX86)
    return reinterpret_cast<const char*>(Address);
#else
    return fallback;
#endif
}
TelemetryManager*& Manager() {
#if defined(_M_IX86)
    return *reinterpret_cast<TelemetryManager**>(0x011D987C);
#else
    static TelemetryManager* manager = nullptr; return manager;
#endif
}
TelemetryManager* EnsureManager() {
    auto*& manager = Manager();
    if (!manager) {
        auto* p = CallGameFunction<TelemetryManager*, 0x004E3F90>(sizeof(TelemetryManager),
            Tag<0x0102F7B4>("Misc/Network/Telemetry/Manager"), 0u, 0u, 0u, 0u);
        manager = p ? p->Init() : nullptr;
    }
    return manager;
}
intptr_t Span(void* end, void* begin) { return static_cast<intptr_t>(reinterpret_cast<uintptr_t>(end) - reinterpret_cast<uintptr_t>(begin)); }
}
// 0x00892E20: five arguments to 892A70 (the decompile omits source).
TelemetryManager* TelemetryManager::CopyQueue(TelemetryBuffer* source) {
    const auto count = Span(source->end, source->begin) / 40;
    CallGameMethod<void, 0x0081F2C0>(this, static_cast<int32_t>(count), &source->tag);
    void* copiedEnd = source;
    CallGameFunction<void, 0x00892A70>(&copiedEnd, source->begin, source->end, queue.begin, source);
    queue.end = copiedEnd;
    return this;
}
// 0x00892E80
TelemetryManager* TelemetryManager::Init() {
    TelemetryBuffer empty{nullptr, nullptr, nullptr, Tag<0x0102F790>("Misc/Network/Telemetry/eventQueue")};
    CopyQueue(&empty);
    path.tag = Tag<0x0102F770>("Misc/Network/Telemetry/filepath");
#if defined(_M_IX86)
    path.begin = path.end = reinterpret_cast<void*>(0x011C7E54);
    path.capacity = reinterpret_cast<void*>(0x011C7E56);
    using Initialize = void (WINAPI*)(CRITICAL_SECTION*);
    (*reinterpret_cast<Initialize*>(0x00F95124))(reinterpret_cast<CRITICAL_SECTION*>(&lock));
#else
    static char16_t emptyString = 0;
    path.begin = path.end = &emptyString; path.capacity = &emptyString + 1;
    AbortNotRewritten("InitializeCriticalSection telemetry");
#endif
    return this;
}
// 0x00893460: retain the destructor-derived return, not the submit result.
uint32_t TelemetryManager::SubmitText(const void* a, const void* b, const char16_t* text) {
    TelemetryBuffer source{nullptr, nullptr, nullptr, Tag<0x0102F824>("Misc/Network/Telemetry/tempStr")};
    CallGameMethod<void, 0x0040B700>(&source, text);
    const auto length = Span(source.end, source.begin) / 2;
    TelemetryBuffer wide{nullptr, nullptr, nullptr, source.tag};
    CallGameMethod<void, 0x00408830>(&wide, static_cast<uint32_t>(length + 1));
    if (length) std::memmove(wide.begin, source.begin, static_cast<size_t>(length) * 2);
    wide.end = static_cast<char16_t*>(wide.begin) + length;
    *static_cast<char16_t*>(wide.end) = 0;
    TelemetryBuffer narrow{};
    auto* converted = CallGameFunction<TelemetryBuffer*, 0x004F39F0>(&narrow, &wide);
    CallGameMethod<int32_t, 0x00893250>(this, a, b, converted->begin);
    if (Span(narrow.capacity, narrow.begin) > 1 && narrow.begin)
        CallGameFunction<uint32_t, 0x004E3F70>(narrow.begin);
    if ((Span(wide.capacity, wide.begin) & ~intptr_t(1)) > 2 && wide.begin)
        CallGameFunction<uint32_t, 0x004E3F70>(wide.begin);
    if (source.begin != source.end) *static_cast<char16_t*>(source.begin) = 0;
    uint32_t result = static_cast<uint32_t>(Span(source.capacity, source.begin)) & ~1u;
    if (static_cast<int32_t>(result) > 2 && source.begin)
        result = CallGameFunction<uint32_t, 0x004E3F70>(source.begin);
    return result;
}
// 0x00893590
int32_t SubmitEvent(const void* a, const void* b, const char* text) {
    return CallGameMethod<int32_t, 0x00893250>(EnsureManager(), a, b, text);
}
// 0x008935E0
uint32_t SubmitWideEvent(const void* a, const void* b, const char16_t* text) {
    return EnsureManager()->SubmitText(a, b, text);
}
// 0x00745B70
uint32_t ForwardTelemetry(uint32_t a, uint32_t b, uint32_t key) {
    auto* service = CallGameFunction<void*, 0x007692B0>();
    if (!service) return 0;
    auto* text = CallGameVirtual<const char16_t*, 22>(service, key);
    return SubmitWideEvent(&a, &b, text);
}
}
