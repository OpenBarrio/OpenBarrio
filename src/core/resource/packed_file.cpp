#include "ts3/resource/packed_file.h"

#include "ts3/resource/package_header.h"

#include "ts3/io/memory_stream.h"
#include "ts3/resource/packed_record.h"

#include <cstring>
#include <new>

namespace ts3::resource {

io::size_type DatabasePackedFile::ReadAtPosition(void* dst, uint32_t size)
{
    if (!memoryData)
        return stream->Read(dst, size);
    uint32_t count = size;
    if (memoryPosition + size > memorySize)  // 32-bit arithmetic, as in the original
        count = memorySize - memoryPosition;
    std::memcpy(dst, memoryData + memoryPosition, count);
    memoryPosition += count;
    return count;
}

bool DatabasePackedFile::ReadRaw(void* dst, uint32_t offset, uint32_t size)
{
    fileMutex.Lock();
    bool ok = false;
    if (streamFlags) {
        const uint32_t position = offset + headerOffset;
        bool positioned;
        if (memoryData) {
            positioned = position < memorySize;
            if (positioned)
                memoryPosition = position;
        } else {
            positioned = stream->SetPosition(position, io::kPositionTypeBegin);
        }
        if (positioned)
            ok = ReadAtPosition(dst, size) != io::kSizeTypeError;
    }
    fileMutex.Unlock();
    return ok;
}

bool DatabasePackedFile::DecompressRecord(uint32_t compression, const void* src, uint32_t srcLen, uint32_t memSize,
                                          void* dst)
{
    return resource::DecompressRecord(allocator, resourceManager, "ResourceLoad/PackedFile/CompressionRefpack",
                                      compression, src, srcLen, memSize, dst);
}

bool DatabasePackedFile::ReadAndDecompress(uint32_t compression, uint32_t offset, uint32_t size, uint32_t memSize,
                                           void* dst)
{
    if (offset == 0)
        return false;
    void* raw = allocator->Alloc(size, "ResourceLoad/PackedFile/Raw", 0);
    if (!raw)
        return false;
    const bool ok = ReadRaw(raw, offset, size) && DecompressRecord(compression, raw, size, memSize, dst);
    allocator->Free(raw, 0);
    return ok;
}

bool DatabasePackedFile::ReadRecordRaw(const ResourceKey* key, void* dst, uint32_t maxSize)
{
    recordMutex.Lock();
    bool ok = false;
    if (const RecordInfo* record = index->GetRecord(key)) {
        const uint32_t size = maxSize > record->size ? record->size : maxSize;
        if (!dst || !size)
            ok = true;
        else if (openFlags || EnsureOpen())
            ok = ReadRaw(dst, record->offset, size);
    }
    recordMutex.Unlock();
    return ok;
}

bool DatabasePackedFile::EnsureOpen()
{
    if (openFlags)
        return true;
    if (!reopenFlags)
        return false;
    // Through the vtable (slot 6), as the original.
    const bool ok = CallGameVirtual<bool, 6>(this, reopenFlags, 0u, 0u);
    // The original tests the low byte of the flags after the failed attempt.
    if (!ok && (static_cast<uint8_t>(openFlags) & 3) == 3)
        return CallGameVirtual<bool, 6>(this, 1u, 0u, 0u);
    return ok;
}

}  // namespace ts3::resource

namespace ts3::resource {

namespace {

constexpr const char* kRecordRead = "ResourceLoad/PackedFile/RecordRead";
constexpr const char* kRecordWrite = "ResourceLoad/PackedFile/RecordWrite";
constexpr uint32_t kWriteRecordSize = 648;

}  // namespace

PackedFileRecord* DatabasePackedFile::CopyRecord(uint32_t, PackedFileRecord* source, const ResourceKey* key)
{
    auto* record = static_cast<PackedFileRecord*>(io::EaNew(sizeof(PackedFileRecord), allocator, kRecordRead));
    if (!record)
        return nullptr;
    record->ConstructCopy(*source, *key, this);
    return record;
}

void* DatabasePackedFile::CreateRecordOverMemory(uint32_t flags, void* data, uint32_t size, const ResourceKey* key)
{
    if (flags & 2) {
        auto* record = static_cast<PackedFileWriteRecord*>(io::EaNew(kWriteRecordSize, allocator, kRecordWrite));
        if (!record)
            return nullptr;
        record->ConstructOverMemory(data, size, true, *key, this);  // the original passes usePointer 1
        return record;
    }
    auto* record = static_cast<PackedFileRecord*>(io::EaNew(sizeof(PackedFileRecord), allocator, kRecordRead));
    if (!record)
        return nullptr;
    record->ConstructOverMemory(data, size, true, *key, this);
    return record;
}

void* DatabasePackedFile::CreateRecordOverRegion(uint32_t flags, uint32_t offset, uint32_t size, const ResourceKey* key)
{
    if (flags & 2) {
        auto* record = static_cast<PackedFileWriteRecord*>(io::EaNew(kWriteRecordSize, allocator, kRecordWrite));
        if (!record)
            return nullptr;
        record->ConstructOverRegion(offset, size, *key, this);
        return record;
    }
    auto* record = static_cast<PackedFileRecord*>(io::EaNew(sizeof(PackedFileRecord), allocator, kRecordRead));
    if (!record)
        return nullptr;
    record->ConstructOverRegion(offset, size, *key, this);
    return record;
}

}  // namespace ts3::resource

