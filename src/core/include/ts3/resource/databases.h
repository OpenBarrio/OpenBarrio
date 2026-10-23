// The resource system's other database classes, besides DatabasePackedFile. Partial
// layouts: fields not yet understood are padding sized so the x86 offsets match the
// game's objects (see packed_file.h for the same caveat about x64).
#pragma once

#include <cstdint>

#include <EASTL/hash_map.h>
#include <EASTL/map.h>

#include "ts3/allocator/core_allocator.h"
#include "ts3/eastl_game.h"
#include "ts3/io/memory_stream.h"
#include "ts3/layout.h"
#include "ts3/resource/compression_codec.h"
#include "ts3/resource/index_modifiable.h"
#include "ts3/resource/package_index.h"
#include "ts3/resource/packed_file.h"
#include "ts3/thread/mutex.h"

namespace ts3::resource {

// DatabasePackedStream: a package read through a stream (vtable 0x00FFD790, 22 virtual
// methods; the interface view at +0x004 is 0x00FFD780, whose slot 17 is the same
// 0x0072D470 that reads records). Allocation tags "ResourceLoad/PackedStream/...".
struct DatabasePackedStream {
    void* const* vtable;                    // 0x000
    void* const* interfaceVtable;           // 0x004 0x00FFD780
    int32_t refCount;                       // 0x008
    bool usable;                            // 0x00C Open requires it; the constructor leaves it false
    uint8_t unknown00D[3];
    allocator::ICoreAllocator* allocator;   // 0x010
    uint32_t openFlags;                     // 0x014 set by Open; the read path requires it nonzero
    uint8_t unknown018[0x14];
    void* primaryHandle;                    // 0x02C the open package file (async API handle)
    void* throttledHandle;                  // 0x030 a second handle for per-key reads
    bool throttledOpen;                     // 0x034 set while the throttled handle is being opened
    uint8_t unknown035[3];
    void* perKeyObject;                     // 0x038 slot 1 answers the read flag for a record's key
    uint8_t unknown03C[4];
    thread::Mutex mutex;                    // 0x040 the open flag and the reads
    IPackageIndex* index;                   // 0x070
    uint8_t unknown074[0x38];
    IResourceManager* resourceManager;      // 0x0AC

