// DatabasePackedFile, the engine's .package (DBPF) file object (TS3W.exe vtable 0x00FB2600,
// 41 virtual methods). Rewritten so far: the path
// that reads a record's bytes and decompresses them.
//
// The layout is partial. Fields not yet understood are padding sized so the x86 offsets
// match the game's objects, which the harness passes straight to these methods; the x64
// layout is only meaningful once every field is known.
#pragma once

#include <cstdint>

#include <EASTL/map.h>

#include "ts3/allocator/core_allocator.h"
#include "ts3/io/stream.h"
#include "ts3/layout.h"
#include "ts3/resource/compression_codec.h"
#include "ts3/resource/package_index.h"
#include "ts3/thread/mutex.h"

namespace ts3::resource {

struct PackedFileRecord;

// EA's ResourceKey order in the record multimap: instanceLow, then instanceHigh (each as an
// unsigned 32-bit number), then type and group together as one 64-bit number (type low).
// The member keeps the comparator 4 bytes wide, as in the game's tree.
struct ResourceKeyLess {
    uint32_t unused = 0;
    bool operator()(const ResourceKey& a, const ResourceKey& b) const
    {
        if (a.instanceLow != b.instanceLow)
            return a.instanceLow < b.instanceLow;
        if (a.instanceHigh != b.instanceHigh)
            return a.instanceHigh < b.instanceHigh;
        return (static_cast<uint64_t>(a.group) << 32 | a.type) < (static_cast<uint64_t>(b.group) << 32 | b.type);
    }
};

// The open-records multimap ("ResourceLoad/PackedFile/RecordMultiMap"): an EASTL red-black
// tree, byte-compatible with the game's on x86 (comparator, anchor, size, allocator).
using RecordMultiMap = eastl::multimap<ResourceKey, PackedFileRecord*, ResourceKeyLess, GameAllocator>;
TS3_X86_SIZE(RecordMultiMap, 0x24);
static_assert(sizeof(RecordMultiMap::value_type) == 24, "map nodes are 40 bytes, as in the game");

// Free regions of a package being written, as offset -> size with neighbours merged;
// `total` is where the space ends. DatabasePackedFile keeps two (0x344 and 0x368).
// eastl::less for the free-space map, kept 4 bytes wide as in the game's tree.
struct OffsetLess {
    uint32_t unused = 0;
    bool operator()(int32_t a, int32_t b) const { return a < b; }
};

struct FreeSpaceMap {
    using BlockMap = eastl::map<int32_t, int32_t, OffsetLess, NamedAllocator>;
    int32_t total;            // 0x00
    uint32_t limit;           // 0x04 allocations end at or before this (unless unlimited)
    BlockMap blocks;          // 0x08

