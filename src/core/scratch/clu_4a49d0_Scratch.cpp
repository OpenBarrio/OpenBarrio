#include "ts3/scratch/clu_4a49d0_Scratch.h"
#include "ts3/scratch/clu_4e3f70_Scratch.h"
#include <ctime>
#include <cstddef>

namespace ts3::scratch {
namespace {
template<unsigned Address, class T> T* GlobalPointer() {
#if defined(_M_IX86)
    return *reinterpret_cast<T**>(static_cast<uintptr_t>(Address));
#else
    AbortNotRewritten("Scratch game singleton"); return nullptr;
#endif
}
template<unsigned Address> void* GameData() {
#if defined(_M_IX86)
    return reinterpret_cast<void*>(static_cast<uintptr_t>(Address));
#else
    AbortNotRewritten("Scratch game data"); return nullptr;
#endif
}
void Init(StreamBinding& b) { CallGameMethod<void, 0x007D76D0>(&b); }
void Destroy(StreamBinding& b) { CallGameMethod<void, 0x007D7760>(&b); }
void Close(StreamBinding& b) { CallGameMethod<void, 0x007D7840>(&b); }
bool Open(StreamBinding& b, const ResourceKey* key, uint32_t mode) {
    return CallGameMethod<uint8_t, 0x007D8290>(&b, key, mode) != 0;
}
// The hash container stores an iterator sentinel at buckets[bucketCount].
void* Sentinel(const KeySet* set) { return set->buckets[set->bucketCount]; }
uint32_t Intern(KeySet* set, const ResourceKey* key, bool create) {
    return CallGameMethod<uint16_t, 0x007D7230>(set, key, uint32_t(create));
}
uintptr_t Find(KeySet* set, uint32_t* id, void** iterator) {
    return reinterpret_cast<uintptr_t>(CallGameMethod<void**, 0x00A90200>(set, iterator, id));
}
uint8_t ClearMark(KeySet* set, uint32_t* id, void** iterator) {
    return CallGameMethod<uint8_t, 0x007D6530>(set, iterator, id, uint32_t(0));
}
void Release(void* p) { if (p) CallGameVirtual<void, 1>(p); }
// The sidecar is eight dwords, including the deliberately untouched reserved dword.
struct Sidecar { uint32_t words[8]; };
TS3_X86_SIZE(Sidecar, 32);
// Fixed-vector header and five inline interface pointers. The game assignment helper
// may grow it; the original cleans up references but does not free spilled storage.
struct SmallRefs {
    void **begin, **end, **capacity;
    uint32_t allocator;
    void* local[5];
    SmallRefs() : begin(local), end(local), capacity(local + 5) {}
    void Assign(void** first, void** last, ResourceKey* hint) {
        CallGameMethod<void, 0x005D3450>(this, first, last, hint);
    }
    void ReleaseAll() { for (auto p = begin; p < end; ++p) Release(*p); }
};
TS3_X86_OFFSET(SmallRefs, local, 16);
TS3_X86_SIZE(SmallRefs, 36);
struct GameString { void *begin, *end, *capacity; const char* name; };
TS3_X86_SIZE(GameString, 16);
struct SeenSet {
    uint32_t unused; void** buckets; uint32_t bucketCount, size;
    float maxLoad, growth; uint32_t next; const char* name;
};
TS3_X86_OFFSET(SeenSet, buckets, 4);
TS3_X86_SIZE(SeenSet, 32);
}

// 0x004A49D0 (__thiscall, ret 4).
ResourceKey* KeyList::Append(const ResourceKey* key) {
    auto slot = end;
    if (reinterpret_cast<uintptr_t>(slot) >= reinterpret_cast<uintptr_t>(capacity))
        return CallGameMethod<ResourceKey*, 0x006D3810>(this, slot, key);
    end = reinterpret_cast<ResourceKey*>(reinterpret_cast<uintptr_t>(slot) + sizeof(ResourceKey));
    if (slot) { slot->low = key->low; slot->high = key->high; slot->type = key->type; slot->group = key->group; }
    return slot;
}

// 0x005BD410 (__stdcall, ret 28). The search is stdcall too: Method supplies an
// ignored ECX and the same callee-clean stack ABI, rather than using a cdecl bridge.
#if defined(_M_IX86)
uint8_t __stdcall
#else
uint8_t
#endif
EnsureKey(KeyList* list, uint32_t low, uint32_t high, uint32_t flags, uint32_t category, void* db, ResourceKey* key) {
    if (!CallGameMethod<uint8_t, 0x005BCE20>(list, list, category, key)) {
        if (!CallGameFunction<uint8_t, 0x005BCB50>(db, low, high, category, key)) return 0;
        list->flags |= flags;
        list->Append(key);
    }
    return 1;
}

// 0x005E5340 (__thiscall, ret 4). Signed bounds, including the missing lower bound.
Preset* PresetOwner::Get(int32_t index) const {
    const auto bytes = static_cast<intptr_t>(reinterpret_cast<uintptr_t>(end) - reinterpret_cast<uintptr_t>(begin));
    const auto count = bytes / static_cast<intptr_t>(sizeof(Preset));
    if (index >= count) return nullptr;
    auto p = reinterpret_cast<Preset*>(reinterpret_cast<uintptr_t>(begin) + intptr_t(index) * sizeof(Preset));
    return p->active ? p : nullptr;
}

// 0x005EDAB0 (__thiscall, ret 4).
bool ContentManager::Contains(const void* key) {
    void* iterator[2]; // AD3F40 writes both the node and its bucket pointer.
    CallGameMethod<void, 0x00AD3F40>(&keys, iterator, key);
    return iterator[0] != Sentinel(&keys);
}

// 0x007D81A0 (__thiscall, ret 8). Reload file after the first virtual call.
uint32_t StreamBinding::Bind(void* db, uint32_t newMode) {
    if (file) {
        CallGameVirtual<void, 8>(file);
        CallGameVirtual<void, 2>(file);
        file = nullptr;
        stream = nullptr;
        opened = 0;
    }
    stream = nullptr;
    mode = newMode;
    database = db;
    return newMode;
}

// 0x00984680 (__thiscall, ret 8).
uint8_t ScratchStore::Write(const ResourceKey* key, void* source) {
    if (!database || !source) return 0;
    CallGameMethod<void, 0x004E16F0>(mutex, GameData<0x010468AC>());
    StreamBinding resource; Init(resource); resource.Bind(database, 1);
    uint8_t result = 0;
    if (Open(resource, key, 1)) {
        int32_t size = CallGameVirtual<int32_t, 11>(source);
        if (size != -1 && CallGameFunction<int32_t, 0x004DD410>(source, resource.stream, size) == size) {
            StreamBinding metadata; Init(metadata); metadata.Bind(database, 1);
            ResourceKey sideKey{key->low, key->high, 0x206E4AFE, 0};
            if (Open(metadata, &sideKey, 1)) {
                Sidecar side; side.words[0] = 0;
                for (unsigned i = 2; i < 8; ++i) side.words[i] = 0;
                const auto now = static_cast<uint64_t>(std::time(nullptr));
                side.words[2] = static_cast<uint32_t>(now); side.words[3] = static_cast<uint32_t>(now >> 32);
                side.words[4] = key->low; side.words[5] = key->high;
                side.words[6] = key->type; side.words[7] = key->group;
                CallGameMethod<void, 0x00984150>(&side, metadata.stream);
                Close(metadata); result = 1;
            }
            Close(resource); CallGameVirtual<void, 9>(database); Destroy(metadata);
        }
    }
    Destroy(resource); CallGameMethod<void, 0x004E17B0>(mutex);
    return result;
}

// 0x00985B90 (__thiscall, ret 12).
uint8_t ScratchStore::SetTimes(const ResourceKey* key, uint32_t low, uint32_t high) {
    if (!database) return 0;
    CallGameMethod<void, 0x004E16F0>(mutex, GameData<0x010468AC>());
    StreamBinding resource; Init(resource); resource.Bind(database, 1);
    uint8_t result = 0;
    if (Open(resource, key, 0)) {
        ResourceKey sideKey{key->low, key->high, 0x206E4AFE, 0};
        void* file = nullptr;
        if (CallGameVirtual<uint8_t, 13>(database, &sideKey, &file, uint32_t(1), uint32_t(6), uint32_t(1), uint32_t(0))) {
            Sidecar side; side.words[0] = 0;
            for (unsigned i = 2; i < 8; ++i) side.words[i] = 0;
            CallGameMethod<void, 0x009840D0>(&side, CallGameVirtual<void*, 6>(file));
            side.words[2] = low; side.words[3] = high;
            CallGameVirtual<void, 10>(CallGameVirtual<void*, 6>(file), uint32_t(0), uint32_t(0));
            CallGameMethod<void, 0x00984150>(&side, CallGameVirtual<void*, 6>(file));
            CallGameVirtual<void, 8>(file); result = 1;
        }
        Close(resource); CallGameVirtual<void, 9>(database);
        if (file) CallGameVirtual<void, 2>(file);
    }
    Destroy(resource); CallGameMethod<void, 0x004E17B0>(mutex);
    return result;
}

// 0x009DC470 (cdecl, bool in AL): the receiver is a temporary, not the key.
uint8_t IsSpecial(const void* key) {
    uint32_t scratch;
    return CallGameMethod<uint8_t, 0x009DC200>(&scratch, key);
}
// 0x0099AB70 (cdecl, bool in AL).
uint8_t IsDependent(const void* input) {
    auto key = static_cast<const ResourceKey*>(input);
    switch (key->type) {
    case 0x0341ACC9: case 0x025ED6F4: case 0x033A1435:
    case 0xB4DD716B: case 0x044AE110: case 0xDEA2951C: return 1;
    case 0x00B2D882: case 0x0B2CB440: return IsSpecial(key);
    case 0x01661233: return CallGameFunction<uint8_t, 0x007099A0>(key);
    default: return 0;
    }
}

// 0x0099E7A0 (cdecl). No caller consumes this helper's incidental AL return.
uint8_t AddFabric(KeySet* seen, void* db, ContentManager* content, void* options, KeyList* output, StreamBinding* binding, const ResourceKey* key) {
    uint8_t result = content->Contains(key);
    if (!result) return result;
    uint32_t id = Intern(static_cast<KeySet*>(options), key, true);
    void* iterator[3];
    const uintptr_t found = Find(seen, &id, iterator);
    result = static_cast<uint8_t>(found);
    if (*reinterpret_cast<void**>(found) == Sentinel(seen)) {
        output->Append(key);
        void* resource = nullptr;
        const bool loaded = CallGameVirtual<uint8_t, 3>(db, key, &resource, 0u, 0u, 0u, 0u) != 0;
        if (loaded) result = CallGameVirtual<uint8_t, 7>(db, resource, 0u, binding->database, 0u, 0u);
        if (!loaded || result) result = ClearMark(seen, &id, iterator);
        if (resource) result = CallGameVirtual<uint8_t, 1>(resource);
    }
    return result;
}

// 0x0099E880 (cdecl).
uint8_t AddCompositor(KeySet* seen, void* db, ContentManager* content, void* options, KeyList* output, StreamBinding* binding, void* compositor) {
    if (!compositor) return 0;
    auto key = reinterpret_cast<const ResourceKey*>(static_cast<uint8_t*>(compositor) + 2 * sizeof(void*));
    const bool present = content->Contains(key);
    uint32_t id = Intern(static_cast<KeySet*>(options), key, present);
    if (present) {
        void* iterator[3];
        auto found = Find(seen, &id, iterator);
        if (*reinterpret_cast<void**>(found) == Sentinel(seen)) {
            output->Append(key);
            auto resource = CallGameVirtual<void*, 3>(compositor, 0x02269ED1u, 0u, binding->database, 0u, 0u);
            if (!CallGameVirtual<uint8_t, 7>(db, resource)) return 0;
            ClearMark(seen, &id, iterator);
            if (CallGameVirtual<uint32_t, 6>(compositor) == 0x033A1435) {
                KeyList fabrics{nullptr, nullptr, nullptr, "Scratch/DownloadContent/AddDependentCompositor/FabricKeys", 0};
                CallGameMethod<void, 0x006038E0>(compositor, &fabrics, 1u);
                for (auto p = fabrics.begin; p != fabrics.end; ++p) AddFabric(seen, db, content, options, output, binding, p);
                if (fabrics.begin) Free4E3F70(fabrics.begin);
            }
        }
    }
    return 1;
}

// 0x009A0560 (cdecl). Preserve EA's duplicate primary iteration and ignored helper failures.
uint8_t AddTextureDependencies(KeyList* outfits, void* options, KeyList* output) {
    void* db = CallGameFunction<void*, 0x004AFD20>();
    auto content = CallGameFunction<ContentManager*, 0x005EB7C0>();
    auto bin = GlobalPointer<0x011E96A4, ExportBin>();
    StreamBinding binding; Init(binding);
    if (!bin->GetBinding(&binding)) { Destroy(binding); return 0; }
    SeenSet seen{0, static_cast<void**>(GameData<0x0114C900>()), 1, 0, 1.0f, 2.0f, 0, "Scratch/DownloadContentUtils/AddDependentTexComps"};
    bool success = true;
    for (auto key = outfits->begin; key != outfits->end; ++key) {
        void* resource = nullptr;
        if (!CallGameVirtual<uint8_t, 3>(db, key, &resource, 0u, 0u, 0u, 0u)) { Release(resource); success = false; break; }
        auto owner = resource ? CallGameVirtual<PresetOwner*, 3>(resource, 0x025ED708u) : nullptr;
        if (owner) CallGameVirtual<void, 0>(owner);
        // Deliberately no null guard here: the original dereferences a missing preset interface.
        const auto count = (reinterpret_cast<intptr_t>(owner->end) - reinterpret_cast<intptr_t>(owner->begin)) / static_cast<intptr_t>(sizeof(Preset));
        for (intptr_t i = 0; i < static_cast<intptr_t>(count); ++i) {
            SmallRefs primary, secondary;
            GameString text{GameData<0x011C7E54>(), GameData<0x011C7E54>(), GameData<0x011C7E56>(), "Scratch/DownloadcontentUtils/AddDependentTextureCompositors/PresetStr"};
            auto preset = owner->Get(static_cast<int32_t>(i));
            if (preset) {
                primary.Assign(preset->primaryBegin, preset->primaryEnd, outfits->end);
                secondary.Assign(preset->secondaryBegin, preset->secondaryEnd, outfits->end);
                if (preset->text) CallGameMethod<void, 0x005BF4C0>(preset->text, &text);
            }
            for (unsigned pass = 0; pass < 2; ++pass)
                for (auto p = primary.begin; p != primary.end; ++p)
                    AddCompositor(reinterpret_cast<KeySet*>(&seen), db, content, options, output, &binding, *p);
            const auto span = static_cast<intptr_t>(reinterpret_cast<uintptr_t>(text.capacity) - reinterpret_cast<uintptr_t>(text.begin)) & ~intptr_t(1);
            if (span > 2 && text.begin) Free4E3F70(text.begin);
            secondary.ReleaseAll(); primary.ReleaseAll();
        }
        const bool added = CallGameVirtual<uint8_t, 7>(db, resource, 0u, binding.database, 0u, 0u) != 0;
        Release(owner); Release(resource);
        if (!added) { success = false; break; }
    }
    CallGameMethod<void, 0x007F3790>(&seen, seen.buckets, seen.bucketCount);
    seen.size = 0;
    if (seen.bucketCount > 1) Free4E3F70(seen.buckets);
    Destroy(binding);
    return uint8_t(success);
}

// 0x009A0A00 (cdecl).
bool AddDependencies(KeyList* input, void* options) {
    void* db = CallGameFunction<void*, 0x004AFD20>();
    if (!db) return false;
    KeyList outfits{nullptr, nullptr, nullptr, "Scratch/DownloadContent/SimoutfitKeyList", 0};
    for (auto key = input->begin; key != input->end; ++key) {
        Intern(reinterpret_cast<KeySet*>(options), key, IsDependent(key) != 0);
        if (key->type == 0x025ED6F4 && CallGameVirtual<uint8_t, 3>(db, key, nullptr, 0u, 0u, 0u, 0u)) outfits.Append(key);
    }
    const bool result = AddTextureDependencies(&outfits, options, input) != 0;
    if (outfits.begin) Free4E3F70(outfits.begin);
    return result;
}

// 0x009A2EC0 (__thiscall, ret 4).
uint8_t ExportBin::GetBinding(StreamBinding* output) {
    if (database) { output->Bind(database, mode); return 1; }
    auto db = CallGameFunction<void*, 0x004AFD20>();
    if (!db) return 0;
    void* memory = CallGameFunction<void*, 0x004E41F0>(160u, "Scratch/ExportBin/MemoryDB", 0u, 0u, 0u, 0u);
    void* created = memory ? CallGameMethod<void*, 0x0072B8F0>(memory, 0u, "Scratch/ExportBinMemoryDB") : nullptr;
    auto previous = database;
    if (created != previous) {
        if (created) CallGameVirtual<void, 0>(static_cast<uint8_t*>(created) + sizeof(void*));
        database = created;
        if (previous) CallGameVirtual<void, 1>(static_cast<uint8_t*>(previous) + sizeof(void*));
    }
    if (!database) return 0;
    if (CallGameVirtual<uint8_t, 1>(database) && CallGameVirtual<uint8_t, 6>(database, 3u, 6u, 0u) &&
        CallGameVirtual<uint8_t, 13>(db, 1u, database, 0u)) {
        output->Bind(database, mode); return 1;
    }
    CallGameFunction<void, 0x007D8E50>(database, 0u, 0u);
    if (database) {
        auto old = database; database = nullptr;
        CallGameVirtual<void, 1>(static_cast<uint8_t*>(old) + sizeof(void*));
    }
    return 0;
}

// 0x00C60CC0 (__thiscall, ret 4).
uint8_t ResourceStore::GetBinding(StreamBinding* output) {
    if (!database) return 0;
    output->Bind(database, readOnly == 0);
    return 1;
}
}