    // TS3W.exe 0x0072C980.
    bool DecompressRecord(uint32_t compression, const void* src, uint32_t srcLen, uint32_t memSize, void* dst)
    {
        return resource::DecompressRecord(allocator, resourceManager, "ResourceLoad/PackedStream/CompressionRefpack",
                                          compression, src, srcLen, memSize, dst);
    }
    // TS3W.exe 0x0072D300. Reads `size` stored bytes at `offset` through the async request
    // API; `flag` picks the throttled handle. False only when the stream is not open —
    // see packed_stream.cpp for the kept quirk that a failed request still reports true.
    bool ReadRaw(void* dst, uint32_t offset, uint32_t size, uint8_t flag);
    // TS3W.exe 0x0072D360. ReadRaw into a temporary buffer, then DecompressRecord. Quirk
    // kept: offset 0 always fails.
    bool ReadAndDecompress(uint32_t compression, uint32_t offset, uint32_t size, uint32_t memSize, void* dst,
                           uint8_t flag);
    // TS3W.exe 0x0072D3E0. Reads up to maxSize stored (still compressed) bytes of a record.
    // True without reading when dst is null or nothing is to be read; false when the key is
    // missing or the stream is not open.
    bool ReadRecordRaw(const ResourceKey* key, void* dst, uint32_t maxSize);
};
TS3_X86_OFFSET(DatabasePackedStream, interfaceVtable, 0x004);
TS3_X86_OFFSET(DatabasePackedStream, usable, 0x00C);
TS3_X86_OFFSET(DatabasePackedStream, allocator, 0x010);
TS3_X86_OFFSET(DatabasePackedStream, openFlags, 0x014);
TS3_X86_OFFSET(DatabasePackedStream, primaryHandle, 0x02C);
TS3_X86_OFFSET(DatabasePackedStream, throttledHandle, 0x030);
TS3_X86_OFFSET(DatabasePackedStream, throttledOpen, 0x034);
TS3_X86_OFFSET(DatabasePackedStream, perKeyObject, 0x038);
TS3_X86_OFFSET(DatabasePackedStream, mutex, 0x040);
TS3_X86_OFFSET(DatabasePackedStream, index, 0x070);
TS3_X86_OFFSET(DatabasePackedStream, resourceManager, 0x0AC);

// One entry of MemoryDB's data map: the record's bytes and both sizes (the three dwords
// after the key in every tree node, at node+0x20).
struct MemoryDBRecordData {
    void* data;           // the stored bytes; null in the entry a missing key creates
    uint32_t storedSize;  // bytes as stored (compressed ones differ from memSize)
    uint32_t memSize;     // bytes once decompressed (equal when stored uncompressed)
};

// The data map at 0x50: the game's EASTL tree, byte-compatible on x86 (comparator, anchor,
// size, allocator; nodes are the key + payload pair, 48 bytes as the game allocates them).
using RecordMap = eastl::multimap<ResourceKey, MemoryDBRecordData, ResourceKeyLess, GameAllocator>;
TS3_X86_SIZE(RecordMap, 0x24);
static_assert(sizeof(void*) != 4 || sizeof(RecordMap::value_type) == 32, "a key + data pair is 32 bytes");
static_assert(sizeof(void*) != 4 || sizeof(RecordMap::node_type) == 48, "tree nodes are 48 bytes, as in the game");

// The record cache at 0x74: the records the read path handed out, by key (hash and equality
// are the index maps'; TS3W.exe's find is 0x00AD3F40). A multimap because the epilogue of
// 0x0072BD10 inserts unconditionally, so duplicates pile up (EA quirk, kept).
using RecordCache = eastl::hash_multimap<ResourceKey, void*, IndexKeyHash, IndexKeyEqual, GameAllocator>;
TS3_X86_SIZE(RecordCache, 0x28);
static_assert(sizeof(void*) != 4 || sizeof(RecordCache::value_type) == 24, "a key + record pair is 24 bytes");
static_assert(sizeof(void*) != 4 || sizeof(RecordCache::node_type) == 32, "hash nodes are 32 bytes, as in the game");

struct MemoryDB;

// What Read's `info` argument receives when the key is in the map (the four dwords
// 0x0072BD10 writes).
struct MemoryDBRecordInfo {
    uint32_t unknown0;     // always 0
    uint32_t storedSize;
    uint32_t memSize;
    uint32_t compression;  // 0, or 0xFFFF when storedSize != memSize (not a compression id)
};

// A record the read path hands out (TS3W.exe vtable 0x00FFEBE4, constructor 0x00739A80,
// clone 0x00739B40; 104 bytes — the same size as PackedFileRecord but a different class,
// with the stream at +0x2C instead of +0x30). Partial layout, x86 offsets.
struct MemoryDBReadRecord {
    void* const* vtable;      // 0x00 the game's on x86
    int32_t refCount;         // 0x04 slot 1, TS3W.exe 0x005044C0
    uint32_t recordType;      // 0x08 0x12E4A891 (kPackedFileRecordType)
    uint32_t unknown0C;       // 0x0C never written by the constructors
    ResourceKey key;          // 0x10
    MemoryDB* owner;          // 0x20 the database
    uint32_t accessFlags;     // 0x24 1
    uint32_t unknown28;       // 0x28 0
    io::MemoryStream stream;  // 0x2C the record's data
    uint8_t unknown54[4];     // 0x54 never written here
    uint32_t unknown58;       // 0x58 0
    uint32_t unknown5C;       // 0x5C 0
    uint32_t size;            // 0x60 the stream size at construction
    uint8_t unknown64[4];     // 0x64 never written by the constructors

    // Constructors, on uninitialised memory (x86 builds give the object the game's vtable).
    // TS3W.exe 0x00739A80: a record over `dataSize` bytes (the stream takes `src` itself
    // when usePointer, else copies it).
    void Construct(const void* src, uint32_t dataSize, bool usePointer, const ResourceKey& recordKey,
                   MemoryDB* database);
    // TS3W.exe 0x00739B40: a copy of `other`, sharing its stream data, positioned at 0.
    void ConstructCopy(const MemoryDBReadRecord& other);
};
TS3_X86_OFFSET(MemoryDBReadRecord, key, 0x10);
TS3_X86_OFFSET(MemoryDBReadRecord, owner, 0x20);
TS3_X86_OFFSET(MemoryDBReadRecord, stream, 0x2C);
TS3_X86_OFFSET(MemoryDBReadRecord, size, 0x60);
TS3_X86_SIZE(MemoryDBReadRecord, 104);

// A record opened for writing through the read path (TS3W.exe vtable 0x00FFEC0C,
// constructors 0x00739BE0 empty and 0x00739CD0 over data; 88 bytes, the stream at +0x28).
struct MemoryDBWriteRecord {
    void* const* vtable;      // 0x00 the game's on x86
    int32_t refCount;         // 0x04
    uint32_t recordType;      // 0x08 0x12E4A892 (kPackedFileWriteRecordType)
    uint32_t unknown0C;       // 0x0C never written
    ResourceKey key;          // 0x10
    MemoryDB* owner;          // 0x20
    uint32_t accessFlags;     // 0x24 3 while open
    io::MemoryStream stream;  // 0x28 grows as the game writes
    uint8_t unknown50[8];     // 0x50 never written by the constructors