namespace ts3::resource {

namespace {

constexpr uint32_t kWriteRecordType = 316975250;

}  // namespace

bool DatabasePackedFile::CloseRecord(void* record)
{
    bool ok = false;
    recordMutex.Lock();
    if (openFlags) {
        // The record's own GetKey (its slot 4); the key lives in the record.
        const ResourceKey* key = CallGameVirtual<const ResourceKey*, 4>(static_cast<PackedFileRecord*>(record));
        for (auto it = openRecords.find(*key); it != openRecords.end(); ++it) {
            if (it->first.instanceLow != key->instanceLow || it->first.type != key->type ||
                it->first.group != key->group || it->first.instanceHigh != key->instanceHigh)
                break;
            if (it->second != record)
                continue;
            ok = true;
            if (static_cast<PackedFileRecord*>(record)->recordType == kWriteRecordType)
                ok = FlushWriteRecord(record, key);
            CallGameVirtual<void, 9>(static_cast<PackedFileRecord*>(record));  // Close
            CallGameVirtual<void, 2>(static_cast<PackedFileRecord*>(record));  // Release
            openRecords.erase(it);
            break;
        }
        if (closeWhenIdle && openRecords.empty())
            CloseFile();
    }
    recordMutex.Unlock();
    return ok;
}

}  // namespace ts3::resource

