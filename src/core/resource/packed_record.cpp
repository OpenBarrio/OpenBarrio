#include "ts3/resource/packed_record.h"

#include "ts3/resource/packed_file.h"

namespace ts3::resource {

namespace {

// What every constructor does first (the original also passes through the base classes'
// vtables; only the final ones remain).
void ConstructHeader(PackedFileRecord& r, const ResourceKey& key, DatabasePackedFile* package)
{
#if defined(_M_IX86)
    r.vtable = reinterpret_cast<void* const*>(static_cast<uintptr_t>(0x00FB2A70));
    r.streamVtable = reinterpret_cast<void* const*>(static_cast<uintptr_t>(0x00FB2A34));
#else
    r.vtable = nullptr;
    r.streamVtable = nullptr;
#endif
    r.refCount = 0;
    r.recordType = kPackedFileRecordType;
    r.key = key;
    r.owner = package;
    r.accessFlags = 1;
    r.unknown2C = 0;
}

}  // namespace

void PackedFileRecord::ConstructOverRegion(uint32_t recordOffset, uint32_t recordSize, const ResourceKey& recordKey,
                                           DatabasePackedFile* package)
{
    ConstructHeader(*this, recordKey, package);
    memoryStream.Construct(nullptr, 0, false, true, nullptr, nullptr);
    size = recordSize;
    offset = recordOffset;
    position = 0;
}

void PackedFileRecord::ConstructOverMemory(const void* src, uint32_t dataSize, bool usePointer,
                                           const ResourceKey& recordKey, DatabasePackedFile* package)
{
    ConstructHeader(*this, recordKey, package);
    memoryStream.Construct(src, dataSize, usePointer, true, nullptr, nullptr);
    size = dataSize;
    offset = 0;
    position = 0;
}

void PackedFileRecord::ConstructCopy(PackedFileRecord& other, const ResourceKey& recordKey, DatabasePackedFile* package)
{
    ConstructHeader(*this, recordKey, package);
    memoryStream.ConstructCopy(other.memoryStream);
    offset = 0;
    position = 0;
    size = static_cast<uint32_t>(other.memoryStream.GetSize());
    memoryStream.SetPosition(0, io::kPositionTypeBegin);
}

io::size_type PackedFileRecord::GetSize()
{
    return offset ? size : memoryStream.GetSize();
}

io::off_type PackedFileRecord::GetPosition(io::PositionType type)
{
    if (!offset)
        return memoryStream.GetPosition(type);
    if (type == io::kPositionTypeBegin)
        return static_cast<io::off_type>(position);
    if (type == io::kPositionTypeEnd)
        return static_cast<io::off_type>(position - size);
    return 0;
}

bool PackedFileRecord::SetPosition(io::off_type newPosition, io::PositionType type)
{
    if (!accessFlags)
        return false;
    if (!offset)
        return memoryStream.SetPosition(newPosition, type);
    const auto delta = static_cast<int32_t>(newPosition);
    switch (type) {
    case io::kPositionTypeBegin:
        if (delta < 0)
            position = 0;
        else
            position = static_cast<uint32_t>(delta) <= size ? static_cast<uint32_t>(delta) : size;
        return true;
    case io::kPositionTypeCurrent:
        if (delta < 0 && 0u - static_cast<uint32_t>(delta) > position) {
            position = 0;
        } else if (delta <= 0 || (position <= size && static_cast<uint32_t>(delta) <= size - position)) {
            position += static_cast<uint32_t>(delta);
        } else {
            position = size;
        }
        return true;
    case io::kPositionTypeEnd:
        position = delta < 0 ? size + static_cast<uint32_t>(delta) : size;
        return true;
    default:
        return true;
    }
}

io::size_type PackedFileRecord::GetAvailable()
{
    return offset ? size - position : memoryStream.GetAvailable();
}

io::size_type PackedFileRecord::Read(void* dst, io::size_type count)
{
    if (!offset)
        return memoryStream.Read(dst, count);
    if (!accessFlags)
        return io::kSizeTypeError;
    auto n = static_cast<uint32_t>(count);
    if (position + n > size || position + n < position)  // 32-bit, as in the original
        n = size - position;
    if (!owner->ReadRaw(dst, offset + position, n))
        n = 0;
    position += n;
    return n;
}

}  // namespace ts3::resource

