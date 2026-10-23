// DatabasePackedFile's remaining vtable methods: the stream / memory-image / index and
// resource-manager plumbing around the class laid out in ts3/resource/packed_file.h. The
// addresses are slots 14, 17, 18, 20, 21, 22, 23, 24 and 26 of the base vtable 0x00FB2600
// (the record factories, header readers and record deletion of the same vtable are the
// packed-file pilot's rewrites and are called directly).
#pragma once

#include <cstdint>

#include "ts3/io/stream.h"
#include "ts3/resource/packed_file.h"
#include "ts3/resource/package_index.h"

namespace ts3::misc {

struct PackedFileLifecycle : resource::DatabasePackedFile {
    // TS3W.exe 0x004A6690 (slot 23, ret 4): returns the index object and, while the
    // package is not open (openFlags == 0), replaces it with the argument. The old pointer
    // is returned on both paths.
    void* SetIndexIfClosed(void* newIndex);
    // TS3W.exe 0x004A66B0 (slot 24): the index object at +0x2D0.
    void* GetIndex();
    // TS3W.exe 0x00AE71E0 (slot 20): the file stream at +0x260.
    io::IStream* GetStream();
    // TS3W.exe 0x004A6B40 (slot 21, ret 4): adopts `newStream` when the package is not
    // open. The new stream is add-ref'd and the old one released (stream slots 1 and 2)
    // only when they differ; adopting the same stream returns true without touching the
    // reference counts. Always true while closed, false while open.
    bool SetStream(io::IStream* newStream);
    // TS3W.exe 0x004A6B90 (slot 22, ret 8): adopts a memory image when the package is not
    // open. The path string at +0x24 is cleared in place when non-empty: a 16-bit zero at
    // its first character and +0x28 (the end pointer) becomes the begin pointer. The size
    // (+0x268) is stored before the data pointer (+0x264). Quirk kept: no null/unaligned
    // check on the begin pointer.
    bool SetMemoryImage(void* data, uint32_t size);
    // TS3W.exe 0x004A6E90 (slot 17, ret 4): the package's *modified* timestamp as a
    // 64-bit value (EDX:EAX, low half first), obtained through 0x004A6E00. The wrapper
    // passes the created output to +0x10 and the modified one to +0x00 of its frame and
    // returns the latter, so the created timestamp is discarded. The stack argument is
    // unused; when the package can not be opened the timestamp is 0.
    uint64_t ModifiedTime(int32_t unused);
    // TS3W.exe 0x004A8D80 (slot 14, ret 4): how many open records carry `key` exactly,
    // counted under the record mutex; 0 while the package is not open. Records are ordered
    // by instanceLow, instanceHigh, then type/group, so equal keys are adjacent.
    uint32_t CountOpenRecords(const resource::ResourceKey* key);
    // TS3W.exe 0x004A6A00 (slot 18, ret 0xC): attaches (attach = true) or detaches the
    // resource manager at +0x390. Attaching a different manager first detaches the old one
    // through slot 18 itself (attach false, result ignored). Without `force` the manager is
    // only notified through its slot 13 (`1, this, 0` on attach, `0, this, 0` on detach);
    // with it the field is set/cleared directly. Detaching a non-null manager that is not
    // the current one returns false. EA bug kept: detaching with no manager set calls
    // through the null pointer.
    bool AttachResourceManager(bool attach, resource::IResourceManager* manager, bool force);
    // TS3W.exe 0x004A66C0 (slot 26, ret 0x10): makes an index of `kind` (slot 25) and loads
    // `image` into it (index slot 13, third argument 1). With `store` the new index also
    // replaces the package's index (slot 23). The new index is returned even when loading
    // fails; null when creating it failed.
    resource::IPackageIndex* LoadIndexImage(const void* image, uint32_t size, bool store, uint32_t kind);
};

// The fields this unit reads, at the offsets the game's objects have.
TS3_X86_OFFSET(resource::DatabasePackedFile, path, 0x024);
TS3_X86_OFFSET(resource::DatabasePackedFile, unknown028, 0x028);
TS3_X86_OFFSET(resource::DatabasePackedFile, stream, 0x260);
TS3_X86_OFFSET(resource::DatabasePackedFile, memoryData, 0x264);
TS3_X86_OFFSET(resource::DatabasePackedFile, memorySize, 0x268);
TS3_X86_OFFSET(resource::DatabasePackedFile, index, 0x2D0);
TS3_X86_OFFSET(resource::DatabasePackedFile, openRecords, 0x2E8);
TS3_X86_OFFSET(resource::DatabasePackedFile, resourceManager, 0x390);

}  // namespace ts3::misc