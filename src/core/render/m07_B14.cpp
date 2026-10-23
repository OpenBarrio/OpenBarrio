#include "ts3/render/m07_B14.h"
#include <intrin.h>
#include <cstring>
namespace ts3::render::m07::b14 {
namespace {
template<class T> T& Field(void* p,size_t offset) { return *reinterpret_cast<T*>(static_cast<uint8_t*>(p)+offset); }
void* Owner(void* p) { return static_cast<uint8_t*>(p)-offsetof(GpuTexture,secondary); }
uint32_t Flags(void* p) {
    uint32_t flags;
    std::memcpy(&flags,static_cast<GpuTexture*>(Owner(p))->opaque0020,sizeof flags);
    return flags;
}
void* Descriptor(uint32_t a) {
#if defined(_M_IX86)
    return reinterpret_cast<void*>(a);
#else
    (void)a; NotRewritten("B14 x86 named allocation descriptor");
#endif
}
void Assign(void** output,void* value) {
    void* old=*output;
    if(value==old) return;
    if(value) CallGameVirtual<void,1>(value);
    *output=value;
    if(old) CallGameVirtual<void,2>(old);
}
}
GpuTexture* __fastcall Construct(GpuTexture* p,void*) {
    p->primary=Descriptor(0x00FF39F8);
    _InterlockedExchange(reinterpret_cast<volatile long*>(&p->engineRefs),0);
    CallGameMethod<void,0x00613EE0>(&p->secondary);
    p->secondary=Descriptor(0x00FE94D4);
    p->gpuHandle=nullptr; p->word18=0; p->word1C=0; p->word24=0; p->word30=0;
    p->primary=Descriptor(0x00FE94C4);
    return p;
}
uintptr_t __fastcall Destruct(GpuTexture* p,void*) {
    p->primary=Descriptor(0x00FE94C4); p->secondary=Descriptor(0x00FE94D4);
    CallGameMethod<void,0x00613F30>(&p->secondary);
    CallGameMethod<void,0x006196B0>(p);
#if defined(_M_IX86)
    auto* sentinel=reinterpret_cast<uint8_t*>(0x01157FFC);
    auto* link=*reinterpret_cast<uint8_t**>(0x01158000);
    auto* node=link?link-4:nullptr;
    while(node!=sentinel) {
        CallGameVirtual<void,0>(node,p);
        link=Field<uint8_t*>(node,4);
        node=link?link-4:nullptr;
    }
#else
    NotRewritten("B14 texture observer list");
#endif
    auto result=CallGameMethod<uintptr_t,0x00613F00>(&p->secondary);
    p->primary=Descriptor(0x00FF39F8);
    return result;
}
GpuTexture* __fastcall Delete(GpuTexture* p,void*,uint32_t flags) {
    Destruct(p,nullptr);
    if(flags&1) CallGameFunction<void,0x004E3F70>(p);
    return p;
}
GpuTexture* __fastcall DeleteInterface(void* p,void*,uint32_t flags) {
    return Delete(static_cast<GpuTexture*>(Owner(p)),nullptr,flags);
}
uint32_t __fastcall TotalSize(void* p,void*) {
    auto* owner=static_cast<GpuTexture*>(Owner(p));
    return 0x34u+((Flags(p)&0x102u)?CallGameMethod<uint32_t,0x00619AB0>(owner):0u);
}
uint32_t __fastcall BackingSize(void* p,void*) {
    return (Flags(p)&0x100u)?0u:CallGameMethod<uint32_t,0x00619AB0>(Owner(p));
}
uint32_t __fastcall FormatSize(void* p,void*) {
    auto* owner=reinterpret_cast<VertexFormat*>(static_cast<uint8_t*>(p)-offsetof(VertexFormat,secondary));
    return owner->word1C*8u+0x24u;
}
uint32_t __fastcall StreamSize(void* p,void*) {
    auto* owner=reinterpret_cast<VertexBuffer*>(static_cast<uint8_t*>(p)-offsetof(VertexBuffer,secondary));
    return owner->countOrSize+0x20u;
}
void* __fastcall Create(void*,void*) {
    void* p=CallGameFunction<void*,0x004E3F90>(0x68,Descriptor(0x00FFCC24),0,0,0,0);
    return p?CallGameMethod<void*,0x00723310>(p):nullptr;
}
void* __fastcall CreateAd(void*,void*) {
    void* p=CallGameFunction<void*,0x004E3F90>(0x68,Descriptor(0x0105387C),0,0,0,0);
    return p?CallGameMethod<void*,0x00723310>(p):nullptr;
}
void __fastcall Convert(void*,void*,void* input,void** output) {
    void* stream=CallGameVirtual<void*,6>(input);
    if(CallGameVirtual<uint32_t,3>(stream)==0x347223D2u) { Assign(output,input); return; }
    uint32_t size=CallGameVirtual<uint32_t,7>(stream);
    void* manager=CallGameFunction<void*,0x00931020>();
    void* bytes=CallGameVirtual<void*,2>(manager,size,Descriptor(0x00FFCC90),0);
    CallGameVirtual<void,12>(stream,bytes,size);
    void* memory=CallGameFunction<void*,0x004E3F90>(0x2c,Descriptor(0x00FFCC68),0,0,0,0);
    memory=memory?CallGameMethod<void*,0x004DF120>(memory,bytes,size,1,1,0,0):nullptr;
    void* adapter=CallGameFunction<void*,0x004E41F0>(0x28,Descriptor(0x00FFCC40),0,0,0,0);
    if(adapter) {
        void* name=CallGameVirtual<void*,4>(input);
        adapter=CallGameMethod<void*,0x004B3650>(adapter,0,memory,name,1,1);
    }
    Assign(output,adapter);
}
uint8_t __fastcall LoadAd(void* self,void*,void* input,TextureResource* output) {
    uint32_t count=0;
    void* key=CallGameVirtual<void*,4>(input);
    void* services=CallGameFunction<void*,0x009E5310>();
    void* bytes=CallGameMethod<void*,0x009E5390>(services,key,&count);
    bool success=false;
    if(bytes && count>0) {
        void* stream=CallGameFunction<void*,0x004E3F90>(0x2c,Descriptor(0x010538AC),0,0,0,0);
        stream=stream?CallGameMethod<void*,0x004DF120>(stream,bytes,count,1,0,0,0):nullptr;
        void* image=CallGameFunction<void*,0x00444080>(0x30);
        image=image?CallGameMethod<void*,0x00444720>(image):nullptr;
        // The local decoder has a proven 0x30-byte stack extent (1C..4B).
        alignas(void*) uint8_t decoder[0x30];
        CallGameMethod<void,0x00445430>(decoder);
        CallGameFunction<void,0x00417130>();
        if(CallGameMethod<uint8_t,0x00446110>(decoder,stream,2,image,10)) {
            uint32_t width=Field<uint32_t>(image,0x18),height=Field<uint32_t>(image,0x1c);
            struct Locked { void* pixels; uint32_t pitch; } locked;
            CallGameVirtual<void,4>(image,&locked);
            void* texture=&output->textureInterface;
            CallGameMethod<void,0x00619A90>(texture);
            CallGameMethod<void,0x00619C80>(texture,width,height,1,0x3d,2,Descriptor(0x010538A0));
            success=CallGameMethod<uint8_t,0x006191D0>(texture,0,locked.pixels,locked.pitch,0,0)!=0;
            CallGameVirtual<void,5>(image);
            if(success) _InterlockedExchange(reinterpret_cast<volatile long*>(&output->refs60),1);
            else CallGameMethod<void,0x00619A90>(texture);
        }
        if(image) CallGameVirtual<void,0>(image,1);
        CallGameMethod<void,0x00445470>(decoder);
        if(success) return 1;
    }
    return CallGameMethod<uint8_t,0x00722E00>(self,input,output);
}
}