namespace ts3::resource {

namespace {

constexpr uint32_t kMaxCompressInput = 0x26259FF;  // 40,000,000 - 1: sizes past this are stored raw
constexpr const char* kPackedRecordTag = "ResourceLoad/PackedFile/PackedRecord";
constexpr const char* kRefpackCodecTag = "ResourceLoad/PackedFile/CompressionRefpack";
constexpr uint32_t kAutoFlagsLargeInput = 250000;  // the auto-flag rule switches to 1 past this size
constexpr float kRatioAutoAccept = 0.001f;         // TS3W.exe 0x01010960

}  // namespace

// TS3W.exe 0x004A7030 (__thiscall, ret 0x14; the only caller is CompressForWrite): picks
// the codec for a record being written and runs it on `src`. Sizes outside [1, 0x26259FF]
// are never compressed: with a caller buffer (*bufOut != null) the raw bytes are copied
// into it when *sizeOut (its capacity) allows, else the call fails; without one a fresh
// buffer of exactly `size` is allocated and filled, and the call succeeds even for size 0
// or a failed allocation - the copy runs on the null pointer and crashes (EA bug kept).
// Otherwise *typeOut is cleared and the resource manager (0x390) is asked for its codecs
// (its slot 30: a null-terminated array; an empty list falls back to RefPack) and, when it
// has one, for a policy (slot 28: 1 = "ratio rule", with a per-call limit written to the
// float out-param; 2 = "smallest probe wins"; any other value takes no codec) and the
// parallel type-id array (slot 31). Each candidate is probed with Compress(src, size,
// null, 0, 2). Policy 1 takes the first whose probe/size stays strictly under the limit -
// any probe at all when the limit is below the 0.001 constant at 0x01010960. Policy 2
// takes the smallest probe, first on a tie, where a 0 probe neither ends the search nor
// blocks a later 0 from replacing it. No suitable codec fails the call (outputs stay
// cleared, *sizeOut untouched); a policy of 0 or an empty list falls back to a RefPack
// codec made here through the game's constructor 0x4EBFD0, whose Compress is the game's
// encoder (not rewritten yet). That constructor is called without checking the 8-byte
// allocation, and the probe then runs on the null pointer: a failed allocation crashes,
// as in the original. The winner compresses into *bufOut - a fresh buffer of the probe
// size when the caller passed null (nothing happens when the probe was 0 or the
// allocation failed), else the caller's buffer with *sizeOut as its capacity - with flags
// from compressionMode (0x20): 1 gives 0x10001 for RefPack and 1 otherwise, 3 gives 2,
// anything else 1 past 250,000 bytes and 2 below. The result lands in *sizeOut; -1
// (0xFFFFFFFF) resets it to 0 and frees a buffer this function allocated. At the end a
// codec whose type came out 0xFFFF - RefPack itself, or a manager codec with that id -
// is destroyed here; other manager codecs stay with the manager.
bool DatabasePackedFile::ChooseWriteCodec(const void* src, uint32_t size, void** bufOut, uint32_t* sizeOut,
                                          uint16_t* typeOut)
{
    const bool callerBuffer = *bufOut != nullptr;
    if (size - 1 > kMaxCompressInput - 1) {  // unsigned: 0 and everything past 0x26259FF
        if (!callerBuffer) {
            *sizeOut = size;
            void* copy = allocator->Alloc(size, kPackedRecordTag, 0);
            *bufOut = copy;
            std::memcpy(copy, src, size);  // allocation not checked, as in the original
            return true;
        }
        if (*sizeOut < size) {
            *sizeOut = 0;
            return false;
        }
        *sizeOut = size;
        std::memcpy(*bufOut, src, size);
        return true;
    }
    bool ok = false;
    *typeOut = kCompressionNone;
    float ratioLimit = 0.0f;
    ICompressionCodec* codec = nullptr;
    uint32_t compressedSize = 0;
    bool fromManager = false;
    if (resourceManager && *CallGameVirtual<ICompressionCodec**, 30>(resourceManager)) {
        const int32_t policy = CallGameVirtual<int32_t, 28>(resourceManager, &ratioLimit);
        if (policy) {
            fromManager = true;
            ICompressionCodec* const* codecs = CallGameVirtual<ICompressionCodec**, 30>(resourceManager);
            const uint16_t* types = CallGameVirtual<uint16_t*, 31>(resourceManager);
            int32_t best = -1;
            for (int32_t i = 0; codecs[i]; ++i) {
                const uint32_t probe = codecs[i]->Compress(src, size, nullptr, 0, 2);
                if (policy == 1) {
                    if (kRatioAutoAccept > ratioLimit ||
                        static_cast<double>(ratioLimit) > static_cast<double>(probe) / static_cast<double>(size)) {
                        best = i;
                        compressedSize = probe;
                        break;
                    }
                } else if (policy == 2 && (compressedSize == 0 || probe < compressedSize)) {
                    best = i;
                    compressedSize = probe;
                }
            }
            if (best >= 0) {
                codec = codecs[best];
                *typeOut = types[best];
            }
        }
    }
    if (!fromManager) {
        void* memory = allocator->Alloc(sizeof(RefPackCodec), kRefpackCodecTag, 0, 4, 0);
        // Through the game's constructor, so the codec's Compress stays the game's encoder.
        // Unchecked allocation: the probe below runs on null when it failed, as the original.
        codec = memory
                    ? CallGameMethod<ICompressionCodec*, 0x004EBFD0>(static_cast<ICompressionCodec*>(memory), allocator)
                    : nullptr;
        compressedSize = codec->Compress(src, size, nullptr, 0, 1);
        *typeOut = kCompressionRefPack;
    }
    if (!callerBuffer && compressedSize) {
        *sizeOut = compressedSize;
        *bufOut = allocator->Alloc(compressedSize, kPackedRecordTag, 0);
    }
    if (codec && *bufOut) {
        uint32_t flags;
        if (compressionMode == 1)
            flags = *typeOut == kCompressionRefPack ? 0x10001 : 1;
        else if (compressionMode == 3)
            flags = 2;
        else
            flags = size > kAutoFlagsLargeInput ? 1 : 2;
        const uint32_t written = codec->Compress(src, size, *bufOut, *sizeOut, flags);
        *sizeOut = written;
        if (written == 0xFFFFFFFFu) {  // the codec's -1
            *sizeOut = 0;
            if (!callerBuffer && *bufOut) {
                allocator->Free(*bufOut, 0);
                *bufOut = nullptr;
            }
        } else {
            ok = true;
        }
    }
    if (*typeOut == kCompressionRefPack && codec) {
        codec->~ICompressionCodec();
        allocator->Free(codec, 0);
    }
    return ok;
}

// TS3W.exe 0x004A82C0: compresses `size` bytes of `src` for a record being written. The
// codec choice (0x004A7030, ChooseWriteCodec) is rewritten with this; the decision kept
// here: nothing outside [50, 16000000] bytes, or with compressionMode (0x20) off, is
// compressed, and the compressed form is dropped again when its size/raw ratio is not
// strictly under the float at 0x2E4.
bool DatabasePackedFile::CompressForWrite(const void* src, uint32_t size, void** bufOut, uint32_t* sizeOut,
                                          uint16_t* typeOut)
{
    *bufOut = nullptr;
    *sizeOut = 0;
    if (size < 50 || !compressionMode || size > 16000000u)
        return false;
    if (!ChooseWriteCodec(src, size, bufOut, sizeOut, typeOut))
        return false;
    // The original divides the two fild values on the x87 stack with no store in between
    // (the quotient keeps its excess precision) and compares with FCOMIP + jbe: equal (and
    // NaN) rejects; the FSTP after FCOMIP pops the already-empty stack and leaves the
    // compare flags alone. Double is the nearest C type to that arithmetic.
    const double ratio = static_cast<double>(*sizeOut) / static_cast<double>(size);
    if (static_cast<double>(maxCompressRatio) > ratio)
        return true;
    if (*bufOut)
        allocator->Free(*bufOut, 0);
    *bufOut = nullptr;
    *sizeOut = 0;
    return false;
}

// TS3W.exe 0x004A8910: writes a closed write record's data into the package (CloseRecord
// calls it under the record lock). Needs write access (openFlags & 2). The key's old index
// entry goes first (its region freed for reuse), so a flush that fails all write attempts
// leaves the key out of the index (sic). The new bytes go in compressed when the package
// compresses (0x20) and the size is in [50, 16000000] (CompressForWrite; a failed
// SetPosition, short Read or rejected compression falls through to the raw write), else
// through WriteOut. An empty record only updates the index (true).
bool DatabasePackedFile::FlushWriteRecord(void* record, const ResourceKey* key)
{
    if (!(openFlags & 2))
        return false;
    CallGameVirtual<void, 39>(this);  // MakeIndexModifiable, through the vtable as in the original
    RecordInfo removed{};
    if (index->RemoveRecord(key, &removed)) {
        FreeRegion(removed.offset, removed.size, !removed.valid);
        indexChanged = true;
    }
    auto& rec = *static_cast<PackedFileWriteRecord*>(record);
    const uint32_t size = static_cast<uint32_t>(rec.GetSize());
    if (!size) {
        RecordInfo empty{};
        empty.valid = 1;
        index->SetRecord(key, &empty);
        indexChanged = true;
        return true;
    }
    bool result = false;
    if (compressionMode && size - 50u <= 16000000u - 50u) {
        // The two zeroed bytes past the data are lookahead room for the encoder.
        auto* buffer = static_cast<uint8_t*>(allocator->Alloc(size + 16, "ResourceLoad/PackedFile/RecordData", 0));
        if (buffer) {
            buffer[size] = 0;
            buffer[size + 1] = 0;
            void* compressed = nullptr;
            uint32_t compressedSize = 0;
            uint16_t compressedType = 0;
            if (rec.SetPosition(0, io::kPositionTypeBegin) && rec.Read(buffer, size) == size &&
                CompressForWrite(buffer, size, &compressed, &compressedSize, &compressedType)) {
                uint32_t offset = 0;
                if (AllocateRegion(&offset, compressedSize, false)) {
                    if (WriteRaw(compressed, offset, compressedSize)) {
                        RecordInfo info{offset, compressedSize, size, compressedType, 1};
                        index->SetRecord(key, &info);
                        indexChanged = true;
                        result = true;
                    } else {
                        FreeRegion(offset, compressedSize, true);
                    }
                }
                allocator->Free(compressed, 0);
            }
            allocator->Free(buffer, 0);
        }
    }
    if (!result) {
        // The raw write, and the fallback when compression was skipped or failed. Only a
        // real write record goes in (sic: the compressed path has no such check).
        uint32_t offset = 0;
        if (AllocateRegion(&offset, size, false)) {
            if (rec.recordType == kPackedFileWriteRecordType && rec.WriteOut(offset, size)) {
                RecordInfo info{offset, size, size, 0, 1};
                index->SetRecord(key, &info);
                indexChanged = true;
                result = true;
            } else {
                FreeRegion(offset, size, true);
            }
        }
    }
    return result;
}

}  // namespace ts3::resource

