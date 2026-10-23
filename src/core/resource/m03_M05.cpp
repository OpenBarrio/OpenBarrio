#include "ts3/resource/m03_M05.h"
namespace ts3::resource::m03::m05 {
namespace {
template<uint32_t A> void* Address() {
#if defined(_M_IX86)
    return reinterpret_cast<void*>(A);
#else
    NotRewritten("m03_M05 original global boundary");
#endif
}
struct Path { char16_t* begin; char16_t* end; char16_t* capacity; const char* name; char16_t* local; char16_t buffer[260]; };
struct Reader { void* vtable; void* record; void* stream; uint32_t reserved[2]; uint8_t valid; };
struct Memory { void* vtable; void* block; };
struct MemoryBlock { void* reserved[2]; void* data; };
void Release(void* p) { if(p) CallGameVirtual<void,2>(p); }
}
uint8_t StartCache(CacheRegistration* self) {
    *static_cast<CacheRegistration**>(Address<0x011D3E40>())=self;
    void* service=CallGameFunction<void*,0x0072F1C0>();
    void* base=CallGameVirtual<void*,10>(service);
    if(!base) return 1;
    Path path{};
    path.begin=path.end=path.local=path.buffer;
    path.capacity=path.buffer+260;
    path.name=static_cast<const char*>(Address<0x00FFF8E8>());
    CallGameMethod<void,0x0058BC30>(&path,base);
    auto suffix=static_cast<const char16_t*>(Address<0x00FFF8C0>());
    const char16_t* end=suffix;
    do { ++end; } while(*end);
    CallGameMethod<void,0x00455F50>(&path,suffix,end);
    void* allocation=CallGameFunction<void*,0x004E41F0>(0x398,Address<0x00FFF8AC>(),0,0,0,0);
    self->database=allocation ? CallGameMethod<void*,0x004A8F70>(allocation,static_cast<void*>(nullptr),0) : nullptr;
    auto cleanup=[&] {
        const auto bytes=reinterpret_cast<uintptr_t>(path.capacity)-reinterpret_cast<uintptr_t>(path.begin);
        if(static_cast<int32_t>(bytes&~uintptr_t(1))>2 && path.begin && path.begin!=path.local)
            CallGameFunction<void,0x004E3F70>(path.begin);
    };
    if(!self->database) { cleanup(); return 1; }
    auto ref=[&] { return static_cast<uint8_t*>(self->database)+sizeof(void*); };
    CallGameVirtual<void,0>(ref());
    if(!CallGameVirtual<uint8_t,1>(self->database)) {
        CallGameVirtual<void,1>(ref()); self->database=nullptr; cleanup(); return 1;
    }
    CallGameVirtual<void,11>(self->database,path.begin);
    if(!CallGameVirtual<uint8_t,6>(self->database,3,4,0)) {
        CallGameVirtual<void,2>(self->database);
        CallGameVirtual<void,1>(ref()); self->database=nullptr;
    }
    // Deliberate EA hazard: open failure clears the DB but continues to dereference it.
    uint64_t hash=CallGameFunction<uint64_t,0x007CFE40>(Address<0x00FFF88C>());
    struct Key { uint64_t hash; uint32_t type,group; } key{hash,0,0};
    void* first=nullptr;
    auto open=[&](void** out,int mode) { return CallGameVirtual<uint8_t,13>(self->database,&key,out,mode,6,1,0); };
    if(open(&first,1)) CallGameVirtual<void,8>(first);
    else CallGameMethod<void,0x0073CDF0>(self);
    char16_t filename[260];
    if(!CallGameFunction<int,0x007D0580>(filename,260)) { Release(first); cleanup(); return 1; }
    uint32_t version=CallGameMethod<uint32_t,0x009F0DE0>(Address<0x011CDDE0>());
    uint32_t stamp=CallGameFunction<uint32_t,0x004DBC60>(filename);
    uint64_t fileHash=CallGameFunction<uint64_t,0x004DBCF0>(filename,2);
    struct Lookup { uint32_t kind,version; uint64_t hash; uint32_t stamp; } lookup{1,version,fileHash,stamp};
    void** asset=CallGameMethod<void**,0x00594A10>(Address<0x011CDDE0>(),&lookup);
    uint64_t assetHash=0;
    if(asset) {
        const char* name=static_cast<const char*>(Address<0x00FFF868>());
        struct Text { void* data; uintptr_t fields[3]; } text{};
        CallGameMethod<void,0x00408890>(&text,&name);
        CallGameMethod<void,0x00409010>(&text,*asset);
        CallGameMethod<void,0x004F1F50>(&text,Address<0x00FFF840>());
        assetHash=CallGameFunction<uint64_t,0x004DBCF0>(text.data,2);
        CallGameMethod<void,0x004EF330>(&text);
    }
    void* metadata=nullptr;
    bool matches=false;
    if(open(&metadata,1)) {
        void* stream=CallGameVirtual<void*,6>(metadata);
        if(stream) {
            Reader reader{};
            CallGameMethod<void,0x007D91B0>(&reader,stream,1);
            uint32_t format=0,storedVersion=0,storedStamp=0;
            uint64_t storedFile=0,storedAsset=0;
            CallGameMethod<void,0x00565700>(&reader,&format);
            CallGameMethod<void,0x00565700>(&reader,&storedVersion);
            CallGameMethod<void,0x00565700>(&reader,&storedStamp);
            CallGameMethod<void,0x005FE3C0>(&reader,&storedFile);
            CallGameMethod<void,0x005FE3C0>(&reader,&storedAsset);
            CallGameVirtual<void,15>(self->database,metadata);
            // Format is read but never compared by EA.
            matches=reader.valid && storedVersion==version && storedStamp==stamp && storedFile==fileHash && storedAsset==assetHash;
            CallGameMethod<void,0x007D7760>(&reader);
        }
    }
    if(matches) {
        Release(metadata); Release(first);
        CallGameMethod<void,0x00447420>(&path);
        return 1;
    }
    CallGameMethod<void,0x0073CDF0>(self);
    Release(metadata); metadata=nullptr;
    if(open(&metadata,2)) {
        void* stream=CallGameVirtual<void*,6>(metadata);
        if(stream) {
            Reader reader{};
            CallGameMethod<void,0x007D91B0>(&reader,stream,1);
            CallGameMethod<void,0x00565730>(&reader,1);
            CallGameMethod<void,0x00565730>(&reader,version);
            CallGameMethod<void,0x00565730>(&reader,stamp);
            CallGameMethod<void,0x005FE3F0>(&reader,fileHash);
            CallGameMethod<void,0x005FE3F0>(&reader,assetHash);
            CallGameMethod<void,0x007D7760>(&reader);
        }
        CallGameVirtual<void,15>(self->database,metadata);
        CallGameVirtual<void,9>(self->database);
    }
    Release(metadata); Release(first); cleanup(); return 1;
}
void AdaptStream(StreamAdapter* self) {
    if(!self->enabled) return;
    if(!self->record || !self->stream) { self->enabled=0; return; }
    if(CallGameVirtual<uint32_t,3>(self->stream)==0x347223d2) return;
    void* storage=CallGameFunction<void*,0x004E3F90>(0x2c,Address<0x0101A540>(),0,0,0,0);
    Memory* memory=storage ? CallGameMethod<Memory*,0x004DF0C0>(storage,0,0,Address<0x0101A540>()) : nullptr;
    if(!memory) { self->enabled=0; return; }
    CallGameVirtual<void,1>(memory);
    CallGameMethod<void,0x004DEAC0>(memory,1,1.0f);
    uint32_t size=CallGameVirtual<uint32_t,7>(self->stream);
    if(CallGameVirtual<uint8_t,8>(memory,size)) {
        CallGameMethod<void,0x004DEAC0>(memory,1,0.0f);
        void* data=memory->block ? static_cast<MemoryBlock*>(memory->block)->data : nullptr;
        if(CallGameVirtual<uint32_t,12>(self->stream,data,size)) {
            void* block=CallGameFunction<void*,0x004E41F0>(0x28,Address<0x0101A514>(),0,0,0,0);
            if(block) {
                void* dependency=CallGameVirtual<void*,4>(self->record,1,1);
                void* record=CallGameMethod<void*,0x004B3650>(block,0,memory,dependency);
                if(record) {
                    CallGameVirtual<void,1>(record);
                    CallGameVirtual<void,8>(self->record);
                    CallGameVirtual<void,2>(self->record);
                    self->record=record; self->stream=memory; return;
                }
            }
        }
    }
    CallGameVirtual<void,0>(memory,1);
    self->enabled=0;
}
void DestroyLocaleMutex() { CallGameMethod<void,0x004E1670>(Address<0x011CE2D8>()); }
void DestroyDirectoryMutex() { CallGameMethod<void,0x004E1670>(Address<0x011D3C98>()); }
}
