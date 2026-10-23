#include "ts3/resource/m03_A11.h"
#include <cstring>
#include <atomic>
namespace ts3::resource {
namespace {
uintptr_t Low(uintptr_t value, uint8_t byte) { return (value & ~uintptr_t(255)) | byte; }
}
namespace {
uintptr_t CopyEntry(A11Entry* to,const A11Entry* from) {
    to->first=from->first; to->second=from->second;
    CallGameMethod<uintptr_t,0x00408EB0>(&to->name,&from->name);
    return CallGameMethod<uintptr_t,0x00500310>(&to->metadata,&from->metadata);
}
void FreeName(A11String& text) {
    const auto bytes=static_cast<intptr_t>(reinterpret_cast<uintptr_t>(text.capacity)-reinterpret_cast<uintptr_t>(text.begin)) & ~intptr_t(1);
    if(bytes>2 && text.begin) CallGameFunction<void,0x004E3F70>(text.begin);
}
}
uintptr_t A11Insert(A11Vector* self,A11Entry* position,const A11Entry* source) {
    if(self->end!=self->capacity) {
        if(source>=position && source<self->end) ++source;
        if(self->end) CopyEntry(self->end,self->end-1);
        CallGameFunction<uintptr_t,0x0072EA00>(position,self->end-1,self->end);
        position->first=source->first; position->second=source->second;
        if(&position->name!=&source->name)
            CallGameMethod<uintptr_t,0x00408E20>(&position->name,source->name.begin,source->name.end);
        const auto result=CallGameMethod<uintptr_t,0x00500310>(&position->metadata,&source->metadata);
        ++self->end; return result;
    }
    const auto count=static_cast<int32_t>((reinterpret_cast<intptr_t>(self->end)-reinterpret_cast<intptr_t>(self->begin))/sizeof(A11Entry));
    const uint32_t capacity=count ? static_cast<uint32_t>(count)*2 : 1;
    A11Entry* allocation=nullptr;
    if(capacity) {
#if defined(_M_IX86)
        const char* tag=reinterpret_cast<const char*>(0x00F98980);
#else
        const char* tag="vector allocation";
#endif
        allocation=CallGameFunction<A11Entry*,0x004E3F90>(capacity*sizeof(A11Entry),self->tag,0,0,tag,0xE7);
    }
    A11Entry* output=nullptr;
    CallGameFunction<void,0x0072E920>(&output,self->begin,position,allocation,source);
    if(output) CopyEntry(output,source);
    A11Entry* end=nullptr;
    CallGameFunction<void,0x0072E920>(&end,position,self->end,output+1,source);
    CallGameMethod<void,0x0072E8E0>(self,self->begin,self->end);
    if(self->begin) CallGameFunction<void,0x004E3F70>(self->begin);
    self->begin=allocation; self->end=end; self->capacity=allocation+capacity;
    return capacity;
}
uintptr_t A11Append(A11Vector* self,const A11Entry* source) {
    if(self->end<self->capacity) {
        auto* target=self->end++;
        return target ? CopyEntry(target,source) : reinterpret_cast<uintptr_t>(self->end);
    }
    return A11Insert(self,self->end,source);
}
uint8_t A11AddDirectory(A11Directory* self,const char16_t* name,uint32_t first,uint32_t second) {
#if defined(_M_IX86)
    void* mutex=reinterpret_cast<void*>(0x011D3C98);
    const uint32_t* timeout=reinterpret_cast<const uint32_t*>(0x00FFDA44);
    auto* empty=reinterpret_cast<char16_t*>(0x011C7E54);
    const char* tag=reinterpret_cast<const char*>(0x00FFDA48);
#else
    void* mutex=nullptr; const uint32_t* timeout=nullptr;
    static char16_t blank=0; auto* empty=&blank; const char* tag="ResourceSystem directory";
#endif
    CallGameMethod<uintptr_t,0x004E16F0>(mutex,timeout);
    if(!name) { CallGameMethod<uintptr_t,0x004E17B0>(mutex); return 0; }
    for(auto* p=self->entries.begin;p!=self->entries.end;++p) {
        if(!CallGameFunction<int32_t,0x004ED300>(p->name.begin,name,5)) {
            CallGameMethod<uintptr_t,0x004E17B0>(mutex); return 0;
        }
    }
    A11Entry temporary{};
    temporary.name={empty,empty,empty+1,tag};
    CallGameMethod<void,0x00500A30>(&temporary.metadata);
    temporary.first=first; temporary.second=second;
    auto* end=name; while(*end) ++end;
    CallGameMethod<uintptr_t,0x00408E20>(&temporary.name,name,end);
    const auto result=CallGameMethod<uint8_t,0x0072E790>(self,first,second,&temporary.metadata);
    if(result) A11Append(&self->entries,&temporary);
    FreeName(temporary.name);
    CallGameMethod<uintptr_t,0x004E17B0>(mutex);
    return result ? 1 : 0;
}
uint8_t A11LoadNames(void* self,void* stream) {
    struct Parser { uintptr_t unknown[2]; void* stream; uint32_t unused, endian; uint8_t valid, owned; } parser;
    CallGameMethod<void,0x007D91B0>(&parser,stream,1);
    // The map vector begins at +28 on x86; its four pointer-sized header cells
    // follow the ten pointer-sized cells in the debug map's native view.
    auto* vector=reinterpret_cast<uintptr_t*>(self)+10;
    CallGameMethod<void,0x00913940>(vector,vector[1],vector[2]);
    vector[3]=0;
    uint32_t ignored,count;
    auto read32=[&](uint32_t* value) {
        auto ok=CallGameFunction<uint8_t,0x004DD700>(parser.stream,value,parser.endian);
        CallGameMethod<void,0x007D77C0>(&parser,static_cast<uint32_t>(ok));
    };
    read32(&ignored); read32(&count);
    auto finish=[&](uint8_t result) { CallGameMethod<void,0x007D7760>(&parser); return result; };
    if(!parser.valid || !CallGameMethod<uint8_t,0x007D77D0>(&parser,count,12)) return finish(0);
    struct Buffer { uint8_t *begin,*end,*capacity; const char* tag; uint8_t* local; uint8_t bytes[256]; } buffer;
    buffer.begin=buffer.end=buffer.local=buffer.bytes; buffer.capacity=buffer.bytes+256;
#if defined(_M_IX86)
    buffer.tag=reinterpret_cast<const char*>(0x00FFDBA0);
#else
    buffer.tag="Debug/KeyNameMap/LoadBuffer";
#endif
    auto release=[&] { if(buffer.begin && buffer.begin!=buffer.local) CallGameFunction<void,0x004E3F70>(buffer.begin); };
    for(uint32_t i=0;i<count;++i) {
        uint32_t key[2],length;
        auto ok=CallGameFunction<uint8_t,0x004DD660>(parser.stream,key,parser.endian);
        CallGameMethod<void,0x007D77C0>(&parser,static_cast<uint32_t>(ok));
        read32(&length);
        if(!parser.valid || !CallGameMethod<uint8_t,0x007D77D0>(&parser,length,1)) { release(); return finish(0); }
        uint8_t zero=0;
        CallGameMethod<void,0x0072F440>(&buffer,length+1,&zero);
        buffer.begin[length]=0;
        CallGameFunction<void,0x004DD510>(stream,buffer.begin,length);
        CallGameMethod<void,0x0072F3C0>(self,key,buffer.begin);
    }
    release(); return finish(1);
}
uintptr_t A11Write(A11Factory* self, void* resource, void* stream, uintptr_t, uint32_t type) {
    uint8_t result=0;
    if(type==self->type) result=static_cast<uint8_t>(CallGameVirtual<uintptr_t,16>(self,stream,resource));
    return Low(CallGameVirtual<uintptr_t,8>(stream),result);
}
uint32_t A11Types(A11Factory* self,uint32_t* out,uint32_t capacity) {
    if(capacity>=1) *out=self->type;
    return 1;
}
void* A11Create(A11Factory* self,uint32_t type) {
    return type==self->type ? CallGameVirtual<void*,13>(self) : nullptr;
}
uintptr_t A11Load(A11Factory* self,void* stream,void** out,uintptr_t context,uint32_t type) {
    if(type==0xFFFFFFFFu && stream) {
        const auto* key=CallGameVirtual<const uint32_t*,4>(stream);
        type=key[2];
    }
    void* resource=CallGameVirtual<void*,14>(self,type);
    if(!resource) {
        uintptr_t result=0;
        if(stream) result=CallGameVirtual<uintptr_t,8>(stream);
        return Low(result,0);
    }
    CallGameVirtual<uintptr_t,0>(resource);
    if(stream && !static_cast<uint8_t>(CallGameVirtual<uintptr_t,9>(self,stream,resource,context,type)))
        return Low(CallGameVirtual<uintptr_t,1>(resource),0);
    *out=resource;
    return Low(reinterpret_cast<uintptr_t>(out),1);
}
uintptr_t A11Read(A11Factory* self,void* stream,void* resource,uintptr_t,uint32_t type) {
    uint8_t result=0;
    if(type==self->type) {
        const auto* key=CallGameVirtual<const uint32_t*,4>(stream);
        std::memcpy(static_cast<uint8_t*>(resource)+8,key,16);
        result=static_cast<uint8_t>(CallGameVirtual<uintptr_t,15>(self,stream,resource));
    }
    return Low(CallGameVirtual<uintptr_t,8>(stream),result);
}
A11Factory* A11Construct(A11Factory* self,uint32_t type,void* allocator) {
    std::atomic_ref<int32_t>(self->refs).exchange(0);
#if defined(_M_IX86)
    self->vtable=reinterpret_cast<void**>(0x00FFDB18);
#else
    self->vtable=nullptr; // No native common-factory implementation exists yet.
#endif
    self->allocator=allocator; self->type=type; return self;
}
void* A11Exchange(void* value) {
#if defined(_M_IX86)
    auto& cell=*reinterpret_cast<void**>(0x011D3E28);
#else
    static void* cell=nullptr;
#endif
    auto* old=cell; cell=value; return old;
}
A11Cursor* A11Advance(A11Cursor* self,int32_t delta) {
    const auto distance=static_cast<intptr_t>(self->current-self->first);
    const int32_t index=static_cast<int32_t>(static_cast<uint32_t>(distance>>4)+static_cast<uint32_t>(delta));
    if(static_cast<uint32_t>(index)<16) {
        self->current+=static_cast<intptr_t>(delta)*16;
    } else {
        // Assembly adds 0x1000000, rounds signed division toward zero,
        // then subtracts 0x100000. Do not replace by ordinary index/16.
        const int32_t shifted=static_cast<int32_t>(static_cast<uint32_t>(index)+0x1000000u);
        const int32_t page=shifted/16-0x100000;
        self->node+=page;
        self->first=*self->node;
        self->last=self->first+256;
        self->current=self->first+static_cast<intptr_t>(index-page*16)*16;
    }
    return self;
}
A11Cursor* A11Subtract(A11Cursor* self,A11Cursor* out,int32_t delta) {
    A11Cursor temporary=*self;
    A11Advance(&temporary,static_cast<int32_t>(0u-static_cast<uint32_t>(delta)));
    *out=temporary; return out;
}
}