namespace ts3::resource {

bool DatabasePackedFile::ReadRecord(const ResourceKey* key, void** out, uint32_t flags, uint32_t mode, uint32_t options,
                                    uint32_t* info)
{
    void* record = nullptr;
    if (flags & 3) {
        recordMutex.Lock();
        if (!openFlags) {
            EnsureOpen();
            if (!openFlags || !index) {
                recordMutex.Unlock();
                return false;
            }
        }
        const RecordInfo* r = index->GetRecord(key);
        const bool found = r != nullptr;
        if (r && info) {
            info[0] = r->offset;
            info[1] = r->size;
            info[2] = r->memSize;
            info[3] = r->compression;
        }
        if (!out) {
            recordMutex.Unlock();
            return found;
        }
        const bool deleted = r && r->memSize == 0xFFFFFFFFu && r->size == 1;
        uint32_t m = mode;
        if (m == 6)
            m = (flags & 2) ? ((flags & 1) ? 4 : 2) : 3;
        const bool reject = deleted || (r ? m == 1 : (m == 3 || m == 5));
        if (reject || (flags & openFlags) != flags) {
            recordMutex.Unlock();
            return false;
        }
        const bool create = m == 2 || m == 5;
        void* buffer = nullptr;   // a record over memory takes it over
        bool addToMap = false;
        if (!found) {
            // A new, empty record (write modes): the package and its index learn about it.
            record = CreateRecordOverRegion(flags, 0, 0, key);
            if (record) {
                CallGameVirtual<void, 39>(this);
                RecordInfo empty{};
                index->SetRecord(key, &empty);
                addToMap = true;
            }
        } else if (flags & 2) {
            if (openRecords.find(*key) == openRecords.end()) {
                if (create) {
                    record = CreateRecordOverRegion(flags, 0, 0, key);
                    if (record) {
                        CallGameVirtual<void, 39>(this);
                        CallGameVirtual<void, 16>(this, key);
                        RecordInfo empty{};
                        index->SetRecord(key, &empty);
                        addToMap = true;
                    }
                } else if (r->compression && (options & 1)) {
                    buffer = allocator->Alloc(r->memSize, "ResourceLoad/PackedFile/Raw", 0);
                    if (buffer) {
                        if (ReadAndDecompress(r->compression, r->offset, r->size, r->memSize, buffer))
                            record = CreateRecordOverMemory(flags, buffer, r->memSize, key);
                        if (!record)
                            allocator->Free(buffer, 0);
                        addToMap = record != nullptr;
                    }
                } else {
                    record = CreateRecordOverRegion(flags, r->offset, r->size, key);
                    addToMap = record != nullptr;
                }
            }
        } else {
            auto open = openRecords.find(*key);
            if (open == openRecords.end()) {
                if (r->compression && (options & 1)) {
                    buffer = allocator->Alloc(r->memSize, "ResourceLoad/PackedFile/RecordData", 0);
                    if (buffer) {
                        if (ReadAndDecompress(r->compression, r->offset, r->size, r->memSize, buffer))
                            record = CreateRecordOverMemory(flags, buffer, r->memSize, key);
                        if (!record)
                            allocator->Free(buffer, 0);
                    }
                } else if (r->size > 0x200 || (options & 2)) {
                    record = CreateRecordOverRegion(flags, r->offset, r->size, key);
                } else {
                    buffer = allocator->Alloc(r->size, "ResourceLoad/PackedFile/RecordData", 0);
                    if (buffer) {
                        if (ReadRaw(buffer, r->offset, r->size))
                            record = CreateRecordOverMemory(flags, buffer, r->size, key);
                        if (!record)
                            allocator->Free(buffer, 0);
                    }
                }
            } else {
                auto* existing = open->second;
                if (existing->recordType == kPackedFileRecordType) {
                    if (!existing->offset && !existing->unknown2C)
                        record = CopyRecord(flags, existing, key);
                    else
                        record = CreateRecordOverRegion(flags, r->offset, r->size, key);
                }
            }
            addToMap = record != nullptr;
        }
        if (record && addToMap) {
            CallGameVirtual<void, 1>(static_cast<PackedFileRecord*>(record));  // the map's reference
            openRecords.insert({*key, static_cast<PackedFileRecord*>(record)});
        }
        recordMutex.Unlock();
        if (record)
            CallGameVirtual<void, 1>(static_cast<PackedFileRecord*>(record));  // the caller's reference
    }
    if (out)
        *out = record;
    return record != nullptr;
}

}  // namespace ts3::resource

