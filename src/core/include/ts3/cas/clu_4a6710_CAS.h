#pragma once
#include "ts3/resource/packed_file.h"

namespace ts3::cas {
struct CacheLimit {
    int32_t total;
    uint32_t limit;
    // TS3W.exe 0x004A9C30: sets the limit; zero means INT_MAX.
    int32_t SetLimit(int32_t value);
};
TS3_X86_OFFSET(CacheLimit, limit, 4);
TS3_X86_SIZE(CacheLimit, 8);

struct CachePackedFile : resource::DatabasePackedFile {
    // TS3W.exe 0x004A6710: stores and returns the index observer.
    void* SetObserver(void* observer);
    // TS3W.exe 0x004A69E0: sets the primary free-space limit.
    int32_t SetLimit(int32_t value);
    // TS3W.exe 0x004A6EB0: reads the primary space total under the record mutex.
    int32_t SpaceTotal();
    // TS3W.exe 0x004A6F80: allocates space only when write access is set.
    bool Reserve(int32_t* offset, uint32_t size);
    // TS3W.exe 0x004A6FD0: returns space under the record mutex; zero offset/size is ignored.
    bool ReturnSpace(int32_t offset, int32_t size);
};
TS3_X86_SIZE(CachePackedFile, 0x398);

struct CacheFreeSpaceMap : resource::FreeSpaceMap {
    // TS3W.exe 0x004AC730: merges source blocks into this map, clears source, and returns its anchor.
    void* Drain(CacheFreeSpaceMap* source);
};
TS3_X86_SIZE(CacheFreeSpaceMap, 0x24);

struct CacheReference {
    struct Object { void* primary; void* secondary; };
    Object* object;
    // TS3W.exe 0x005BB380: clears the reference before releasing its secondary interface.
    intptr_t Clear();
};
TS3_X86_SIZE(CacheReference, 4);
TS3_X86_OFFSET(CacheReference::Object, secondary, 4);
// TS3W.exe 0x004FF380: bounded UTF-16 copy, returning the full source length even if truncated.
uint32_t CopyUtf16(uint16_t* destination, const uint16_t* source, uint32_t capacity);
struct CompositorKey { uint32_t words[4]; };
struct CompositorLink { CompositorLink* next; CompositorLink* previous; };
struct CompositorNode { CompositorLink link; CompositorKey key; };
struct CompositorKeyList {
    CompositorLink anchor;
    void* allocator;
    const char* label;
    CompositorKey key;
    // TS3W.exe 0x005BB770: serialize the key list, restoring the file limit on every path.
    uint8_t Save(CachePackedFile* file);
};
struct CompositorCache {
    uintptr_t opaque[4];
    CompositorKeyList keys;
    CachePackedFile* file;
    void* evictionObserver;
    uint32_t limit;
    // TS3W.exe 0x005BB8B0: remove oldest keys until the byte target is reached, then flush.
    int32_t Evict(uint32_t bytes);
    // TS3W.exe 0x005BBC10: unregister, save and close the file, preserving release reentrancy.
    void Close();
    // TS3W.exe 0x005BC6D0: lazily open/register the cache and compact bounded files.
    uint8_t Open(const uint16_t* suffix);
};
TS3_X86_SIZE(CompositorKey, 16);
TS3_X86_SIZE(CompositorNode, 24);
TS3_X86_OFFSET(CompositorKeyList, key, 16);
TS3_X86_SIZE(CompositorKeyList, 32);
TS3_X86_OFFSET(CompositorCache, keys, 16);
TS3_X86_OFFSET(CompositorCache, file, 48);
TS3_X86_OFFSET(CompositorCache, evictionObserver, 52);
TS3_X86_OFFSET(CompositorCache, limit, 56);
#if defined(_M_IX86)
// ABI adapter for the side-effecting 0x005BC6D0 harness replacement.
inline uint8_t __fastcall CompositorOpenHook(CompositorCache* self, void*, const uint16_t* suffix)
{
    return self->Open(suffix);
}
#endif
} // namespace ts3::cas
