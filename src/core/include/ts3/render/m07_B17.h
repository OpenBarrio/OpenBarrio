#pragma once
// Windows headers alias LoadLibrary to LoadLibraryA; this is an engine method.
#ifdef LoadLibrary
#undef LoadLibrary
#endif
#include "ts3/render/m07_interfaces.h"
#include "ts3/renderer/clu_58aad0_Renderer.h"
namespace ts3::render::m07::b17 {
// These are pointer-triple vectors (the donor constructor alone does not
// distinguish end/capacity pointers from scalar size/capacity).
struct Array { void* data; void* end; void* capacity; const char* name; };
struct ShaderArrays { Array shaders,defaults; };
struct Package { ShaderArrays* shaders; Array* samplers; void* names; Array sets,list; };
struct String { char* begin; char* end; char* capacity; const char* tag; };
struct Map { uint32_t untouched; void* buckets; uint32_t bucketCount,count; float minLoad,maxLoad; void* storage; const char* tag; };
struct Library { String strings[4]; uint32_t selected,mask; uint8_t force,skip,geometry,padding; Package* package; Map shaders,sets; };
struct StatePairs { void* pairs; uint32_t count,id; };
struct Vector { void** begin; void** end; void** capacity; const char* tag; };
struct Reader { uint8_t opaque0[8]; void* stream; uint32_t opaqueC; uint32_t endian; uint8_t good; };
struct Bytecode { void* table; uint16_t id,flags; const void* bytes; uint32_t size; void* shader; };
struct Buffer { void* data; uint32_t size,capacity; const char* tag; };
struct RenderData;
struct TextureHook { void* table; uint8_t untouched[8]; RenderData* owner; };
struct RenderData { uint8_t prefix[16]; uint16_t one,zero; uint8_t cleared[0x10c]; uint32_t invalid,word124,untouched128; TextureHook* hook; };
struct ShaderEntry { uint32_t key; void* value; };
struct SetEntry { uint32_t prefix[2],key; };
struct MapNode { uint32_t prefix[2]; void* value; MapNode* next; };
struct LibraryDependencies {
    Thiscall<uint32_t,Library> select=nullptr;
    uint8_t (__cdecl* compatible)(uint32_t,uint32_t)=nullptr;
    Thiscall<void,void,const char*> fileCtor=nullptr;
    Thiscall<void,void> fileInit=nullptr,fileDtor=nullptr,bufferDtor=nullptr,readerDtor=nullptr;
    Thiscall<uint8_t,void,uint32_t,uint32_t,uint32_t,uint32_t> open=nullptr;
    Thiscall<void,void,uint32_t,uint32_t,void*,uint32_t> bufferCtor=nullptr;
    Thiscall<void,void,void*,uint32_t> readerCtor=nullptr;
    Thiscall<uint8_t,Package> databases=nullptr;
    Thiscall<uint8_t,Package,void*,uint32_t,uint8_t,uint32_t,uint8_t> deserialize=nullptr;
    Thiscall<void,Package,Package*> merge=nullptr;
    Thiscall<uint32_t,Package> shaderCount=nullptr,setCount=nullptr;
    Thiscall<ShaderEntry*,Package,uint32_t> shaderAt=nullptr;
    Thiscall<SetEntry*,Package,uint32_t> setAt=nullptr;
    Thiscall<void,Map,void*,uint32_t> clearShaders=nullptr,clearSets=nullptr;
    Thiscall<void,Map,void*,const ShaderEntry*,uint8_t> insertShader=nullptr;
    struct SetPair { uint32_t key; SetEntry* value; };
    Thiscall<void,Map,void*,const SetPair*,uint8_t> insertSet=nullptr;
    Thiscall<void,void,ShaderArrays*,Array*> bind=nullptr;
    Thiscall<void,Package> packageDtor=nullptr;
    void (__cdecl* free)(void*)=nullptr;
};
struct Dependencies {
    void* (__cdecl* allocate)(uint32_t,const char*,uint32_t,uint32_t,uint32_t,uint32_t)=nullptr;
    uint8_t (__cdecl* readWord)(void*,uint32_t*,uint32_t)=nullptr;
    uint8_t (__cdecl* readWords)(void*,void*,uint32_t,uint32_t)=nullptr;
    Thiscall<void,Reader,uint8_t> acknowledge=nullptr;
    Thiscall<uint8_t,Reader,uint32_t,uint32_t> validate=nullptr;
    Thiscall<void,Buffer,uint32_t> resize=nullptr;
    uint8_t (__cdecl* fetch)(void*,Buffer*)=nullptr;
    void (__cdecl* attach)(TextureHook*)=nullptr;
    Thiscall<void,RenderData,uint32_t> renderData=nullptr;
    Thiscall<void,Package> clearPackage=nullptr;
    Thiscall<void*,void> keyMap=nullptr;
    Thiscall<StatePairs*,StatePairs> stateCtor=nullptr;
    Thiscall<void,Vector,void**,void**> grow=nullptr;
    Thiscall<uint8_t,StatePairs,Reader*> loadPairs=nullptr;
    void* hookTable=nullptr;
    const void* declaration=nullptr; const void* geometryBytes=nullptr;
    void** vertexFormat=nullptr; void** vertexBuffer=nullptr;
    Thiscall<void*,void> formatCtor=nullptr,bufferCtor=nullptr;
    Thiscall<uint8_t,void,const void*,uint32_t> setFormat=nullptr;
    Thiscall<uint8_t,void,uint32_t,uint32_t,const char*> createBuffer=nullptr;
    Thiscall<uint8_t,void,const void*,uint32_t> upload=nullptr;
    void (__cdecl* clearGeometry)()=nullptr;
    uint8_t (__cdecl* readByte)(void*,uint8_t*)=nullptr;
    uint8_t (__cdecl* readShort)(void*,uint16_t*,uint32_t)=nullptr;
    Thiscall<void,Reader,uint32_t,uint32_t> skipBytes=nullptr;
    Thiscall<Bytecode*,Bytecode> pixelCtor=nullptr,vertexCtor=nullptr;
    Thiscall<uint8_t,Bytecode,Reader*,uint8_t> loadShader=nullptr;
    Thiscall<void,Array,uint32_t> resizeDefaults=nullptr;
    Thiscall<void,ShaderArrays,void**,void**> growShaders=nullptr;
};
extern Dependencies dependencies;
extern LibraryDependencies libraryDependencies;
// TS3W.exe 0x0079A830: initialize both shader arrays and their allocation tags.
ShaderArrays* __fastcall InitShaderArrays(ShaderArrays*,void*);
// TS3W.exe 0x0079A8C0: initialize the sampler database (exact twin of 006E0540 apart from tag).
Array* __fastcall InitSamplerArray(Array*,void*);
// TS3W.exe 0x0079BC70: initialize a package and its two tagged lists.
Package* __fastcall InitPackage(Package*,void*);
// TS3W.exe 0x0079B390: initialize library strings and maps, leaving padding/map prefix untouched.
Library* __fastcall InitLibrary(Library*,void*);
// TS3W.exe 0x0079A0A0: initialize render data and allocate/attach its texture callback; +128 is untouched.
RenderData* __fastcall InitRenderData(RenderData*,void*,uint32_t);
// TS3W.exe 0x00799BF0: copy cached bytecode or fetch it from the underlying shader; return is AL only.
uint8_t __fastcall GetBytecode(Bytecode*,void*,Buffer*);
// TS3W.exe 0x0079A1E0: load state pairs, truncating the serialized byte count by eight.
uint8_t __fastcall LoadStatePairs(StatePairs*,void*,Reader*);
// TS3W.exe 0x0079AC30: load samplers; ignores individual LoadStatePairs failures and returns one.
uint8_t __fastcall LoadSamplers(Vector*,void*,Reader*);
// TS3W.exe 0x0079BCD0: clear a package then allocate databases, retaining partial state on failure.
uint8_t __fastcall CreateDatabases(Package*,void*);
// TS3W.exe 0x0079ADC0: lazily construct shared shader geometry, cleaning up on any failure.
uint8_t __cdecl CreateGeometry();
// TS3W.exe 0x0079A930: load filtered shaders and default constants, preserving skipped null entries.
uint8_t __fastcall LoadShaders(ShaderArrays*,void*,Reader*,uint16_t,uint8_t);
// TS3W.exe 0x0079B870: open and load a library package, populate maps, bind shaders, and unwind temporaries.
uint8_t __fastcall LoadLibrary(Library*,void*,uint8_t);
TS3_X86_SIZE(ShaderArrays,32);
TS3_X86_SIZE(Package,44);
TS3_X86_SIZE(Library,144);
TS3_X86_OFFSET(RenderData,hook,0x12c);
TS3_X86_OFFSET(Bytecode,shader,0x10);
TS3_X86_OFFSET(Reader,good,0x14);
}
