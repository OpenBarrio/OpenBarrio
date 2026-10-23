#include "ts3/anim/m05_B13.h"
#include <cstring>
namespace ts3::anim::b13_impl {
static uintptr_t Low(uintptr_t r,uint8_t b) { return (r & ~uintptr_t(255)) | b; }
static uintptr_t Write(Stream* s) {
    auto r=CallGameFunction<uintptr_t,0x004ddbd0>(s->stream,1u,s->endian);
    s->good &= static_cast<uint8_t>(r);
    return Low(r,s->good);
}
static uintptr_t Read(Stream* s,uint32_t* v) {
    auto r=CallGameFunction<uintptr_t,0x004dd700>(s->stream,v,s->endian);
    s->good &= static_cast<uint8_t>(r);
    return r;
}
uintptr_t SaveVersion(void*,Stream* s) { return Write(s); }
uintptr_t SavePair(void*,Stream* s) {
    auto r=Write(s);
    if (!s->good) return Low(r,0);
    return Write(s);
}
uintptr_t LoadVersion(void*,Stream* s) {
    uint32_t first=~0u,second=~0u;
    auto r=Read(s,&first);
    if (!s->good) return Low(r,0);
    r=Read(s,&second);
    if (!s->good) return Low(r,0);
    return second<=1 ? s->good : 0;
}
// As in the verified Index::Count twin: signed byte difference, arithmetic shift.
uintptr_t CountFirst(Records* self) {
    return static_cast<int32_t>(reinterpret_cast<uintptr_t>(self->first.end)-reinterpret_cast<uintptr_t>(self->first.begin)) >> 5;
}
uintptr_t CountLast(Records* self) {
    return static_cast<int32_t>(reinterpret_cast<uintptr_t>(self->last.end)-reinterpret_cast<uintptr_t>(self->last.begin)) >> 5;
}
static uintptr_t Set(Range& r,const void* v) {
    auto p=r.begin;
    for (;p!=r.end;p+=32) { unsigned char tmp[16]; std::memcpy(tmp,v,16); std::memcpy(p,tmp,16); }
    return reinterpret_cast<uintptr_t>(p);
}
static uintptr_t Find(Range& r,uint32_t key,const void* v) {
    auto p=r.begin;
    for (;p!=r.end;p+=32) {
        uint32_t k; std::memcpy(&k,p+16,4);
        if (k==key) { std::memmove(p,v,16); return Low(reinterpret_cast<uintptr_t>(p),1); }
    }
    return Low(reinterpret_cast<uintptr_t>(p),0);
}
uintptr_t SetFirst(Records* self,const void* v) { return Set(self->first,v); }
uintptr_t FindFirst(Records* self,uint32_t key,const void* v) { return Find(self->first,key,v); }
uintptr_t SetLast(Records* self,const void* v) { return Set(self->last,v); }
uintptr_t FindLast(Records* self,uint32_t key,const void* v) { return Find(self->last,key,v); }
static uintptr_t Scalar(Range& r,float v,size_t offset) {
    auto p=r.begin;
    for (;p!=r.end;p+=32) std::memcpy(p+offset,&v,4);
    return reinterpret_cast<uintptr_t>(p);
}
uintptr_t SetTime(Records* self,float v) { return Scalar(self->middle,v,20); }
uintptr_t SetWeight(Records* self,float v) { return Scalar(self->middle,v,24); }
uintptr_t FindTime(Records* self,uint32_t key,float v) {
    auto& r=self->middle;
    auto p=r.begin;
    for (;p!=r.end;p+=32) {
        uint32_t k; std::memcpy(&k,p+16,4);
        if (k==key) { std::memcpy(p+20,&v,4); return Low(reinterpret_cast<uintptr_t>(p),1); }
    }
    return Low(reinterpret_cast<uintptr_t>(p),0);
}
uintptr_t Clear(Table* self) {
    auto r=CallGameMethod<uintptr_t,0x005c9d90>(self,self->buckets,self->bucketCount);
    self->count=0;
    return r;
}
uintptr_t Insert(Table* self,InsertResult* out,const uint32_t* key,uint32_t) {
    uint32_t hash=*key, index=hash%self->bucketCount;
    auto bucket=self->buckets+index;
    for (auto p=*bucket;p;p=p->next) if (p->key==hash) {
        out->node=p; out->bucket=bucket; out->inserted=0; return reinterpret_cast<uintptr_t>(out);
    }
    struct Growth { uint8_t grow; unsigned char padding[3]; uint32_t count; } growth;
    CallGameMethod<void,0x004e09b0>(self->policy,&growth,self->bucketCount,self->count,1u);
    const char* file=nullptr;
#if defined(_M_IX86)
    file=reinterpret_cast<const char*>(0x00f98980);
#endif
    auto p=CallGameFunction<Node*,0x004e3fb0>(96u,16u,0u,self->descriptor,0u,0u,file,251u);
    if (p) p->key=*key;
    p->next=nullptr; // EA faults here if allocation failed.
    if (growth.grow) { index=hash%growth.count; CallGameMethod<void,0x0056b460>(self,growth.count); }
    bucket=self->buckets+index;
    p->next=*bucket; *bucket=p; ++self->count;
    out->node=p; out->bucket=bucket; out->inserted=1;
    return reinterpret_cast<uintptr_t>(out);
}
uintptr_t Load(Object* self,Stream* s) {
    uint32_t version=~0u;
    auto r=Read(s,&version);
    if (!s->good) return Low(r,0);
    if (version>2) return Low(version,0);
    if (version==1) return 1;
    uint32_t count=0;
    r=Read(s,&count);
    if (!s->good) return Low(r,0);
    r=Clear(&self->table);
    for (uint32_t i=0;i<count;++i) {
        uint32_t key;
        r=Read(s,&key);
        if (!s->good) return Low(r,0);
        InsertResult result;
        r=Insert(&self->table,&result,&key,0);
        if (!result.inserted) return Low(r,0);
        auto payload=result.node->payload;
        for (unsigned j=0;j<6;++j) {
            if (j<3) r=CallGameMethod<uintptr_t,0x007d7930>(s,payload+j*16);
            else r=CallGameMethod<uintptr_t,0x00565700>(s,payload+48+(j-3)*4);
            if (!s->good) return Low(r,0);
        }
    }
    return Low(r,s->good);
}
}
