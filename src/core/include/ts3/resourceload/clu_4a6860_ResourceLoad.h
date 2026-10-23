// Batch rewrite of cluster `clu_4a6860_ResourceLoad` (subsystem ResourceLoad), the
// DatabasePackedFile / MemoryDB write and copy helpers that sit around the already
// rewritten read path. The functions here are free functions (the class headers in
// ts3/resource/*.h are owned by other units and cannot grow members), so each takes the
// game object explicitly; on x86 the harness hands them the game's own objects.
//
// Companion already-rewritten functions of the same cluster (called through their classes,
// not repeated here):
//   0x004A6860 DatabasePackedFile::EnsureOpen        packed_file.h
//   0x004A6910 DatabasePackedFile::AllocateRegion    packed_file.h
//   0x004A6950 DatabasePackedFile::FreeRegion        packed_file.h
//   0x004A7DE0 DatabasePackedFile::ReadRaw           packed_file.h
//   0x004A7E70 DatabasePackedFile::WriteRaw          packed_file.h
//   0x004A7F20 DatabasePackedFile::ReadRecordRaw     packed_file.h
//   0x004AC4E0 FreeSpaceMap::Allocate                packed_file.h
//   0x004E3DB0 ts3::io::EaNew                        memory_stream.h
//   0x0072AFB0 MemoryDB::DecompressRecord            databases.h
//   0x0072BD10 MemoryDB::Read                        databases.h
#pragma once

#include <cstdint>

#include "ts3/resource/databases.h"
#include "ts3/resource/packed_file.h"

