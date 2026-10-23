#include "ts3/render/m07_B22.h"
#include <intrin.h>
namespace ts3::render::b22 {
namespace {
template<unsigned A> void* Image() {
#if defined(_M_IX86)
    return reinterpret_cast<void*>(uintptr_t(A));
#else
    NotRewritten("B22 original image data unavailable");
#endif
}
void Store(void* object, unsigned offset, void* value) {
#if defined(_M_IX86)
    *reinterpret_cast<void**>(static_cast<unsigned char*>(object)+offset)=value;
#else
    (void)object; (void)offset; (void)value;
    NotRewritten("B22 opaque original object layout");
#endif
}
void* Allocate(unsigned size) {
    return CallGameFunction<void*,0x004E3F90>(size,Image<0x00FF698C>(),0,0,0,0);
}
void* Resource(void* table) {
    void* object=Allocate(0x30);
    if(object) {
        auto* header=static_cast<m07::ResourceHeader*>(object);
        header->primary=Image<0x00FB2C04>();
        _InterlockedExchange(reinterpret_cast<volatile long*>(&header->refs),0);
        for(unsigned offset=8;offset<=0x24;offset+=4) Store(object,offset,nullptr);
        header->primary=table;
        Store(object,0x28,nullptr); // +2C is deliberately untouched.
    }
    return object;
}
}
void* __cdecl CreateAsyncModel() {
    void* object=Allocate(0x40);
    if(!object) return nullptr;
    CallGameMethod<void,0x006CAF80>(object);
    Store(object,0x0,Image<0x00FF658C>());
    return object;
}
void* __cdecl CreateModel() {
    void* object=Allocate(0x68);
    if(!object) return nullptr;
    CallGameMethod<void,0x006CAF80>(object);
    Store(object,0x40,Image<0x00FF662C>());
    Store(object,0x0,Image<0x00FF66DC>());
    Store(object,0x40,Image<0x00FF66B0>());
    Store(object,0x44,nullptr);
    Store(object,0x48,nullptr);
    Store(object,0x4C,nullptr);
    Store(object,0x50,nullptr);
    Store(object,0x54,nullptr);
    Store(object,0x58,nullptr);
    Store(object,0x5C,nullptr);
    Store(object,0x60,nullptr);
    Store(object,0x64,Image<0x00FF6694>());
    return object;
}
void* __cdecl CreateBuffer() {
    void* object=Allocate(0x48);
    if(!object) return nullptr;
    CallGameMethod<void,0x006CAF80>(object);
    Store(object,0x40,Image<0x00FFBD20>());
    Store(object,0x0,Image<0x00FF65C8>());
    Store(object,0x40,Image<0x00FF65B4>());
    return object;
}
void* __cdecl CreateSkin() {
    void* object=Allocate(0x38);
    if(!object) return nullptr;
    CallGameMethod<void,0x0070D190>(object);
    Store(object,0x0,Image<0x00FF6A7C>());
    return object;
}
void* __cdecl CreateComposite() {
    void* object=Allocate(0x90);
    if(!object) return nullptr;
    CallGameMethod<void,0x006DB0F0>(object);
    Store(object,0x0,Image<0x00FF6678>());
    Store(object,0x80,nullptr);
    Store(object,0x84,nullptr);
    Store(object,0x88,nullptr);
    Store(object,0x8C,Image<0x00FF6658>());
    return object;
}
void* __cdecl CreateLod() {
    void* object=Allocate(0x48);
    if(!object) return nullptr;
    CallGameMethod<void,0x006D0570>(object);
    Store(object,0x0,Image<0x00FF6B24>());
    return object;
}
uintptr_t __fastcall Unload(void* self,void*) {
    return (CallGameMethod<uintptr_t,0x0070CF90>(self)&~uintptr_t(255))|1;
}
void* __fastcall DeleteSkin(void* self,void*,uint32_t flags) {
    CallGameMethod<void,0x0070D070>(self);
    if(flags&1) CallGameFunction<void,0x004E3F70>(self);
    return self;
}
void* __fastcall DeleteLod(void* self,void*,uint32_t flags) {
    Store(self,0,Image<0x00FF6B24>());
    CallGameMethod<void,0x006D03A0>(self);
    if(flags&1) CallGameFunction<void,0x004E3F70>(self);
    return self;
}
void* __cdecl CreateProxyA() {
    void* object=Allocate(0x48);
    return object ? CallGameMethod<void*,0x006D8F40>(object) : nullptr;
}
void* __cdecl CreateProxyB() {
    void* object=Allocate(0x50);
    return object ? CallGameMethod<void*,0x006D9090>(object) : nullptr;
}
void* __cdecl CreateResourceA() { return Resource(Image<0x00FF686C>()); }
void* __cdecl CreateResourceB() { return Resource(Image<0x00FF68CC>()); }
void* __cdecl CreateResourceC() { return Resource(Image<0x00FF692C>()); }
uintptr_t __cdecl RegisterFactories() {
    void* manager=CallGameFunction<void*,0x004AFD20>();
    const unsigned descriptors[]={0xFF6CDC,0xFF6CC4,0xFF6CA8,0xFF6C8C,0xFF6C74,0xFF6C50,0xFF6C2C,0xFF6C08,0xFF6BEC,0xFF6BD0,0xFF6BB8,0xFF6B98};
    const unsigned tables[]={0xFF6718,0xFF67A0,0xFF6828,0xFF6888,0xFF68E8,0xFF6948,0xFF69A8,0xFF69F0,0xFF6A38,0xFF6A98,0xFF6AE0,0xFF6B40};
    uintptr_t result=0;
    for(unsigned i=0;i<12;++i) {
#if defined(_M_IX86)
        void* object=CallGameFunction<void*,0x004E41F0>(sizeof(m07::ResourceHeader),reinterpret_cast<const char*>(uintptr_t(descriptors[i])),0,0,0,0);
        if(object) {
            auto* header=static_cast<m07::ResourceHeader*>(object);
            header->primary=Image<0x00FB21E8>();
            _InterlockedExchange(reinterpret_cast<volatile long*>(&header->refs),0);
            header->primary=reinterpret_cast<void*>(uintptr_t(tables[i]));
        }
        result=CallGameVirtual<uintptr_t,10>(manager,1,object,0);
#else
        (void)descriptors; (void)tables; (void)manager;
        NotRewritten("B22 registration image descriptors unavailable");
#endif
    }
    return result;
}
}
