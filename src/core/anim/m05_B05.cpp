#include "ts3/anim/m05_B05.h"
#include <cstring>
namespace ts3::anim::b05_impl {
uint32_t TypeBCC0(void*) { return 0x060d998eu; }
void* QueryBCD0(void* self,uint32_t id) {
    if (id == 0x060d998eu || id == 0x73deb8c8u) return self;
    return nullptr;
}
uint32_t TypeBFD0(void*) { return 0x065f6bc2u; }
void* QueryBFE0(void* self,uint32_t id) {
    if (id == 0x065f6bc2u || id == 0x73deb8c8u) return self;
    return nullptr;
}
uint32_t TypeCFF0(void*) { return 0xb3f291f8u; }
void* QueryD000(void* self,uint32_t id) {
    if (id == 0xb3f291f8u || id == 0x73deb8c8u) return self;
    return nullptr;
}
uint32_t TypeD3E0(void*) { return 0x06662b00u; }
void* QueryD3F0(void* self,uint32_t id) {
    if (id == 0x06662b00u || id == 0x73deb8c8u) return self;
    return nullptr;
}
// Copy the payload, not the stride: EA leaves padding in its destination untouched.
static void RecordCopy(unsigned char* to,const unsigned char* from,size_t bytes) {
    if (to) std::memcpy(to,from,bytes);
}
static uintptr_t Insert(Range* r,unsigned char* pos,const unsigned char* value,uint32_t stride) {
    const uint32_t payload=stride-12;
    if (r->end != r->capacity) {
        auto source=value;
        if (reinterpret_cast<uintptr_t>(source)>=reinterpret_cast<uintptr_t>(pos) &&
            reinterpret_cast<uintptr_t>(source)<reinterpret_cast<uintptr_t>(r->end)) source+=stride;
        RecordCopy(r->end,r->end-stride,payload);
        for (auto p=r->end-stride;p!=pos;) { p-=stride; RecordCopy(p+stride,p,payload); }
        RecordCopy(pos,source,payload);
        r->end+=stride;
        return reinterpret_cast<uintptr_t>(pos+stride);
    }
    uint32_t count=static_cast<uint32_t>((reinterpret_cast<uintptr_t>(r->end)-reinterpret_cast<uintptr_t>(r->begin))/stride);
    uint32_t capacity=count ? count*2u : 1u;
    auto storage=capacity ? CallGameFunction<unsigned char*,0x004e3fb0>(capacity*stride,16u,0u,r->descriptor,0u,0u,
#if defined(_M_IX86)
        reinterpret_cast<const char*>(0x00f98980),
#else
        static_cast<const char*>(nullptr),
#endif
        251u) : nullptr;
    auto out=storage;
    for (auto p=r->begin;p!=pos;p+=stride,out+=stride) RecordCopy(out,p,payload);
    RecordCopy(out,value,payload);
    out+=stride;
    for (auto p=pos;p!=r->end;p+=stride,out+=stride) RecordCopy(out,p,payload);
    if (r->begin) CallGameFunction<void,0x004e3f70>(r->begin);
    r->begin=storage; r->end=out; r->capacity=storage+capacity*stride;
    // The two otherwise parallel bodies leave different incidental EAX values.
    return stride==32 ? reinterpret_cast<uintptr_t>(r->capacity) : capacity;
}
uintptr_t InsertBD40(Range* r,unsigned char* p,const unsigned char* v) { return Insert(r,p,v,32); }
uintptr_t InsertD460(Range* r,unsigned char* p,const unsigned char* v) { return Insert(r,p,v,48); }
uintptr_t SetD030(Retained* self,ResourcePrefix* replacement) {
    auto old=self->resource;
    if (replacement != old) {
        if (replacement) ++replacement->atomicRefs; // Deliberately non-atomic.
        self->resource=replacement;
        if (old && --old->atomicRefs==0) {
            old->atomicRefs=1;
            return CallGameVirtual<uintptr_t,0>(old,1u);
        }
    }
    return reinterpret_cast<uintptr_t>(old);
}
void CopyD0C0(LocalPoseBuffer* self,const RigPose* pose) {
    if (!pose) return;
    std::memcpy(self->srt,pose->srt,48u*pose->boneCount);
    if (!pose->vertexWeights) return;
    auto input=static_cast<Range*>(pose->vertexWeights);
    const void* descriptor=nullptr;
#if defined(_M_IX86)
    descriptor=reinterpret_cast<const void*>(0x00fc1520);
#endif
    void* storage=CallGameFunction<void*,0x004e3f90>(16u,descriptor,0u,0u,0u,0u);
    void* vector=nullptr;
    if (storage) {
        auto count=static_cast<uint32_t>((input->end-input->begin)/4);
        vector=CallGameMethod<void*,0x00795f50>(storage,count,&descriptor);
    }
    self->vertexWeights=vector;
    // EA calls assign even if allocation failed; do not add a null guard.
    CallGameMethod<void,0x00810df0>(vector,input);
}
static uint32_t ReadWord(Reader* s) {
    s->offset=static_cast<int32_t>(static_cast<uint32_t>(s->offset)+4u);
    if (s->offset>=0) CallGameMethod<uintptr_t,0x00593ff0>(s,4u);
    uint32_t value; // Failed reads leave the original stack temporary unspecified.
    if (s->offset<0) std::memcpy(&value,s->buffer+s->offset-3,4);
    return value;
}
static void ReadBlock(Reader* s,void* to,int32_t size) {
    if (s->offset>=-size && !static_cast<uint8_t>(CallGameMethod<uintptr_t,0x00593ff0>(s,0u))) {
        s->failed=1;
        return;
    }
    std::memcpy(to,s->buffer+s->offset+1,size);
    s->offset+=size;
}
static uintptr_t Load(Records* self,Reader* s,uint32_t stride) {
    auto& r=self->records;
    r.end=r.begin;
    auto count=ReadWord(s);
    uintptr_t result=count;
    while (count--) {
        alignas(16) unsigned char record[48];
        if (stride==32) ReadBlock(s,record,16);
        else { ReadBlock(s,record+16,16); ReadBlock(s,record,12); }
        auto index=ReadWord(s);
        std::memcpy(record+stride-16,&index,4);
        result=reinterpret_cast<uintptr_t>(r.end);
        if (reinterpret_cast<uintptr_t>(r.end)<reinterpret_cast<uintptr_t>(r.capacity)) {
            auto out=r.end; r.end+=stride;
            RecordCopy(out,record,stride-12);
        } else result=Insert(&r,r.end,record,stride);
    }
    return result;
}
uintptr_t LoadBEC0(Records* self,Reader* s) { return Load(self,s,32); }
uintptr_t LoadD610(Records* self,Reader* s) { return Load(self,s,48); }
static uintptr_t WriteWord(Writer* s,uint32_t word) {
    s->offset=static_cast<int32_t>(static_cast<uint32_t>(s->offset)+4u);
    uintptr_t result=0;
    if (s->offset>=0) result=CallGameMethod<uintptr_t,0x00594140>(s,4u);
    if (s->offset<0) { std::memcpy(s->buffer+s->offset-3,&word,4); result=reinterpret_cast<uintptr_t>(s->buffer); }
    return result;
}
static void WriteBlock(Writer* s,const void* data,int32_t size) {
    if (s->offset>=-size && !static_cast<uint8_t>(CallGameMethod<uintptr_t,0x00594140>(s,0u))) {
        s->failed=1;
        return;
    }
    std::memcpy(s->buffer+s->offset+1,data,size);
    s->offset+=size;
}
uintptr_t SaveD220(const Records* self,Writer* s) {
    const auto& r=self->records;
    auto result=WriteWord(s,static_cast<uint32_t>((r.end-r.begin)/48));
    for (auto p=r.begin;p!=r.end;p+=48) {
        WriteBlock(s,p+16,16);
        WriteBlock(s,p,12);
        uint32_t index; std::memcpy(&index,p+32,4);
        result=WriteWord(s,index);
    }
    return result;
}
}
