// DatabasePackedFile's core methods (TS3W.exe 0x004A6630 group). The original addresses
// are named per function; the object layout is DatabasePackedFile's (see
// resource/packed_file.h). Calls into units that are not rewritten yet go through the
// bridge; the already-verified ones (record mutex, free-space maps, the services helpers)
// are called directly.
#include "ts3/misc/cls_4a6630_x.h"

#include "ts3/cas/clu_4a6710_CAS.h"
#include "ts3/io/memory_stream.h"
#include "ts3/layout.h"
#include "ts3/services/cls_4a4ba0_Services.h"
#include "ts3/thread/mutex.h"

namespace ts3::misc {

void* PackedFileInterface::QueryInterface(uint32_t id) // 004A6630
{
    if (id == 0x0226A25Bu)
        return reinterpret_cast<uint8_t*>(this) - 4;
    if (id == 0xEE3F516Eu) {
        // The game's neg/sbb/and: the subobject is returned unless its address is 4.
        return reinterpret_cast<uintptr_t>(this) != 4 ? this : nullptr;
    }
    return nullptr;
}

uint32_t PackedFileInterface::InterfaceType() // 004A84E0
{
    return 0x2A43BD44u;
}

int32_t PackedFileCore::LockRecordMutex(uint8_t lock) // 004A6660
{
    // The game byte-tests the argument and forwards kTimeoutNone (0xFFFFFFFF) as a pointer
    // to the timeout constant, so Lock's argument is a reference.
    if (lock)
        return recordMutex.Lock(thread::kTimeoutNone);
    return recordMutex.Unlock();
}

bool PackedFileCore::Close() // 004A6AE0
{
    if (opensFile) {
        if (openFlags)
            CallGameVirtual<void, 7>(this);
        CallGameVirtual<void, 38>(this, index);
        index = nullptr;
        io::IStream* own = stream;  // the field is cleared before the stream is released
        if (own) {
            stream = nullptr;
            CallGameVirtual<void, 2>(own);
        }
        opensFile = false;
    }
    return true;
}

int32_t PackedFileCore::IndexCall(uint32_t a0, uint32_t a1) // 004A6BD0
{
    recordMutex.Lock(thread::kTimeoutNone);
    if (!index) {
        recordMutex.Unlock();
        return 0;
    }
    const int32_t result = CallGameVirtual<int32_t, 7>(index, a0, a1);
    recordMutex.Unlock();
    return result;
}

void* PackedFileCore::Delete(uint32_t flags) // 004A8D00
{
    CallGameMethod<void, 0x004A8440>(this);  // base destructor (cls_4a7360_x, not rewritten)
    if (static_cast<uint8_t>(flags) & 1)
        io::EaDelete(this);
    return this;
}

uint8_t PackedFileCore::FlushAndClose() // 004A8D20
{
    // The flag is tested for zero/nonzero only; pass it as the boolean it is.
    return CallGameMethod<uint8_t, 0x004A8500>(this, true);
}

bool PackedFileCore::SetMemoryPath(const wchar_t* source) // 004A8D30
{
    // A refusal (access flags set) leaves every field untouched; EA returns 0 then (the
    // success path returns 1).
    const bool accepted = openFlags == 0;
    if (accepted) {
        memoryData = nullptr;
        memorySize = 0;
        const wchar_t* end = source;
        if (*source) {
            do
                ++end;
            while (*end);
        }
        // eastl::wstring::assign on the string object at this + 0x24 (clu_408e20_Scratch,
        // not rewritten): called with the string as `this` and [source, end) on the stack.
        CallGameMethod<void, 0x00408E20>(reinterpret_cast<void*>(&path), source, end);
    }
    return accepted;
}

bool PackedFileCore::Commit() // 004A9B70
{
    recordMutex.Lock(thread::kTimeoutNone);
    uint8_t result = 1;
    const uint32_t access = openFlags;
    if (access != 0 && (access & 2) != 0 &&
        (indexChanged || freeSpaceChanged || headerDirty)) {
        if ((writeFlags & 1) != 0 && services::ExceedsFreeRatio(this))
            CallGameMethod<void, 0x004A9B20>(this);  // clu_4a9b20_Scratch, not rewritten
        result = CallGameVirtual<uint8_t, 36>(this);
        if (result) {
            result = CallGameVirtual<uint8_t, 33>(this);
            if (result) {
                result = CallGameMethod<uint8_t, 0x004A7630>(this);  // cls_4a7360_x
                if (result) {
                    // Drain free space 2 (this + 0x368) into free space (this + 0x344),
                    // then truncate the file to where the free space ends.
                    reinterpret_cast<cas::CacheFreeSpaceMap*>(&freeSpace2)
                        ->Drain(reinterpret_cast<cas::CacheFreeSpaceMap*>(&freeSpace));
                    SetFileSize(static_cast<uint32_t>(freeSpace.total));
                }
            }
        }
    }
    recordMutex.Unlock();
    return result != 0;
}

}  // namespace ts3::misc