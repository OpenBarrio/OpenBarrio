#include "ts3/sacs/clu_582090_SACS.h"

#include "ts3/layout.h"

#include <cstddef>
#include <cstdint>

namespace ts3::sacs {
namespace {

struct Pair { void* key; void* object; };

struct PairVector {
    Pair* begin;
    Pair* end;
    Pair* capacity;
};
TS3_X86_OFFSET(PairVector, begin, 0x00);
TS3_X86_OFFSET(PairVector, end, 0x04);
TS3_X86_OFFSET(PairVector, capacity, 0x08);
TS3_X86_SIZE(PairVector, 0x0C);

struct Registry {
    uint8_t unknown00[8];
    PairVector pairs;
};
TS3_X86_OFFSET(Registry, pairs, 0x08);

struct RefCounted {
    void** vtable;
    int32_t references;
};
TS3_X86_OFFSET(RefCounted, references, 0x04);

struct EntryOwner {
    uint8_t unknown00[0x10];
    void* source;
    uint32_t value20;
    uint32_t value24;
    float value28;
    float value32;
    uint8_t flag36;
    uint8_t flag37;
};
TS3_X86_OFFSET(EntryOwner, source, 0x10);
TS3_X86_OFFSET(EntryOwner, value20, 0x14);
TS3_X86_OFFSET(EntryOwner, value24, 0x18);
TS3_X86_OFFSET(EntryOwner, value28, 0x1C);
TS3_X86_OFFSET(EntryOwner, value32, 0x20);
TS3_X86_OFFSET(EntryOwner, flag36, 0x24);
TS3_X86_OFFSET(EntryOwner, flag37, 0x25);

}  // namespace

// TS3W.exe 0x00582090. Duplicate keys and null inputs do not change refcounts. The
// game's vector-growth helper is still called through the bridge when storage is full.
uint8_t __fastcall AddUniqueObject(void* self, void*, void* key, void* object)
{
    uint8_t result = 0;
    auto* registry = static_cast<Registry*>(self);
    if (key && object) {
        bool found = false;
        for (Pair* it = registry->pairs.begin; it != registry->pairs.end; ++it) {
            if (it->key == key) {
                found = true;
                break;
            }
        }
        if (!found) {
            ts3::CallGameVirtual<void, 1>(static_cast<RefCounted*>(object));
            const Pair pair{key, object};
            if (registry->pairs.end < registry->pairs.capacity) {
                *registry->pairs.end++ = pair;
            } else {
                ts3::CallGameMethod<void, 0x006202E0>(&registry->pairs, registry->pairs.end, &pair);
            }
            result = 1;
        }
    }
    return result;
}

// TS3W.exe 0x00582320. A non-null current source redirects to the game's existing
// 0x00582270 path; otherwise this updates the source reference and cached fields.
uint8_t __fastcall UpdateEntry(void* self, void*, void* source, uint32_t value20, uint32_t value24,
                               float value28, float value32, uint8_t flag36, uint8_t flag37)
{
    auto* owner = static_cast<EntryOwner*>(self);
    if (owner->source) {
        return ts3::CallGameMethod<uint8_t, 0x00582270>(self, source, value20, value24, value28, value32,
                                                        flag36, uint32_t{0});
    }

    auto* oldSource = static_cast<RefCounted*>(owner->source);
    if (source != owner->source) {
        if (source)
            ++static_cast<RefCounted*>(source)->references;
        owner->source = source;
        if (oldSource) {
            const int32_t remaining = --oldSource->references;
            if (remaining == 0) {
                oldSource->references = 1;
                ts3::CallGameVirtual<void, 0>(oldSource, 1);
            }
        }
    }

    owner->value24 = value24;
    owner->value28 = value28;
    owner->value20 = value20;
    owner->value32 = value32;
    owner->flag36 = flag36;
    owner->flag37 = flag37;
    return flag37;
}

}  // namespace ts3::sacs