    // TS3W.exe 0x004AB070: forgets every block; the space now ends at newTotal.
    void Reset(int32_t newTotal);
    // TS3W.exe 0x004AC610: frees [start, start + size), merged with the blocks that end at
    // `start` and begin at its end. Unless keepAtEnd, a range ending exactly at `total`
    // (with no block after it) shrinks the space instead, by the merged size.
    void AddFree(int32_t start, int32_t size, bool keepAtEnd);
    // TS3W.exe 0x004ACD70: takes [offset, offset + size) out of the free space. A range past
    // the end grows the space (the gap before it becomes free); one straddling the end
    // fails. Otherwise the block starting at or before `offset` is split around the range
    // (false when there is none). Quirk kept: the range isn't checked to fit that block.
    bool MarkUsed(int32_t offset, int32_t size);
    // TS3W.exe 0x004AC4E0: finds room for `size` bytes: the smallest free block that fits
    // (the first one on a tie, an exact fit ends the search) and ends within the limit
    // (0x7FFFFFFF when unlimited); the rest of that block stays free. With no such block the
    // space grows at its end, if that stays within limit + 1 (sic). -1 when nothing fits.
    int32_t Allocate(uint32_t size, bool unlimited);
};
TS3_X86_SIZE(FreeSpaceMap, 0x24);
TS3_X86_OFFSET(FreeSpaceMap, blocks, 0x08);

struct DatabasePackedFile {
    void* const* vtable;                    // the game's methods (not rewritten yet)
    uint8_t unknown004[8];                  // probably vtables of further base classes
    bool opensFile;                         // 0x00C Open may open the file (else only a memory image)
    bool closeWhenIdle;                     // 0x00D close the file when the last record closes
    bool keepInvalidIndex;                  // 0x00E LoadIndex succeeds even when validation (slot 37) fails
    bool resetInvalidOnWrite;               // 0x00F Open for writing starts over when the header/index is bad
    allocator::ICoreAllocator* allocator;   // 0x010
    uint32_t openFlags;                     // 0x014 current access: 1 read, 2 write
    uint32_t reopenFlags;                   // 0x018 access to reopen with
    uint32_t streamFlags;                   // 0x01C access the file was opened with; 0 = not open
    uint32_t compressionMode;               // 0x020 0: records are stored uncompressed; else the codec runs
    const wchar_t* path;                    // 0x024 the file's path (first field of a string object)
    uint8_t unknown028[0xC];
    // The game's EA::IO::FileStream, which Open (slot 6) sets up when no stream was given
    // (it probably fills the space up to headerOffset).
    uint8_t ownFileStream[0x228];           // 0x034
    uint32_t headerOffset;                  // 0x25C where the package starts in the file
    io::IStream* stream;                    // 0x260 read when memoryData is null
    const uint8_t* memoryData;              // 0x264 package image held in memory
    uint32_t memorySize;                    // 0x268
    uint32_t memoryPosition;                // 0x26C
    thread::Mutex recordMutex;              // 0x270 index and record state
    thread::Mutex fileMutex;                // 0x2A0 stream / memory position
    IPackageIndex* index;                   // 0x2D0
    void* indexObserver;                    // 0x2D4 told about every index read (its slot 0)
    bool indexChanged;                      // 0x2D8
    uint8_t unknown2D9[3];
    // The user version written into headers this package writes (slot 33).
    uint32_t newUserVersionMajor;           // 0x2DC
    uint32_t newUserVersionMinor;           // 0x2E0
    float maxCompressRatio;                 // 0x2E4 keep a compressed record only below this compressed/raw ratio
    RecordMultiMap openRecords;             // 0x2E8 records handed out, by key
    uint8_t unknown30C[4];
    // Copied from the header by ReadHeader.
    uint32_t userVersionMajor;              // 0x310 header 0x0C
    uint32_t userVersionMinor;              // 0x314 header 0x10
    uint32_t createdTime[2];                // 0x318 header 0x18, widened to 64 bits
    uint32_t modifiedTime[2];               // 0x320 header 0x1C, widened to 64 bits
    uint32_t indexMajorVersion;             // 0x328 header 0x20
    uint32_t indexOffset;                   // 0x32C
    uint32_t indexCount;                    // 0x330
    uint32_t indexSize;                     // 0x334
    bool headerDirty;                       // 0x338 the header must be written (slot 33)
    uint8_t unknown339[3];
    uint32_t writeFlags;                    // 0x33C 2: track free space, 8: mark the file BUSY while writing
    uint8_t unknown340[4];
    FreeSpaceMap freeSpace;                 // 0x344 where new records go
    FreeSpaceMap freeSpace2;                // 0x368
    bool freeSpaceChanged;                  // 0x38C
    uint8_t unknown38D[3];
    IResourceManager* resourceManager;      // 0x390 supplies codecs other than RefPack

