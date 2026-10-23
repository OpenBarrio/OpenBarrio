#pragma once
#include "ts3/layout.h"
#include <cstdint>

namespace ts3::scratch {
struct ResourceKey { uint32_t low, high, type, group; };
TS3_X86_SIZE(ResourceKey, 16);
struct KeyList {
    ResourceKey *begin, *end, *capacity;
    const char* name;
    uint32_t flags;
    // TS3W.exe 0x004A49D0: append a resource key, returning the constructed slot (or the growth result).
    ResourceKey* Append(const ResourceKey* key);
};
TS3_X86_OFFSET(KeyList, end, 4);
TS3_X86_OFFSET(KeyList, flags, 16);
struct StreamBinding {
    void *database, *file, *stream;
    uint32_t reserved, mode;
    uint8_t enabled, opened;
    // TS3W.exe 0x007D81A0: close/release the old file and bind a database; leaves opened unchanged if no file.
    uint32_t Bind(void* db, uint32_t newMode);
};
TS3_X86_OFFSET(StreamBinding, file, 4);
TS3_X86_OFFSET(StreamBinding, mode, 16);
TS3_X86_OFFSET(StreamBinding, opened, 21);
TS3_X86_SIZE(StreamBinding, 24);
struct Preset {
    uint32_t id[2]; void* active;
    uint8_t pad0c[8];
    void **primaryBegin, **primaryEnd;
    uint8_t pad1c[28];
    void **secondaryBegin, **secondaryEnd;
    uint8_t pad40[28];
    void* text;
    uint8_t pad60[8];
};
TS3_X86_OFFSET(Preset, active, 8);
TS3_X86_OFFSET(Preset, primaryBegin, 20);
TS3_X86_OFFSET(Preset, secondaryBegin, 56);
TS3_X86_OFFSET(Preset, text, 92);
TS3_X86_SIZE(Preset, 104);
struct PresetOwner {
    uint8_t pad00[152]; Preset *begin, *end;
    // TS3W.exe 0x005E5340: get an active preset; intentionally does not reject negative indices.
    Preset* Get(int32_t index) const;
};
TS3_X86_OFFSET(PresetOwner, begin, 152);
struct KeySet { void* unknown; void** buckets; uint32_t bucketCount; };
TS3_X86_OFFSET(KeySet, buckets, 4);
TS3_X86_OFFSET(KeySet, bucketCount, 8);
struct ContentManager {
    uint8_t pad00[260]; KeySet keys;
    // TS3W.exe 0x005EDAB0: look up a key in the embedded hash set.
    bool Contains(const void* key);
};
TS3_X86_OFFSET(ContentManager, keys, 260);
struct ScratchStore {
    uint8_t pad00[28]; void* database;
    uint8_t pad20[1424]; uint8_t mutex[24];
    // TS3W.exe 0x00984680: write a resource and its timestamp/key sidecar under the store mutex.
    uint8_t Write(const ResourceKey* key, void* source);
    // TS3W.exe 0x00985B90: update the time fields of a resource's sidecar under the store mutex.
    uint8_t SetTimes(const ResourceKey* key, uint32_t low, uint32_t high);
};
TS3_X86_OFFSET(ScratchStore, database, 28);
TS3_X86_OFFSET(ScratchStore, mutex, 1456);
struct ExportBin {
    uint8_t pad00[124]; void* database;
    uint32_t reserved, mode;
    // TS3W.exe 0x009A2EC0: lazily create/register an export database, then bind the output.
    uint8_t GetBinding(StreamBinding* output);
};
TS3_X86_OFFSET(ExportBin, database, 124);
TS3_X86_OFFSET(ExportBin, mode, 132);
struct ResourceStore {
    uint8_t pad00[67]; uint8_t readOnly;
    uint8_t pad44[60]; void* database;
    // TS3W.exe 0x00C60CC0: bind output to the store database using the inverse read-only bit.
    uint8_t GetBinding(StreamBinding* output);
};
TS3_X86_OFFSET(ResourceStore, database, 128);
#if defined(_M_IX86)
#define TS3_SCRATCH_STDCALL __stdcall
#else
#define TS3_SCRATCH_STDCALL
#endif
// TS3W.exe 0x005BD410: find or generate a category key, OR flags only on insertion (stdcall ret 28).
uint8_t TS3_SCRATCH_STDCALL EnsureKey(KeyList*, uint32_t, uint32_t, uint32_t, uint32_t, void*, ResourceKey*);
// TS3W.exe 0x0099AB70: classify dependent content keys, delegating two special families.
uint8_t IsDependent(const void*);
// TS3W.exe 0x009DC470: invoke the special-family classifier with a temporary receiver.
uint8_t IsSpecial(const void*);
// TS3W.exe 0x0099E7A0: add one fabric dependency; return preserves the original incidental low byte.
uint8_t AddFabric(KeySet*, void*, ContentManager*, void*, KeyList*, StreamBinding*, const ResourceKey*);
// TS3W.exe 0x0099E880: add a compositor dependency and its fabric keys.
uint8_t AddCompositor(KeySet*, void*, ContentManager*, void*, KeyList*, StreamBinding*, void*);
// TS3W.exe 0x009A0560: add all outfit texture dependencies. EA iterates the primary list twice, not the secondary.
uint8_t AddTextureDependencies(KeyList*, void*, KeyList*);
// TS3W.exe 0x009A0A00: select installed outfits and add their dependent texture content.
bool AddDependencies(KeyList*, void*);
#undef TS3_SCRATCH_STDCALL
}
