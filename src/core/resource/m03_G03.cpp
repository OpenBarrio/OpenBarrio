#include "ts3/resource/m03_G03.h"
#include <atomic>
namespace ts3::resource::m03::g03 {
namespace {
void Retain(void* p) { if(p) std::atomic_ref<int32_t>(*reinterpret_cast<int32_t*>(static_cast<char*>(p)+sizeof(void*))).fetch_add(1); }
void Release(void* p) {
    if(!p) return;
    std::atomic_ref<int32_t> ref(*reinterpret_cast<int32_t*>(static_cast<char*>(p)+sizeof(void*)));
    if(ref.fetch_sub(1)==1) { ref.exchange(1); CallGameVirtual<void,0>(p,1); }
}
const char* Name(uintptr_t ea) {
#if defined(_M_IX86)
    return reinterpret_cast<const char*>(ea);
#else
    (void)ea; return "m03.G03";
#endif
}
void* Table(uintptr_t ea) {
#if defined(_M_IX86)
    return reinterpret_cast<void*>(ea);
#else
    (void)ea; NotRewritten("G03 native vtables require adapter");
#endif
}
void InitHash(b01::Hash& h,allocator::ICoreAllocator* a,uintptr_t name) {
    h.buckets=static_cast<b01::Node**>(Table(0x0114c900)); h.count=1; h.size=0;
    h.policy[0]=0x3f800000; h.policy[1]=0x40000000; h.policy[2]=0;
    h.allocator=GameAllocator(Name(name),a,0);
}
}
uint8_t __fastcall Callback(Manager* m,void*,uint32_t enable,void* object,int32_t priority) {
    auto& v=m->callbacks;
    if(uint8_t(enable)) {
        Retain(object); Retain(object); b01::Pair value{object,uintptr_t(uint32_t(priority))}; Release(object);
        auto* pos=v.begin;
        for(;pos!=v.end;++pos) {
            if(pos->object==object) { Release(object); return 0; }
            if(priority>int32_t(pos->flags)) break;
        }
        if(pos!=v.end) CallGameMethod<b01::Pair*,0x004B3040>(&v,pos,&value);
        else if(uintptr_t(v.end)>=uintptr_t(v.capacity)) CallGameMethod<uintptr_t,0x004B2620>(&v,pos,&value);
        else { auto* dest=v.end++; if(dest) { dest->object=object; Retain(object); dest->flags=value.flags; } }
        CallGameVirtual<void,1>(object);
        Release(object); return 1;
    }
    auto* pos=v.begin;
    for(;pos!=v.end;++pos) if(pos->object==object) break;
    if(pos==v.end) return 0;
    if(*reinterpret_cast<int32_t*>(static_cast<char*>(object)+sizeof(void*))==1) CallGameVirtual<void,2>(object);
    if(pos+1<v.end) CallGameFunction<b01::Pair*,0x004B1000>(pos+1,v.end,pos);
    --v.end; Release(v.end->object); return 1;
}
Manager* __fastcall Construct(Manager* m,void*,allocator::ICoreAllocator* a) {
    m->vtable=Table(0x00fb2da0); m->initialized=0;
    if(!a) a=CallGameFunction<allocator::ICoreAllocator*,0x00931020>();
    m->allocator=a; m->flag=0;
    m->files.left=&m->files; m->files.right=&m->files; m->files.parent=nullptr;
    m->files.color=0; m->files.count=0; m->files.allocator=GameAllocator(Name(0xfb2f08),a,0);
    m->databases.begin=m->databases.end=m->databases.capacity=nullptr;
    m->databases.allocator=GameAllocator(Name(0xfb2ee0),a,0);
    CallGameMethod<void,0x004E17E0>(&m->mutex,0,1);
    InitHash(m->factories,a,0xfb2eb8);
    m->callbacks.begin=m->callbacks.end=m->callbacks.capacity=nullptr;
    m->callbacks.allocator=GameAllocator(Name(0xfb2e90),a,0);
    m->policy=2; m->factor=0.8f;
    InitHash(m->typeNames,a,0xfb2e68); InitHash(m->keyNames,a,0xfb2e40);
    for(auto& lock:m->locks) CallGameMethod<void,0x004E17E0>(&lock,0,1);
    for(unsigned i=0;i<8;++i) { m->controls[i]=nullptr; m->types[i]=0; }
    return m;
}
Record* __fastcall Destroy(Record* r,void*,uint32_t flags) {
    r->vtable=Table(0xfb2f30);
    if(r->pending) {
        r->pending=0;
        if(r->open && r->stream) {
            CallGameVirtual<void,6>(r->stream);
            if(r->owned && r->stream) CallGameVirtual<void,0>(r->stream,1);
        }
    }
    r->vtable=Table(0xfb84d0);
    if(flags&1) CallGameFunction<void,0x004E4230>(r);
    return r;
}
}