    // TS3W.exe 0x004A7490. Reads at the current position; with a memory image, as much as
    // is left (the count can be short, never an error).
    io::size_type ReadAtPosition(void* dst, uint32_t size);
    // TS3W.exe 0x004A7DE0. Reads `size` stored bytes at `offset` from the package start.
    // A short read from a memory image still counts as success.
    bool ReadRaw(void* dst, uint32_t offset, uint32_t size);
    // TS3W.exe 0x004A7360. Decompresses one record into dst (memSize bytes). The low 16
    // bits of `compression` select RefPack (0xFFFF); any other value goes to the resource
    // manager unchanged (callers pass the index entry's uint16 zero-extended).
    bool DecompressRecord(uint32_t compression, const void* src, uint32_t srcLen, uint32_t memSize, void* dst);
    // TS3W.exe 0x004A8380. ReadRaw into a temporary buffer, then DecompressRecord.
    bool ReadAndDecompress(uint32_t compression, uint32_t offset, uint32_t size, uint32_t memSize, void* dst);
    // TS3W.exe 0x004A7F20. Reads up to maxSize stored (still compressed) bytes of a record.
    // True without reading when dst is null or nothing is to be read; false when the key
    // is missing or the package can't be (re)opened.
    bool ReadRecordRaw(const ResourceKey* key, void* dst, uint32_t maxSize);
    // TS3W.exe 0x004A6860. Reopens a package the game closed, with reopenFlags; when that
    // fails and the current flags say read+write, tries read-only.
    bool EnsureOpen();
    // Virtual slot 6 (TS3W.exe 0x004A76E0). flags: 1 read, 2 write. mode: file creation
    // mode for the own file stream (6 picks one from the flags). reopen: also remember the
    // flags for later reopens; with an index already loaded the file isn't reopened. A
    // memory image only accepts read with mode 3 or 6. Reads the header and index (slot
    // 30); an empty file opened for writing gets a new header (slot 32), and with
    // resetInvalidOnWrite so does one whose header or index is bad. Any failure closes the
    // file (CloseFile). Quirk kept: a successful reopen through the own file stream closes
    // it again before returning true.
    // The own file stream's slots 15 (set path) and 19 (open), the index's slot 4 and
    // slots 30, 32 and 40 are reached through vtables.
    bool Open(uint32_t flags, uint32_t mode, bool reopen);

