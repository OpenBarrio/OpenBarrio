#include "ts3/resource/m03_C04.h"
#if defined(_M_IX86)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif
namespace ts3::resource::m03::c04 {
void* __fastcall Query(Table* self,void*,uint32_t id) {
    return id==0xa058e66au || id==0x02269ed1u || id==0xeb54b802u || id==0xee3f516eu ? self : nullptr;
}
uint8_t __cdecl RegisterFactory() {
    void* manager=CallGameFunction<void*,0x004AFD20>();
    if (!manager) return 0;
#if defined(_M_IX86)
    void* factory=CallGameFunction<void*,0x004E41F0>(0x10,reinterpret_cast<const char*>(0x00FDDEBC),0,0,0,0);
    if (factory) {
        CallGameMethod<void,0x0072F190>(factory,0x220557dau,0x749203e8u);
        *static_cast<uintptr_t*>(factory)=0x00FDDE78;
    }
    return CallGameVirtual<uint8_t,10>(manager,1,factory,0);
#else
    NotRewritten("C04 factory registration");
#endif
}
uintptr_t __fastcall Destroy(Table* self,void*) {
#if defined(_M_IX86)
    self->vtable=reinterpret_cast<void* const*>(0x00FDDED0);
#else
    NotRewritten("C04 table vtable");
#endif
    CallGameMethod<void,0x0059F6C0>(self);
    return CallGameMethod<uintptr_t,0x00731CF0>(self);
}
InsertResult* __fastcall Insert(Hash* self,void*,InsertResult* out,Node* node,uint32_t) {
    Node** bucket=&self->buckets[node->low%7919u];
    for (Node* p=*bucket;p;p=p->next) {
        if (p->low==node->low && p->high==node->high) {
            out->node=p; out->bucket=bucket; out->inserted=0;
            return out;
        }
    }
    node->next=*bucket;
    *bucket=node;
    out->bucket=bucket; out->node=node;
    ++self->count;
    out->inserted=1;
    return out;
}
Table* __fastcall Delete(Table* self,void*,uint32_t flags) {
    Destroy(self,nullptr);
    if (flags&1) CallGameFunction<void,0x004E3F70>(self);
    return self;
}
uint8_t __fastcall Write(Table* self,void*,void* stream) {
    if (!stream) return 0;
    // 7D91B0 constructs the 24-byte writer; only the fields read here are named.
    struct Writer { uint8_t prefix[8]; void* stream; uint32_t reserved; uint32_t endian; uint8_t good; uint8_t pad[3]; } w{};
    CallGameMethod<void,0x007D91B0>(&w,stream,1);
    auto status=[&](uint8_t ok) { CallGameMethod<void,0x007D77C0>(&w,ok); };
    const uint32_t magic=0x4c425453;
    status(CallGameFunction<uint8_t,0x004DDAB0>(w.stream,&magic,4));
    status(CallGameFunction<uint8_t,0x004DDAD0>(w.stream,2,w.endian));
    status(CallGameFunction<uint8_t,0x004DDA90>(w.stream,0));
    status(CallGameFunction<uint8_t,0x004DDD20>(w.stream,self->hash.count,0,w.endian));
    status(CallGameFunction<uint8_t,0x004DDA90>(w.stream,0));
    status(CallGameFunction<uint8_t,0x004DDA90>(w.stream,0));
    Node** bucket=self->hash.buckets;
    while (!*bucket) ++bucket;
    Node* p=*bucket;
    while (p!=self->hash.sentinel) {
        status(CallGameFunction<uint8_t,0x004DDD20>(w.stream,p->low,p->high,w.endian));
        const char16_t* end=p->text;
        while (*end) ++end;
        CallGameMethod<void,0x007D8520>(&w,p->text,static_cast<uint32_t>(end-p->text));
        p=p->next;
        if (!p) { do { ++bucket; } while (!*bucket); p=*bucket; }
    }
    uint8_t result=w.good;
    CallGameMethod<void,0x007D7760>(&w);
    return result;
}
Table* __fastcall Create(void*,void*) {
#if defined(_M_IX86)
    auto* object=CallGameFunction<Table*,0x004E3F90>(0x7bf8,reinterpret_cast<const char*>(0x00FDDEE8),0,0,0,0);
    if (!object) return nullptr;
    return CallGameMethod<Table*,0x0059F7C0>(object);
#else
    NotRewritten("C04 table creation");
#endif
}
uint8_t __fastcall ReadBody(Table* self,void*,void* stream,uint32_t encoding) {
    uint8_t ignored;
    if (!CallGameFunction<uint8_t,0x004DD4F0>(stream,&ignored)) return 0;
    uint32_t count[2];
    CallGameFunction<void,0x004DD660>(stream,count,encoding);
    CallGameFunction<void,0x004DD4F0>(stream,&ignored);
    CallGameFunction<void,0x004DD4F0>(stream,&ignored);
    CallGameMethod<void,0x0059FA20>(self,stream,count[0],count[1],encoding);
    return 1;
}
uint8_t __fastcall Read(Table* self,void*,void* stream) {
    uint8_t ignored;
    for (unsigned i=0;i<4;++i) {
        if (!CallGameFunction<uint8_t,0x004DD4F0>(stream,&ignored)) return 0;
    }
    uint16_t version;
    if (!CallGameFunction<uint8_t,0x004DD540>(stream,&version,1)) return 0;
    return ReadBody(self,nullptr,stream,version>1u ? 1u : 0u);
}
uint8_t __fastcall Load(void*,void*,void* stream,void* resource) {
    if (!resource) return 0;
    auto* table=CallGameVirtual<Table*,3>(resource,0xa058e66au);
    if (!table) return 0;
    void* reader=CallGameVirtual<void*,6>(stream);
    return Read(table,nullptr,reader);
}
void __cdecl CssLocale(const char16_t* locale) {
#if defined(_M_IX86)
    void* service=*reinterpret_cast<void**>(0x011CF618);
    if (service) CallGameMethod<void,0x006921A0>(service,locale);
    else *reinterpret_cast<const char16_t**>(0x01158900)=locale;
#else
    (void)locale; NotRewritten("C04 CSS locale");
#endif
}
char* __cdecl RegistryLocale(char* buffer,uint32_t size) {
#if defined(_M_IX86)
    HKEY key;
    using Open=LSTATUS(WINAPI*)(HKEY,LPCSTR,DWORD,REGSAM,PHKEY);
    using QueryValue=LSTATUS(WINAPI*)(HKEY,LPCSTR,LPDWORD,LPDWORD,LPBYTE,LPDWORD);
    using Close=LSTATUS(WINAPI*)(HKEY);
    auto open=*reinterpret_cast<Open*>(0x00F95090);
    auto query=*reinterpret_cast<QueryValue*>(0x00F95050);
    auto close=*reinterpret_cast<Close*>(0x00F95088);
    if (open(HKEY_LOCAL_MACHINE,*reinterpret_cast<const char**>(0x011634B0),0,0x20019,&key)!=0) return nullptr;
    DWORD bytes=size;
    auto result=query(key,reinterpret_cast<const char*>(0x01017C0C),nullptr,nullptr,reinterpret_cast<LPBYTE>(buffer),&bytes);
    close(key);
    return result==0 ? buffer : nullptr;
#else
    (void)buffer; (void)size; NotRewritten("C04 registry locale");
#endif
}
}
