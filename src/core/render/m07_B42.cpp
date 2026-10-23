#include "ts3/render/m07_B42.h"
#include <cstring>
#include <limits>
namespace ts3::render::m07::b42 {
namespace {
template<class T> T& At(void* p,size_t offset) { return *reinterpret_cast<T*>(static_cast<uint8_t*>(p)+offset); }
void* Allocate(uint32_t size,uint32_t tag) { return CallGameFunction<void*,0x004E3F90>(size,tag,0,0,0,0); }
float GlobalFloat(uint32_t address) {
#if defined(_M_IX86)
    return *reinterpret_cast<const float*>(address);
#else
    (void)address; AbortNotRewritten("m07_B42 global float"); return 0;
#endif
}
void* GlobalManager() {
#if defined(_M_IX86)
    return *reinterpret_cast<void**>(0x011D753C);
#else
    AbortNotRewritten("m07_B42 submission manager"); return nullptr;
#endif
}
}
void* __fastcall Query(Scene* self,void*,uint32_t id) {
    if(id==0x017f1d88) return self;
    return CallGameMethod<void*,0x006FABF0>(self,id);
}
uint32_t __fastcall Insert(Scene* self,void*,uint32_t value,const uint32_t* key) {
    return CallGameMethod<uint32_t,0x006DD600>(self->member,value,*key,0);
}
uint32_t __fastcall Contains(Scene* self,void*,const uint32_t* key) {
    return CallGameMethod<uint32_t,0x006DC110>(self->member,*key)!=0;
}
uint32_t __fastcall Remove(Scene* self,void*,const uint32_t* key) {
    return CallGameMethod<uint32_t,0x006DD530>(self->member,*key);
}
uintptr_t __fastcall Bounds(Scene* self,void*,uint32_t* output) {
    for(unsigned i=0;i<4;++i) output[i]=self->bounds[i];
    return (reinterpret_cast<uintptr_t>(output)&~uintptr_t(255))|1;
}
#if defined(_M_IX86)
// MSVC long double is only 64 bits. Keep ST0 live across the flag test instead
// of silently rounding the unscaled 80-bit result through a C++ local.
__declspec(naked) long double __fastcall Distance(Scene*,void*) {
    __asm {
        push ecx
        push esi
        mov esi,ecx
        mov eax,0x006FCD30
        call eax
        fst dword ptr [esp+4]
        test byte ptr [esi+0x29e],2
        pop esi
        jz unscaled
        mov eax,0x0115B200
        movss xmm0,dword ptr [eax]
        fstp st(0)
        mulss xmm0,dword ptr [esp]
        movss dword ptr [esp],xmm0
        fld dword ptr [esp]
    unscaled:
        pop ecx
        ret
    }
}
#else
long double __fastcall Distance(Scene*,void*) {
    AbortNotRewritten("m07_B42 x87 distance bridge"); return 0;
}
#endif
uint32_t __fastcall Member(Scene* self,void*) {
    return CallGameMethod<uint32_t,0x006DB940>(self->member);
}
void __fastcall Submit(Scene* self,void*,uint32_t index,uint32_t argument) {
    if(At<uint32_t>(self,0x40)&0x1000) return;
    void* saved=self->handle;
    void* table=CallGameMethod<void*,0x006DB810>(self->member);
    if(table) {
        void* item=At<void**>(table,0x40)[index];
        if(item) {
            alignas(16) uint8_t descriptor[0x40];
            CallGameMethod<void,0x0079F710>(descriptor);
            auto value=CallGameMethod<uint32_t,0x0093ECC0>(item);
            CallGameMethod<void,0x0079F7E0>(descriptor,value);
            void* manager=GlobalManager(); void* resource=nullptr;
            uint32_t kind=0x26d1704a;
            void* material=CallGameMethod<void*,0x00444710>(item);
            if(material) {
                void* info=CallGameMethod<void*,0x006C93D0>(material);
                if(info && (At<uint32_t>(info,8)==0x05f75ef1 || At<uint32_t>(info,8)==0x14de3feb)) kind=0x6f5eccf5;
            }
            CallGameMethod<void,0x0079B190>(manager,kind,&resource,1);
            if(resource) {
                void* prepared=CallGameMethod<void*,0x0079FAF0>(resource,descriptor,0,0);
                self->handle=CallGameMethod<void*,0x0079B790>(manager,kind,prepared,1);
                CallGameVirtual<void,6>(self,index,argument);
                CallGameVirtual<void,1>(resource);
            }
        }
    }
    self->handle=saved;
}
uint8_t __fastcall EnsureFormat(Geometry* self,void*) {
    if(!self->format) {
        void* memory=Allocate(0x24,0x00ff24e4);
        self->format=memory?CallGameMethod<void*,0x0061A300>(memory):nullptr;
        if(!self->format) return 0;
        const uint8_t elements[16]={0,0,0,0,1,0,0,0,0,0,8,0,4,0,2,0};
        CallGameMethod<void,0x0061A010>(self->format,elements,2);
    }
    return 1;
}
void* __fastcall Model(Lot* self,void*,uint64_t key,uint32_t mode,uint32_t a,uint32_t b,uint32_t c,uint32_t d,void* owner) {
    void* iterator=nullptr;
    void** found=CallGameMethod<void**,0x00D726F0>(self->map,&iterator,&key);
    void* sentinel=At<void**>(self->map,4)[At<uint32_t>(self->map,8)];
    if(*found!=sentinel) return At<void*>(*found,8);
    void* memory=CallGameFunction<void*,0x004E3FB0>(0x340,16,0,0x00ffa1a4,0,0,0,0);
    void* model=memory?CallGameMethod<void*,0x006F9A10>(memory,key):nullptr;
    if(model) CallGameVirtual<void,0>(model);
    CallGameMethod<void,0x006F4860>(model,mode,0,uint32_t(self->enabled),0);
    struct Entry { uint64_t key; void* model; } entry{key,model};
    if(model) CallGameVirtual<void,0>(model);
    uint8_t inserted[12];
    CallGameMethod<void,0x007009E0>(self->map,inserted,&entry,mode&0xffffff00u);
    void* context=owner?At<void*>(owner,0x1c):nullptr;
    CallGameMethod<void,0x00701C00>(self,model,context,0,0,-1);
    CallGameMethod<void,0x00701FF0>(self,model,a,b,c,d,0);
    CallGameMethod<void,0x006FB2E0>(model,0.0f,0.0f);
    CallGameMethod<void,0x006FB330>(model,0.0f);
    if(entry.model) CallGameVirtual<void,1>(entry.model);
    if(model) CallGameVirtual<void,1>(model);
    return model;
}
void* __fastcall Cluster(void* self,void*,uint32_t key) {
    void* found=nullptr;
    CallGameMethod<void,0x0082AD60>(self,&found,key);
    void* sentinel=At<void**>(self,4)[At<uint32_t>(self,8)];
    if(found!=sentinel) return static_cast<uint8_t*>(found)+8;
    struct Value { void* begin; void* end; void* capacity; const char* tag; } value{nullptr,nullptr,nullptr,nullptr};
#if defined(_M_IX86)
    value.tag=reinterpret_cast<const char*>(0x00ff9f04);
#else
    value.tag="Renderer/SceneObjectManager/SpeedTreeClusterModelSet";
#endif
    alignas(8) uint8_t pair[24];
    void* temporary=CallGameMethod<void*,0x0082F790>(pair,key,&value);
    void* result=nullptr;
    CallGameMethod<void,0x00705150>(self,&result,temporary,key&0xffffff00u);
    if(At<void*>(pair,8)) CallGameFunction<void,0x004E3F70>(At<void*>(pair,8));
    return static_cast<uint8_t*>(result)+8;
}
uint8_t __fastcall BuildTree(Tree* self,void*,const float* box,int32_t depth) {
    CallGameMethod<void,0x00705400>(self);
    self->depth=depth;
    for(unsigned i=0;i<4;++i) self->extent[i]=box[4+i]-box[i];
    for(unsigned i=0;i<4;++i) self->origin[i]=box[i];
    self->inverse[0]=GlobalFloat(0x00fcc24c)/self->extent[0];
    self->inverse[1]=GlobalFloat(0x01020c78)/self->extent[1];
    self->inverse[2]=GlobalFloat(0x00fcc24c)/self->extent[2];
    self->top=box[5]+GlobalFloat(0x0107a538);
    self->bottom=box[1]-GlobalFloat(0x0107a538);
    self->bytes=0; self->nodes=0;
    for(int32_t level=0;level<depth;++level) {
        uint32_t side=1u<<(uint32_t(level)&31);
        uint32_t count=side*side;
        uint64_t extent=uint64_t(count)*48;
        uint32_t size=extent>0xffffffffu?0xffffffffu:uint32_t(extent);
        size=size>0xfffffffbu?0xffffffffu:size+4;
        uint8_t* raw=static_cast<uint8_t*>(Allocate(size,0x00ffa330));
        uint8_t* nodes=nullptr;
        if(raw) {
            *reinterpret_cast<uint32_t*>(raw)=count; nodes=raw+4;
            for(int32_t i=0;i<static_cast<int32_t>(count);++i) CallGameMethod<void,0x00706270>(nodes+size_t(i)*48);
        }
        self->levels[level]=nodes;
        self->nodes+=count; self->bytes+=count*48;
    }
    auto node=[&](int32_t level,uint32_t x,uint32_t z)->void* {
        if(level<0 || level>=self->depth) return nullptr;
        uint32_t index=(z<<(uint32_t(level)&31))+x;
        return self->levels[level]+size_t(index)*48;
    };
    for(int32_t level=0;level<depth;++level) {
        int32_t side=static_cast<int32_t>(1u<<(uint32_t(level)&31));
        for(int32_t z=0;z<side;++z) for(int32_t x=0;x<side;++x) {
            // The EA child level is level+1; order is (00,10,01,11).
            CallGameMethod<void,0x00706070>(node(level,x,z),node(level-1,uint32_t(x)>>1,uint32_t(z)>>1),
                node(level+1,2*x,2*z),node(level+1,2*x+1,2*z),node(level+1,2*x,2*z+1),node(level+1,2*x+1,2*z+1),level,x,z);
        }
    }
    return 1;
}
}
