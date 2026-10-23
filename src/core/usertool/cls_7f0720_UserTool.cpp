#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <cstdint>
#include <cstring>

#include "ts3/layout.h"
#include "ts3/usertool/cls_7f0720_UserTool.h"

namespace ts3::usertool {
namespace {

template <class T> T& At(void* p, size_t offset)
{
    return *reinterpret_cast<T*>(static_cast<uint8_t*>(p) + offset);
}

// The game's bucket array contains null-terminated chains. When a chain ends,
// these routines advance to the next non-empty bucket, stopping at bucket[count].
struct Cursor {
    void** bucket;
    void* node;
    void* end;
};
struct CursorResult { void* node; void** bucket; };

Cursor Begin(void* self)
{
    auto** buckets = At<void**>(self, 0x14);
    void** first = buckets;
    void* node = *first;
    while (!node) node = *++first;
    auto* end = buckets[At<uint32_t>(self, 0x18)];
    return {first, node, end};
}

void Advance(Cursor& c)
{
    c.node = At<void*>(c.node, 0x24);
    while (!c.node) c.node = *++c.bucket;
}
bool AdvanceAndContinue(Cursor& c) { Advance(c); return true; }

void Lock(void* self) { EnterCriticalSection(reinterpret_cast<CRITICAL_SECTION*>(static_cast<uint8_t*>(self) + 0x380)); }
void Unlock(void* self) { LeaveCriticalSection(reinterpret_cast<CRITICAL_SECTION*>(static_cast<uint8_t*>(self) + 0x380)); }

} // namespace

// Original 0x007F0720. Retains the repeated recursive acquisition of the
// primary critical section when a matching row is encountered.
void __fastcall ProcessUserToolNode(void* self, void*, uint32_t key)
{
    Lock(self);
    for (Cursor cursor = Begin(self); cursor.node != cursor.end; Advance(cursor)) {
        auto* node = static_cast<uint8_t*>(cursor.node);
        if (At<uint32_t>(node, 0x18) == key) {
            Lock(self);
            const auto manager = CallGameFunction<uintptr_t, 0x0049D3F0>();
            if (manager && !At<uint32_t>(node, 0x08)) {
                auto* secondary = static_cast<uint8_t*>(self) + 0x3A0;
                EnterCriticalSection(reinterpret_cast<CRITICAL_SECTION*>(secondary));
                CallGameMethod<void, 0x0049E660>(reinterpret_cast<void*>(manager), node + 4);
                LeaveCriticalSection(reinterpret_cast<CRITICAL_SECTION*>(secondary));
            }
            Unlock(self);
        }
    }
    Unlock(self);
}

// Original 0x007F07F0.
void __fastcall RemoveUserToolNodes(void* self, void*, uint32_t key)
{
    Lock(self);
    for (Cursor cursor = Begin(self); cursor.node != cursor.end; Advance(cursor)) {
        auto* node = static_cast<uint8_t*>(cursor.node);
        if (At<uint32_t>(node, 0x18) == key)
            CallGameMethod<void, 0x007EFED0>(self, node + 4);
    }
    Unlock(self);
}

// Original 0x007F0FB0.
void __fastcall RefreshUserToolNodes(void* self, void*, void* object)
{
    Lock(self);
    Cursor cursor = Begin(self);
    while (cursor.node != cursor.end) {
        auto* node = static_cast<uint8_t*>(cursor.node);
        if ((At<uint8_t>(node, 0x1C) & 0x10) && At<void*>(node, 0x18) == object) {
            CallGameMethod<void, 0x007EFED0>(self, node + 4);
            auto** table = *reinterpret_cast<void***>(object);
            using Setter = void(__thiscall*)(void*, uint32_t);
            reinterpret_cast<Setter>(table[67])(object, At<uint32_t>(self, 0x3D0));
            reinterpret_cast<Setter>(table[67])(object, At<uint32_t>(self, 0x3D4));
            // The EA helper updates the bucket iterator after the removal. Preserve
            // its result and its output pair exactly; the helper is not yet rewritten.
            CursorResult updated{};
            void* result = CallGameMethod<void*, 0x007F0DB0>(static_cast<uint8_t*>(self) + 0x10,
                                                              &updated, cursor.node, cursor.bucket);
            cursor.node = At<void*>(result, 0);
            cursor.bucket = At<void**>(result, sizeof(void*));
        } else if (AdvanceAndContinue(cursor)) {
        }
    }
    Unlock(self);
}

// Original body at 0x00EC6060; 0x00EC6190 is the 8-byte vtable thunk which
// subtracts 16 from ECX and jumps here.
uint8_t __fastcall ShowIntroScreen(void* self, void*)
{
    auto* bytes = static_cast<uint8_t*>(self);
    void* lot = CallGameFunction<void*, 0x0044C470>();
    if (lot) {
        auto** vtable = *reinterpret_cast<void***>(lot);
        using Open = void(__thiscall*)(void*, void*, uint32_t, uint32_t, uint32_t);
        reinterpret_cast<Open>(vtable[9])(lot, bytes + 0x10, 0x06168BE5, 0, 0);
    }
    const float gain = CallGameFunction<float, 0x00EC5480>(reinterpret_cast<const wchar_t*>(0x010F4004));
    if (!CallGameMethod<uint8_t, 0x00EC5630>(bytes + 0x10,
            reinterpret_cast<const wchar_t*>(0x010F3FEC), 640, 368, gain))
        return 0;

    void* manager = CallGameFunction<void*, 0x0050AB70>();
    if (!manager) return 1;
    void* callback = CallGameFunction<void*, 0x00504E70>(0x10u, 4u,
        reinterpret_cast<const char*>(0x010F3FD0), 0u);
    if (callback) {
        At<uint32_t>(callback, 0) = 0x010F3F38;
        At<uint32_t>(callback, 4) = 0x0101E318;
        At<uint32_t>(callback, 8) = 0;
        At<uint8_t>(callback, 12) = 0;
        // The binary writes the vtable pointer at +4 twice; retain its final value.
        At<uint32_t>(callback, 4) = 0x0101E318;
    }
    CallGameMethod<void, 0x009DC510>(bytes + 0x1C, callback);
    void* child = At<void*>(bytes, 0x1C);
    auto** managerVtable = *reinterpret_cast<void***>(manager);
    using Register = int(__thiscall*)(void*, void*, uintptr_t);
    if (child) {
        auto** childVtable = *reinterpret_cast<void***>(child);
        using GetId = uint32_t(__thiscall*)(void*, uint32_t);
        const uint32_t id = reinterpret_cast<GetId>(childVtable[3])(child, 0xEE3F516E);
        reinterpret_cast<Register>(managerVtable[7])(manager,
            reinterpret_cast<void*>(static_cast<uintptr_t>(0x00EC5AF0)), id);
    } else {
        reinterpret_cast<Register>(managerVtable[7])(manager,
            reinterpret_cast<void*>(static_cast<uintptr_t>(0x00EC5AF0)), 0);
    }
    return 1;
}

} // namespace ts3::usertool

extern "C" void __fastcall TS3UserToolProcess(void* self, void* edx, uint32_t key)
{ ts3::usertool::ProcessUserToolNode(self, edx, key); }
extern "C" void __fastcall TS3UserToolRemove(void* self, void* edx, uint32_t key)
{ ts3::usertool::RemoveUserToolNodes(self, edx, key); }
extern "C" void __fastcall TS3UserToolRefresh(void* self, void* edx, void* object)
{ ts3::usertool::RefreshUserToolNodes(self, edx, object); }
