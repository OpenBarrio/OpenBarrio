// DatabasePackedFile's core object methods that the resource-reading paths in
// resource/packed_file.cpp do not contain: the secondary interface (vtable 0x00FB25F8),
// the record-mutex wrapper, the close/commit sequences and the destructor thunks.
// DatabasePackedFile's layout (resource/packed_file.h) is the object's.
//
// Already rewritten in resource/packed_file.cpp and not repeated here: slot 6
// (0x004A76E0 Open) and slot 13 (0x004A94C0 ReadRecord).
#pragma once

#include <cstdint>

#include "ts3/resource/packed_file.h"

namespace ts3::misc {

// DatabasePackedFile's secondary interface subobject (this = DatabasePackedFile + 4); the
// game's vtable for it is at 0x00FB25F8. It only answers the two interface ids, so the
// object itself is never dereferenced.
struct PackedFileInterface {
    // TS3W.exe 0x004A6630: id 0x0226A25B returns the primary object (this - 4), id
    // 0xEE3F516E returns this subobject unless its address is literally 4; anything else 0.
    void* QueryInterface(uint32_t id);
    // TS3W.exe 0x004A84E0: the interface's own type id.
    uint32_t InterfaceType();
};

// The class's own methods, at DatabasePackedFile's addresses.
struct PackedFileCore : resource::DatabasePackedFile {
    // TS3W.exe 0x004A6660: locks (arg nonzero, byte-tested) or unlocks the record mutex.
    int32_t LockRecordMutex(uint8_t lock);
    // TS3W.exe 0x004A6AE0, vtable slot 4: when the file is open, flushes through slot 7,
    // gives the index to slot 38, releases the stream (slot 2 of the stream) and clears the
    // open flag. Always returns true.
    bool Close();
    // TS3W.exe 0x004A6BD0: under the record mutex, calls the index's slot 7 with (a0, a1);
    // no index means no call and a 0 result.
    int32_t IndexCall(uint32_t a0, uint32_t a1);
    // TS3W.exe 0x004A8D00, vtable slot 0: runs the pre-destructor 0x004A8440 and, when the
    // low byte of flags is set, frees the object; returns this (the deleting destructor).
    void* Delete(uint32_t flags);
    // TS3W.exe 0x004A8D20: 0x004A8500(1) - flush the open package for writing and close it.
    uint8_t FlushAndClose();
    // TS3W.exe 0x004A8D30: with no access flags, forgets the memory image and assigns the
    // package path (this + 0x24) from the wide string source; true on both paths' low byte.
    bool SetMemoryPath(const wchar_t* source);
    // TS3W.exe 0x004A9B70, vtable slot 9: writes a changed package. Under the record mutex,
    // only with write access and a changed header, index or free space: rebuilds free space
    // (0x004A9B20) when the flags say so, then slots 36, 33 and 0x004A7630 in order, then
    // drains free space 2 into free space and truncates the file to its total.
    bool Commit();
};

}  // namespace ts3::misc