    // Records handed to readers, allocated with EaNew from this package's allocator.
    // Virtual slot 27 (TS3W.exe 0x004A6820): a read record sharing `source`'s memory data.
    PackedFileRecord* CopyRecord(uint32_t unused, PackedFileRecord* source, const ResourceKey* key);
    // Virtual slot 28 (TS3W.exe 0x004A67A0): a record over `data`, which it takes over (no
    // copy, freed with the record). Write access (flags & 2) makes a write record (648 bytes,
    // constructor 0x004AE2E0, not rewritten yet).
    void* CreateRecordOverMemory(uint32_t flags, void* data, uint32_t size, const ResourceKey* key);
    // Virtual slot 29 (TS3W.exe 0x004A6720): a record over a region of the package; write
    // access makes a write record (constructor 0x004AE0F0, not rewritten yet).
    void* CreateRecordOverRegion(uint32_t flags, uint32_t offset, uint32_t size, const ResourceKey* key);
    // Virtual slot 30 (TS3W.exe 0x004A68B0): ReadHeader then LoadIndex, both through the
    // vtable (slots 31 and 35).
    bool ReadHeaderAndIndex();
    // Virtual slot 31 (TS3W.exe 0x004A7950; a derived class overrides it): reads the 96-byte
    // header at headerOffset, keeps its version, dates and index location, and validates it
    // (slot 34). When that fails it scans the file for the DBPF signature (0x004AE870, not
    // rewritten yet) and retries once from there; the global flag at 0x011C78D4 stops a
    // second retry. EA bug, kept: when that nested retry fails validation too, it reports
    // success.
    bool ReadHeader();
    // Virtual slot 32 (TS3W.exe 0x004A7B80): starts a new, empty package. Resets the header
    // fields (user version from newUserVersionMajor/Minor), marks the header dirty, writes
    // it (slot 33) and cuts the file to the 96-byte header.
    bool NewHeader();
    // Virtual slot 33 (TS3W.exe 0x004A7C30): writes the header at headerOffset when it is
    // dirty. Needs write access and an open stream; a memory image can't be written
    // (false). The header carries no dates. EA quirk, kept: the DBPF 1.x index-offset
    // field gets the offset only when it is 2 GB or more (0 otherwise), and the high half
    // of the 64-bit offset is its sign.
    bool WriteHeader();
    // TS3W.exe 0x004A7E70: writes `size` bytes at `offset` from the package start, under the
    // file lock. Needs a stream opened for writing (streamFlags & 2). A memory image is never
    // written (false), though an in-range offset still moves memoryPosition.
    bool WriteRaw(const void* data, uint32_t offset, uint32_t size);
    // TS3W.exe 0x004A6910: room for `size` bytes in the package (freeSpace.Allocate); needs
    // write access. False when there is none; *offset gets the result either way (-1).
    bool AllocateRegion(uint32_t* offset, uint32_t size, bool unlimited);
    // TS3W.exe 0x004A6950: gives a dropped record's region back. With writeFlags & 0x10, a
    // region the file's index still points to (reusableNow false) waits in freeSpace2;
    // otherwise it goes to freeSpace. Always true; an empty region changes nothing.
    bool FreeRegion(uint32_t offset, uint32_t size, bool reusableNow);
    // Virtual slot 16 (TS3W.exe 0x004A85A0): deletes the record for `key` from the index
    // (reopening for writing first when needed). Only a record that exists and isn't open
    // is deleted: the index becomes modifiable (slot 39), drops the entry (index slot 12)
    // and its region is freed (FreeRegion). Under the record lock.
    bool DeleteRecord(const ResourceKey* key);
    // Virtual slot 39 (TS3W.exe 0x004A6C80): makes sure the index can be changed. A missing
    // or read-only index is replaced by a new modifiable one (slot 25, kind 3) holding the
    // old one's records (index slots 14 and 13), the old one is destroyed (slot 38) and the
    // free space rebuilt (slot 40). Under the record lock.
    void MakeIndexModifiable();
    // Virtual slot 40 (TS3W.exe 0x004A8810): sizes both free-space maps to the file. With
    // writeFlags & 2 it rebuilds freeSpace from the header, the index and the records (index
    // slot 9); writeFlags & 8 first overwrites the DBPF magic with "BUSY" (WriteRaw) and
    // marks the header and index dirty. If the index region or a record doesn't fit, both
    // maps cover the whole file again and bit 2 is cleared.
    void RebuildFreeSpace();
    // TS3W.exe 0x004A7510: the file's size (the memory image's, or the stream's), under
    // the file lock.
    uint32_t GetFileSize();
    // TS3W.exe 0x004A7560: resizes the file (stream slot 8) under the file lock; a memory
    // image can't be resized (false).
    bool SetFileSize(uint32_t size);
    // TS3W.exe 0x004A75C0: closes the stream (its slot 6), releases it (slot 2) if it is
    // the package's own file stream, rewinds a memory image and clears the open flags.
    bool CloseFile();
    // Virtual slot 37 (TS3W.exe 0x004A7D80): the index checks its records against the
    // file's size (index slot 15).
    bool ValidateIndex();
    // TS3W.exe 0x004A7450: moves the file position (memory image or stream) to `position`.
    bool SeekInFile(uint32_t position);
    // Virtual slot 35 (TS3W.exe 0x004A9950): reads the package index into the index object
    // (creating it through slot 25 when missing) unless it already has records, tells the
    // observer, then validates through slot 37 (keepInvalidIndex accepts a failure). An
    // empty package marks the index dirty (slot 40). Slots 25, 37 and 40 are game code.
    bool LoadIndex();
    // Virtual slot 13 (TS3W.exe 0x004A94C0), the package's main read: a record for `key`.
    // flags: 1 read, 2 write. mode (1-6) says whether the record must exist, must not, or
    // either (6 picks from the flags). options: 1 decompress compressed records into memory,
    // 2 always read through the package (no memory copy). Fills `info` (offset, size,
    // memSize, compression) when given; with no `out`, only reports whether the key exists.
    // The record gets one reference for the open-records map and one for the caller.
    // Write paths call game code not rewritten yet (slots 16 and 39, index slot 11).
    bool ReadRecord(const ResourceKey* key, void** out, uint32_t flags, uint32_t mode, uint32_t options, uint32_t* info);
    // Virtual slot 15 (TS3W.exe 0x004A8E00): a reader is done with `record`. Finds it among
    // the open records with its key, flushes it if it is a write record (FlushWriteRecord),
    // closes and releases it (its slots 9 and 2) and drops it from the map.
    // With closeWhenIdle set, the last close also closes the file (CloseFile). False when
    // the record isn't open here.
    bool CloseRecord(void* record);
    // TS3W.exe 0x004A7030: picks the codec a record is compressed with (the resource
    // manager at 0x390 or a fresh RefPack codec) and runs it on `src`. Outputs: *bufOut
    // (a fresh allocation when the caller passed null, else the caller's buffer),
    // *sizeOut and *typeOut. See the definition for the exact rules.
    bool ChooseWriteCodec(const void* src, uint32_t size, void** bufOut, uint32_t* sizeOut, uint16_t* typeOut);
    // TS3W.exe 0x004A82C0: compresses `src` through ChooseWriteCodec (0x004A7030) when
    // compressionMode is on and `size` is in [50, 16000000]. Keeps the result only when
    // compressed/raw stays strictly below maxCompressRatio (x87 compare; NaN rejects
    // too), otherwise frees it and reports failure. Always zeroes *bufOut and *sizeOut
    // first; *typeOut is written by the codec.
    bool CompressForWrite(const void* src, uint32_t size, void** bufOut, uint32_t* sizeOut, uint16_t* typeOut);
    // TS3W.exe 0x004A8910: writes a closed write record into the package. Drops the old
    // index entry for `key` first (index slot 12) and frees its region (with writeFlags &
    // 0x10, a still-indexed region waits in freeSpace2: FreeRegion's reusableNow arg is
    // `valid == 0`). Then, with compression on and the size in [50, 16000000], compresses
    // the record's stream into a temporary and writes that (AllocateRegion, WriteRaw,
    // index slot 11); any failure falls back to the raw bytes through the record itself
    // (write records only, WriteOut 0x004ADCE0). An empty record just sets a zero entry.
    // True when a new entry landed in the index.
    bool FlushWriteRecord(void* record, const ResourceKey* key);
};
TS3_X86_OFFSET(DatabasePackedFile, opensFile, 0x00C);
TS3_X86_OFFSET(DatabasePackedFile, closeWhenIdle, 0x00D);
TS3_X86_OFFSET(DatabasePackedFile, resetInvalidOnWrite, 0x00F);
TS3_X86_OFFSET(DatabasePackedFile, allocator, 0x010);
TS3_X86_OFFSET(DatabasePackedFile, openFlags, 0x014);
TS3_X86_OFFSET(DatabasePackedFile, compressionMode, 0x020);
TS3_X86_OFFSET(DatabasePackedFile, streamFlags, 0x01C);
TS3_X86_OFFSET(DatabasePackedFile, headerOffset, 0x25C);
TS3_X86_OFFSET(DatabasePackedFile, stream, 0x260);
TS3_X86_OFFSET(DatabasePackedFile, memoryPosition, 0x26C);
TS3_X86_OFFSET(DatabasePackedFile, recordMutex, 0x270);
TS3_X86_OFFSET(DatabasePackedFile, fileMutex, 0x2A0);
TS3_X86_OFFSET(DatabasePackedFile, index, 0x2D0);
TS3_X86_OFFSET(DatabasePackedFile, openRecords, 0x2E8);
TS3_X86_OFFSET(DatabasePackedFile, maxCompressRatio, 0x2E4);
TS3_X86_OFFSET(DatabasePackedFile, indexChanged, 0x2D8);
TS3_X86_OFFSET(DatabasePackedFile, indexObserver, 0x2D4);
TS3_X86_OFFSET(DatabasePackedFile, keepInvalidIndex, 0x00E);
TS3_X86_OFFSET(DatabasePackedFile, path, 0x024);
TS3_X86_OFFSET(DatabasePackedFile, ownFileStream, 0x034);
TS3_X86_OFFSET(DatabasePackedFile, userVersionMajor, 0x310);
TS3_X86_OFFSET(DatabasePackedFile, indexMajorVersion, 0x328);
TS3_X86_OFFSET(DatabasePackedFile, indexOffset, 0x32C);
TS3_X86_OFFSET(DatabasePackedFile, indexSize, 0x334);
TS3_X86_OFFSET(DatabasePackedFile, newUserVersionMajor, 0x2DC);
TS3_X86_OFFSET(DatabasePackedFile, headerDirty, 0x338);
TS3_X86_OFFSET(DatabasePackedFile, writeFlags, 0x33C);
TS3_X86_OFFSET(DatabasePackedFile, freeSpace, 0x344);
TS3_X86_OFFSET(DatabasePackedFile, freeSpace2, 0x368);
TS3_X86_OFFSET(DatabasePackedFile, freeSpaceChanged, 0x38C);
TS3_X86_OFFSET(DatabasePackedFile, resourceManager, 0x390);

}  // namespace ts3::resource
