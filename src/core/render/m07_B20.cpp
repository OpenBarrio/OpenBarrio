#include "ts3/render/m07_B20.h"
#include <cstring>
#include <cfloat>
#include <xmmintrin.h>
namespace ts3::render::m07::b20 {
namespace {
void* Table(uint32_t ea) {
#if defined(_M_IX86)
    return reinterpret_cast<void*>(uintptr_t(ea));
#else
    (void)ea; AbortNotRewritten("m07_B20 image vtable"); return nullptr;
#endif
}
void CopyFloat(float& d,const float& s) {
#if defined(_M_IX86)
    const float* from=&s; float* to=&d;
    __asm { mov eax,from }
    __asm { fld dword ptr [eax] }
    __asm { mov eax,to }
    __asm { fstp dword ptr [eax] }
#else
    uint32_t b; std::memcpy(&b,&s,4);
    if ((b&0x7f800000)==0x7f800000 && (b&0x7fffff)) b|=0x400000;
    std::memcpy(&d,&b,4);
#endif
}
uint8_t Replace(SoftwareBuffer* s,const void* source,uint32_t bytes,uint32_t tag) {
    void* p=CallGameFunction<void*,0x004E3FB0>(bytes,16,0,Table(tag),0,0,0,0);
    if(!p) return 0;
    s->byteExtent=bytes;
    if(p!=s->payload) { CallGameFunction<void,0x004E3F70>(s->payload); s->payload=p; }
    std::memcpy(p,source,bytes);
    CallGameMethod<void,0x00731CC0>(s,bytes);
    return 1;
}
}
void* __fastcall QueryModel(Proxy* s,void*,uint32_t id) {
    if(id==0x0202F7D5) return s ? &s->secondary : nullptr;
    return id==0x02269ED1 || id==0xEB54B802 || id==0xEE3F516E ? s : nullptr;
}
uint32_t* __fastcall Bounds(Proxy* s,void*,uint32_t* out) {
    out[0]=s->bounds[0]; out[1]=s->bounds[1]; out[2]=s->bounds[2]; out[3]=s->bounds[3]; return out;
}
void* __fastcall DestroyScratch(void* s,void*,uint32_t flags) {
    *static_cast<void**>(s)=Table(0x00FF658C);
    CallGameMethod<void,0x006CAF20>(s);
    if(flags&1) CallGameFunction<void,0x004E3F70>(s);
    return s;
}
void* __fastcall DestroyAdjusted(void* s,void*,uint32_t flags) {
    return DestroySoftware(reinterpret_cast<Proxy*>(static_cast<uint8_t*>(s)-offsetof(Proxy,secondary)),nullptr,flags);
}
void* __fastcall QuerySoftware(Proxy* s,void*,uint32_t id) {
    if(id==0x02044986) return s ? &s->secondary : nullptr;
    return id==0x02269ED1 || id==0xEE3F516E ? s : nullptr;
}
uintptr_t __fastcall GetResource(Proxy* s,void*,void** out) {
    uintptr_t r;
    if(!s->resource) { r=reinterpret_cast<uintptr_t>(out); *out=nullptr; return r&~uintptr_t(255); }
    r=CallGameVirtual<uintptr_t,0>(s->resource);
    *out=s->resource; return (r&~uintptr_t(255))|1;
}
int32_t __fastcall Count(Proxy* s,void*) {
    // The subtraction wraps to signed 32 bits before division, including ragged spans.
    const int32_t span=static_cast<int32_t>(reinterpret_cast<uintptr_t>(s->end)-reinterpret_cast<uintptr_t>(s->begin));
    return span/24;
}
uint32_t __fastcall GetEntry(Proxy* s,void*,uint32_t i,uint32_t* id,uint32_t* out) {
    *id=s->begin[i].words[5];
    const auto* p=s->begin[i].words;
    out[0]=p[0]; out[1]=p[1]; out[2]=p[2]; const auto r=p[3]; out[3]=r; return r;
}
uintptr_t __fastcall Clear(ReleaseList* s,void*) {
    while(s->begin!=s->end) { void* p=s->end[-1]; --s->end; CallGameVirtual<void,1>(p); }
    return CallGameMethod<uintptr_t,0x006DAF60>(s);
}
void* __fastcall QueryAdjusted(void* s,void*,uint32_t id) {
    return QuerySoftware(reinterpret_cast<Proxy*>(static_cast<uint8_t*>(s)-offsetof(Proxy,secondary)),nullptr,id);
}
uint8_t __fastcall LoadVertices(SoftwareVB* s,void*,uint32_t format,const void* source,uint32_t bytes) {
    uint8_t ok=CallGameFunction<uint8_t,0x006D78F0>(format,source,bytes,&source,&bytes,&format);
    if(!ok) return ok;
    return Replace(s,source,bytes,0x00FF65FC);
}
uint8_t __fastcall LoadIndices(SoftwareIB* s,void*,uint32_t count,const void* source,uint8_t format) {
    const void* converted; uint32_t scratch;
    uint8_t ok=CallGameFunction<uint8_t,0x006D7C00>(count,source,format,&converted,&count,&source,&format,&scratch);
    if(!ok) return ok;
    return Replace(s,converted,count,0x00FF6614);
}
void* __fastcall DestroySoftware(Proxy* s,void*,uint32_t flags) {
    s->primary=Table(0x00FF65C8); s->secondary=Table(0x00FF65B4); s->secondary=Table(0x00FA1B78);
    CallGameMethod<void,0x006CAF20>(s);
    if(flags&1) CallGameFunction<void,0x004E3F70>(s);
    return s;
}
int32_t __fastcall Contains(Proxy* s,void*,uint32_t id) {
    const int32_t n=Count(s,nullptr);
    for(int32_t i=0;i<n;++i) if(s->begin[i].words[5]==id) return 1;
    return 0;
}
uint8_t __fastcall LoadModel(ModelState* s,void*,Stream* stream,void* resolver,uint8_t swap) {
    uint32_t* data=stream->storage ? stream->storage->data : nullptr;
    const uint32_t size=CallGameVirtual<uint32_t,7>(stream);
    if(size<12) return 0;
    if(swap) CallGameFunction<void,0x00739270>(data,size>>2);
    const uint32_t version=data[1], count=data[2];
    if(version<1 || version>0x1ff || count>=0x10000) return 0;
    uint32_t required=4*count+12;
    if(version>=0x100) required+=count+8+2*count+16; // EA's unusually weak bound (7, not 20, bytes per LOD).
    if(size<required) return 0;
    CallGameMethod<void,0x006D0230>(s);
    if(!CallGameMethod<uint8_t,0x006D04F0>(s,count)) return 0;
    float radius=0;
    auto* cursor=data+(version>=0x100?9:3);
    if(version>=0x102) {
        s->boxCount=*cursor++;
        if(s->boxCount) s->boxes=CallGameFunction<void*,0x004E3F90>(s->boxCount>0x07ffffff?0xffffffff:s->boxCount*32,Table(0x00FF65E0),0,0,0,0);
        for(uint32_t i=0;i<s->boxCount;++i) {
            auto* box=static_cast<uint32_t*>(s->boxes)+i*8;
            // movss/movaps copies, not floating-point arithmetic: preserve signaling NaNs.
            box[0]=cursor[0]; box[1]=cursor[1]; box[2]=cursor[2]; box[3]=cursor[0];
            box[4]=cursor[3]; box[5]=cursor[4]; box[6]=cursor[5]; box[7]=cursor[3]; cursor+=6;
        }
    }
    if(version>=0x101) {
        s->selector=*cursor<4?*cursor:0;
        CopyFloat(s->distance,*reinterpret_cast<float*>(cursor+1)); cursor+=2;
    }
    for(uint32_t i=0;i<count;++i) {
        const uint32_t key=*cursor++;
        if((key&0xf0000000)==0x30000000) {
            void* p=CallGameMethod<void*,0x00726430>(resolver,key);
            CallGameMethod<void,0x006D02E0>(s,i,p);
        } else {
            void* p=CallGameMethod<void*,0x00726690>(resolver,key);
            CallGameMethod<void,0x006D0410>(s,i,p);
            void* bounds=CallGameMethod<void*,0x005042A0>(p);
            alignas(16) uint32_t temp[4];
            CallGameMethod<void,0x0068BBB0>(bounds,temp,&radius);
        }
        if(version>=0x100) {
            alignas(16) uint32_t words[4]; std::memcpy(words,cursor,16);
            CallGameMethod<void,0x006D0330>(s,i,words); cursor+=4;
        }
    }
    if(version>=0x100) {
        uint32_t flags=0;
        for(uint32_t i=0;i<count;++i) flags|=*CallGameMethod<uint32_t*,0x006D0310>(s,i);
        s->flags[0]=uint8_t(flags&1); s->flags[1]=uint8_t((flags&2)!=0);
    }
    if(radius>0 && version<0x100) {
        float lower=0, upper=_mm_cvtss_f32(_mm_mul_ss(_mm_set_ss(radius),_mm_set_ss(8.0f)));
        uint32_t last=0;
        for(uint32_t i=0;i<count;++i) {
            alignas(16) uint32_t words[4];
            std::memcpy(words,CallGameMethod<uint32_t*,0x006D0310>(s,i),16);
            // This branch is always taken for these legacy versions, just as in EA.
            words[0]=0; words[1]=i;
            std::memcpy(words+2,&lower,4);
            float fade=_mm_cvtss_f32(_mm_mul_ss(_mm_set_ss(upper),_mm_set_ss(1.1f)));
            std::memcpy(words+3,&fade,4); lower=upper;
            upper=_mm_cvtss_f32(_mm_mul_ss(_mm_set_ss(upper),_mm_set_ss(2.0f)));
            CallGameMethod<void,0x006D0330>(s,i,words); last=i;
        }
        alignas(16) uint32_t words[4];
        std::memcpy(words,CallGameMethod<uint32_t*,0x006D0310>(s,last),16);
        words[3]=0x7f7fffff;
        CallGameMethod<void,0x006D0330>(s,last,words);
    }
    CallGameMethod<void,0x00731CC0>(s,40*s->base.lodCount+72);
    return 1;
}
}
