#pragma once
#include "ts3/layout.h"
#include "ts3/resource/packed_file.h"
#include <cstdint>
namespace ts3::resourcesystem {
struct ArenaBlock { ArenaBlock* previous; uint8_t* end; };
using ArenaAllocate = void* (*)(uint32_t, uint32_t*, void*);
struct ResourceArena {
    void* reserved;
    ArenaBlock* blocks;
    uint8_t* limit;
    uint8_t* cursor;
    uint8_t* usedEnd;
    ArenaAllocate allocate;
    void* release;
    void* context;
    // TS3W.exe 0x004E8650: grow the arena, retaining its active allocation in the new block.
    bool Grow(uint32_t requested);
    // TS3W.exe 0x004E8800: allocate with alignment and offset, optionally growing.
    void* AllocateAligned(uint32_t bytes, uint32_t alignment, uint32_t offset, bool grow);
};
TS3_X86_OFFSET(ResourceArena, cursor, 12);
TS3_X86_OFFSET(ResourceArena, allocate, 20);
TS3_X86_OFFSET(ResourceArena, context, 28);
TS3_X86_SIZE(ResourceArena, 32);
TS3_X86_SIZE(ArenaBlock, 8);
// TS3W.exe 0x004A0A80: allocate an eight-byte-rounded region from a resource arena.
void* AllocateResource(uint32_t bytes, ResourceArena* arena);
struct PatternStringPool { void* reserved; ResourceArena arena; };
TS3_X86_OFFSET(PatternStringPool, arena, 4);
struct ResourceLinks { ResourceLinks *next, *previous; };
struct ResourceKey12 { uint32_t words[3]; };
struct ResourceMapEntry {
    void* reserved[2]; ResourceLinks links; ResourceKey12 first, second;
    uint32_t kind; uint32_t* values; uint32_t count; uint32_t tail;
};
TS3_X86_SIZE(ResourceMapEntry, 56);
TS3_X86_OFFSET(ResourceMapEntry, values, 44);
struct ResourceStringPool {
    void* reserved[2]; ResourceArena arena;
    // TS3W.exe 0x0054B1B0: allocate and initialize a map entry and optional zeroed value array.
    ResourceMapEntry* CreateEntry(const ResourceKey12* first, uint32_t kind, const ResourceKey12* second, uint32_t count);
    // TS3W.exe 0x0054ABB0: copy UTF-16 including the terminator; EA does not handle allocation failure.
    char16_t* CopyString(const char16_t* text);
};
TS3_X86_OFFSET(ResourceStringPool, arena, 8);
struct ResourceObjectLink { ResourceObjectLink *next, *previous; void* object; };
struct ResourceObjectPool {
    void* buckets[4096]; ResourceArena arena; void* reserved;
    ResourceLinks objects; ResourceObjectLink* freeLinks;
    // TS3W.exe 0x0054C770: resolve a factory, allocate its object and link the result into the pool.
    void* Create(uint32_t code);
};
TS3_X86_OFFSET(ResourceObjectPool, arena, 0x4000);
TS3_X86_OFFSET(ResourceObjectPool, objects, 0x4024);
TS3_X86_OFFSET(ResourceObjectPool, freeLinks, 0x402C);
// No extra state: retain the original thiscall ABI while reusing verified methods.
struct PackedFileAdapter : ts3::resource::DatabasePackedFile {
    // TS3W.exe 0x004A7450: delegate to the already verified packed-file seek implementation.
    bool SeekPacked(uint32_t position);
    // TS3W.exe 0x004A7490: delegate to the already verified packed-file read implementation.
    uint32_t ReadPacked(void* output, uint32_t size);
    // TS3W.exe 0x004A7560: delegate to the already verified packed-file resize implementation.
    bool ResizePacked(uint32_t size);
};
static_assert(sizeof(PackedFileAdapter) == sizeof(ts3::resource::DatabasePackedFile));
struct PathVector {
    const char16_t** begin; const char16_t** end; const char16_t** capacity;
    void* allocator; uint32_t flags; const char* name;
    // TS3W.exe 0x0093B2C0: insert one path pointer; the in-place return is memmove's destination.
    uintptr_t Insert(const char16_t** position, const char16_t* const* value);
    // TS3W.exe 0x004A1BA0: append one path pointer, using Insert only on growth.
    uintptr_t Append(const char16_t* const* value);
};
TS3_X86_SIZE(PathVector, 24);
TS3_X86_OFFSET(PathVector, allocator, 12);
struct ResourcePair { uint32_t key; void* value; };
TS3_X86_SIZE(ResourcePair, 8);
struct PairVector {
    ResourcePair* begin; ResourcePair* end; ResourcePair* capacity;
    void* allocator; uint32_t flags; const char* name;
    // TS3W.exe 0x004AB770: insert one key/object pair, accounting for aliased input.
    uintptr_t Insert(ResourcePair* position, const ResourcePair* value);
};
TS3_X86_SIZE(PairVector, 24);
// TS3W.exe 0x004AFD30: exchange the process-global resource setting (not compare-safe).
int32_t ExchangeResourceSetting(int32_t value);
}
namespace ts3::resourcesystem {
struct ProviderLink { ProviderLink *next, *previous; void* provider; uint32_t priority; };
struct ResourceProviders {
    void* reserved; ResourceLinks providers; void* opaque[2]; void* manager;
    // TS3W.exe 0x005833C0: construct a provider aggregate; retain the priority-10000 retry quirk.
    void* BuildAggregate(ResourceObjectPool* pool, uintptr_t unused);
};
TS3_X86_OFFSET(ResourceProviders, providers, 4);
TS3_X86_OFFSET(ResourceProviders, manager, 20);
TS3_X86_SIZE(ResourceProviders, 24);
TS3_X86_OFFSET(ProviderLink, priority, 12);
}
namespace ts3::resourcesystem {
// TS3W.exe 0x004AE870: find the embedded package marker using EA's overlapping 512-byte scan.
int32_t FindEmbeddedPackage(const wchar_t* filename);
}
