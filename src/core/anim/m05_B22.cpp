#include "ts3/anim/m05_B22.h"
#include "ts3/layout.h"
#include <cstring>
#include <algorithm>
namespace ts3::anim::b22 {
namespace {
void* Address(uintptr_t a) {
#if defined(_M_IX86)
    return reinterpret_cast<void*>(a);
#else
    (void)a; AbortNotRewritten("m05 namespace globals"); return nullptr;
#endif
}
uint64_t Mask() { uint64_t v; std::memcpy(&v,Address(0x114f7c0),8); return v; }
void Free(void* p) { CallGameFunction<void,0x4e3f70>(p); }
template<class T> void Fix(T*& p,void* base) {
    if(p) p=reinterpret_cast<T*>(reinterpret_cast<uintptr_t>(p)+reinterpret_cast<uintptr_t>(base));
}
uint8_t ParseByte(String* s) { return CallGameFunction<uint8_t,0x579870>(s); }
}
ClipIkInfo* Destroy(ClipIkInfo* s,uint32_t flags) {
    s->vtable=Address(0xfc2f80);
    if(s->hashBegin) Free(s->hashBegin);
    if(s->namespaceBegin) Free(s->namespaceBegin);
    if(s->slotBegin) Free(s->slotBegin);
    if(flags&1) Free(s);
    return s;
}
ClipIkInfo* Construct(ClipIkInfo* s) {
    s->vtable=Address(0xfc2f80);
    s->begin=s->end=s->inlineStorage; s->capacity=s->inlineStorage+0x60;
    s->inlineStorage[0]=0;
    s->slotBegin=s->slotEnd=s->slotCapacity=nullptr; s->slotDescriptor=Address(0xfc2fd8);
    s->namespaceBegin=s->namespaceEnd=s->namespaceCapacity=nullptr; s->namespaceDescriptor=Address(0xfc2fb0);
    s->hashBegin=s->hashEnd=s->hashCapacity=nullptr; s->hashDescriptor=Address(0xfc2f84);
    const float q[4]={0,0,0,1}; std::memcpy(&s->rotation,q,16);
    std::memset(s->opaquec0,0,12); s->valueCC=4; s->valueD0=0;
    return s;
}
bool Relocate(PackedClip* s) {
    void* previous=nullptr;
    if(s && s->signature!=0x6b20c4f3) return false;
    while(s) {
        Fix(s->next,&s->next); Fix(s->data0c,&s->data0c); Fix(s->groups,&s->groups);
        Fix(s->data14,&s->data14); Fix(s->data18,&s->data18); Fix(s->data24,&s->data24);
        if(s->data18) { auto p=reinterpret_cast<void**>(static_cast<unsigned char*>(s->data18)+16); Fix(*p,p); }
        if(s->groups && s->groups!=previous) {
            auto base=static_cast<unsigned char*>(s->groups);
            auto n=*reinterpret_cast<uint32_t*>(base);
            auto entries=reinterpret_cast<void**>(base+sizeof(void*));
            for(uint32_t i=0;i<n;++i) {
                Fix(entries[i],entries); // All offsets use the first entry as their base, not their own slot.
                if(entries[i]) {
                    auto group=static_cast<unsigned char*>(entries[i]);
                    auto count=*reinterpret_cast<uint32_t*>(group+4);
                    auto values=reinterpret_cast<void**>(group+8);
                    for(uint32_t j=0;j<count;++j) Fix(values[j],values);
                }
            }
        }
        previous=s->groups; s=s->next;
    }
    return true;
}
uint8_t NamespaceByte(uint32_t id) {
    auto table=static_cast<unsigned char*>(Address(0x114f6b8));
    for(size_t i=0;;i+=16) {
        uint32_t key; std::memcpy(&key,table+i+4,4);
        if(key==0xffffffff) return 0;
        if(key==id) return table[i];
    }
}
uint64_t NamespaceKey(uint32_t a,uint32_t b) {
    return (uint64_t(NamespaceByte(a))<<56)|(uint64_t(NamespaceByte(b))<<48);
}
uint64_t Combine(uint64_t a,uint64_t b) {
    auto result=a^b;
    if(b) result|=Mask();
    return result;
}
const char* FindByte(const char* begin,const char* end,uint8_t c) {
    auto p=end;
    while(p!=begin) { --p; if(static_cast<uint8_t>(*p)==c) return p; }
    return end;
}
uint64_t HashName(const char* name,uint64_t key,uint64_t* out) {
    auto base=CallGameFunction<uint64_t,0x7cfe10>(name)&~Mask();
    if(out) *out=base;
    return Combine(base,key);
}
const char* FindLast(const char* begin,const char* end,const char* p,const char* pe) {
    if(begin==end || p==pe) return begin;
    auto n=pe-p;
    if(n>end-begin) return end;
    for(auto at=end-n;;--at) {
        if(std::memcmp(at,p,n)==0) return at;
        if(at==begin) return end;
    }
}
uint32_t RFind(String* s,const char* p,uint32_t pos,uint32_t n) {
    auto size=static_cast<uint32_t>(s->end-s->begin);
    if(n>size) return 0xffffffff;
    if(!n) return std::min(pos,size);
    auto end=s->begin+std::min(pos,size-n)+n;
    auto found=FindLast(s->begin,end,p,p+n);
    return found==end?0xffffffff:static_cast<uint32_t>(found-s->begin);
}
String* Prefix(String* s,String* out,uint32_t n) {
    if(n<static_cast<size_t>(s->end-s->begin)) CallGameMethod<void,0x4059f0>(out,s->begin,s->begin+n,&s->descriptor);
    else CallGameMethod<void,0x405ad0>(out,s);
    return out;
}
String* Suffix(String* s,String* out,uint32_t n) {
    if(n<static_cast<size_t>(s->end-s->begin)) CallGameMethod<void,0x4059f0>(out,s->end-n,s->end,&s->descriptor);
    else CallGameMethod<void,0x405ad0>(out,s);
    return out;
}
String* Concat(String* out,const String* a,const String* b) {
    out->begin=out->end=out->capacity=nullptr; out->descriptor=a->descriptor;
    CallGameMethod<void,0x4058b0>(out,uint32_t((a->end-a->begin)+(b->end-b->begin)+1));
    *out->end=0;
    CallGameMethod<void,0x405790>(out,a->begin,a->end);
    CallGameMethod<void,0x405790>(out,b->begin,b->end);
    return out;
}
uint16_t Parse(String* s) {
    auto n=s->end-s->begin;
    if(n>5) return 0;
    auto split=CallGameMethod<uint32_t,0x9b4220>(s,static_cast<const char*>(Address(0xfb0fd8)),0u);
    if(split==0xffffffff) return uint16_t(ParseByte(s))<<8;
    auto part=[&](char* b,char* e) {
        String tmp{}; tmp.descriptor=s->descriptor;
        CallGameMethod<void,0x4059f0>(&tmp,b,e,&s->descriptor);
        auto value=ParseByte(&tmp);
        if(tmp.capacity-tmp.begin>1 && tmp.begin) Free(tmp.begin);
        return value;
    };
    auto high=part(s->begin,s->begin+std::min<uint32_t>(uint32_t(n),split));
    auto low=part(s->begin+split+1,s->end);
    return uint16_t(high)<<8|low;
}
void* Query(void* s,uint32_t id) {
    return id==0x02269ed1 || id==0xeb54b802 || id==0xee3f516e?s:nullptr;
}
}