namespace ts3::resource {

bool DatabasePackedFile::SeekInFile(uint32_t position)
{
    if (!memoryData)
        return stream->SetPosition(position, io::kPositionTypeBegin);
    if (position >= memorySize)
        return false;
    memoryPosition = position;
    return true;
}

bool DatabasePackedFile::LoadIndex()
{
    bool ok = false;
    if (index && index->Count())
        indexCount = index->Count();
    if (!indexCount) {
        if (!index)
            index = CallGameVirtual<IPackageIndex*, 25>(this, 3u);
        if (!index->Count())
            CallGameVirtual<void, 40>(this);
        return true;
    }
    if (!index)
        index = CallGameVirtual<IPackageIndex*, 25>(this, 3u);
    if (!index)
        return false;
    if (!index->Count()) {
        // The index bytes, zero-filled first, in a vector named like the original's.
        GameVector<uint8_t> data(indexSize, uint8_t{0},
                                 GameAllocator("ResourceSystem/ResourceMan/DPF", io::DefaultAllocatorDirect()));
        fileMutex.Lock();
        if (SeekInFile(headerOffset + indexOffset))
            ok = ReadAtPosition(data.data(), indexSize) == indexSize;
        fileMutex.Unlock();
        if (ok) {
            ok = index->Load(data.data(), indexSize, 1);
            if (ok && indexObserver)
                ok = CallGameVirtual<bool, 0>(indexObserver, this, static_cast<void*>(data.data()), indexSize);
        }
        if (!ok)
            return false;
    }
    if (CallGameVirtual<bool, 37>(this) || keepInvalidIndex) {
        if (openFlags & 2)
            CallGameVirtual<void, 40>(this);
        indexChanged = false;
        return true;
    }
    return false;
}

}  // namespace ts3::resource

