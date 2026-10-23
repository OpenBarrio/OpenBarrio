#include "ts3/gameutils/clu_799d10_GameUtils.h"

#include "ts3/layout.h"

#include <cstddef>
#include <cstdint>

namespace ts3::gameutils {
namespace {

struct GameUtilsState {
    uint32_t unknown00;
    void* entries;
    uint32_t count;
    uint32_t unknown0C;
    uint16_t state;
    uint16_t mirror;
};
TS3_X86_OFFSET(GameUtilsState, entries, 0x04);
TS3_X86_OFFSET(GameUtilsState, count, 0x08);
TS3_X86_OFFSET(GameUtilsState, state, 0x10);
TS3_X86_OFFSET(GameUtilsState, mirror, 0x12);
TS3_X86_SIZE(GameUtilsState, 0x14);

struct Entry {
    uint32_t unknown00;
    uint16_t state;
    uint8_t reserved[10];
};
static_assert(offsetof(Entry, state) == 4);
static_assert(sizeof(Entry) == 0x10);

}  // namespace

// TS3W.exe 0x00799D10. The input state bits gate propagation to nonzero entry states;
// the 16-bit increment intentionally wraps like the game's word arithmetic.
int32_t __fastcall AdvanceState(void* self, void*)
{
    auto* object = static_cast<GameUtilsState*>(self);
    if ((object->state & 0x7FFEu) == 0) {
        auto* entries = static_cast<Entry*>(object->entries);
        for (uint32_t i = 0; i < object->count; ++i) {
            if (entries[i].state != 0)
                entries[i].state = object->state;
        }
    }
    object->state = static_cast<uint16_t>(object->state + 2);
    object->mirror = object->state;
    return object->state;
}

// TS3W.exe 0x00799D60. Clears only the mirror word at offset 0x12.
void __fastcall ClearMirror(void* self, void*)
{
    static_cast<GameUtilsState*>(self)->mirror = 0;
}

}  // namespace ts3::gameutils