namespace ts3::resource {

namespace {

allocator::ICoreAllocator* OwnerAllocator(DatabasePackedFile* owner)
{
    return CallGameVirtual<allocator::ICoreAllocator*, 19>(owner);
}

void ConstructWriteHeader(PackedFileWriteRecord& r, const ResourceKey& key, DatabasePackedFile* package)
{
#if defined(_M_IX86)
    r.vtable = reinterpret_cast<void* const*>(static_cast<uintptr_t>(0x00FB2AD4));
    r.streamVtable = reinterpret_cast<void* const*>(static_cast<uintptr_t>(0x00FB2A98));
#else
    r.vtable = nullptr;
    r.streamVtable = nullptr;
#endif
    r.refCount = 0;
    r.recordType = kPackedFileWriteRecordType;
    r.key = key;
    r.owner = package;
    r.accessFlags = 3;
    r.memoryStream.Construct(nullptr, 0, false, true, nullptr, nullptr);
    r.spillFile.Construct(static_cast<const char*>(nullptr));
    r.spillTried = false;
    ++r.memoryStream.refCount;
    r.spillFile.AddRef();
    r.memoryStream.resizable = true;  // SetOption(1, 1.0f)
}

// L"Record_%08x_%08x_" (EA::StdC::Sprintf in the original).
void SpillName(wchar_t* out, uint32_t type, uint32_t instanceLow)
{
    static const wchar_t kDigits[] = L"0123456789abcdef";
    const wchar_t* prefix = L"Record_";
    while (*prefix)
        *out++ = *prefix++;
    for (const uint32_t value : {type, instanceLow}) {
        for (int shift = 28; shift >= 0; shift -= 4)
            *out++ = kDigits[(value >> shift) & 15];
        *out++ = L'_';
    }
    *out = 0;
}

}  // namespace

void PackedFileWriteRecord::ConstructOverRegion(uint32_t recordOffset, uint32_t recordSize, const ResourceKey& recordKey,
                                                DatabasePackedFile* package)
{
    ConstructWriteHeader(*this, recordKey, package);
    if (!recordSize)
        return;
    if (recordSize > kSpillThreshold && !spillTried) {
        spillTried = true;
        if (CreateSpillFile()) {
            allocator::ICoreAllocator* chunkAllocator = OwnerAllocator(owner);
            void* chunk = chunkAllocator->Alloc(0x4000, "ResourceLoad/PackedFile/Raw", 0);
            if (chunk) {
                bool ok = true;
                for (uint32_t at = recordOffset; at < recordSize && ok;) {  // sic: up to recordSize
                    const uint32_t n = at + 0x4000 > recordSize ? recordSize - at : 0x4000;
                    ok = owner->ReadRaw(chunk, at, n);
                    if (ok) {
                        ok = spillFile.Write(chunk, n);
                        at += n;
                    }
                }
                if (ok)
                    spillFile.SetPosition(0, io::kPositionTypeBegin);
                chunkAllocator->Free(chunk, 0);
                if (ok)
                    return;
            }
            CloseSpillFile();
        }
    }
    if (memoryStream.SetData(nullptr, recordSize, false, true, OwnerAllocator(owner)))
        owner->ReadRaw(memoryStream.Data(), recordOffset, recordSize);
}

void PackedFileWriteRecord::ConstructOverMemory(const void* src, uint32_t dataSize, bool usePointer,
                                                const ResourceKey& recordKey, DatabasePackedFile* package)
{
    ConstructWriteHeader(*this, recordKey, package);
    memoryStream.SetData(src, dataSize, usePointer, true, OwnerAllocator(owner));
}

void PackedFileWriteRecord::Destruct()
{
#if defined(_M_IX86)
    vtable = reinterpret_cast<void* const*>(static_cast<uintptr_t>(0x00FB2AD4));
    streamVtable = reinterpret_cast<void* const*>(static_cast<uintptr_t>(0x00FB2A98));
#endif
    if (accessFlags)
        CallGameVirtual<bool, 15>(owner, static_cast<void*>(this));
    Close();
    spillFile.Destruct();
#if defined(_M_IX86)
    memoryStream.vtable = reinterpret_cast<void* const*>(static_cast<uintptr_t>(0x00FB5BCC));
#endif
    if (memoryStream.buffer)
        memoryStream.buffer->Release();
#if defined(_M_IX86)
    memoryStream.vtable = reinterpret_cast<void* const*>(static_cast<uintptr_t>(0x00FA99FC));
    streamVtable = reinterpret_cast<void* const*>(static_cast<uintptr_t>(0x00FA99FC));
    vtable = reinterpret_cast<void* const*>(static_cast<uintptr_t>(0x00FB84D0));
#else
    memoryStream.vtable = nullptr;
    streamVtable = nullptr;
    vtable = nullptr;
#endif
}

void PackedFileWriteRecord::DeletingDestruct(uint32_t flags)
{
    Destruct();
    if (flags & 1)
        io::EaDelete(this);
}

bool PackedFileWriteRecord::CreateSpillFile()
{
    wchar_t name[io::kMaxPath];
    SpillName(name, key.type, key.instanceLow);
    if (!CallGameFunction<bool, 0x004DC4A0>(static_cast<wchar_t*>(name), static_cast<const wchar_t*>(nullptr),
                                            static_cast<const wchar_t*>(name), static_cast<const wchar_t*>(nullptr),
                                            static_cast<uint32_t>(io::kMaxPath)))
        return false;
    spillFile.SetPath(static_cast<const wchar_t*>(name));
    if (!spillFile.Open(3, 4, 1, 0))
        return false;
    spillFile.SetSize(0);
    return true;
}

void PackedFileWriteRecord::CloseSpillFile()
{
    if (!spillFile.GetAccessFlags())
        return;
    spillFile.Close();
    wchar_t path[io::kMaxPath];
    spillFile.GetPath(path, io::kMaxPath);
    CallGameFunction<bool, 0x004DBBD0>(static_cast<const wchar_t*>(path));
}

void PackedFileWriteRecord::Close()
{
    accessFlags = 0;
    owner = nullptr;
    spillTried = false;
    CloseSpillFile();
}

void* PackedFileWriteRecord::ActiveStream()
{
    return spillFile.GetAccessFlags() ? static_cast<void*>(&spillFile) : static_cast<void*>(&memoryStream);
}

bool PackedFileWriteRecord::CloseThroughOwner()
{
    return accessFlags ? CallGameVirtual<bool, 15>(owner, static_cast<void*>(this)) : true;
}

bool PackedFileWriteRecord::WriteOut(uint32_t offset, uint32_t size)
{
    if (!accessFlags)
        return false;
    if (!spillFile.GetAccessFlags()) {
        const auto n = static_cast<uint32_t>(size < memoryStream.GetSize() ? size : memoryStream.GetSize());
        return owner->WriteRaw(memoryStream.Data(), offset, n);
    }
    allocator::ICoreAllocator* chunkAllocator = OwnerAllocator(owner);
    void* chunk = chunkAllocator->Alloc(0x4000, "ResourceLoad/PackedFile/RecordData", 0);
    if (!chunk)
        return false;
    const io::off_type saved = spillFile.GetPosition(io::kPositionTypeBegin);
    bool ok = true;
    spillFile.SetPosition(0, io::kPositionTypeBegin);
    for (uint32_t done = 0; done < size && ok;) {
        const uint32_t n = done + 0x4000 > size ? size - done : 0x4000;
        ok = spillFile.Read(chunk, n) == n;
        if (ok) {
            ok = owner->WriteRaw(chunk, offset, n);
            done += n;
            offset += n;
        }
    }
    chunkAllocator->Free(chunk, 0);
    spillFile.SetPosition(saved, io::kPositionTypeBegin);
    return ok;
}

io::size_type PackedFileWriteRecord::GetSize()
{
    return spillFile.GetAccessFlags() ? spillFile.GetSize() : memoryStream.GetSize();
}

bool PackedFileWriteRecord::SetSize(io::size_type newSize)
{
    return spillFile.GetAccessFlags() ? spillFile.SetSize(newSize) : memoryStream.SetSize(newSize);
}

io::off_type PackedFileWriteRecord::GetPosition(io::PositionType type)
{
    return spillFile.GetAccessFlags() ? spillFile.GetPosition(type) : memoryStream.GetPosition(type);
}

bool PackedFileWriteRecord::SetPosition(io::off_type position, io::PositionType type)
{
    return spillFile.GetAccessFlags() ? spillFile.SetPosition(position, type) : memoryStream.SetPosition(position, type);
}

io::size_type PackedFileWriteRecord::GetAvailable()
{
    return spillFile.GetAccessFlags() ? spillFile.GetAvailable() : memoryStream.GetAvailable();
}

io::size_type PackedFileWriteRecord::Read(void* dst, io::size_type count)
{
    return spillFile.GetAccessFlags() ? spillFile.Read(dst, count) : memoryStream.Read(dst, count);
}

bool PackedFileWriteRecord::Flush()
{
    return spillFile.GetAccessFlags() ? spillFile.Flush() : true;  // MemoryStream::Flush is `return true`
}

bool PackedFileWriteRecord::Write(const void* src, io::size_type count)
{
    if (!accessFlags)
        return false;
    if (spillFile.GetAccessFlags())
        return spillFile.Write(src, count);
    if (static_cast<uint32_t>(memoryStream.GetSize() + count) >= kSpillThreshold && !spillTried && CreateSpillFile()) {
        bool ok = spillFile.Write(memoryStream.Data(), memoryStream.GetSize());
        if (ok)
            ok = spillFile.Write(src, count);
        memoryStream.SetData(nullptr, 0, false, true, OwnerAllocator(owner));
        if (ok)
            return true;
        CloseSpillFile();
    }
    return spillFile.Write(src, count);  // sic: the closed temporary file, not the memory stream
}

}  // namespace ts3::resource
