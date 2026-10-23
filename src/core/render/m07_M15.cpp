#include "ts3/render/m07_M15.h"
#include <cstring>
namespace ts3::render::m07 {
namespace {
void Check(ShaderReaderM15* r, uint8_t ok) { CallGameMethod<void,0x007D77C0>(r,ok); }
void Read(ShaderReaderM15* r, void* p) { Check(r,CallGameFunction<uint8_t,0x004DD700>(r->stream,p,r->mode)); }
void Block(ShaderReaderM15* r, void* p,uint32_t n) { Check(r,CallGameFunction<uint8_t,0x004DD5E0>(r->stream,p,n,r->mode)); }
uint8_t Fail(ShaderReaderM15* r) { Check(r,0); r->good=0; return 0; }
void Free(void* p) { CallGameFunction<void,0x004E3F70>(p); }
void FreeStates(void* p) {
#if defined(_M_IX86)
    auto allocator=*reinterpret_cast<void**>(0x011cb864);
    CallGameMethod<void,0x004E5920>(allocator,p);
#else
    (void)p; AbortNotRewritten("shader render-state allocator");
#endif
}
void* Allocate(uint32_t n,const char* tag) { return CallGameFunction<void*,0x004E3F90>(n,tag,0,0,0,0); }
struct String { char* begin; char* end; char* capacity; const char* tag; };
void Destroy(String& s) { if (intptr_t(s.capacity)-intptr_t(s.begin)>1 && s.begin) Free(s.begin); }
struct Field { uint32_t id; String name; uint32_t a,b,c; };
struct Set { void** vtable; uint32_t ref, name; VectorM15<uint64_t> pairs; VectorM15<uint32_t> bytecode; VectorM15<Field> fields; };
TS3_X86_OFFSET(Set, fields,0x2c);
template<class T> struct Small {
    T* begin; T* end; void* capacity; const char* tag; T* local; uint8_t bytes[2048];
    Small(const char* t):begin(reinterpret_cast<T*>(bytes)),end(begin),capacity(bytes+2048),tag(t),local(begin) {}
    void Release() { if(begin && begin!=local) Free(begin); }
};
}
uint8_t ReadShaderPassM15(ShaderPassM15* s, ShaderReaderM15* r, void* library) {
    uint32_t magic; Read(r,&magic);
    if (magic != 0x53534150) return Fail(r);
    Read(r,&s->id); Read(r,&s->flags);
    std::memset(s->values,0,7*4);
    if (s->flags & 0xf8) { Block(r,s->values,4); Read(r,&s->values[4]); }
    if (s->flags & 7) { uint32_t discarded; Read(r,&s->values[6]); Read(r,&discarded); Read(r,&s->values[5]); }
    Read(r,&s->values[7]); Read(r,&s->values[8]);
    for (unsigned stage=0;stage<2;++stage) {
        Read(r,&magic); if (magic != 0x4d524150) return Fail(r);
        uint32_t& shaderId=stage?s->pixelId:s->vertexId;
        uint32_t& count=stage?s->pixelCount:s->vertexCount;
        uint32_t& extent=stage?s->pixelExtent:s->vertexExtent;
        auto& vec=stage?s->pixel:s->vertex;
        Read(r,&shaderId); Read(r,&count);
        if (!stage) { s->vertex.end=s->vertex.begin; s->pixel.end=s->pixel.begin; }
        extent=0;
        if (count) {
            void* shader=CallGameMethod<void*,0x0079A7F0>(library,shaderId);
            CallGameMethod<void,0x0079ECE0>(&vec,count);
            for(uint32_t i=0;i<count;++i) {
                uint32_t a,b,c; Read(r,&a); Read(r,&b); Read(r,&c);
                ParamM15 p;
                CallGameMethod<void,0x0079D030>(&p,a,b,c,shader);
                if (vec.end<vec.capacity) { auto out=vec.end++; if(out) *out=p; }
                else CallGameMethod<void,0x00827AE0>(&vec,vec.end,&p);
                if (uint16_t(p.words[1])!=2) {
                    uint32_t e=(p.words[6]&255)+((p.words[6]>>8)&1023);
                    if (extent<e) extent=e;
                }
            }
            CallGameFunction<void,0x0079EE40>(vec.begin,vec.end,0);
        }
    }
    uint32_t packed; Read(r,&packed);
    uint32_t n=packed&65535, m=packed>>16;
    if(n!=s->stateCount) {
        if(s->states) { FreeStates(s->states); s->states=nullptr; }
        s->stateCount=n;
        if(n) { s->states=Allocate(n*8,"Shader/RenderStates"); if(!s->states) { s->stateCount=0; return Fail(r); } }
    }
    if(s->stateCount) Block(r,s->states,2*s->stateCount);
    if(m!=s->dynamicCount) {
        s->dynamicCount=m;
        if(s->dynamic) { Free(s->dynamic); s->dynamic=nullptr; }
        if(m) { s->dynamic=Allocate(m*12,"Shader/DynamicRenderStates"); if(!s->dynamic) { s->dynamicCount=0; return Fail(r); } }
    }
    if(s->dynamicCount) Block(r,s->dynamic,3*s->dynamicCount);
    Read(r,&s->mask);
    std::memset(s->texture,0,sizeof(s->texture)); std::memset(s->sampler,0,sizeof(s->sampler));
    for(unsigned i=0;i<16;++i) if(s->mask&(1u<<i)) { Read(r,&s->texture[i]); Read(r,&s->samplerValue[i]); }
    Read(r,&n);
    if(s->textureCount!=n) {
        s->textureCount=n;
        if(s->dynamicTexture) { Free(s->dynamicTexture); s->dynamicTexture=nullptr; }
        if(n) { s->dynamicTexture=Allocate(n>0xffffffffu/12?0xffffffffu:n*12,"Shader/DynamicRenderStates");
            if(!s->dynamicTexture) { s->textureCount=0; return Fail(r); } }
    }
    if(s->textureCount) Block(r,s->dynamicTexture,3*s->textureCount);
    return r->good;
}
uint8_t __stdcall ReadShaderSetM15(ShaderReaderM15* r, void** output, void* context) {
    uint32_t magic; Read(r,&magic);
    if(magic!=0x54455353) return Fail(r);
    String name;
#if defined(_M_IX86)
    name={reinterpret_cast<char*>(0x011c7e54),reinterpret_cast<char*>(0x011c7e54),reinterpret_cast<char*>(0x011c7e55),"Shader/Library"};
#else
    char empty=0; name={&empty,&empty,&empty+1,"Shader/Library"};
#endif
    CallGameMethod<void,0x007D94B0>(r,&name);
    uint32_t key=CallGameFunction<uint32_t,0x007CFD80>(name.begin,uint32_t(name.end-name.begin));
    Set* s=static_cast<Set*>(Allocate(80,"Shader/ShaderFactory/AnnotatedShaderSet"));
    if(s) { s=CallGameMethod<Set*,0x0079FD00>(s); if(s) CallGameVirtual<void,0>(s); }
    uint32_t count; Read(r,&count);
    for(uint32_t i=0;i<count;++i) {
        uint32_t pair[2]; Read(r,&pair[0]); Read(r,&pair[1]);
        auto& v=s->pairs;
        if(v.end<v.capacity) { auto p=v.end++; if(p) std::memcpy(p,pair,8); }
        else CallGameMethod<void,0x006202E0>(&v,v.end,pair);
    }
    uint32_t size; Read(r,&size);
    if(!CallGameMethod<uint8_t,0x007D77D0>(r,size,1)) {
        Fail(r); if(s) CallGameVirtual<void,1>(s); Destroy(name); return 0;
    }
    Small<uint32_t> code("Shader/ShaderLibrary/ShaderSetBytecode");
    CallGameMethod<void,0x006D48D0>(&code,size);
    Block(r,code.begin,size); Read(r,&count);
    if(!CallGameMethod<uint8_t,0x007D77D0>(r,count,16)) {
        Fail(r); code.Release(); if(s) CallGameVirtual<void,1>(s); Destroy(name); return 0;
    }
    Small<Field> fields("Shader/ShaderLibrary/ShaderSetFields");
    CallGameMethod<void,0x0079C770>(&fields,count);
    for(uint32_t i=0;i<count;++i) {
        auto& f=fields.begin[i]; Read(r,&f.id);
        CallGameMethod<void,0x007D9410>(r,&f.name);
        Read(r,&f.a); Read(r,&f.b); Read(r,&f.c);
    }
    s->name=key;
    CallGameMethod<void,0x00B7B530>(&s->bytecode,code.begin,code.end,key);
    CallGameMethod<void,0x0079C4C0>(&s->fields,fields.begin,fields.end,key);
    String wide; CallGameFunction<void,0x004F3890>(&wide,&name);
    CallGameVirtual<void,5>(s,wide.begin);
    if(((intptr_t(wide.capacity)-intptr_t(wide.begin))&~intptr_t(1))>2 && wide.begin) Free(wide.begin);
    bool ok=r->good && CallGameMethod<uint8_t,0x007A04E0>(s,r->stream,r->mode,context);
    if(!ok) Fail(r); else *output=s;
    CallGameMethod<void,0x0079BD70>(&fields,fields.begin,fields.end);
    fields.Release(); code.Release();
    if(!ok) CallGameVirtual<void,1>(s);
    Destroy(name);
    return ok?1:0;
}
}