namespace ts3::resource {

namespace {

// Set while ReadHeader retries from a scanned header offset (the game's global, in x86).
bool& HeaderRetryInProgress()
{
#if defined(_M_IX86)
    return *reinterpret_cast<bool*>(static_cast<uintptr_t>(0x011C78D4));
#else
    static bool retrying = false;
    return retrying;
#endif
}

}  // namespace

bool DatabasePackedFile::ReadHeader()
{
    PackageHeader header{};
    fileMutex.Lock();
    const bool read = SeekInFile(headerOffset) && ReadAtPosition(&header, sizeof header) == sizeof header;
    fileMutex.Unlock();
    if (!read)
        return false;
    userVersionMajor = header.unknown0C[0];
    userVersionMinor = header.unknown0C[1];
    createdTime[0] = header.unknown0C[3];
    createdTime[1] = 0;
    modifiedTime[0] = header.unknown0C[4];
    modifiedTime[1] = 0;
    indexMajorVersion = header.unknown0C[5];
    indexOffset = header.indexOffsetLow ? header.indexOffsetLow : header.indexOffsetV1;
    indexCount = header.indexCount;
    indexSize = header.indexSize;
    if (CallGameVirtual<bool, 34>(this, static_cast<const void*>(&header)))
        return true;
    const auto found = CallGameFunction<uint32_t, 0x004AE870>(path);
    if (found == 0xFFFFFFFFu) {
        headerOffset = 0;
        return false;
    }
    headerOffset = found;
    bool& retrying = HeaderRetryInProgress();
    if (retrying)
        return true;  // EA bug, kept: the nested retry failed validation too
    retrying = true;
    const bool ok = CallGameVirtual<bool, 31>(this);
    retrying = false;
    return ok;
}

}  // namespace ts3::resource

namespace ts3::resource {

uint32_t DatabasePackedFile::GetFileSize()
{
    fileMutex.Lock();
    const auto fileSize = memoryData ? memorySize : static_cast<uint32_t>(stream->GetSize());
    fileMutex.Unlock();
    return fileSize;
}

bool DatabasePackedFile::NewHeader()
{
    userVersionMajor = newUserVersionMajor;
    userVersionMinor = newUserVersionMinor;
    createdTime[0] = createdTime[1] = 0;
    modifiedTime[0] = modifiedTime[1] = 0;
    indexMajorVersion = 0;
    indexOffset = 0;
    indexCount = 0;
    indexSize = 0;
    headerDirty = true;
    return CallGameVirtual<bool, 33>(this) && SetFileSize(sizeof(PackageHeader));
}

bool DatabasePackedFile::WriteHeader()
{
    if (!(openFlags & 2))
        return false;
    if (!memoryData && stream->GetState() == io::kStateNotOpen)
        return false;
    if (!headerDirty)
        return true;
    const bool past2GB = static_cast<int32_t>(indexOffset) < 0;
    PackageHeader header{};
    header.magic = kDbpfMagic;
    header.majorVersion = 2;
    header.unknown0C[0] = newUserVersionMajor;
    header.unknown0C[1] = newUserVersionMinor;
    header.unknown0C[5] = indexMajorVersion;
    header.indexCount = indexCount;
    header.indexOffsetV1 = past2GB ? indexOffset : 0;
    header.indexSize = indexSize;
    header.indexVersion = 3;
    header.indexOffsetLow = indexOffset;
    header.indexOffsetHigh = past2GB ? 0xFFFFFFFFu : 0;
    bool written = false;
    fileMutex.Lock();
    if (SeekInFile(headerOffset) && !memoryData)
        written = stream->Write(&header, sizeof header);
    fileMutex.Unlock();
    if (!written)
        return false;
    headerDirty = false;
    return true;
}

bool DatabasePackedFile::SetFileSize(uint32_t size)
{
    fileMutex.Lock();
    const bool ok = !memoryData && stream->SetSize(size);
    fileMutex.Unlock();
    return ok;
}

bool DatabasePackedFile::CloseFile()
{
    fileMutex.Lock();
    if (memoryData)
        memoryPosition = 0;
    if (stream)
        stream->Close();
    if (stream && stream == reinterpret_cast<io::IStream*>(ownFileStream)) {
        io::IStream* own = stream;
        stream = nullptr;
        own->Release();
    }
    openFlags = 0;
    streamFlags = 0;
    fileMutex.Unlock();
    return true;
}

bool DatabasePackedFile::ValidateIndex()
{
    return index->Validate(0, GetFileSize());
}

}  // namespace ts3::resource

