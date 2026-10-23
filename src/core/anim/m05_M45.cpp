#include "ts3/anim/m05_M45.h"
namespace ts3::anim {
using RandomController=m45::RandomController;
using Handles=m45::Handles;
using Reader=m45::Reader;
using Record=m45::Record;
namespace {
void* Manager() {
#if defined(_M_IX86)
    return *reinterpret_cast<void**>(0x011d3c84);
#else
    return nullptr;
#endif
}
void* Callback(uintptr_t a) { return reinterpret_cast<void*>(a); }
uint32_t Count(void* const* a,void* const* b) { return uint32_t((uintptr_t(b)-uintptr_t(a))/sizeof(void*)); }
void** Copy(void** a,void** b,void** dest,uint32_t extra) {
    void** result;
    CallGameFunction<void,0x005e54f0>(&result,a,b,dest,extra);
    return result;
}
uintptr_t Status(Reader* r,uintptr_t value) { return CallGameMethod<uintptr_t,0x007d77c0>(r,value); }
uintptr_t ReadWord(Reader* r,void* out) { return Status(r,CallGameFunction<uintptr_t,0x004dd700>(r->stream,out,r->flags)); }
void Append(Handles* v,uint32_t value) {
    auto* end=reinterpret_cast<uint32_t*>(v->end);
    if(uintptr_t(end)<uintptr_t(v->capacity)) {
        v->end=reinterpret_cast<void**>(end+1);
        if(end) *end=value;
    } else CallGameMethod<void,0x00405fb0>(v,end,&value);
}
}
RandomController* m45::Destroy(RandomController* self,uint32_t flags) {
    CallGameMethod<void,0x0056d6a0>(self);
    if(flags & 1) CallGameFunction<void,0x004e3f70>(self);
    return self;
}
uintptr_t m45::Update(RandomController* self,float dt) {
    // EAX is unspecified if no fill call and no child call occur.
    uintptr_t result=0;
    while(self->state<2) {
        result=CallGameMethod<uintptr_t,0x0056d470>(self);
        if(!uint8_t(result)) break;
    }
    if(self->child) result=CallGameVirtual<uintptr_t,2>(self->child,dt);
    return result;
}
uintptr_t m45::Insert(Handles* self,void** at,uint32_t n,void** value) {
    uintptr_t result=uintptr_t(self->end);
    if(n>Count(self->end,self->capacity)) {
        uint32_t size=Count(self->begin,self->end);
        uint32_t capacity=size*2; if(!capacity) capacity=1;
        if(capacity<=size+n) capacity=size+n;
        auto* data=capacity?CallGameFunction<void**,0x004e3f90>(uint32_t(sizeof(void*)*capacity),self->descriptor,0,0,Callback(0x00f98980),0xe7):nullptr;
        auto* mid=Copy(self->begin,at,data,n);
        CallGameFunction<void,0x0056d000>(mid,n,value,n);
        auto* end=Copy(at,self->end,mid+n,n);
        CallGameMethod<void,0x005af640>(self,self->begin,self->end);
        if(self->begin) CallGameFunction<void,0x004e3f70>(self->begin);
        self->begin=data; self->end=end; self->capacity=data+capacity;
        return capacity;
    }
    if(n) {
        void* temp=nullptr;
        if(auto* manager=Manager()) CallGameVirtual<void,14>(manager,&temp,value,Callback(0x0056cd70),Callback(0x00d4b150),1,0,0);
        auto* oldEnd=self->end;
        uint32_t tail=Count(at,oldEnd);
        if(n>=tail) {
            CallGameFunction<void,0x0056d000>(oldEnd,n-tail,&temp,n-tail);
            self->end+=n-tail;
            Copy(at,oldEnd,self->end,n-tail); self->end+=tail;
            CallGameFunction<void,0x0056d270>(at,oldEnd,&temp);
        } else {
            auto* split=oldEnd-n;
            Copy(split,oldEnd,oldEnd,uint32_t(uintptr_t(split)));
            self->end+=n;
            CallGameFunction<void,0x00a87f70>(at,split,oldEnd);
            CallGameFunction<void,0x0056d270>(at,at+n,&temp);
        }
        return CallGameMethod<uintptr_t,0x005e11d0>(&temp);
    }
    return result;
}
uintptr_t m45::Resize(Handles* self,uint32_t n) {
    auto* oldEnd=self->end;
    uint32_t count=Count(self->begin,oldEnd);
    if(n<=count) {
        auto* end=self->begin+n;
        auto* start=CallGameFunction<void**,0x0056d050>(oldEnd,oldEnd,end);
        // Empty destruction leaves the copy helper's EAX (the new end) live.
        auto result=uintptr_t(start)>=uintptr_t(self->end) ? uintptr_t(start) :
            CallGameMethod<uintptr_t,0x005af640>(self,start,self->end);
        self->end-=Count(end,oldEnd);
        return result;
    }
    void* temp=nullptr; Insert(self,self->end,n-count,&temp);
    uintptr_t result=uintptr_t(temp);
    if(temp) {
        if(auto* manager=Manager()) CallGameVirtual<void,15>(manager,&temp,Callback(0x00d4b150));
        result=uintptr_t(temp);
        if(temp && !(uintptr_t(temp)&3)) { result=CallGameVirtual<uintptr_t,1>(temp); temp=nullptr; }
    }
    return result;
}
uintptr_t m45::Synchronize(RandomController* self) {
    uint32_t n=uint32_t((uintptr_t(self->recordsEnd)-uintptr_t(self->records))/sizeof(Record));
    uintptr_t result=Resize(&self->handles,n);
    for(uint32_t i=0;i<n;++i) {
        result=uintptr_t(self->records+i);
        if(auto* manager=Manager()) result=CallGameVirtual<uintptr_t,13>(manager,self->records+i,0x6b20c4f3,self->handles.begin+i,0,0,Callback(0x00d4b150),0,1);
    }
    return result;
}
uintptr_t m45::Load(RandomController* self,Reader* reader) {
    self->recordsEnd=self->records; self->weightsEnd=self->weights; self->indicesEnd=self->indices;
    uint32_t version=~0u;
    uintptr_t result=ReadWord(reader,&version);
    auto failure=[&]() { return result & ~uintptr_t(255); };
    if(!reader->good || version>1) return failure();
    result=Status(reader,CallGameFunction<uintptr_t,0x004dd4c0>(reader->stream,&self->seed));
    if(!reader->good) return failure();
    uint32_t count=0;
    result=Status(reader,CallGameFunction<uintptr_t,0x004dd540>(reader->stream,&count,reader->flags));
    if(!reader->good) return failure();
    CallGameMethod<void,0x00d750a0>(&self->records,uint32_t(uint16_t(count)));
    CallGameMethod<void,0x004d4fd0>(&self->weights,uint32_t(uint16_t(count)));
    for(uint32_t i=0;i<uint16_t(count);++i) {
        Record record;
        result=Status(reader,CallGameFunction<uintptr_t,0x004dd660>(reader->stream,record.key,reader->flags));
        if(!reader->good) return failure();
        result=ReadWord(reader,&record.key[2]); if(!reader->good) return failure();
        result=CallGameMethod<uintptr_t,0x00565700>(reader,&record.handle); if(!reader->good) return failure();
        CallGameMethod<void,0x004a49d0>(&self->records,&record);
    }
    for(uint32_t i=0;i<uint16_t(count);++i) {
        uint32_t value=0; result=ReadWord(reader,&value); if(!reader->good) return failure();
        Append(reinterpret_cast<Handles*>(&self->weights),value);
    }
    for(uint32_t i=0;i<uint16_t(count);++i) {
        uint32_t value=0; result=ReadWord(reader,&value); if(!reader->good) return failure();
        Append(reinterpret_cast<Handles*>(&self->indices),value);
    }
    result=Synchronize(self);
    while(self->state<2) { result=CallGameMethod<uintptr_t,0x0056d470>(self); if(!uint8_t(result)) break; }
    return (result & ~uintptr_t(255)) | reader->good;
}
}
