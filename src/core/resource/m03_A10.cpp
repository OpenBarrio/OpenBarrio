#include "ts3/resource/m03_A10.h"
#include "ts3/layout.h"
#include <cstring>
namespace ts3::resource::m03::a10 {
namespace {
template<unsigned A> void* Address() {
#if defined(_M_IX86)
    return reinterpret_cast<void*>(uintptr_t(A));
#else
    AbortNotRewritten("A10 game global/callback"); return nullptr;
#endif
}
void Free(void* p) { CallGameFunction<void,0x004E3F70>(p); }
}
void ReleaseWrapper(void* p) {
    if(p) {
        CallGameMethod<void,0x00401040>(p);
        CallGameFunction<void,0x00401F10>(p);
    }
}
Driver* ConstructName(Driver* p,const char* name) {
    CallGameMethod<void,0x00401860>(p,name);
    p->table=Address<0x00FFD9A8>();
    return p;
}
StreamDriver* ConstructStream(StreamDriver* p,const char* name,void* arg) {
    CallGameMethod<void,0x00401860>(p,name);
    p->argument=arg;
    p->base.table=Address<0x00FFD9F8>();
    CallGameMethod<void,0x004E43D0>(&p->pool,16,0,128,0,-1,0,0,0);
    CallGameMethod<void,0x004E17E0>(&p->mutex,0,1);
    return p;
}
uint32_t ConfigurePool(StreamDriver* p) {
    uint32_t r=CallGameMethod<uint32_t,0x004E4350>(&p->pool,16,0,16,0,0,Address<0x0072E1E0>(),Address<0x004E8590>(),static_cast<void*>(nullptr));
    return (r & 0xffffff00u) | ((r & 255u)!=0 ? 1u:0u);
}
void Publish(StreamDriver* p,Published* r) {
    if(!r) return;
    if(r->handle) {
        void* wrapper=CallGameFunction<void*,0x00401EF0>(0x48);
        if(wrapper) wrapper=CallGameMethod<void*,0x00401000>(wrapper);
        CallGameMethod<void,0x004013A0>(wrapper,r->handle,Address<0x0072E200>(),0,0);
    }
    if(r->record) {
        CallGameVirtual<void,21>(r->record);
        CallGameVirtual<void,1>(static_cast<uint8_t*>(r->record)+sizeof(void*));
    }
    CallGameMethod<void,0x004E16F0>(&p->mutex,Address<0x00FFD9A0>());
    // Pool free-list at pool+10 (driver+24), independent from live-list fields.
    r->next=p->pool.head;
    p->pool.head=r;
    CallGameMethod<void,0x004E17B0>(&p->mutex);
}
Driver* DeleteName(Driver* p,uint32_t flags) {
    p->table=Address<0x00FFD9A8>();
    CallGameMethod<void,0x004018A0>(p);
    if(flags & 1u) Free(p);
    return p;
}
void* Open(Driver* p,const char* path,uint32_t flags,uint32_t unused) {
    void* manager=CallGameFunction<void*,0x004AFD20>();
    uint32_t parsed[4]{};
    if(!(CallGameFunction<uint32_t,0x0072DF80>(path,p->first->name,parsed,manager)&255)) return nullptr;
    uint32_t key[4];
    CallGameFunction<void,0x007D80E0>(key,parsed);
    void* database=CallGameVirtual<void*,17>(manager,key);
    if(!database) return nullptr;
    void* ref=static_cast<uint8_t*>(database)+sizeof(void*);
    CallGameVirtual<void,0>(ref);
    uint32_t mode=flags,access=unused,one=0;
    CallGameFunction<void,0x0072DFF0>(flags,&one,&access,&mode);
    void* result=nullptr;
    uint32_t status=CallGameVirtual<uint32_t,13>(database,key,&result,one,access,mode,0);
    if(!(status&255)) {
        if(result) CallGameVirtual<void,2>(result);
        CallGameVirtual<void,1>(ref);
        return nullptr;
    }
    CallGameVirtual<void,1>(ref);
    return result;
}
StreamDriver* DeleteStream(StreamDriver* p,uint32_t flags) {
    p->base.table=Address<0x00FFD9F8>();
    CallGameMethod<void,0x004E1670>(&p->mutex);
    CallGameMethod<void,0x004E42E0>(&p->pool);
    CallGameMethod<void,0x004018A0>(p);
    if(flags & 1u) Free(p);
    return p;
}
uint8_t Category(uint32_t type,uint32_t index,void* out) {
    const uint8_t* source=nullptr;
    if(type==1 && index<12) source=static_cast<uint8_t*>(Address<0x011D3CC8>())+16*index;
    else if(type==2 && uint32_t(index-1)<=9) source=static_cast<uint8_t*>(Address<0x011D3D78>())+16*index;
    if(!source) return 0;
    CallGameMethod<void,0x00500310>(out,source);
    return 1;
}
uint8_t IsCategory(const void* in) {
    auto* first=static_cast<uint8_t*>(Address<0x011D3CC8>());
    for(unsigned i=0;i<12;++i) if(CallGameFunction<uint8_t,0x00500AC0>(first+16*i,in)) return 1;
    auto* second=static_cast<uint8_t*>(Address<0x011D3D88>());
    for(unsigned i=0;i<10;++i) if(CallGameFunction<uint8_t,0x00500AC0>(second+16*i,in)) return 1;
    return 0;
}
void __stdcall DestroyRange(Mapping* first,Mapping* last) {
    for(auto* p=first;reinterpret_cast<uintptr_t>(p)<reinterpret_cast<uintptr_t>(last);++p) {
        intptr_t bytes=static_cast<intptr_t>(reinterpret_cast<uintptr_t>(p->path.capacity)-reinterpret_cast<uintptr_t>(p->path.begin));
        if((bytes & ~intptr_t(1))>2 && p->path.begin) Free(p->path.begin);
    }
}
Mapping** CopyRange(Mapping** cell,const Mapping* first,const Mapping* last,Mapping* dest,uintptr_t) {
    *cell=dest;
    for(auto* p=first;p!=last;++p) {
        auto* d=*cell;
        if(d) {
            d->scalar[0]=p->scalar[0]; d->scalar[1]=p->scalar[1];
            d->path.begin=nullptr; d->path.end=nullptr; d->path.capacity=nullptr;
            d->path.name=p->path.name;
            intptr_t bytes=static_cast<intptr_t>(reinterpret_cast<uintptr_t>(p->path.end)-reinterpret_cast<uintptr_t>(p->path.begin));
            int32_t n=static_cast<int32_t>(bytes)>>1;
            uint32_t capacity=uint32_t(n)+1;
            char16_t* text;
            if(capacity>1) {
                text=CallGameFunction<char16_t*,0x004E3F90>(capacity*2,p->path.name,0,0,Address<0x00F98980>(),0xe7);
                d->path.capacity=reinterpret_cast<char16_t*>(reinterpret_cast<uintptr_t>(text)+capacity*2);
            } else {
                text=static_cast<char16_t*>(Address<0x011C7E54>());
                d->path.capacity=text+1;
            }
            d->path.begin=text; d->path.end=text;
            std::memcpy(text,p->path.begin,uint32_t(n)*2);
            d->path.end=text+n;
            text[n]=0;
            CallGameMethod<void,0x00500310>(&d->identifier,&p->identifier);
        }
        *cell=reinterpret_cast<Mapping*>(reinterpret_cast<uintptr_t>(*cell)+sizeof(Mapping));
    }
    return cell;
}
Mapping* AssignRange(const Mapping* first,const Mapping* last,Mapping* destEnd) {
    while(last!=first) {
        --last; --destEnd;
        destEnd->scalar[1]=last->scalar[1]; destEnd->scalar[0]=last->scalar[0];
        if(&destEnd->path!=&last->path) CallGameMethod<void,0x00408E20>(&destEnd->path,last->path.begin,last->path.end);
        CallGameMethod<void,0x00500310>(&destEnd->identifier,&last->identifier);
    }
    return destEnd;
}
Directory* DeleteDirectory(Directory* p,uint32_t flags) {
    p->table=Address<0x00FFDAE8>();
    DestroyRange(p->begin,p->end);
    p->end=p->begin;
    DestroyRange(p->begin,p->begin);
    if(p->begin) Free(p->begin);
    p->table=Address<0x00FB84D0>();
    if(flags&1u) CallGameFunction<void,0x004E4230>(p);
    return p;
}
}
