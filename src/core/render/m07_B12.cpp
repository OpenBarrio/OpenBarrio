#include "ts3/render/m07_B12.h"
#include <cstring>
#include <vector>
#include <intrin.h>
namespace ts3::render::m07::b12 {
namespace {
void* Table(uintptr_t a) { return reinterpret_cast<void*>(a); }
void* Global(uint32_t a) {
#if defined(_M_IX86)
    return *reinterpret_cast<void**>(a);
#else
    (void)a; AbortNotRewritten("B12 image global"); return nullptr;
#endif
}
template<class R,class... A> R Com(void* p,unsigned slot,A... args) {
    return reinterpret_cast<Stdcall<R,void*,A...>>((*reinterpret_cast<void***>(p))[slot])(p,args...);
}
void* Device() { return CallGameFunction<DeviceHost*,0x60e2c0>()->device; }
void Unregister(Links* p) { CallGameMethod<void,0x613f30>(p); }
void Register(Links* p) { CallGameMethod<void,0x613f10>(p); }
void Free(void* p) { CallGameFunction<void,0x4e3f70>(p); }
Stream* Base(Links* p) { return reinterpret_cast<Stream*>(reinterpret_cast<char*>(p)-offsetof(Stream,secondary)); }
void Tables(Stream* p) { p->primary=Table(0xfe9590);p->secondary.table=Table(0xfe95a0); }
// Direct CALL in EA leaves EAX live across helpers with no defined return. The
// general typed bridge may use EAX for its target, so seed these calls explicitly.
uint32_t MethodEax(uint32_t address,void* self,uint32_t residue) {
#if defined(_M_IX86)
    uint32_t result;
    __asm {
        mov ecx,self
        mov eax,residue
        call address
        mov result,eax
    }
    return result;
#else
    (void)address;(void)self;(void)residue;
    AbortNotRewritten("B12 EAX-live game method");return 0;
#endif
}
struct Iterator { void* node; void* bucket; void* end; };
struct Key { const Element* elements; int32_t count; };
}
uint8_t __fastcall CreateFormat(Format* p,void*,const Element* src,int32_t count) {
    uint32_t streams=0;
    for(int32_t i=0;i<count;++i) if(streams<src[i].stream) streams=src[i].stream;
    ++streams;p->count=count;
    uint32_t bytes=4u*(streams+2u*uint32_t(count));
    p->elements=CallGameMethod<Element*,0x4e69d0>(Global(0x11cb864),bytes,0u);
    if(!p->elements) return 0;
    std::memcpy(p->elements,src,8u*uint32_t(count));
    p->strides=reinterpret_cast<uint32_t*>(reinterpret_cast<uint8_t*>(p->elements)+8u*uint32_t(count));
    std::memset(p->strides,0,4u*streams);
    static constexpr uint8_t types[]={0,1,2,3,5,4,6,7,8,9,10,11,12,13,14,15,16};
    static constexpr uint8_t usages[]={0,3,5,2,1,6,10};
    static constexpr uint8_t methods[]={0,0,3,5,2,1,6,10};
    static constexpr uint8_t widths[]={4,8,12,16,4,4,4,8,4,4,8,4,8,4,4,4,8};
    std::vector<Element> converted(size_t(uint32_t(count))+1);
    for(int32_t i=0;i<count;++i) {
        const auto& e=p->elements[i];converted[i]={e.stream,e.offset,types[e.type],methods[e.method],usages[e.usage],e.index};
        uint32_t end=uint32_t(e.offset)+widths[e.type];
        if(p->strides[e.stream]<end) p->strides[e.stream]=end;
    }
    converted[uint32_t(count)]={255,0,17,0,0,0};
    int32_t hr=Com<int32_t>(Device(),0x158/4,converted.data(),&p->declaration);
    if(hr<0) {
        Unregister(&p->secondary);
        if(p->declaration) {
            CallGameMethod<void,0x60efe0>(Global(0x11cecc8),p);
            Com<uint32_t>(p->declaration,2);p->declaration=nullptr;
        }
        if(p->elements) {
            CallGameMethod<void,0x4e5920>(Global(0x11cb864),p->elements);
            p->elements=nullptr;p->strides=nullptr;
        }
        return 0;
    }
    Register(&p->secondary);return 1;
}
uint8_t __fastcall CreateStream(Stream* p,void*,uint32_t size,uint32_t flags,uint32_t) {
    p->flags=flags;p->size=size;
    if(Com<int32_t>(Device(),26,size,(flags&1)?520u:0u,0u,(flags&1)?0u:1u,&p->buffer,0u)<0) return 0;
    Register(&p->secondary);return 1;
}
extern "C" uint32_t __cdecl B12Release(Stream* p,uint32_t residue) {
    residue=MethodEax(0x613f30,&p->secondary,residue);
    return MethodEax(0x619f10,p,residue);
}
uint8_t __fastcall Upload(Stream* p,void*,const void* source,uint32_t bytes) {
    if(bytes>p->size || !p->buffer) return 0;
    void* dest=nullptr;
    if(Com<int32_t>(p->buffer,11,0u,bytes,&dest,0u)<0 || !dest) return 0;
    std::memcpy(dest,source,bytes);Com<uint32_t>(p->buffer,12);return 1;
}
// These wrappers retain the incoming EAX for paths that only touch AL (or no return register).
extern "C" uint32_t __cdecl B12Poll(Links* l,uint32_t residue) {
    auto* p=Base(l);
    if(p->buffer) return (residue&0xffffff00u)|1;
    int32_t hr=Com<int32_t>(Device(),26,p->size,(p->flags&1)?520u:0u,0u,(p->flags&1)?0u:1u,&p->buffer,0u);
    return (uint32_t(hr)&0xffffff00u)|uint32_t(hr>=0);
}
extern "C" uint32_t __cdecl B12Notify(Links* l,uint32_t residue) {
    auto* p=Base(l);
    if(p->flags&1) return MethodEax(0x619f10,p,residue);
    return residue;
}
#if defined(_M_IX86)
__declspec(naked) uint32_t __fastcall ReleaseStream(Stream*,void*) {
    __asm {
        push eax
        push ecx
        call B12Release
        add esp,8
        ret
    }
}
__declspec(naked) uint32_t __fastcall PollStream(Links*,void*) {
    __asm {
        push eax
        push ecx
        call B12Poll
        add esp,8
        ret
    }
}
__declspec(naked) uint32_t __fastcall NotifyStream(Links*,void*) {
    __asm {
        push eax
        push ecx
        call B12Notify
        add esp,8
        ret
    }
}
#else
uint32_t __fastcall ReleaseStream(Stream* p,void*) { return B12Release(p,0); }
uint32_t __fastcall PollStream(Links* p,void*) { return B12Poll(p,0); }
uint32_t __fastcall NotifyStream(Links* p,void*) { return B12Notify(p,0); }
#endif
Format* __fastcall ConstructFormat(Format* p,void*) {
    p->primary=Table(0xff39f8);_InterlockedExchange(reinterpret_cast<volatile long*>(&p->refs),0);
    CallGameMethod<void,0x613ee0>(&p->secondary);p->secondary.table=Table(0xfe9568);
    p->declaration=nullptr;p->elements=nullptr;p->count=0;p->primary=Table(0xfe9580);return p;
}
void* __fastcall DeleteFormat(Format* p,void*,uint32_t flags) {
    p->primary=Table(0xfe9580);p->secondary.table=Table(0xfe9568);
    CallGameMethod<void,0x619e10>(p);CallGameMethod<void,0x613f00>(&p->secondary);
    p->primary=Table(0xff39f8);if(flags&1) Free(p);return p;
}
Stream* __fastcall ConstructStream(Stream* p,void*) {
    p->primary=Table(0xff39f8);_InterlockedExchange(reinterpret_cast<volatile long*>(&p->refs),0);
    CallGameMethod<void,0x613ee0>(&p->secondary);p->secondary.table=Table(0xfe95a0);
    p->buffer=nullptr;p->size=0;p->primary=Table(0xfe9590);return p;
}
void* __fastcall DeleteAdjusted(Links* p,void*,uint32_t flags) { return DeleteStream(Base(p),nullptr,flags); }
extern "C" uint32_t __cdecl B12Destruct(Stream* p,uint32_t residue) {
    Tables(p);
    residue=MethodEax(0x613f30,&p->secondary,residue);
    residue=MethodEax(0x619f10,p,residue);
    residue=MethodEax(0x613f00,&p->secondary,residue);
    p->primary=Table(0xff39f8);return residue;
}
#if defined(_M_IX86)
__declspec(naked) uint32_t __fastcall DestructStream(Stream*,void*) {
    __asm {
        push eax
        push ecx
        call B12Destruct
        add esp,8
        ret
    }
}
#else
uint32_t __fastcall DestructStream(Stream* p,void*) { return B12Destruct(p,0); }
#endif
void* __fastcall DeleteStream(Stream* p,void*,uint32_t flags) {
    DestructStream(p,nullptr);if(flags&1) Free(p);return p;
}
uint8_t __fastcall GetFormat(Cache* p,void*,const Element* elements,int32_t count,Format** out) {
    Key key{elements,count};Iterator it{};
    CallGameMethod<void,0x61a600>(p,&it,&key,uint32_t(count)&0xffffff00u,uint32_t(reinterpret_cast<uintptr_t>(elements))&0xffffff00u);
    if(it.node!=p->buckets[p->bucketCount]) {
        Format* f=*reinterpret_cast<Format**>(it.node);CallGameVirtual<void,1>(f);*out=f;return 1;
    }
    auto* memory=CallGameFunction<Format*,0x4e3f90>(uint32_t(sizeof(Format)),"Services/CachedVertexFormat",0u,0u,0u,0u);
    if(!memory) return 0;
    Format* f=ConstructFormat(memory,nullptr);CallGameVirtual<void,1>(f);
    if(!CreateFormat(f,nullptr,elements,count)) { CallGameVirtual<void,2>(f);return 0; }
    CallGameVirtual<void,1>(f);
    CallGameMethod<void,0x61a820>(p,&it,&f,uint32_t(count)&0xffffff00u);
    *out=f;return 1;
}
Cache* __fastcall ConstructCache(Cache* p,void*) {
    char unused;CallGameMethod<void,0x61ab00>(p,&unused,&unused);
    p->tag="Services/VertexFormatCache/mCachedFormts";return p;
}
}
