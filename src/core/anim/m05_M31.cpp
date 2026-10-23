#include "ts3/anim/m05_M31.h"
#include <cstring>
namespace ts3::anim::m31 {
namespace {
template<class T> size_t Count(T* first,T* last) {
    return (reinterpret_cast<uintptr_t>(last)-reinterpret_cast<uintptr_t>(first))/sizeof(T);
}
template<class T> T* Advance(T* p,size_t n) {
    return reinterpret_cast<T*>(reinterpret_cast<uintptr_t>(p)+n*sizeof(T));
}
uintptr_t ConstructEntries(Entry** cursor,Entry* first,Entry* last,Entry* dest,uintptr_t tag) {
    return CallGameFunction<uintptr_t,0x00569110>(cursor,first,last,dest,tag);
}
Entry* CopyEntries(Entry* first,Entry* last,Entry* dest) {
    return CallGameFunction<Entry*,0x00577d30>(first,last,dest);
}
}
uintptr_t AssignEntries(EntryRange* self,Entry* first,Entry* last,uintptr_t tag) {
    const size_t n=Count(first,last);
    if(n>Count(self->begin,self->capacity)) {
        Entry* cursor;
        auto result=ConstructEntries(&cursor,first,last,nullptr,tag);
        // EA deliberately leaves a null base and byte-count end/capacity here.
        self->begin=nullptr;
        self->end=self->capacity=Advance<Entry>(nullptr,n);
        return result;
    }
    const size_t old=Count(self->begin,self->end);
    if(n<=old) {
        self->end=CopyEntries(first,last,self->begin);
        return reinterpret_cast<uintptr_t>(self->end);
    }
    Entry* middle=Advance(first,old);
    CopyEntries(first,middle,self->begin);
    Entry* cursor;
    auto result=ConstructEntries(&cursor,middle,last,self->end,tag);
    self->end=cursor;
    return result;
}
Slot* CopySlots(Slot* first,Slot* last,Slot* dest) {
    while(first!=last) {
        dest->id=first->id;
        if(&dest->entries!=&first->entries) {
            dest->entries.end=dest->entries.begin;
            AssignEntries(&dest->entries,first->entries.begin,first->entries.end,reinterpret_cast<uintptr_t>(last));
        }
        ++first; ++dest;
    }
    return dest;
}
Slot** ConstructSlots(Slot** out,Slot* first,Slot* last,Slot* dest,uintptr_t tag) {
    *out=dest;
    while(first!=last) {
        if(*out) {
            Slot* p=*out;
            p->id=first->id;
            p->entries.begin=p->entries.end=p->storage;
            p->entries.capacity=p->storage+10;
            AssignEntries(&p->entries,first->entries.begin,first->entries.end,reinterpret_cast<uintptr_t>(out));
        }
        *out=Advance(*out,1);
        ++first;
    }
    (void)tag;
    return out;
}
Slot* AllocateSlots(SlotRange* self,uint32_t count,Slot* first,Slot* last) {
    Slot* dest=nullptr;
    if(count) {
#if defined(_M_IX86)
        dest=CallGameFunction<Slot*,0x004e3f90>(uint32_t(count*uint32_t(sizeof(Slot))),self->descriptor,0,0,
            reinterpret_cast<const char*>(0x00f98980u),0xe7);
#else
        AbortNotRewritten("m31 slot allocator");
#endif
    }
    Slot* cursor;
    ConstructSlots(&cursor,first,last,dest,count);
    return dest;
}
SlotRange* AssignSlots(SlotRange* self,const SlotRange* source) {
    if(self==source) return self;
    const size_t n=Count(source->begin,source->end);
    if(n>Count(self->begin,self->capacity)) {
        Slot* dest=AllocateSlots(self,static_cast<uint32_t>(n),source->begin,source->end);
        if(self->begin) CallGameFunction<void,0x004e3f70>(self->begin);
        self->begin=dest;
        self->end=self->capacity=Advance(dest,n);
    } else {
        const size_t old=Count(self->begin,self->end);
        if(n>old) {
            CopySlots(source->begin,Advance(source->begin,old),self->begin);
            Slot* cursor;
            ConstructSlots(&cursor,Advance(source->begin,old),source->end,self->end,reinterpret_cast<uintptr_t>(source->end));
        } else {
            CopySlots(source->begin,source->end,self->begin);
        }
        self->end=Advance(self->begin,n);
    }
    return self;
}
ClipIkInfo* AssignIk(ClipIkInfo* self,const ClipIkInfo* source) {
    self->rotation=source->rotation;
    if(&self->begin!=&source->begin) {
        if(self->begin!=self->end) {
            *static_cast<unsigned char*>(self->begin)=0;
            self->end=self->begin;
        }
        CallGameMethod<void,0x0099c9c0>(&self->begin,source->begin,source->end);
    }
    AssignSlots(reinterpret_cast<SlotRange*>(&self->slotBegin),reinterpret_cast<const SlotRange*>(&source->slotBegin));
    CallGameMethod<void,0x00569710>(&self->namespaceBegin,&source->namespaceBegin);
    CallGameMethod<void,0x00810df0>(&self->hashBegin,&source->hashBegin);
    std::memcpy(self->opaquec0,source->opaquec0,sizeof(self->opaquec0));
    self->valueCC=source->valueCC;
    self->valueD0=source->valueD0;
    return self;
}
#if defined(_M_IX86)
namespace hooks {
uintptr_t __fastcall AssignEntries(EntryRange* s,void*,Entry* a,Entry* b,uintptr_t t) { return m31::AssignEntries(s,a,b,t); }
Slot* __fastcall AllocateSlots(SlotRange* s,void*,uint32_t n,Slot* a,Slot* b) { return m31::AllocateSlots(s,n,a,b); }
SlotRange* __fastcall AssignSlots(SlotRange* s,void*,const SlotRange* a) { return m31::AssignSlots(s,a); }
ClipIkInfo* __fastcall AssignIk(ClipIkInfo* s,void*,const ClipIkInfo* a) { return m31::AssignIk(s,a); }
}
#endif
}
