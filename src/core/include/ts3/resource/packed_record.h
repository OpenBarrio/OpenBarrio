// A record of a package opened for reading (TS3W.exe vtables 0x00FB2A70 and, for its
// IStream part at +0x28, 0x00FB2A34; constructors 0x004ADE60 over a region of the package,
// 0x004ADF30 over a memory buffer, 0x004ADFD0 copying another record). Resource readers
// get one of these and read through its IStream.
//
// A record over a package region (offset != 0) reads through the package's ReadRaw; any
// other record defers to the memory stream embedded at +0x30. Partial layout, x86 offsets.
#pragma once

#include <cstdint>

#include "ts3/io/file_stream.h"
#include "ts3/io/memory_stream.h"
#include "ts3/io/stream.h"
#include "ts3/layout.h"
#include "ts3/resource/compressed_index.h"

namespace ts3::resource {

struct DatabasePackedFile;

inline constexpr uint32_t kPackedFileRecordType = 316975249;  // what GetType returns

struct PackedFileRecord {
    void* const* vtable;
    int32_t refCount;            // 0x04
    uint32_t recordType;         // 0x08 kPackedFileRecordType
    uint32_t unknown0C;
    ResourceKey key;             // 0x10
    DatabasePackedFile* owner;   // 0x20
    uint32_t accessFlags;        // 0x24 0 = closed
    void* const* streamVtable;   // 0x28 the IStream part
    uint32_t unknown2C;
    io::MemoryStream memoryStream;  // 0x30 used when offset == 0
    uint32_t unknown58;
    uint32_t offset;             // 0x5C where the record starts in the package; 0 = memory
    uint32_t position;           // 0x60
    uint32_t size;               // 0x64

    // Through the memory stream's vtable, as the original calls it.
    io::IStream* Inner() { return reinterpret_cast<io::IStream*>(&memoryStream); }
    static PackedFileRecord* FromStream(void* stream) { return reinterpret_cast<PackedFileRecord*>(static_cast<uint8_t*>(stream) - 0x28); }

    // Constructors, on uninitialised memory (x86 builds give the object the game's vtables).
    // TS3W.exe 0x004ADE60: a record over `recordSize` bytes at `recordOffset` in the package.
    void ConstructOverRegion(uint32_t recordOffset, uint32_t recordSize, const ResourceKey& recordKey,
                             DatabasePackedFile* package);
    // TS3W.exe 0x004ADF30: a record over memory (a copy of it unless usePointer).
    void ConstructOverMemory(const void* src, uint32_t dataSize, bool usePointer, const ResourceKey& recordKey,
                             DatabasePackedFile* package);
    // TS3W.exe 0x004ADFD0: a record sharing another record's memory data, positioned at 0.
    void ConstructCopy(PackedFileRecord& other, const ResourceKey& recordKey, DatabasePackedFile* package);

    // IStream methods, TS3W.exe 0x004AD680, 0x004AD6A0, 0x004AD6E0, 0x004AD790, 0x004AD7B0.
    io::size_type GetSize();
    io::off_type GetPosition(io::PositionType type);
    // Clamps into [0, size] except for a negative offset from the end, which is not
    // checked against the start (as in the original).
    bool SetPosition(io::off_type position, io::PositionType type);
    io::size_type GetAvailable();
    // Reads at most what is left; a failed package read counts as 0 bytes read.
    io::size_type Read(void* dst, io::size_type size);
};
TS3_X86_OFFSET(PackedFileRecord, key, 0x10);
TS3_X86_OFFSET(PackedFileRecord, owner, 0x20);
TS3_X86_OFFSET(PackedFileRecord, streamVtable, 0x28);
TS3_X86_OFFSET(PackedFileRecord, memoryStream, 0x30);
TS3_X86_OFFSET(PackedFileRecord, offset, 0x5C);
TS3_X86_OFFSET(PackedFileRecord, size, 0x64);
TS3_X86_SIZE(PackedFileRecord, 104);

inline constexpr uint32_t kPackedFileWriteRecordType = 316975250;  // 0x12E4A892, what GetType returns
inline constexpr uint32_t kSpillThreshold = 2000000;  // bytes kept in memory before a temporary file

// A record of a package opened for writing (TS3W.exe vtables 0x00FB2AD4 and, for its IStream
// part at +0x28, 0x00FB2A98; constructors 0x004AE0F0 over a region of the package and
// 0x004AE2E0 over memory). The data sits in the memory stream at +0x2C; past
// kSpillThreshold it moves to a temporary file (the FileStream at +0x58). Writers use the
// stream ActiveStream returns; DatabasePackedFile's flush (0x004A8910) puts the data into
// the package with WriteOut. Partial layout, x86 offsets.
struct PackedFileWriteRecord {
    void* const* vtable;
    int32_t refCount;               // 0x04
    uint32_t recordType;            // 0x08 kPackedFileWriteRecordType
    uint32_t unknown0C;
    ResourceKey key;                // 0x10
    DatabasePackedFile* owner;      // 0x20
    uint32_t accessFlags;           // 0x24 3 while open, 0 once closed
    void* const* streamVtable;      // 0x28 the IStream part
    io::MemoryStream memoryStream;  // 0x2C
    uint32_t unknown54;
    io::FileStream spillFile;       // 0x58
    bool spillTried;                // 0x280 the region constructor already tried a temporary file
    uint8_t unknown281[7];

