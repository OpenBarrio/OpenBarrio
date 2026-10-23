#include "ts3/render/m07_M05.h"
#include <cstring>
#include <new>
#include <xmmintrin.h>
namespace ts3::render::m07::m05 {
Resolver resolve=nullptr;
namespace {
template<class R,uint32_t A,class... T> R M(void* s,T... args) {
    if(resolve) if(void* p=resolve(A)) return reinterpret_cast<R(__fastcall*)(void*,void*,T...)>(p)(s,nullptr,args...);
    return CallGameMethod<R,A>(s,args...);
}
template<class R,uint32_t A,class... T> R F(T... args) {
    if(resolve) if(void* p=resolve(A)) return reinterpret_cast<R(__cdecl*)(T...)>(p)(args...);
    return CallGameFunction<R,A>(args...);
}
template<class T> uint32_t Size(const Vector<T>& v) {return uint32_t((reinterpret_cast<intptr_t>(v.end)-reinterpret_cast<intptr_t>(v.begin))/sizeof(T));}
void Free(void* p) {if(p) F<void,0x004E3F70>(p);}
struct Fixed {
    Vector<uint32_t> v;
    uint32_t* local;
    uint32_t storage[9];
    Fixed() :v{storage,storage,storage+9,"EASTL fixed_vector"},local(storage) {}
};
struct KeyValue { Fixed key; uint32_t value; };
struct Allocator { void* freeList; uintptr_t state; const char* tag; int32_t count; };
struct Hash {
    uintptr_t prefix; void* buckets; uint32_t bucketCount,size; float minLoad,maxLoad;
    uint32_t unknown; Allocator* allocator;
};
TS3_X86_SIZE(Fixed,56);
TS3_X86_SIZE(Hash,32);
void* EmptyBuckets() {
#if defined(_M_IX86)
    return reinterpret_cast<void*>(0x0114C900);
#else
    AbortNotRewritten("m07_M05 native hash empty buckets");return nullptr;
#endif
}
void* GlobalAllocator() {
#if defined(_M_IX86)
    return *reinterpret_cast<void**>(0x011CB864);
#else
    AbortNotRewritten("m07_M05 native hash bucket allocator");return nullptr;
#endif
}
void Destroy(Fixed& f) {if(f.v.begin!=f.local) Free(f.v.begin);}
uint32_t Width(uint32_t t) {
    switch(t) {
    case 0:case 4:case 5:case 6:case 8:case 9:case 11:case 13:case 14:case 15:return 1;
    case 1:case 7:case 10:case 12:case 16:return 2;
    case 2:return 3;case 3:return 4;default:return 0;
    }
}
void Append(Vector<uint32_t>& v,const uint32_t* p) {
    if(v.end>=v.capacity) M<void,0x008621C0>(&v,v.end,p);
    else {uint32_t* at=v.end++;if(at) *at=*p;}
}
void Extend(Vector<uint32_t>& v,uint32_t n) {
    uint32_t old=Size(v),zero=0;
    if(old+n>old) M<void,0x007AAB20>(&v,v.end,n,&zero);
    else v.end=v.begin+old+n;
}
void Expand(Builder* s,Format* format,uint32_t n) {
    Allocator allocator{nullptr,0,"Renderer/ModelBuilder/IndexHashMap",255};
    Hash hash{0,EmptyBuckets(),1,0,1.0f,2.0f,0,&allocator};
    Vector<uint32_t> packed{nullptr,nullptr,nullptr,"Renderer/ModelBuilder"};
    Vector<Fixed> keys{nullptr,nullptr,nullptr,"Renderer/ModelBuilder"};
    M<void,0x006D6370>(&keys,n);
    Vector<uint32_t> vertex{};
    const char* tag="Renderer/ModelBuilder";
    M<void,0x0069DB60>(&vertex,format->stride,&tag);
    if(format->stride) std::memset(vertex.begin,0,4*format->stride);
    vertex.end=vertex.begin+format->stride;
    Fixed key;
    M<void,0x006D48D0>(&key,Size(format->elements));
    uint32_t unique=0;
    for(uint32_t i=0;i<n;++i) {
        for(unsigned a=0;a<13;++a) if(format->slots[a]!=-1)
            key.v.begin[format->slots[a]]=s->input[a].begin[i];
        if(keys.end>=keys.capacity) M<void,0x006D6540>(&keys,keys.end,&key);
        else {
            Fixed* dest=keys.end++;
            if(dest) {new(dest) Fixed;dest->v.tag=key.v.tag;
                M<void,0x0046AD60>(dest,key.v.begin,key.v.end,tag);}
        }
        if(!M<uint32_t,0x006D3720>(&hash,&key)) {
            // EA snapshots every source pointer vector, even for absent attributes.
            for(unsigned a=0;a<13;++a) {
                auto& source=s->sources[a];uint32_t count=Size(source);
                void** copy=count?F<void**,0x004E3F90>(uint32_t(count*sizeof(void*)),source.tag,0u,0u,0u,0u):nullptr;
                std::memmove(copy,source.begin,count*sizeof(void*));
                int32_t slot=format->slots[a];
                if(slot!=-1) {
                    const Element& e=format->elements.begin[slot];
                    std::memcpy(vertex.begin+e.offset,copy[key.v.begin[slot]],4*Width(e.type));
                }
                Free(copy);
            }
            KeyValue pair;pair.key.v.tag=key.v.tag;
            M<void,0x0046AD60>(&pair.key,key.v.begin,key.v.end,tag);
            pair.value=unique;
            uintptr_t iterator[3];
            // Only AL is meaningful to the insert helper (the original upper bytes are stale).
            M<void,0x006D5F40>(&hash,iterator,&pair,0u);
            Destroy(pair.key);
            uint32_t old=Size(packed),count=Size(vertex);
            Extend(packed,count);
            std::memcpy(packed.begin+old,vertex.begin,4*count);
            ++unique;
        }
    }
    uint32_t old=Size(s->vertices),count=Size(packed);
    Extend(s->vertices,count);
    std::memcpy(s->vertices.begin+old,packed.begin,4*count);
    s->vertexCount+=unique;
    for(Fixed* k=keys.begin;k!=keys.end;++k) {
        auto* index=M<uint32_t*,0x006D6410>(&hash,k);
        Append(s->indices,index);
    }
    Destroy(key);Free(vertex.begin);
    for(Fixed* k=keys.begin;k!=keys.end;++k) Destroy(*k);
    Free(keys.begin);Free(packed.begin);
    M<void,0x006D4930>(&hash,hash.buckets,hash.bucketCount);
    hash.size=0;
    if(hash.bucketCount>1) {
        if(4*hash.bucketCount==60) {
            *static_cast<void**>(hash.buckets)=allocator.freeList;allocator.freeList=hash.buckets;
        } else M<void,0x004E5920>(GlobalAllocator(),hash.buckets);
    }
    if(allocator.count>=0) M<void,0x00B9C560>(&allocator.state,uint32_t(allocator.count)&0x7fffffff);
    M<void,0x007CCE20>(&allocator);
}
}
uintptr_t __fastcall Flush(Builder* s,void*) {
    uint32_t n=Size(s->input[0]);uintptr_t result=n;
    if(!n) return result;
    result=reinterpret_cast<uintptr_t>(s->formats.begin);
    if(s->formats.begin==s->formats.end) return result;
    Format* format=s->formats.end-1;
    if(s->expand) {
        uint8_t valid=1;
        M<uintptr_t,0x006D5490>(s,&format->slots[1],&s->input[1],n,&valid,"Illegal mesh - number of normal indices does not match number of position indices");
        result=M<uintptr_t,0x006D5490>(s,&format->slots[2],&s->input[2],n,&valid,"Illegal mesh - number of tangent indices does not match number of position indices");
        for(unsigned a=3;a<13;++a) if(format->slots[a]!=-1) {
            result=Size(s->input[a]);
            if(result==1) result=M<uintptr_t,0x009C5280>(&s->input[a],n,s->input[a].begin);
            else if(result!=n) valid=0;
        }
        if(!valid) return result;
    }
    uint32_t vertexStart=Size(s->vertices),indexStart=Size(s->indices);
    if(s->expand) Expand(s,format,n);
    else {
        uint32_t count=Size(s->interleaved);
        M<void,0x00AEF830>(&s->vertices,vertexStart+count);
        std::memcpy(s->vertices.begin+vertexStart,s->interleaved.begin,4*count);
        s->vertexCount+=(Size(s->vertices)-vertexStart)/format->stride;
        for(uint32_t i=0;i<n;++i) Append(s->indices,s->input[0].begin+i);
    }
    uint32_t vertexEnd=Size(s->vertices);
    s->commands.end=s->commands.begin;
    alignas(16) float position[4]; // Other formats deliberately leave the prior position untouched.
    for(Ref* r=s->refs.begin;r!=s->refs.end;++r) {
        Part& p=s->banks.begin[r->bank].begin[r->part];
        if(p.resource) {
            Command c{p.resource,p.resourceIndex,p.resourceIndex};
            auto& v=s->commands;
            if(v.end>=v.capacity) M<void,0x006D3940>(&v,v.end,&c);
            else {auto* at=v.end++;if(at) *at=c;}
        }
        p.indexBegin+=indexStart;p.indexEnd+=indexStart;
        p.vertexBegin=vertexStart;p.vertexEnd=vertexEnd;
        for(uint32_t i=p.indexBegin;i<p.indexEnd;++i) {
            uint32_t index=s->indices.begin[i];
            if(p.minIndex>index) p.minIndex=index;
            if(p.maxIndex<index) p.maxIndex=index;
            Element& e=format->elements.begin[format->slots[0]];
            uint32_t* data=s->vertices.begin+p.vertexBegin+e.offset+index*format->stride;
            if(e.type==2) std::memcpy(position,data,12);
            else if(e.type==7) {
                int16_t shorts[4];std::memcpy(shorts,data,8);
                __m128 v=_mm_set_ps(float(shorts[0]),float(shorts[2]),float(shorts[1]),float(shorts[0]));
                __m128 scale=_mm_div_ss(_mm_set_ss(1.0f),_mm_set_ss(float(shorts[3])));
                _mm_store_ps(position,_mm_mul_ps(v,_mm_shuffle_ps(scale,scale,0)));
            }
            M<void,0x0068BAC0>(p.bounds,position);
        }
        M<void,0x0068BA30>(s->bounds,p.bounds);
    }
    s->input[0].end=s->input[0].begin;
    M<void,0x006D19B0>(s);
    s->interleaved.end=s->interleaved.begin;
    s->refs.end=s->refs.begin;
    return reinterpret_cast<uintptr_t>(s->refs.begin);
}
}