namespace ts3::resourceload {

// TS3W.exe 0x004A6AB0, thiscall ret 8. Copies the 16-byte key into the first four dwords of
// `dst` and the one dword at `extra` into the fifth; returns `dst`.
void* CopyKeyWithDword(void* dst, const void* key, const void* extra);

// TS3W.exe 0x0072AF40, thiscall ret 8. Copies the 16-byte key and the three dwords at
// `payload` into the seven dwords of `dst`; returns `dst`.
void* CopyKeyWithPayload(void* dst, const void* key, const void* payload);

// TS3W.exe 0x004AB0A0, thiscall ret 8. True when no block of `map` overlaps
// [offset, offset + size). A zero size is free; a range that would wrap fails.
bool FreeSpaceIsFree(resource::FreeSpaceMap* map, uint32_t offset, uint32_t size);

// TS3W.exe 0x004A7FC0, thiscall ret 0x14. Adds `key` to the index over `data` (already
// compressed as `compression`/`memSize` when those describe it), allocating room in the
// free space and writing the bytes. An existing entry for `key` is dropped and its region
// freed first. Needs write access. Under the record lock. Calls the game's index slots 10
// (RemoveRecord) and 11 (SetRecord) and vtable slot 39 (MakeIndexModifiable).
bool WriteRecord(resource::DatabasePackedFile* self, const resource::ResourceKey* key, const void* data, uint32_t size,
                 uint32_t memSize, uint16_t compression);

// TS3W.exe 0x004A8120, thiscall ret 0xC. Moves the record for `key` to `newOffset` (its
// size must equal `size`, which the index entry must also report). The destination range
// must be free; the bytes are read out and written back. Needs write access, record lock.
bool MoveRecord(resource::DatabasePackedFile* self, const resource::ResourceKey* key, uint32_t newOffset, uint32_t size);

// TS3W.exe 0x004A9450, thiscall ret 8. The open-records multimap lookup-and-insert used by
// the read path: `value` is inserted for the key at `key` when it is not there yet, and the
// value argument is returned either way. The tree walk itself is 0x004A8F10, not rewritten.
void* OpenRecordsFindOrInsert(resource::RecordMultiMap* map, void* value, const void* key);

// TS3W.exe 0x0072BB60, thiscall ret 8. Inserts the key/payload pair at `value` into the
// records map and returns the pair (0x0072B710 is the tree walk, not rewritten).
void* RecordsInsert(resource::RecordMap* map, const void* value);

// TS3W.exe 0x004AEEF0, cdecl (5 args). Writes `data` into a record for `key` in `dst` via
// the database's own vtable: slot 13 opens a write record (flags 2, mode 2, `options`),
// slot 6 of the record gives its stream and slot 14 of the stream writes the bytes.
bool WriteRecordFromMemory(void* dst, const resource::ResourceKey* key, void* data, uint32_t size, uint32_t options);

// TS3W.exe 0x004AE510: cdecl (4 args). Copies one record between two DatabasePackedFile
// objects. The fast path reads the stored bytes straight from `src` (ReadRecordRaw) and
// writes them into `dst` (WriteRecord); otherwise the record's stream is copied through
// WriteRecordFromMemory. EA quirk: the fast path deletes when removeFromSource is false;
// the stream fallback deletes when it is true.
bool CopyRecord(resource::DatabasePackedFile* dst, resource::DatabasePackedFile* src, const resource::ResourceKey* key,
                bool removeFromSource);

// TS3W.exe 0x004AE6E0: cdecl (4 args). Copies every record of `src` to `dst` through
// CopyRecord, returning how many were copied. The key list comes from the index (for a
// DBPF) or from the source's slot 12, and is freed with the scratch free 0x004E3F70.
int32_t CopyRecords(resource::DatabasePackedFile* dst, resource::DatabasePackedFile* src, const void* unused,
                    bool removeFromSource);

// TS3W.exe 0x004A9150, usercall (al result, ecx=this, ebp=indexCount). Rebuilds the
// package into a temporary file and swaps it in: with a modifiable index it just resets
// the index/header and re-sizes the file, otherwise it creates a second package, copies
// every record into it and renames the files. Needs write access.
bool SaveCompact(resource::DatabasePackedFile* self, uint32_t indexCount);

// TS3W.exe 0x0072B0A0, thiscall ret 0x14 (used by MemoryDB). Compresses `size` bytes at
// `src` with a fresh RefPack codec: *typeOut 0xFFFF, *sizeOut/*bufOut the compressed copy
// (a fresh allocation, or a zero-size allocation for an empty input). False when nothing
// was produced. The codec is the game's constructor 0x004EBFD0.
bool MemoryDBCompressRecord(resource::MemoryDB* self, const void* src, uint32_t size, void** bufOut, uint32_t* sizeOut,
                            uint16_t* typeOut);

// TS3W.exe 0x0072B2A0, thiscall ret 0x14. MemoryDBCompressRecord for inputs of 50 bytes or
// more, but the result is kept only when its size is strictly under the input's; otherwise
// the fresh copy is freed and the outputs stay zero.
bool MemoryDBCompressIfSmaller(resource::MemoryDB* self, void* src, uint32_t size, void** bufOut, uint32_t* sizeOut,
                               uint16_t* typeOut);

// TS3W.exe 0x0072C130: thiscall ret 8. MemoryDB virtual slot 17 of the 0x00FFD5F8 view:
// stores the bytes of the record object `record` (its stream, via its slot 6/7) under `key`,
// replacing an entry that is already there. Inputs of 50 bytes or more are stored compressed
// when that is smaller (MemoryDBCompressIfSmaller), everything else as a fresh copy.
// Needs write access. Equal-sized compression falls back to raw bytes; empty records
// insert an empty entry. Compressed entries retain the original uncompressed size.
bool MemoryDBStoreRecord(resource::MemoryDB* self, void* record, const resource::ResourceKey* key);

#if defined(_M_IX86)
// TS3W.exe 0x0072C130: thiscall-compatible hook adapter; EDX is unused.
bool __fastcall MemoryDBStoreRecordHook(resource::MemoryDB* self, void*, void* record, const resource::ResourceKey* key);
#endif

}  // namespace ts3::resourceload