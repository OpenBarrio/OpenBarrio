// MemoryDB (TS3W.exe vtable 0x00FFD5E8): the in-memory database. Covered: 0x0072AFB0
// (DecompressRecord), the record read path 0x0072BD10 (virtual slot 17 of the 0x00FFD5E8
// view, slot 13 of 0x00FFD5F8) and the four record constructors it runs (0x00739A80,
// 0x00739B40, 0x00739BE0, 0x00739CD0).
#include "ts3/resource/databases.h"

#include <new>

#include "ts3/resource/packed_record.h"  // the record type numbers the cache check uses

namespace ts3::resource {

namespace {

#if defined(_M_IX86)
// The record classes' final vtables (the originals walk FB21E8 -> FB2A0C -> these).
constexpr uintptr_t kReadRecordVtable = 0x00FFEBE4;
constexpr uintptr_t kWriteRecordVtable = 0x00FFEC0C;
#endif

// The records' reference count (slot 1, TS3W.exe 0x005044C0: a locked increment of the
// dword at +4; a plain ++ puts the same value there).
void RecordAddRef(void* record)
{
    ++*reinterpret_cast<int32_t*>(static_cast<uint8_t*>(record) + 4);
}

// TS3W.exe 0x00739A80: a read record over `dataSize` bytes (the original allocates with
// EaNew 104 "ResourceLoad/MemoryDB/RecordRead" and then runs the constructor in place).
MemoryDBReadRecord* NewReadRecord(allocator::ICoreAllocator* coreAllocator, const void* src, uint32_t dataSize,
                                  bool usePointer, const ResourceKey& key, MemoryDB* database)
{
    void* mem = io::EaNew(sizeof(MemoryDBReadRecord), coreAllocator, "ResourceLoad/MemoryDB/RecordRead");
    if (!mem)
        return nullptr;
    auto* record = new (mem) MemoryDBReadRecord;
    record->Construct(src, dataSize, usePointer, key, database);
    return record;
}

// TS3W.exe 0x00739B40: a copy of an already-open read record (EaNew 104, same tag).
MemoryDBReadRecord* CloneReadRecord(const MemoryDBReadRecord& cached, allocator::ICoreAllocator* coreAllocator)
{
    void* mem = io::EaNew(sizeof(MemoryDBReadRecord), coreAllocator, "ResourceLoad/MemoryDB/RecordRead");
    if (!mem)
        return nullptr;
    auto* record = new (mem) MemoryDBReadRecord;
    record->ConstructCopy(cached);
    return record;
}

// TS3W.exe 0x00739BE0: an empty write record (EaNew 88 "ResourceLoad/MemoryDB/RecordWrite").
MemoryDBWriteRecord* NewEmptyWriteRecord(const ResourceKey& key, MemoryDB* database,
                                         allocator::ICoreAllocator* coreAllocator)
{
    void* mem = io::EaNew(sizeof(MemoryDBWriteRecord), coreAllocator, "ResourceLoad/MemoryDB/RecordWrite");
    if (!mem)
        return nullptr;
    auto* record = new (mem) MemoryDBWriteRecord;
    record->Construct(key, database);
    return record;
}

// TS3W.exe 0x00739CD0: a write record taking `data` (EaNew 88, same tag).
MemoryDBWriteRecord* NewWriteRecord(const void* src, uint32_t dataSize, bool usePointer, const ResourceKey& key,
                                    MemoryDB* database, allocator::ICoreAllocator* coreAllocator)
{
    void* mem = io::EaNew(sizeof(MemoryDBWriteRecord), coreAllocator, "ResourceLoad/MemoryDB/RecordWrite");
    if (!mem)
        return nullptr;
    auto* record = new (mem) MemoryDBWriteRecord;
    record->Construct(src, dataSize, usePointer, key, database);
    return record;
}

}  // namespace

// TS3W.exe 0x0072AFB0, thiscall ret 0x14 (5 stack args). The allocator tag is the string
// at 0x00FFD580, "ResourceLoad/CompressionRefpack"; compression 0xFFFF (low 16 bits) makes
// a RefPack codec through this+0x010's allocator, anything else asks this+0x09C's resource
// manager for the codec and owns nothing. The codec must report IsCompressed, its measure
// (dst == null) must equal memSize, and a non-empty decode must then not fail. Quirk kept:
// an empty record (memSize 0, or srcLen 0) skips the copy and reports success.
bool MemoryDB::DecompressRecord(uint32_t compression, const void* src, uint32_t srcLen, uint32_t memSize, void* dst)
{
    return resource::DecompressRecord(allocator, resourceManager, "ResourceLoad/CompressionRefpack", compression, src,
                                      srcLen, memSize, dst);
}

// TS3W.exe 0x00739A80, thiscall ret 0x14: a read record over `dataSize` bytes. The original
// walks the base classes' vtables first; only the last one (0x00FFEBE4) remains. The stream
// at +0x2C takes `src` itself when usePointer (freed with the record), else copies it; the
// reference after the field writes is the stream's slot 1 (TS3W.exe 0x004559F0).
void MemoryDBReadRecord::Construct(const void* src, uint32_t dataSize, bool usePointer,
                                   const ResourceKey& recordKey, MemoryDB* database)
{
#if defined(_M_IX86)
    vtable = reinterpret_cast<void* const*>(kReadRecordVtable);
#else
    vtable = nullptr;
#endif
    refCount = 0;
    recordType = kPackedFileRecordType;
    key = recordKey;
    owner = database;
    accessFlags = 1;
    unknown28 = 0;
    stream.Construct(src, dataSize, usePointer, true, nullptr, nullptr);
    unknown58 = 0;
    unknown5C = 0;
    size = dataSize;
    ++stream.refCount;
}

// TS3W.exe 0x00739B40, thiscall ret 4: a copy of `other` (key and owner included), sharing
// its stream data with one more reference, positioned at 0 — the last two calls are the new
// stream's slot 1 (AddRef) and slot 10 (SetPosition 0 from the beginning).
void MemoryDBReadRecord::ConstructCopy(const MemoryDBReadRecord& other)
{
#if defined(_M_IX86)
    vtable = reinterpret_cast<void* const*>(kReadRecordVtable);
#else
    vtable = nullptr;
#endif
    refCount = 0;
    recordType = kPackedFileRecordType;
    key = other.key;
    owner = other.owner;
    accessFlags = 1;
    unknown28 = 0;
    stream.ConstructCopy(other.stream);
    unknown58 = 0;
    unknown5C = 0;
    size = static_cast<uint32_t>(other.stream.GetSize());
    ++stream.refCount;
    stream.SetPosition(0, io::kPositionTypeBegin);
}

// TS3W.exe 0x00739BE0, thiscall ret 8: an empty write record (vtable 0x00FFEC0C), the
// stream at +0x28 over nothing, with SetGrowthProperty(1, 1.0f) (0x004DEAC0) so it grows as
// the game writes. The fields at +0x50/+0x54 are never written (EA quirk, kept).
void MemoryDBWriteRecord::Construct(const ResourceKey& recordKey, MemoryDB* database)
{
#if defined(_M_IX86)
    vtable = reinterpret_cast<void* const*>(kWriteRecordVtable);
#else
    vtable = nullptr;
#endif
    refCount = 0;
    recordType = kPackedFileWriteRecordType;
    key = recordKey;
    owner = database;
    accessFlags = 3;
    stream.Construct(nullptr, 0, false, true, nullptr, nullptr);
    ++stream.refCount;
    stream.resizable = true;
}

// TS3W.exe 0x00739CD0, thiscall ret 0x14: the empty constructor's header, then SetData
// (0x004DEC90) with `src` (freed with the record when usePointer). The data allocator is
// the owner's slot 19 — the shared getter 0x00D1AA10 that returns [this+0x10], the
// database's own allocator.
void MemoryDBWriteRecord::Construct(const void* src, uint32_t dataSize, bool usePointer, const ResourceKey& recordKey,
                                    MemoryDB* database)
{
    Construct(recordKey, database);
    stream.SetData(src, dataSize, usePointer, true, database->allocator);
}

// TS3W.exe 0x0072BD10, thiscall ret 0x18 (6 stack args): the record for `key`. With no
// `outRecord` only the map lookup runs (and `info` fills); otherwise the caches are asked
// first and the data paths build a record from the map entry. Quirks kept (EA, see
// the project notes): the cache-hit branch refuses raw requests and cached write records (false
// even though the key exists); a missing key creates an empty write record AND an empty map
// entry; the epilogue pushes another cache entry on every success, duplicates included; the
// raw copy branch sizes the stream at memSize even over stored bytes.
bool MemoryDB::Read(const ResourceKey* key, void** outRecord, uint32_t flags, uint32_t mode, uint8_t flagByte,
                    MemoryDBRecordInfo* info)
{
    if (!flags)
        return false;
    mutex.Lock();
    if (!openFlags) {
        mutex.Unlock();
        return false;
    }
    const auto it = records.find(*key);
    const bool found = it != records.end();
    if (found && info) {
        info->unknown0 = 0;
        info->storedSize = it->second.storedSize;
        info->memSize = it->second.memSize;
        info->compression = it->second.storedSize != it->second.memSize ? 0xFFFFu : 0u;
    }
    if (!outRecord) {
        mutex.Unlock();
        return found;
    }
    if (mode == 6)
        mode = (flags & 2) ? ((flags & 1) ? 4u : 2u) : 3u;
    bool reject = found ? mode == 1 : (mode == 3 || mode == 5 || !(flags & 2));
    if (!reject && (flags & openFlags) != flags)
        reject = true;
    if (reject) {
        mutex.Unlock();
        return false;
    }

    void* record = nullptr;
    const auto cached = recordCache.find(*key);
    if (cached != recordCache.end()) {
        // The key's record is already open: hand out a copy of it — read requests only, and
        // only when the cached one is a read record.
        if (!(flags & 2)) {
            auto* open = static_cast<MemoryDBReadRecord*>(cached->second);
            if (open->recordType == kPackedFileRecordType)
                record = CloneReadRecord(*open, allocator);
        }
        if (!record) {
            mutex.Unlock();
            return false;
        }
    } else if (!found) {
        // Nothing known: a write record over an empty stream, and an empty entry in the
        // map so the key reports as present afterwards (EA quirk).
        record = NewEmptyWriteRecord(*key, this, allocator);
        if (!record) {
            mutex.Unlock();
            return false;
        }
        records.insert(RecordMap::value_type(*key, MemoryDBRecordData{nullptr, 0, 0}));
    } else {
        const uint32_t stored = it->second.storedSize;
        const uint32_t memSize = it->second.memSize;
        const void* data = it->second.data;
        if (flags & 2) {
            // Raw: the stored bytes themselves, decompressed into a temporary only when the
            // caller asks (flagByte bit 0) and the sizes differ.
            if (memSize != stored && (flagByte & 1)) {
                void* buffer = allocator->Alloc(memSize, "ResourceLoad/MemoryDB/Raw", 0);
                if (!buffer) {
                    mutex.Unlock();
                    return false;
                }
                if (!DecompressRecord(0xFFFF, data, stored, memSize, buffer)) {
                    allocator->Free(buffer, 0);
                    mutex.Unlock();
                    return false;
                }
                record = NewWriteRecord(buffer, memSize, true, *key, this, allocator);
                if (!record) {
                    allocator->Free(buffer, 0);
                    mutex.Unlock();
                    return false;
                }
            } else {
                // EA quirk, kept: with no decompression the stream is still sized at memSize
                // even though `data` only holds stored bytes.
                record = NewWriteRecord(data, memSize, false, *key, this, allocator);
                if (!record) {
                    mutex.Unlock();
                    return false;
                }
            }
        } else if (memSize != stored && (flagByte & 1)) {
            // Decompressed record data: a temporary holds the decode, which the record takes
            // over (freed with it).
            void* buffer = allocator->Alloc(memSize, "ResourceLoad/MemoryDB/DecompressedRecordData", 0);
            if (!buffer) {
                mutex.Unlock();
                return false;
            }
            if (!DecompressRecord(0xFFFF, data, stored, memSize, buffer)) {
                allocator->Free(buffer, 0);
                mutex.Unlock();
                return false;
            }
            record = NewReadRecord(allocator, buffer, memSize, true, *key, this);
            if (!record) {
                allocator->Free(buffer, 0);
                mutex.Unlock();
                return false;
            }
        } else {
            // EA quirk, kept: with no decompression asked for, the read record is built over
            // the stored bytes and sized at storedSize (compressed contents in a "decompressed"
            // request when the caller skipped flagByte bit 0).
            record = NewReadRecord(allocator, data, stored, false, *key, this);
            if (!record) {
                mutex.Unlock();
                return false;
            }
        }
    }

    // The epilogue (TS3W.exe 0x0072C0AD): one reference for the cache, the entry (pushed
    // unconditionally — another read of a key already cached adds a duplicate node, EA quirk
    // kept), one more for the caller, then the result.
    RecordAddRef(record);
    recordCache.insert(RecordCache::value_type(*key, record));
    RecordAddRef(record);
    *outRecord = record;
    mutex.Unlock();
    return true;
}

}  // namespace ts3::resource