namespace ts3::resource {

bool DatabasePackedFile::ReadHeaderAndIndex()
{
    return CallGameVirtual<bool, 31>(this) && CallGameVirtual<bool, 35>(this);
}

}  // namespace ts3::resource

namespace ts3::resource {

bool DatabasePackedFile::Open(uint32_t flags, uint32_t mode, bool reopen)
{
    if (reopen)
        reopenFlags = flags;
    if (memoryData) {
        if ((flags & 3) != 1 || (mode != 3 && mode != 6) || !CallGameVirtual<bool, 30>(this))
            return false;
        fileMutex.Lock();
        openFlags = flags;
        streamFlags = openFlags;
        fileMutex.Unlock();
        return true;
    }
    if (!opensFile || openFlags)
        return false;
    auto* own = reinterpret_cast<io::IStream*>(ownFileStream);
    bool ok;
    if (reopen && index) {
        ok = true;
    } else {
        openFlags = flags & 2 ? 3 : 1;
        fileMutex.Lock();
        streamFlags = openFlags;
        fileMutex.Unlock();
        if (stream) {
            ok = (static_cast<uint32_t>(stream->GetAccessFlags()) & openFlags) == openFlags;
        } else {
            uint32_t creation = mode;
            if (flags & 2) {
                if (mode == 6)
                    creation = flags & 1 ? 4 : 2;
            } else if (mode == 6) {
                creation = 3;
            }
            // The stream pointer is reference counted (TS3W.exe 0x007C8D00).
            if (stream != own) {
                own->AddRef();
                io::IStream* old = stream;
                stream = own;
                if (old)
                    old->Release();
            }
            CallGameVirtual<void, 15>(own, path);
            ok = CallGameVirtual<bool, 19>(own, openFlags, creation, 1u, 2u);
        }
        if (!ok) {
            CloseFile();
            return false;
        }
    }
    if (own->GetAccessFlags() && !GetFileSize()) {
        if (!(openFlags & 2)) {
            CloseFile();
            return false;
        }
        ok = CallGameVirtual<bool, 32>(this);  // a new header
    }
    if (!ok) {
        CloseFile();
        return false;
    }
    if (stream || own->GetAccessFlags()) {
        ok = CallGameVirtual<bool, 30>(this);
        if (!ok && index) {
            index->Clear();
            if (resetInvalidOnWrite && (openFlags & 2)) {
                ok = CallGameVirtual<bool, 32>(this);
                if (!ok) {
                    CloseFile();
                    return false;
                }
                CallGameVirtual<void, 40>(this);
                indexChanged = true;
            }
        }
        if (!ok) {
            CloseFile();
            return false;
        }
    }
    if (reopen && own->GetAccessFlags())
        CloseFile();  // quirk kept
    return true;
}

}  // namespace ts3::resource

namespace ts3::resource {

void FreeSpaceMap::Reset(int32_t newTotal)
{
    blocks.clear();
    total = newTotal;
}

void FreeSpaceMap::AddFree(int32_t start, int32_t size, bool keepAtEnd)
{
    const int32_t end = start + size;
    auto next = blocks.lower_bound(start);
    if (next != blocks.begin()) {
        const auto before = eastl::prev(next);
        if (before->first + before->second == start) {
            start = before->first;
            size += before->second;
            blocks.erase(before);
        }
    }
    if (next == blocks.end()) {
        if (!keepAtEnd && end == total) {
            total -= size;
            return;
        }
    } else if (start + size == next->first) {
        size += next->second;
        next = blocks.erase(next);
    }
    blocks.insert(next, BlockMap::value_type(start, size));
}

bool FreeSpaceMap::MarkUsed(int32_t offset, int32_t size)
{
    if (!size)
        return true;
    const int32_t end = offset + size;
    if (end > total) {
        if (offset < total)
            return false;
        const int32_t oldTotal = total;
        total = end;
        if (offset > oldTotal)
            AddFree(oldTotal, offset - oldTotal, false);
        return true;
    }
    const auto after = blocks.upper_bound(offset);
    if (after == blocks.begin())
        return false;
    const auto block = eastl::prev(after);
    const int32_t blockStart = block->first, blockEnd = block->first + block->second;
    blocks.erase(block);
    if (blockStart < offset)
        blocks.insert(after, BlockMap::value_type(blockStart, offset - blockStart));
    if (end < blockEnd)
        blocks.insert(after, BlockMap::value_type(end, blockEnd - end));
    return true;
}

}  // namespace ts3::resource

