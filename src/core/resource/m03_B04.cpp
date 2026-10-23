#include "ts3/resource/m03_B04.h"
namespace ts3::resource::m03::b04 {
using namespace ts3;
namespace {
void*& Global(unsigned address) {
#if defined(_M_IX86)
    return *reinterpret_cast<void**>(address);
#else
    (void)address; NotRewritten("m03 B04 global bridge");
#endif
}
uintptr_t NativeAddress(unsigned address) {
#if defined(_M_IX86)
    return address;
#else
    (void)address; NotRewritten("m03 B04 static data bridge");
#endif
}
uint32_t Export(unsigned kind) {
    if(!Global(0x011CE2D0)) return 0;
    void* script=CallGameFunction<void*,0x007692B0>();
    void* owner=Global(0x011CE2D0);
    void* source=static_cast<uint8_t*>(owner)+0x28;
    if(kind==1) source=CallGameMethod<void*,0x0059B390>(owner);
    if(kind==2) source=CallGameMethod<void*,0x0059B860>(owner);
    struct Text { char16_t* begin; char16_t* end; char16_t* capacity; const char* name; } text{nullptr,nullptr,nullptr,reinterpret_cast<const char*>(NativeAddress(0xFD9CB8))};
    CallGameMethod<void,0x0040B700>(&text,source);
    // The variant's constructor initializes the remaining fields. Only the two
    // flags words are initialized by these wrappers before the constructor call.
    alignas(void*) uint8_t variant[32];
    *reinterpret_cast<uintptr_t*>(variant)=NativeAddress(0xFB96B4);
    *reinterpret_cast<uint16_t*>(variant+sizeof(void*)+4)=0;
    *reinterpret_cast<uint16_t*>(variant+sizeof(void*)+6)=0;
    CallGameMethod<void,0x004F5800>(variant,19u,9u,&text,16u,1u);
    uint32_t result=0;
    CallGameVirtual<void,20>(script,variant,&result);
    *reinterpret_cast<uintptr_t*>(variant)=NativeAddress(0xFB96B4);
    if(variant[sizeof(void*)+4]&4) CallGameMethod<void,0x004F55A0>(variant,0u);
    auto capacity=static_cast<int32_t>((reinterpret_cast<uintptr_t>(text.capacity)-reinterpret_cast<uintptr_t>(text.begin))&~uintptr_t(1));
    if(capacity>2 && text.begin) CallGameFunction<void,0x004E3F70>(text.begin);
    return result;
}
}
uintptr_t Lookup(void* self,const char16_t* text) {
    auto hash=CallGameFunction<uint64_t,0x007CFE40>(text);
    return CallGameMethod<uintptr_t,0x0059C5B0>(self,hash);
}
void SetLocale(const char16_t* text) {
    if(auto owner=Global(0x011CE2D0)) {
        CallGameMethod<void,0x0059C430>(owner,text);
        CallGameMethod<void,0x0059AC80>(Global(0x011CE2D0));
    }
}
uint32_t Locale() { return Export(0); }
uint32_t DerivedLocale() { return Export(1); }
uint32_t AlternateLocale() { return Export(2); }
uintptr_t InsertRange(Map* self,Node* first,Node** bucket,Node* last,Node** lastBucket) {
    auto count=CallGameFunction<uint32_t,0x00D8A180>(first,bucket,last,lastBucket,last);
    struct Rehash { uint32_t required, buckets; } rehash;
    auto result=CallGameMethod<uintptr_t,0x004E09B0>(self->policy,&rehash,self->bucketCount,self->count,count);
    if(static_cast<uint8_t>(rehash.required)) result=CallGameMethod<uintptr_t,0x00625A00>(self,rehash.buckets);
    while(first!=last) {
        uintptr_t out[3];
        result=CallGameMethod<uintptr_t,0x009B4330>(self,out,first,0u);
        first=first->next;
        while(!first) first=*++bucket;
    }
    return result;
}
Map* Assign(Map* dest,const Map* source) {
    if(dest!=source) {
        CallGameMethod<void,0x006258F0>(dest,dest->buckets,dest->bucketCount);
        dest->count=0;
        auto bucket=source->buckets;
        auto lastBucket=bucket+source->bucketCount;
        auto last=*lastBucket;
        while(!*bucket) ++bucket;
        InsertRange(dest,*bucket,bucket,last,lastBucket);
    }
    return dest;
}
void* Construct(void* self) {
    CallGameMethod<void,0x0059ECC0>(self,0x9430C3C4u,0u);
    auto p=static_cast<uintptr_t*>(self);
    p[0]=NativeAddress(0xFDDB4C);
    p[4]=p[5]=p[6]=0;
    p[7]=NativeAddress(0xFDDB20);
    Global(0x011CE308)=self;
    return self;
}
uintptr_t Ensure() {
    auto service=Global(0x011CE3F4);
    auto result=CallGameMethod<uintptr_t,0x0059EFE0>(service,0x9430C3C4u);
    if(static_cast<uint8_t>(result)) return result;
    auto object=CallGameFunction<void*,0x004E3F90>(32u,reinterpret_cast<const char*>(NativeAddress(0xFDDB80)),0u,0u,0u,0u);
    if(object) Construct(object);
    return CallGameMethod<uintptr_t,0x0059F500>(service,object,0u);
}
Map* Backward(const Map* first,const Map* last,Map* dest) {
    while(first!=last) { --last; --dest; Assign(dest,last); }
    return dest;
}
uintptr_t Insert(Vector* self,Map* pos,const Map* value) {
    if(self->end!=self->capacity) {
        auto source=value;
        if(reinterpret_cast<uintptr_t>(value)>=reinterpret_cast<uintptr_t>(pos) && reinterpret_cast<uintptr_t>(value)<reinterpret_cast<uintptr_t>(self->end)) ++source;
        if(self->end) CallGameMethod<void,0x0093C460>(self->end,self->end-1);
        Backward(pos,self->end-1,self->end);
        auto result=Assign(pos,source);
        ++self->end;
        return reinterpret_cast<uintptr_t>(result);
    }
    auto n=self->end-self->begin;
    auto capacity=n?2*n:1;
    auto fresh=capacity?CallGameFunction<Map*,0x004E3F90>(capacity*sizeof(Map),self->name,0u,0u,reinterpret_cast<const char*>(NativeAddress(0xF98980)),0xE7u):nullptr;
    Map* cursor;
    CallGameFunction<void,0x0093C590>(&cursor,self->begin,pos,fresh);
    if(cursor) CallGameMethod<void,0x0093C460>(cursor,value);
    CallGameFunction<void,0x0093C590>(&cursor,pos,self->end,cursor+1);
    CallGameMethod<void,0x00C1C390>(self,self->begin,self->end);
    if(self->begin) CallGameFunction<void,0x004E3F70>(self->begin);
    self->begin=fresh; self->end=cursor; self->capacity=fresh+capacity;
    return reinterpret_cast<uintptr_t>(self->capacity);
}
void* DestroyFactory(void* self,uint32_t flags) {
    *static_cast<uintptr_t*>(self)=NativeAddress(0xFDDE78);
    CallGameMethod<void,0x0072F180>(self);
    if(flags&1u) CallGameFunction<void,0x004E4230>(self);
    return self;
}
}

#if defined(_M_IX86)
namespace ts3::resource::m03::b04 {
uintptr_t __fastcall HookLookup(void* p,void*,const char16_t* text) { return Lookup(p,text); }
uintptr_t __fastcall HookInsertRange(Map* p,void*,Node* a,Node** b,Node* c,Node** d) { return InsertRange(p,a,b,c,d); }
Map* __fastcall HookAssign(Map* p,void*,const Map* s) { return Assign(p,s); }
void* __fastcall HookConstruct(void* p,void*) { return Construct(p); }
uintptr_t __fastcall HookInsert(Vector* p,void*,Map* pos,const Map* value) { return Insert(p,pos,value); }
void* __fastcall HookDestroyFactory(void* p,void*,uint32_t flags) { return DestroyFactory(p,flags); }
}
#endif