    // TS3W.exe 0x00739BE0: an empty write record over an empty stream.
    void Construct(const ResourceKey& recordKey, MemoryDB* database);
    // TS3W.exe 0x00739CD0: a write record taking `data` (freed with the record when
    // usePointer); the data allocator is the database's.
    void Construct(const void* src, uint32_t dataSize, bool usePointer, const ResourceKey& recordKey,
                   MemoryDB* database);
};
TS3_X86_OFFSET(MemoryDBWriteRecord, key, 0x10);
TS3_X86_OFFSET(MemoryDBWriteRecord, stream, 0x28);
TS3_X86_SIZE(MemoryDBWriteRecord, 0x58);

// MemoryDB: an in-memory database (vtable 0x00FFD5F8 with the 0x00FFD5E8 interface view at
// +0x004, 24 virtual methods; slot 17 of that view, slot 13 of the main one, reads records).
// Built by the game's constructor 0x0072B8F0. Allocation tags "ResourceSystem/MemoryDB/...".
struct MemoryDB {
    void* const* vtable;                    // 0x000 0x00FFD5F8
    void* const* interfaceVtable;           // 0x004 0x00FFD5E8
    int32_t refCount;                       // 0x008
    uint8_t unknown00C[4];                  // 0x00C 0
    allocator::ICoreAllocator* allocator;   // 0x010 records and temporaries
    const char* dataName;                   // 0x014 the write path's copy tag (not read here)
    uint32_t openFlags;                     // 0x018 the read path requires it nonzero
    uint8_t unknown01C[4];
    thread::Mutex mutex;                    // 0x020 the open flag and the reads
    RecordMap records;                      // 0x050 the data, by key
    RecordCache recordCache;                // 0x074 the records handed out, by key
    IResourceManager* resourceManager;      // 0x09C codecs other than RefPack

    // TS3W.exe 0x0072AFB0: record decompression; the allocator tag (string at 0x00FFD580)
    // is "ResourceLoad/CompressionRefpack", the low 16 bits of `compression` select RefPack.
    bool DecompressRecord(uint32_t compression, const void* src, uint32_t srcLen, uint32_t memSize, void* dst);

    // Virtual slot 17 of the 0x00FFD5E8 view (slot 13 of 0x00FFD5F8), TS3W.exe 0x0072BD10,
    // thiscall ret 0x18. Reads the record for `key`: flags 1 read, 2 raw (the stored bytes,
    // no decompression); mode 1 must not exist, 2/4 must exist, 3/5 must not, 6 picks from
    // the flags; flagByte bit 0 asks for the decompression below; `info` gets the map's
    // sizes when the key is there. With no `outRecord` only the existence is reported.
    // Quirks kept (see the project notes): a missing key creates a write record and an empty map
    // entry; every successful call pushes another cache entry for the key; a raw or cached
    // miss in the cache hands out no record even though the key exists.
    bool Read(const ResourceKey* key, void** outRecord, uint32_t flags, uint32_t mode, uint8_t flagByte,
              MemoryDBRecordInfo* info);
};
TS3_X86_OFFSET(MemoryDB, interfaceVtable, 0x004);
TS3_X86_OFFSET(MemoryDB, refCount, 0x008);
TS3_X86_OFFSET(MemoryDB, allocator, 0x010);
TS3_X86_OFFSET(MemoryDB, dataName, 0x014);
TS3_X86_OFFSET(MemoryDB, openFlags, 0x018);
TS3_X86_OFFSET(MemoryDB, mutex, 0x020);
TS3_X86_OFFSET(MemoryDB, records, 0x050);
TS3_X86_OFFSET(MemoryDB, recordCache, 0x074);
TS3_X86_OFFSET(MemoryDB, resourceManager, 0x09C);
TS3_X86_SIZE(MemoryDB, 0xA0);

}  // namespace ts3::resource