namespace ts3::resource {

bool DatabasePackedFile::WriteRaw(const void* data, uint32_t offset, uint32_t size)
{
    bool written = false;
    fileMutex.Lock();
    if (streamFlags & 2) {
        const uint32_t position = headerOffset + offset;
        if (memoryData) {
            if (position < memorySize)
                memoryPosition = position;
        } else if (stream->SetPosition(position, io::kPositionTypeBegin)) {
            written = stream->Write(data, size);
        }
    }
    fileMutex.Unlock();
    return written;
}

void DatabasePackedFile::RebuildFreeSpace()
{
    static const char kBusy[4] = {'B', 'U', 'S', 'Y'};
    const auto size = static_cast<int32_t>(GetFileSize() - headerOffset);
    freeSpace.Reset(size);
    freeSpace2.Reset(size);
    if (!(writeFlags & 2))
        return;
    freeSpace.Reset(0);
    freeSpace.MarkUsed(0, sizeof(PackageHeader));
    if (writeFlags & 8) {
        WriteRaw(kBusy, 0, sizeof kBusy);
        headerDirty = true;
        indexChanged = true;
    }
    if (freeSpace.MarkUsed(static_cast<int32_t>(indexOffset), static_cast<int32_t>(indexSize)) &&
        index->MarkRecordsUsed(&freeSpace, sizeof(PackageHeader), static_cast<uint32_t>(size)))
        return;
    freeSpace.Reset(size);
    freeSpace2.Reset(size);
    writeFlags &= ~2u;
}

}  // namespace ts3::resource

namespace ts3::resource {

bool DatabasePackedFile::FreeRegion(uint32_t offset, uint32_t size, bool reusableNow)
{
    if (!offset || !size)
        return true;
    if ((writeFlags & 0x10) && !reusableNow)
        freeSpace2.AddFree(static_cast<int32_t>(offset), static_cast<int32_t>(size), true);
    else
        freeSpace.AddFree(static_cast<int32_t>(offset), static_cast<int32_t>(size), false);
    freeSpaceChanged = true;
    return true;
}

bool DatabasePackedFile::DeleteRecord(const ResourceKey* key)
{
    bool deleted = false;
    recordMutex.Lock();
    if ((openFlags & 2) || ((reopenFlags & 2) && EnsureOpen())) {
        if (index->GetRecord(key) && openRecords.find(*key) == openRecords.end()) {
            CallGameVirtual<void, 39>(this);
            RecordInfo removed{};
            if (index->RemoveRecord(key, &removed)) {
                FreeRegion(removed.offset, removed.size, !removed.valid);
                deleted = true;
                indexChanged = true;
            }
        }
    }
    recordMutex.Unlock();
    return deleted;
}

void DatabasePackedFile::MakeIndexModifiable()
{
    recordMutex.Lock();
    if (!index || !index->IsModifiable()) {
        auto* modifiable = CallGameVirtual<IPackageIndex*, 25>(this, 3u);
        if (index) {
            if (index->Count()) {
                void* data = nullptr;
                uint32_t size = 0;
                if (index->Save(&data, &size, 0)) {
                    modifiable->Load(data, size, 0);
                    allocator->Free(data, 0);
                }
            }
            CallGameVirtual<void, 38>(this, index);
        }
        index = modifiable;
        CallGameVirtual<void, 40>(this);
    }
    recordMutex.Unlock();
}

}  // namespace ts3::resource

namespace ts3::resource {

int32_t FreeSpaceMap::Allocate(uint32_t size, bool unlimited)
{
    const uint32_t end = unlimited ? 0x7FFFFFFFu : limit;
    if (size > end)
        return -1;
    auto best = blocks.end();
    uint32_t bestSize = 0xFFFFFFFFu;
    for (auto it = blocks.begin(); it != blocks.end(); ++it) {
        const auto blockSize = static_cast<uint32_t>(it->second);
        if (blockSize >= size && blockSize < bestSize && static_cast<uint32_t>(it->first) + size <= end) {
            best = it;
            bestSize = blockSize;
            if (bestSize == size)
                break;
        }
    }
    if (best == blocks.end()) {
        if (total >= static_cast<int32_t>(end) || end - static_cast<uint32_t>(total) + 1 < size)
            return -1;
        const int32_t offset = total;
        total += static_cast<int32_t>(size);
        return offset;
    }
    const int32_t offset = best->first;
    const int32_t remaining = best->second - static_cast<int32_t>(size);
    const auto next = blocks.erase(best);
    if (remaining)
        blocks.insert(next, BlockMap::value_type(offset + static_cast<int32_t>(size), remaining));
    return offset;
}

bool DatabasePackedFile::AllocateRegion(uint32_t* offset, uint32_t size, bool unlimited)
{
    if (!(openFlags & 2))
        return false;
    const int32_t found = freeSpace.Allocate(size, unlimited);
    *offset = static_cast<uint32_t>(found);
    return found >= 0;
}

}  // namespace ts3::resource