    static PackedFileWriteRecord* FromStream(void* stream)
    {
        return reinterpret_cast<PackedFileWriteRecord*>(static_cast<uint8_t*>(stream) - 0x28);
    }

    // Constructors on uninitialised memory (x86 builds give the object the game's vtables).
    // TS3W.exe 0x004AE0F0: a record holding a copy of `recordSize` bytes at `recordOffset`.
    // Over kSpillThreshold the copy goes to a temporary file, in 16 KB chunks; EA bug, kept:
    // that loop runs from recordOffset up to recordSize (not recordOffset + recordSize), so
    // it copies the wrong bytes, or none when the record starts past its own size. If the
    // file can't be made or filled, the copy goes to memory as for smaller records.
    void ConstructOverRegion(uint32_t recordOffset, uint32_t recordSize, const ResourceKey& recordKey,
                             DatabasePackedFile* package);
    // TS3W.exe 0x004AE2E0: a record over memory (the data itself when usePointer; the record
    // frees it either way).
    void ConstructOverMemory(const void* src, uint32_t dataSize, bool usePointer, const ResourceKey& recordKey,
                             DatabasePackedFile* package);
    // TS3W.exe 0x004AE3C0: an open record is first handed back to its package (its slot 15),
    // then closed (Close), the temporary file and memory stream destroyed.
    void Destruct();
    // TS3W.exe 0x004AE440: Destruct, then EaDelete when flags & 1.
    void DeletingDestruct(uint32_t flags);

    // TS3W.exe 0x004AD810: opens a new temporary file "Record_<type>_<instance low>_..."
    // (EA::IO::MakeTempPathName, 0x004DC4A0, not rewritten) for reading and writing, empty.
    bool CreateSpillFile();
    // TS3W.exe 0x004AD920: closes and deletes the temporary file, if open.
    void CloseSpillFile();
    // Record slot 9 (TS3W.exe 0x004AD970): detaches the record (access 0, no owner) and drops
    // the temporary file.
    void Close();
    // Record slot 6 (TS3W.exe 0x004AD9D0): the stream holding the data: the temporary file
    // when open, else the memory stream.
    void* ActiveStream();
    // Record slot 8 (TS3W.exe 0x004AD9F0; also stream slot 6 through a thunk): an open record
    // is handed back to its package (slot 15, CloseRecord); true when already closed.
    bool CloseThroughOwner();
    // TS3W.exe 0x004ADCE0: writes the data at `offset` in the package (WriteRaw): from the
    // temporary file in 16 KB chunks (`size` bytes; its position is restored), or from
    // memory (at most what the memory stream holds). False when closed.
    bool WriteOut(uint32_t offset, uint32_t size);

    // IStream slots 7-14 (this = record + 0x28 in the game): the temporary file when open,
    // else the memory stream. TS3W.exe 0x004ADA20, 0x004ADA50, 0x004ADA90, 0x004ADAD0,
    // 0x004ADB10, 0x004ADB40, 0x004ADB80, 0x004ADBC0.
    io::size_type GetSize();
    bool SetSize(io::size_type newSize);
    io::off_type GetPosition(io::PositionType type);
    bool SetPosition(io::off_type position, io::PositionType type);
    io::size_type GetAvailable();
    io::size_type Read(void* dst, io::size_type count);
    bool Flush();
    // Needs access. Data reaching kSpillThreshold moves to a new temporary file (only if
    // the constructor hasn't tried one). EA bug, kept: when the data is in memory, the write
    // goes to the (closed) temporary file and fails; writers use ActiveStream instead.
    bool Write(const void* src, io::size_type count);
};
TS3_X86_OFFSET(PackedFileWriteRecord, owner, 0x20);
TS3_X86_OFFSET(PackedFileWriteRecord, streamVtable, 0x28);
TS3_X86_OFFSET(PackedFileWriteRecord, memoryStream, 0x2C);
TS3_X86_OFFSET(PackedFileWriteRecord, spillFile, 0x58);
TS3_X86_OFFSET(PackedFileWriteRecord, spillTried, 0x280);
TS3_X86_SIZE(PackedFileWriteRecord, 0x288);

}  // namespace ts3::resource
