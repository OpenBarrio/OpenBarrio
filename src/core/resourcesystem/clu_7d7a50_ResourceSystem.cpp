#include "ts3/resourcesystem/clu_7d7a50_ResourceSystem.h"
#include "ts3/resource/packed_file.h"
#include "ts3/resource/databases.h"
namespace ts3::resourcesystem {
namespace {
enum class Source { Memory, Stream, File };
void* ScratchAllocator() {
#if defined(_M_IX86)
    // x64: this singleton must be owned by the port when the scratch unit is rewritten.
    return *reinterpret_cast<void**>(uintptr_t(0x011CB864));
#else
    NotRewritten("the resource-copy scratch allocator singleton");
#endif
}
template<Source source, bool toMemory>
bool CopyOne(void* destination, void* from, const ResourceKey* key, bool remove, void* mapper) {
    CopyRecordInfo info;
    if (!CallGameVirtual<bool, 13>(from, key, 0u, 1u, 6u, 0u, &info)) return false;
    void* bytes = CallGameMethod<void*, 0x004E69D0>(ScratchAllocator(), info.size, 0u);
    if (!bytes) return false;
    bool read;
    if constexpr(source == Source::Memory)
        read = CallGameMethod<bool, 0x0072B510>(from, key, bytes, info.size);
    else if constexpr(source == Source::Stream)
        read = static_cast<resource::DatabasePackedStream*>(from)->ReadRecordRaw(key, bytes, info.size);
    else
        read = static_cast<resource::DatabasePackedFile*>(from)->ReadRecordRaw(key, bytes, info.size);
    bool copied = false;
    if (read) {
        ResourceKey mapped;
        const ResourceKey* result = mapper ? CallGameVirtual<const ResourceKey*, 0>(mapper, &mapped, key) : key;
        const ResourceKey writeKey = *result; // original copies returned key before write
        if constexpr(toMemory)
            copied = CallGameMethod<bool, 0x0072C350>(destination, &writeKey, bytes, info.size, info.memorySize, info.compression);
        else
            copied = CallGameMethod<bool, 0x004A7FC0>(destination, &writeKey, bytes, info.size, info.memorySize, info.compression);
        if (copied && remove) CallGameVirtual<void, 16>(from, key);
    }
    CallGameMethod<void, 0x004E5920>(ScratchAllocator(), bytes);
    return copied;
}
using One = bool(*)(void*, void*, const ResourceKey*, bool, void*);
uint32_t CopyBatch(void* destination, void* from, void* filter, bool remove, void* mapper, uint32_t* failed, One one) {
    if (!from || !destination) return 0;
    uint32_t copied = 0;
    if (failed) *failed = 0;
    CallGameVirtual<void, 5>(destination, true);
    CallGameVirtual<void, 5>(from, true);
    CopyKeyList keys{};
#if defined(_M_IX86)
    keys.tag = reinterpret_cast<const char*>(uintptr_t(0x00FFE150)); // x64: native tag string below
#else
    keys.tag = "ResourceSystem/KeyList";
#endif
    CallGameVirtual<void, 12>(from, &keys, filter);
    for (ResourceKey* key = keys.begin; key != keys.end; ++key) {
        if (one(destination, from, key, remove, mapper)) ++copied;
        else if (failed) ++*failed;
    }
    CallGameVirtual<void, 5>(from, false);
    CallGameVirtual<void, 5>(destination, false);
    if (keys.begin) CallGameFunction<void, 0x004E3F70>(keys.begin);
    return copied;
}
// 0x7D7DD0 is outside this unit's supplied membership; preserve the original dependency.
bool OriginalMemoryToMemory(void* d, void* s, const ResourceKey* k, bool r, void* m) {
    return CallGameFunction<bool, 0x007D7DD0>(d, s, k, r, m);
}
}
// 0x007D7A50
bool CopyRecordMemoryToFile(void* d, void* s, const ResourceKey* k, bool r, void* m) { return CopyOne<Source::Memory, false>(d,s,k,r,m); }
// 0x007D7B30
bool CopyRecordStreamToFile(void* d, void* s, const ResourceKey* k, bool r, void* m) { return CopyOne<Source::Stream, false>(d,s,k,r,m); }
// 0x007D7C10
bool CopyRecordFileToFile(void* d, void* s, const ResourceKey* k, bool r, void* m) { return CopyOne<Source::File, false>(d,s,k,r,m); }
// 0x007D7CF0
bool CopyRecordFileToMemory(void* d, void* s, const ResourceKey* k, bool r, void* m) { return CopyOne<Source::File, true>(d,s,k,r,m); }
// 0x007D7EB0
bool CopyRecordStreamToMemory(void* d, void* s, const ResourceKey* k, bool r, void* m) { return CopyOne<Source::Stream, true>(d,s,k,r,m); }
// 0x007D8870
uint32_t CopyMemoryToFile(void* d,void* s,void* f,bool r,void* m,uint32_t* e) { return CopyBatch(d,s,f,r,m,e,&CopyRecordMemoryToFile); }
// 0x007D8960
uint32_t CopyStreamToFile(void* d,void* s,void* f,bool r,void* m,uint32_t* e) { return CopyBatch(d,s,f,r,m,e,&CopyRecordStreamToFile); }
// 0x007D8A50
uint32_t CopyFileToFile(void* d,void* s,void* f,bool r,void* m,uint32_t* e) { return CopyBatch(d,s,f,r,m,e,&CopyRecordFileToFile); }
// 0x007D8B40
uint32_t CopyFileToMemory(void* d,void* s,void* f,bool r,void* m,uint32_t* e) { return CopyBatch(d,s,f,r,m,e,&CopyRecordFileToMemory); }
// 0x007D8C30
uint32_t CopyMemoryToMemory(void* d,void* s,void* f,bool r,void* m,uint32_t* e) { return CopyBatch(d,s,f,r,m,e,&OriginalMemoryToMemory); }
// 0x007D8D20
uint32_t CopyStreamToMemory(void* d,void* s,void* f,bool r,void* m,uint32_t* e) { return CopyBatch(d,s,f,r,m,e,&CopyRecordStreamToMemory); }
// 0x007D8F00
uint32_t CopyDatabaseRecords(void* d,void* s,void* f,bool r,void* m,uint32_t* e) {
    if (!d || !s) return 0;
    const uint32_t destinationType = CallGameVirtual<uint32_t,3>(d);
    const uint32_t sourceType = CallGameVirtual<uint32_t,3>(s);
    if (destinationType == 0x2A43BD44) {
        if (sourceType == 0x03DB0260) return CopyMemoryToFile(d,s,f,r,m,e);
        if (sourceType == 0x03C4B9A4) return CopyStreamToFile(d,s,f,r,m,e);
        if (sourceType == 0x2A43BD44) return CopyFileToFile(d,s,f,r,m,e);
    } else if (destinationType == 0x03DB0260) {
        if (sourceType == 0x2A43BD44) return CopyFileToMemory(d,s,f,r,m,e);
        if (sourceType == 0x03DB0260) return CopyMemoryToMemory(d,s,f,r,m,e);
        if (sourceType == 0x03C4B9A4) return CopyStreamToMemory(d,s,f,r,m,e);
    }
    return 0;
}
}