namespace ts3::resource {
uintptr_t A11_HOOK HookA11Insert(A11Vector* s, void*, A11Entry* p, const A11Entry* v) { return A11Insert(s,p,v); }
uintptr_t A11_HOOK HookA11Append(A11Vector* s, void*, const A11Entry* v) { return A11Append(s,v); }
uint8_t A11_HOOK HookA11AddDirectory(A11Directory* s, void*, const char16_t* n, uint32_t a, uint32_t b) { return A11AddDirectory(s,n,a,b); }
uintptr_t A11_HOOK HookA11Write(A11Factory* s, void*, void* r, void* v, uintptr_t c, uint32_t t) { return A11Write(s,r,v,c,t); }
uint32_t A11_HOOK HookA11Types(A11Factory* s, void*, uint32_t* o, uint32_t n) { return A11Types(s,o,n); }
void* A11_HOOK HookA11Create(A11Factory* s, void*, uint32_t t) { return A11Create(s,t); }
uintptr_t A11_HOOK HookA11Load(A11Factory* s, void*, void* v, void** o, uintptr_t c, uint32_t t) { return A11Load(s,v,o,c,t); }
uintptr_t A11_HOOK HookA11Read(A11Factory* s, void*, void* v, void* r, uintptr_t c, uint32_t t) { return A11Read(s,v,r,c,t); }
A11Factory* A11_HOOK HookA11Construct(A11Factory* s, void*, uint32_t t, void* a) { return A11Construct(s,t,a); }
void* HookA11Exchange(void* v) { return A11Exchange(v); }
uint8_t A11_HOOK HookA11LoadNames(void* s, void*, void* v) { return A11LoadNames(s,v); }
A11Cursor* A11_HOOK HookA11Advance(A11Cursor* s, void*, int32_t n) { return A11Advance(s,n); }
A11Cursor* A11_HOOK HookA11Subtract(A11Cursor* s, void*, A11Cursor* o, int32_t n) { return A11Subtract(s,o,n); }
}
