#include "ts3/resource/m03_D05.h"
#include <cstring>
namespace ts3::resource::m03::d05 {
namespace {
// Boundary seams intentionally remain original dispatch: the fixture patches these
// dependencies in the mapped image, without running real package/platform services.
template<unsigned A> void* Address() {
#if defined(_M_IX86)
    return reinterpret_cast<void*>(A);
#else
    NotRewritten("m03_D05 game global/vtable (native dispatch not installed)");
#endif
}
uint32_t Read(const void* p,size_t offset) {
    uint32_t result; std::memcpy(&result,static_cast<const uint8_t*>(p)+offset,4); return result;
}
}
uint8_t Mount(const char16_t* path,uint32_t flags) {
    void* manager=CallGameFunction<void*,0x004AFD20>();
    CallGameFunction<void,0x004A56B0>(path,manager,1,0,0,flags);
    void* memory=CallGameFunction<void*,0x004E41F0>(0xb0,Address<0x0101A4FC>(),0,0,0,0);
    Database* db=nullptr;
    if(memory) {
        db=CallGameMethod<Database*,0x004A5500>(memory,path,0);
        if(db) CallGameVirtual<uintptr_t,0>(&db->refVtable);
    }
    // EA intentionally dereferences null here after allocation/constructor failure.
    uint8_t result=0;
    if(CallGameVirtual<uint8_t,1>(db) && CallGameVirtual<uint8_t,6>(db,1,6,0))
        result=CallGameVirtual<uint8_t,13>(manager,1,db,flags);
    CallGameVirtual<uintptr_t,1>(&db->refVtable);
    return result;
}
Key* Resolve(Key* out,const Key* key) {
    const Key* selected=key;
    Key candidate;
    if(!(key->words[3]&0xf0000000u)) {
        void* manager=CallGameFunction<void*,0x004AFD20>();
        if(manager && !CallGameVirtual<uintptr_t,17>(manager,key)) {
            CallGameFunction<Key*,0x007D7580>(&candidate,key);
            if(CallGameFunction<uint8_t,0x0054DD80>(&candidate,key) &&
                CallGameVirtual<uintptr_t,17>(manager,&candidate)) selected=&candidate;
        }
    }
    // Do not replace with memcpy: the original copies individual dwords in this
    // order, including overlapping input/output views.
    const uint32_t a=selected->words[0], b=selected->words[1];
    out->words[0]=a;
    const uint32_t c=selected->words[2]; out->words[1]=b;
    const uint32_t d=selected->words[3]; out->words[2]=c; out->words[3]=d;
    return out;
}
uint8_t Enumerate(void* database,void* predicateContext,void* argument) {
    Predicate predicate{Address<0x00FD8248>(),predicateContext};
    KeyList keys{nullptr,nullptr,nullptr,static_cast<const char*>(Address<0x0101A578>())};
    uint8_t result=0;
    if(CallGameVirtual<uint32_t,12>(database,&keys,&predicate)) {
        void* service=CallGameFunction<void*,0x0072F1C0>();
        result=CallGameVirtual<uint8_t,1>(service,keys.begin,argument,database,1,6);
    }
    if(keys.begin) CallGameFunction<void,0x004E3F70>(keys.begin);
    return result;
}
int32_t Unsupported(void*) { return -1; }
uint32_t Field0C(const void* self) { return Read(self,0x0c); }
const uint64_t* ServiceId(void*) { return static_cast<const uint64_t*>(Address<0x0102F674>()); }
uint8_t Observe(void*,void*,void*) { return 1; }
uint32_t Field18(const void* self) { return Read(self,0x18); }
void* Destroy(void* self,uint32_t flags) {
    *static_cast<void**>(self)=Address<0x00FB2CBC>();
    if(flags&1) CallGameFunction<void,0x004E3F70>(self);
    return self;
}
uint32_t Field74(const void* self) { return Read(self,0x74); }
uint32_t Field78(const void* self) { return Read(self,0x78); }
uint32_t Field08(const void* self) { return Read(self,0x08); }
void* Storage(void* self) { return static_cast<uint8_t*>(self)+0xd8; }
int InitLocaleMutex() {
    CallGameMethod<void,0x004E17E0>(Address<0x011CE2D8>(),static_cast<void*>(nullptr),true);
    return CallGameFunction<int,0x00F7E9A1>(Address<0x00F91910>());
}
int InitDirectoryMutex() {
    CallGameMethod<void,0x004E17E0>(Address<0x011D3C98>(),static_cast<void*>(nullptr),true);
    return CallGameFunction<int,0x00F7E9A1>(Address<0x00F925D0>());
}
}
