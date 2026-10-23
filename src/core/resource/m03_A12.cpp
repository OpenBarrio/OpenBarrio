#include "ts3/resource/m03_A12.h"
#include <atomic>
#include <algorithm>

namespace ts3::resource::m03::a12 {
namespace {
void* Allocate(size_t bytes,const char* tag) {
#if defined(_M_IX86)
    return CallGameFunction<void*,0x004E3F90>(bytes,tag,0u,0u,reinterpret_cast<const char*>(0x00F98980),0xe7u);
#else
    (void)bytes; (void)tag; NotRewritten("A12 resource allocation");
#endif
}
void Free(void* p) { CallGameFunction<void,0x004E3F70>(p); }
void Grow(KeyDeque* q,uint32_t n,bool back) { CallGameMethod<void,0x00A72010>(q,n,uint32_t(back)); }
uintptr_t Clear(PointerDeque* q) { return CallGameMethod<uintptr_t,0x00800890>(q); }
void* Loader() {
#if defined(_M_IX86)
    return *reinterpret_cast<void**>(0x011D3C84);
#else
    NotRewritten("A12 loader singleton");
#endif
}
void Cancel(void* loader,void** slot) {
#if defined(_M_IX86)
    CallGameVirtual<void,15>(loader,slot,reinterpret_cast<void*>(0x00D4B150));
#else
    (void)loader; (void)slot; NotRewritten("A12 cancel callback");
#endif
}
void Release(void* p) { CallGameVirtual<void,1>(p); }
void CopyValue(Value* to,const Value* from) {
    // Four ordered loads/stores, not memmove: aliasing/reentrant observers retain EA order.
    for(unsigned i=0;i<4;++i) to->words[i]=from->words[i];
}
void Next(Iterator& it) {
    if(++it.cur==it.end) { ++it.node; it.begin=*it.node; it.cur=it.begin; it.end=it.begin+16; }
}
}
const Boundaries& OriginalBoundaries() {
    static const Boundaries b{Allocate,Free,Grow,Clear,Loader,Cancel,Release}; return b;
}
Iterator* CopyFlat(Iterator* out,const Value* first,const Value* last,Iterator dst) {
    while(first!=last) { CopyValue(dst.cur,first); Next(dst); ++first; }
    *out=dst; return out;
}
Iterator* Offset(const Iterator* self,Iterator* out,int32_t delta) {
    // Adapted from verified C4BA30: 16-byte values, 16 values per page (not 2076/4).
    Iterator t=*self;
    const int32_t index=static_cast<int32_t>(uint32_t(delta)+uint32_t(t.cur-t.begin));
    if(uint32_t(index)<16) t.cur+=delta;
    else {
        const int32_t biased=static_cast<int32_t>(uint32_t(index)+0x01000000u);
        const int32_t page=biased/16-0x00100000;
        t.node+=page; t.begin=*t.node; t.end=t.begin+16;
        t.cur=t.begin+(int64_t(index)-int64_t(page)*16);
    }
    *out=t; return out;
}
Iterator* Reserve(KeyDeque* q,Iterator* out,uint32_t count,uint32_t back,const Boundaries& b) {
    if(!back) {
        const uint32_t avail=uint32_t(q->first.cur-q->first.begin);
        if(avail<count) {
            const uint32_t blocks=(count-avail+15u)>>4;
            const int32_t room=int32_t(q->first.node-q->map);
            if(int32_t(blocks)>room) b.grow(q,blocks-uint32_t(room),false);
            for(uint32_t i=1;i<=blocks;++i) q->first.node[-ptrdiff_t(i)]=static_cast<Value*>(b.allocate(256,q->tag));
        }
        return Offset(&q->first,out,static_cast<int32_t>(0u-count));
    }
    const uint32_t avail=uint32_t(q->last.end-q->last.cur-1);
    if(avail<count) {
        const uint32_t blocks=(count-avail+15u)>>4;
        const int32_t room=int32_t(q->capacity-(q->last.node-q->map));
        if(int32_t(blocks)>room-1) b.grow(q,blocks-uint32_t(room)+1,true);
        for(uint32_t i=1;i<=blocks;++i) q->last.node[i]=static_cast<Value*>(b.allocate(256,q->tag));
    }
    return Offset(&q->last,out,static_cast<int32_t>(count));
}
Value* ConstructFill(Iterator first,Iterator last,const Value* value) {
    while(first.cur!=last.cur) { if(first.cur) CopyValue(first.cur,value); Next(first); }
    return first.cur;
}
Iterator* ConstructCopy(Iterator* out,Iterator first,Iterator last,Iterator dst) {
    // The original publishes the initial destination before constructing, unlike CopyFlat.
    out->begin=dst.begin; out->node=dst.node; out->cur=dst.cur; out->end=dst.end;
    while(first.cur!=last.cur) {
        if(dst.cur) CopyValue(dst.cur,first.cur);
        Next(first);
        if(++dst.cur==dst.end) {
            ++out->node; dst.cur=*out->node;
            out->begin=dst.cur; dst.end=dst.cur+16;
        }
    }
    out->cur=dst.cur; out->end=dst.end; return out;
}
Value* AssignFill(Iterator first,Iterator last,const Value* value) {
    while(first.cur!=last.cur) { CopyValue(first.cur,value); Next(first); }
    return first.cur;
}
Value* CopyThenFill(Iterator first,Iterator last,Iterator dst,Iterator dstLast,const Value* value) {
    ConstructCopy(&dst,first,last,dst);
    return ConstructFill(dst,dstLast,value);
}
Iterator* FillThenCopy(Iterator* out,Iterator first,Iterator middle,const Value* value,Iterator src,Iterator srcLast) {
    ConstructFill(first,middle,value);
    return ConstructCopy(out,src,srcLast,middle);
}
int32_t Request(Preloader* self,int32_t,int32_t,int32_t) {
    // Verified ExchangeFlag1 donor, adapting only the field displacement +8C -> +4.
    return std::atomic_ref<int32_t>(self->requested).exchange(1);
}
void** Initialize(PointerDeque* q,uint32_t count,const Boundaries& b) {
    const uint32_t pages=(count>>6)+1;
    q->capacity=std::max(pages+2,8u);
    q->map=static_cast<void***>(b.allocate(size_t(q->capacity)*sizeof(void*),q->tag));
    auto first=q->map+((q->capacity-pages)>>1);
    auto last=first+pages;
    for(auto p=first;p<last;++p) *p=static_cast<void**>(b.allocate(64*sizeof(void*),q->tag));
    q->first={*first,*first,*first+64,first};
    --last; q->last={*last+(count&63),*last,*last+64,last};
    return q->last.cur;
}
uintptr_t Drain(Preloader* self,const Boundaries& b) {
    for(auto& slot:self->slots) {
        if(void* loader=b.loader()) b.cancel(loader,&slot);
        if(slot && !(reinterpret_cast<uintptr_t>(slot)&3)) { b.release(slot); slot=nullptr; }
    }
    uintptr_t result=b.clear(&self->auxiliary);
    auto& q=self->pending;
    while(q.first.cur!=q.last.cur) {
        auto end=q.last.cur;
        if(end==q.last.begin) end=q.last.node[-1]+64;
        void* object=end[-1];
        if(q.last.cur!=q.last.begin) --q.last.cur;
        else {
            if(q.last.begin) b.free(q.last.begin);
            --q.last.node; q.last.begin=*q.last.node; q.last.end=q.last.begin+64; q.last.cur=q.last.end-1;
        }
        b.release(object);
        result=reinterpret_cast<uintptr_t>(q.first.cur);
    }
    return result;
}
Iterator* CopyFlatWrapper(Iterator* out,const Value* first,const Value* last,Iterator dst) {
    return CopyFlat(out,first,last,dst);
}
Iterator* CopyRangeWrapper(Iterator* out,Iterator first,Iterator last,Iterator dst) {
    // Verified ChunkedPtrAssignA wrapper shape, but this target calls the flat Value copy,
    // NOT the donor's intrusive-pointer assignment helper.
    return CopyFlat(out,first.cur,last.cur,dst);
}
}
