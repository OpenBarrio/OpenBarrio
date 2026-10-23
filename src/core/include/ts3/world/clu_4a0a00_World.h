#pragma once
#include "ts3/layout.h"
#include <cstdint>

namespace ts3::world {
// Secondary reference-count interface follows the primary interface, pointer-sized on x64.
struct WorldRefObject { void* primary; void* reference; };
TS3_X86_OFFSET(WorldRefObject, reference, 4);
TS3_X86_SIZE(WorldRefObject, 8);
struct WorldRef {
    WorldRefObject* object;
    // TS3W.exe 0x004A0A00: retain new before publishing it and releasing old; self-assignment is inert.
    WorldRef* Assign(WorldRefObject* value);
};
TS3_X86_SIZE(WorldRef, 4);
struct WorldDbOptions {
    uint8_t unknown[828];
    int32_t kind;
    float delay;
    // TS3W.exe 0x004A69B0: set kind, updating delay only for ordered nonnegative inputs.
    uint8_t SetOptions(int32_t value, float seconds);
};
TS3_X86_OFFSET(WorldDbOptions, kind, 828);
TS3_X86_OFFSET(WorldDbOptions, delay, 832);
TS3_X86_SIZE(WorldDbOptions, 836);
// TS3W.exe 0x0072F1C0: read the resource service global (not cached).
uintptr_t WorldResourceService();
struct WorldResourceKey { uint32_t words[4]; };
TS3_X86_SIZE(WorldResourceKey, 16);
// TS3W.exe 0x0093B9A0: resolve a world key, optionally through the world manager.
uint8_t ResolveWorldKey(const WorldResourceKey* key, void* output);
struct WorldKeyPair {
    uint8_t unknown[8];
    WorldResourceKey first, second;
    // TS3W.exe 0x009460B0: resolve first and conditionally second; return presence, not success.
    uint8_t Resolve(void* firstOutput, void* secondOutput);
};
TS3_X86_OFFSET(WorldKeyPair, first, 8);
TS3_X86_OFFSET(WorldKeyPair, second, 24);
TS3_X86_SIZE(WorldKeyPair, 40);
struct WorldResetService {
    // TS3W.exe 0x009E52A0: reset service, notify resource system; preserve upper return bits.
    uint32_t Reset();
};
// Only the record addresses and byte flag are inspected by this unit; record internals
// belong to the original travel database implementation.
struct WorldTravelService {
    uint8_t unknown00[64];
    uint8_t travelRecord[52], worldRecord[52], transientRecord[52];
    uint8_t unknownDC[41];
    uint8_t enabled;
    uint8_t unknown106[10];
    alignas(void*) uint8_t criticalSection[sizeof(void*) == 4 ? 24 : 40];
    // TS3W.exe 0x009CBF20: reset transient DB, lock/open TravelDB, open world DB, set flag.
    int32_t Open(void* path, const char16_t* filename, uint8_t flag);
};
TS3_X86_OFFSET(WorldTravelService, travelRecord, 64);
TS3_X86_OFFSET(WorldTravelService, worldRecord, 116);
TS3_X86_OFFSET(WorldTravelService, transientRecord, 168);
TS3_X86_OFFSET(WorldTravelService, enabled, 261);
TS3_X86_OFFSET(WorldTravelService, criticalSection, 272);
TS3_X86_SIZE(WorldTravelService, 296);
// TS3W.exe 0x00C63500: create/open a plain or user-key protected world database and register it.
// EA deliberately dereferences a failed named-file allocation (no null guard).
uint8_t OpenWorldDatabase(const char16_t* source, WorldRef* output, int32_t mode, int32_t flags);
struct WorldDesigner {
    uint8_t unknown00[99];
    uint8_t enabled;
    uint8_t unknown64[28];
    char* cachedBegin;
    char* cachedEnd;
    uint8_t unknown88[592];
    WorldRef database;
    // TS3W.exe 0x00B31E00: ensure the preset database is open in the requested mode.
    void EnsureDatabase(int32_t mode);
    // TS3W.exe 0x00B324D0: add a color-info preset to the database.
    void AddColor(const WorldResourceKey* key);
    // TS3W.exe 0x00B32500: remove a color-info preset, using an existing DB even when disabled.
    uint8_t RemoveColor(const WorldResourceKey* key);
    // TS3W.exe 0x00B326D0: remove a design preset by the supplied two-word identifier.
    void RemoveDesign(const WorldResourceKey* key, uint32_t low, uint32_t high);
    // TS3W.exe 0x00B33300: load/cache map-tag text, stripping a UTF-8 BOM only when size > 3.
    char* MapTagText(uint32_t low, uint32_t high);
};
TS3_X86_OFFSET(WorldDesigner, enabled, 99);
TS3_X86_OFFSET(WorldDesigner, cachedBegin, 128);
TS3_X86_OFFSET(WorldDesigner, database, 728);
TS3_X86_SIZE(WorldDesigner, 732);
// TS3W.exe 0x00787E20: marshal a script key and 64-bit preset ID into RemoveDesign.
void ScriptRemoveDesign(uint32_t a, uint32_t b, uint32_t c, uint32_t d, uint32_t id);
// TS3W.exe 0x00787E90: marshal a script key into AddColor (void operation).
void ScriptAddColor(uint32_t a, uint32_t b, uint32_t c, uint32_t d);
// TS3W.exe 0x00787ED0: marshal a script key into RemoveColor and return its low-byte result.
uint8_t ScriptRemoveColor(uint32_t a, uint32_t b, uint32_t c, uint32_t d);
// TS3W.exe 0x007950C0: synchronously dispatch map-tag text to the script host.
int32_t ScriptForceMapTagTick(uint32_t low, uint32_t high);
struct SaveWorldPath { char16_t text[520]; };
TS3_X86_SIZE(SaveWorldPath, 1040);
struct WorldWideText { char16_t *begin, *end, *capacity; const char* owner; };
TS3_X86_SIZE(WorldWideText, 16);
struct SaveWorldName { char *begin, *end, *capacity; const char* owner; };
TS3_X86_SIZE(SaveWorldName, 16);
struct WorldSavePaths {
    SaveWorldPath root;
    WorldWideText directory;
    SaveWorldName worldName;
    SaveWorldPath current, fallback;
    WorldWideText code;
    SaveWorldPath source, meta, chosen;
    // TS3W.exe 0x00AAC6B0: derive save filenames and select source/fallback by existence.
    uintptr_t Build(const SaveWorldPath* sourcePath, const WorldWideText* worldCode);
};
TS3_X86_OFFSET(WorldSavePaths, directory, 1040);
TS3_X86_OFFSET(WorldSavePaths, worldName, 1056);
TS3_X86_OFFSET(WorldSavePaths, current, 1072);
TS3_X86_OFFSET(WorldSavePaths, fallback, 2112);
TS3_X86_OFFSET(WorldSavePaths, code, 3152);
TS3_X86_OFFSET(WorldSavePaths, source, 3168);
TS3_X86_OFFSET(WorldSavePaths, meta, 4208);
TS3_X86_OFFSET(WorldSavePaths, chosen, 5248);
TS3_X86_SIZE(WorldSavePaths, 6288);
struct WorldSaveContext {
    // TS3W.exe 0x00AACD80: validate a copied save directory; returns 5/6/7/0.
    int32_t Validate(WorldSavePaths* paths, void* unused);
    // TS3W.exe 0x00AAE950: acquire a root, validate optional directory, then derive paths.
    int32_t Prepare(WorldSavePaths* paths, uint8_t clearCode, uint8_t useCurrentWorld);
};
// TS3W.exe 0x00AAB3F0: copy and validate the current world directory, rejecting null/empty.
uint8_t GetCurrentSaveWorldPath(SaveWorldPath* output);
// TS3W.exe 0x00AAEAC0: enable/disable the travel database; retain incidental pointer/int return.
uintptr_t ConfigureTravelDatabase(uint8_t enabled, uint8_t flag);
// TS3W.exe 0x00AAEFA0: close travel DB, retry removal up to ten times, then reopen it.
uint8_t ResetTravelDatabase();
} // namespace ts3::world
