#include "ts3/render/m07_B17.h"
#include <cstring>
namespace ts3::render::m07::b17 {
Dependencies dependencies;
LibraryDependencies libraryDependencies;
namespace {
const char* Label(uint32_t address,const char* text) {
#if defined(_M_IX86)
    (void)text; return reinterpret_cast<const char*>(uintptr_t(address));
#else
    (void)address; return text;
#endif
}
void* Token(uint32_t address) {
#if defined(_M_IX86)
    return reinterpret_cast<void*>(uintptr_t(address));
#else
    (void)address; throw std::runtime_error("M07 B17 image datum unavailable on native host");
#endif
}
void Init(Array& a,const char* tag) { a.data=nullptr; a.end=nullptr; a.capacity=nullptr; a.name=tag; }
void* Allocate(uint32_t size,uint32_t tag,const char* text) {
    return Original(0x004E3F90,dependencies.allocate)(size,Label(tag,text),0,0,0,0);
}
void Ack(Reader* r,uint8_t value) { CallThis(Original(0x007D77C0,dependencies.acknowledge),r,value); }
uint32_t Word(Reader* r) {
    uint32_t value;
    Ack(r,Original(0x004DD700,dependencies.readWord)(r->stream,&value,r->endian));
    return value;
}
}
ShaderArrays* __fastcall InitShaderArrays(ShaderArrays* s,void*) {
    Init(s->shaders,Label(0x010158D4,"Shader/ShaderArray"));
    Init(s->defaults,Label(0x010158C0,"Shader/DefaultData"));
    return s;
}
Array* __fastcall InitSamplerArray(Array* s,void*) {
    // Adapted from verified RenderArray::InitRenderSet: only the tag differs.
    Init(*s,Label(0x010158E8,"Shader/SamplerStatesDB"));
    return s;
}
Package* __fastcall InitPackage(Package* s,void*) {
    s->shaders=nullptr; s->samplers=nullptr; s->names=nullptr;
    Init(s->sets,Label(0x01015B14,"Shader/ShaderSetList"));
    Init(s->list,Label(0x01015B00,"Shader/ShaderList"));
    return s;
}
Library* __fastcall InitLibrary(Library* s,void*) {
#if defined(_M_IX86)
    auto* empty=static_cast<char*>(Token(0x011C7E54));
    auto* buckets=Token(0x0114C900);
#else
    static char emptyStorage[2]{}; static void* emptyBuckets[2]{};
    auto* empty=emptyStorage; auto* buckets=emptyBuckets;
#endif
    for(auto& str:s->strings) {
        str.tag=Label(0x01015A00,"Shader/Library");
        str.begin=empty; str.end=empty; str.capacity=empty+2;
    }
    s->mask=0; s->force=0; s->skip=0; s->geometry=0; s->package=nullptr;
    auto initMap=[&](Map& m,const char* tag) {
        m.minLoad=1.0f; m.maxLoad=2.0f; m.tag=tag; m.count=0; m.storage=nullptr;
        m.bucketCount=1; m.buckets=buckets;
    };
    initMap(s->shaders,Label(0x010159EC,"Shader/ShaderMap"));
    initMap(s->sets,Label(0x010159D8,"Shader/ShaderSetMap"));
    s->selected=0;
    return s;
}
RenderData* __fastcall InitRenderData(RenderData* s,void*,uint32_t arg) {
    std::memset(s->prefix,0,sizeof(s->prefix)); s->one=1; s->zero=0;
    s->word124=0; std::memset(s->cleared,0,sizeof(s->cleared)); s->invalid=~0u;
    auto* h=static_cast<TextureHook*>(Allocate(sizeof(TextureHook),0x01015874,"Shader/RenderDataArray/TextureHook"));
    if(h) {
        h->table=dependencies.hookTable?dependencies.hookTable:Token(0x01015870); h->owner=s;
        Original(0x00619D00,dependencies.attach)(h);
    }
    s->hook=h;
    CallThis(Original(0x00799F30,dependencies.renderData),s,arg);
    return s;
}
uint8_t __fastcall GetBytecode(Bytecode* s,void*,Buffer* out) {
    if(s->bytes) {
        CallThis(Original(0x007760D0,dependencies.resize),out,s->size);
        std::memcpy(out->data,s->bytes,s->size);
        return 1;
    }
    return Original(0x00799B90,dependencies.fetch)(s->shader,out);
}
uint8_t __fastcall LoadStatePairs(StatePairs* s,void*,Reader* r) {
    const uint32_t bytes=Word(r);
    s->count=bytes>>3;
    uint8_t valid=CallThis(Original(0x007D77D0,dependencies.validate),r,s->count,8u);
    if(!valid) return valid;
    const uint32_t words=2*s->count;
    const uint32_t size=words>0x3fffffffu ? ~0u : words*4;
    s->pairs=Allocate(size,0x01015898,"Shader/StatePairs");
    Ack(r,Original(0x004DD5E0,dependencies.readWords)(r->stream,s->pairs,words,r->endian));
    return r->good;
}
uint8_t __fastcall LoadSamplers(Vector* s,void*,Reader* r) {
    if(Word(r)!=0x53504D53) return 0;
    const uint32_t count=Word(r);
    for(uint32_t i=0;i<count;++i) {
        auto* p=static_cast<StatePairs*>(Allocate(sizeof(StatePairs),0x010158AC,"Shader/SamplerState"));
        if(p) p=CallThis(Original(0x006CFD50,dependencies.stateCtor),p);
        // Deliberate original null dereference on allocation failure.
        p->id=i+1;
        if(dependencies.loadPairs) CallThis(dependencies.loadPairs,p,r); else LoadStatePairs(p,nullptr,r);
        void* item=p;
        if(s->end>=s->capacity) CallThis(Original(0x008621C0,dependencies.grow),s,s->end,&item);
        else { void** out=s->end++; if(out) *out=item; }
    }
    return 1;
}
uint8_t __fastcall CreateDatabases(Package* s,void*) {
    CallThis(Original(0x0079BB90,dependencies.clearPackage),s);
    auto* shaders=static_cast<ShaderArrays*>(Allocate(sizeof(ShaderArrays),0x01015B54,"Shader/ShaderDB"));
    s->shaders=shaders?InitShaderArrays(shaders,nullptr):nullptr;
    if(!s->shaders) return 0;
    auto* samplers=static_cast<Array*>(Allocate(sizeof(Array),0x01015B40,"Shader/SamplerDB"));
    s->samplers=samplers?InitSamplerArray(samplers,nullptr):nullptr;
    if(!s->samplers) return 0;
    s->names=Allocate(72,0x01015B2C,"Shader/KeyNameMap");
    if(s->names) s->names=CallThis(Original(0x0072F690,dependencies.keyMap),s->names);
    return s->names!=nullptr;
}
uint8_t __fastcall LoadShaders(ShaderArrays* s,void*,Reader* r,uint16_t filter,uint8_t mode) {
    if(Word(r)!=0x4B4E494C) return 0;
    uint32_t count=Word(r);
    for(uint32_t i=0;i<count;++i) {
        uint8_t pixel; uint16_t flags;
        Ack(r,Original(0x004DD4C0,dependencies.readByte)(r->stream,&pixel));
        Ack(r,Original(0x004DD540,dependencies.readShort)(r->stream,&flags,r->endian));
        const uint32_t length=Word(r);
        if(!r->good) return 0;
        void* item=nullptr;
        const uint16_t low=filter&0xff,high=filter&0xff00;
        if((low && !(flags&low)) || (high && !(flags&high))) {
            CallThis(Original(0x007D8260,dependencies.skipBytes),r,length,1u);
        } else {
            auto* p=static_cast<Bytecode*>(Allocate(sizeof(Bytecode),pixel?0x00FFC4F8:0x00FFC50C,pixel?"Shader/PixelShader":"Shader/VertexShader"));
            if(p) p=CallThis(Original(pixel?0x007997E0:0x00799760,pixel?dependencies.pixelCtor:dependencies.vertexCtor),p);
            p->flags=flags;
            if(!CallThis(Original(0x00799910,dependencies.loadShader),p,r,mode)) {
                auto** table=static_cast<void**>(p->table);
                CallThis(reinterpret_cast<Thiscall<void,Bytecode,uint32_t>>(table[0]),p,1u);
                return 0;
            }
            p->id=uint16_t(i+1); item=p;
        }
        auto** end=static_cast<void**>(s->shaders.end);
        if(end>=s->shaders.capacity) CallThis(Original(0x008621C0,dependencies.growShaders),s,end,&item);
        else { s->shaders.end=end+1; if(end) *end=item; }
    }
    if(Word(r)!=0x46554244) return 0;
    const uint32_t countDefaults=Word(r);
    if(!CallThis(Original(0x007D77D0,dependencies.validate),r,countDefaults,4u)) return 0;
    CallThis(Original(0x00AEF830,dependencies.resizeDefaults),&s->defaults,countDefaults);
    Ack(r,Original(0x004DD5E0,dependencies.readWords)(r->stream,s->defaults.data,countDefaults,r->endian));
    return r->good;
}
uint8_t __fastcall LoadLibrary(Library* s,void*,uint8_t reload) {
    auto& d=libraryDependencies;
    uint32_t selected=CallThis(Original(0x0079AEC0,d.select),s);
    if(selected<=3 && s->force) selected=2;
    if(Original(0x0079F690,d.compatible)(selected,s->selected)) selected=s->selected;
    else s->selected=selected;
    // Opaque bridge-owned temporaries: original accessed extents, with room for
    // pointer expansion in injected native implementations. No x86 addresses on x64.
    alignas(void*) uint8_t file[552*sizeof(void*)/4];
    alignas(void*) uint8_t buffered[56*sizeof(void*)/4];
    alignas(void*) uint8_t reader[24*sizeof(void*)/4];
    alignas(void*) uint8_t insertion[12*sizeof(void*)/4];
    void* f=file; void* b=buffered; void* r=reader; void* out=insertion;
    CallThis(Original(0x004DB970,d.fileCtor),f,static_cast<const char*>(s->strings[2].begin));
    CallThis(Original(0x004DB380,d.fileInit),f);
    if(!CallThis(Original(0x004DB4C0,d.open),f,1u,6u,1u,0u)) {
        CallThis(Original(0x004DB9D0,d.fileDtor),f); return 0;
    }
    CallThis(Original(0x004DE780,d.bufferCtor),b,0x10000u,0x10000u,f,0u);
    CallThis(Original(0x007D91B0,d.readerCtor),r,b,1u);
    auto* p=static_cast<Package*>(Allocate(sizeof(Package),0x01015ADC,"Shader/ShaderPackage"));
    if(p) p=InitPackage(p,nullptr);
    Package* temporary=p;
    if(!reload) { s->package=p; temporary=nullptr; }
    uint32_t mask=s->mask|(1u<<(selected&31));
    uint8_t result=0;
    uint8_t db=d.databases?CallThis(d.databases,p):CreateDatabases(p,nullptr);
    if(db && CallThis(Original(0x0079CE00,d.deserialize),p,r,mask,uint8_t(!s->skip),0u,s->geometry)) {
        if(reload) CallThis(Original(0x0079C340,d.merge),s->package,p);
        uint32_t n=CallThis(Original(0x0079BC50,d.shaderCount),s->package);
        CallThis(Original(0x00AEBDB0,d.clearShaders),&s->shaders,s->shaders.buckets,s->shaders.bucketCount);
        s->shaders.count=0;
        for(uint32_t i=0;i<n;++i) {
            ShaderEntry pair=*CallThis(Original(0x0079BC60,d.shaderAt),s->package,i);
            CallThis(Original(0x0079B290,d.insertShader),&s->shaders,out,static_cast<const ShaderEntry*>(&pair),uint8_t(0));
        }
        n=CallThis(Original(0x008A6600,d.setCount),s->package);
        CallThis(Original(0x007D3AF0,d.clearSets),&s->sets,s->sets.buckets,s->sets.bucketCount);
        s->sets.count=0;
        for(uint32_t i=0;i<n;++i) {
            auto* set=CallThis(Original(0x0079BC40,d.setAt),s->package,i);
            LibraryDependencies::SetPair pair{set->key,set};
            CallThis(Original(0x00B34440,d.insertSet),&s->sets,out,static_cast<const LibraryDependencies::SetPair*>(&pair),uint8_t(0));
        }
        if(!s->geometry) {
            ShaderArrays* shaders=s->skip?nullptr:s->package->shaders;
            auto** bucket=static_cast<MapNode**>(s->shaders.buckets);
            MapNode* node=*bucket;
            if(!node) { do { ++bucket; } while(!*bucket); node=*bucket; }
            auto* sentinel=static_cast<MapNode**>(s->shaders.buckets)[s->shaders.bucketCount];
            while(node!=sentinel) {
                CallThis(Original(0x0079A570,d.bind),node->value,shaders,s->package->samplers);
                node=node->next;
                if(!node) { do { node=bucket[1]; ++bucket; } while(!node); }
            }
        }
        result=1;
    }
    if(temporary) {
        CallThis(Original(0x0079BCA0,d.packageDtor),temporary);
        Original(0x004E3F70,d.free)(temporary);
    }
    CallThis(Original(0x007D7760,d.readerDtor),r);
    CallThis(Original(0x004DE7F0,d.bufferDtor),b);
    CallThis(Original(0x004DB9D0,d.fileDtor),f);
    return result;
}
uint8_t __cdecl CreateGeometry() {
    void** format=dependencies.vertexFormat;
    void** buffer=dependencies.vertexBuffer;
    if(!format) format=static_cast<void**>(Token(0x011D7538));
    if(!buffer) buffer=static_cast<void**>(Token(0x011D7534));
    auto fail=[&]() -> uint8_t { Original(0x0079AD10,dependencies.clearGeometry)(); return 0; };
    if(!*format) {
        void* p=Allocate(36,0x010159B4,"Shader/ShaderEffect/VertexFormat");
        *format=p?CallThis(Original(0x0061A300,dependencies.formatCtor),p):nullptr;
        if(!*format) return fail();
        auto** table=*static_cast<void***>(*format);
        CallThis(reinterpret_cast<Thiscall<void,void>>(table[1]),*format);
        if(!CallThis(Original(0x0061A010,dependencies.setFormat),*format,(dependencies.declaration?dependencies.declaration:static_cast<const void*>(Token(0x010159A4))),2u)) return fail();
    }
    if(*buffer) return 1;
    void* p=Allocate(32,0x01015980,"Shader/ShaderEffect/VertexBuffer");
    *buffer=p?CallThis(Original(0x0061A380,dependencies.bufferCtor),p):nullptr;
    if(!*buffer) return fail();
    auto** table=*static_cast<void***>(*buffer);
    CallThis(reinterpret_cast<Thiscall<void,void>>(table[1]),*buffer);
    if(!CallThis(Original(0x0061A1C0,dependencies.createBuffer),*buffer,112u,0u,Label(0x01015970,"ShaderGeometry"))) return fail();
    if(!CallThis(Original(0x0061A240,dependencies.upload),*buffer,(dependencies.geometryBytes?dependencies.geometryBytes:static_cast<const void*>(Token(0x01015900))),112u)) return fail();
    return 1;
}
}
