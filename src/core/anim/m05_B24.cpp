#include "ts3/anim/m05_B24.h"
#include <cstring>
#include <algorithm>
namespace ts3::anim::b24 {
void* __fastcall Query(RigResource* self,void*,uint32_t id) {
    return id==0x8eaf13deu || id==0x02269ed1u || id==0xeb54b802u || id==0xee3f516eu ? self : nullptr;
}
uint32_t __fastcall HasIndices(RigResource* self,void*) {
    for(unsigned i=0;i<7;++i) { int32_t n; std::memcpy(&n,self->opaquebc+4*i,4); if(n>=0) return 1; }
    return 0;
}
uint32_t __fastcall ChannelCount(RigResource* self,void*) {
    if(!self->vertexChannelCount) {
        auto manager=CallGameFunction<void*,0x005EB7C0>();
        auto resource=manager?CallGameMethod<void*,0x005EC9E0>(manager,self->base.opaque08):nullptr;
        self->vertexChannelCount=resource?CallGameFunction<uint32_t,0x005C9480>():4;
    }
    return self->vertexChannelCount;
}
int32_t __fastcall ChannelIndex(RigResource* self,void*,uint32_t hash) {
    if(ChannelCount(self,nullptr)!=4) return CallGameFunction<int32_t,0x005CF6A0>(hash);
    uint32_t index=hash+0xab1aa04du;
    return index<=3?int32_t(3-index):-1;
}
SrtValue* __cdecl CopySrt(const SrtValue* first,const SrtValue* last,SrtValue* out) {
    while(first!=last) { std::memcpy(out,first,48); ++first; ++out; }
    return out;
}
Chain* __fastcall InitChain(Chain* self,void*) {
    self->joints.begin=self->joints.end=self->joints.capacity=nullptr;
#if defined(_M_IX86)
    self->joints.descriptor=reinterpret_cast<void*>(0x00FC3284);
#else
    self->joints.descriptor=nullptr;
#endif
    for(auto& index:self->indices) index=-1;
    self->flags=0; return self;
}
static void Free(void* p) { if(p) CallGameFunction<void,0x004E3F70>(p); }
static const char* AllocationSource() {
#if defined(_M_IX86)
    return reinterpret_cast<const char*>(0x00F98980);
#else
    return nullptr;
#endif
}
void __fastcall DestroyChains(Vector<Chain>* self,void*) {
    for(auto p=self->begin;p<self->end;++p) Free(p->joints.begin);
    Free(self->begin);
}
uint32_t __fastcall Type(RigResource*,void*) { return 0x8eaf13deu; }
void __fastcall InsertSrt(Vector<SrtValue>* self,void*,SrtValue* pos,uint32_t count,const SrtValue* value) {
    auto end=self->end;
    if(count<=uint32_t(self->capacity-end)) {
        if(!count) return;
        SrtValue saved=*value;
        auto tail=uint32_t(end-pos);
        if(count<tail) {
            CopySrt(end-count,end,end);
            self->end+=count;
            for(auto p=end-count;p!=pos;) { --p; p[count]=*p; }
            for(uint32_t i=0;i<count;++i) pos[i]=saved;
        } else {
            for(uint32_t i=0;i<count-tail;++i) end[i]=saved;
            self->end+=count-tail;
            CopySrt(pos,end,self->end);
            self->end+=tail;
            for(auto p=pos;p!=end;++p) *p=saved;
        }
    } else {
        auto size=uint32_t(end-self->begin);
        auto capacity=std::max(size?2*size:1u,size+count);
        auto data=CallGameFunction<SrtValue*,0x004E3FB0>(48*capacity,16,0,self->descriptor,0,0,AllocationSource(),0xfb);
        auto out=CopySrt(self->begin,pos,data);
        for(uint32_t i=0;i<count;++i) out[i]=*value;
        out=CopySrt(pos,end,out+count);
        Free(self->begin);
        self->begin=data; self->end=out; self->capacity=data+capacity;
    }
}
void __fastcall ResizeSrt(Vector<SrtValue>* self,void*,uint32_t n) {
    auto size=uint32_t(self->end-self->begin);
    if(n>size) { SrtValue uninitialized; InsertSrt(self,nullptr,self->end,n-size,&uninitialized); }
    else self->end=self->begin+n;
}
Chain* __cdecl CopyChains(const Chain* first,const Chain* last,Chain* out) {
    while(first!=last) {
        CallGameMethod<void,0x00810DF0>(&out->joints,&first->joints);
        std::memcpy(out->indices,first->indices,sizeof(first->indices)); out->flags=first->flags;
        ++first; ++out;
    }
    return out;
}
Chain* __fastcall EraseChains(Vector<Chain>* self,void*,Chain* first,Chain* last) {
    auto out=CopyChains(last,self->end,first);
    for(auto p=out;p<self->end;++p) Free(p->joints.begin);
    self->end-=last-first;
    return first;
}
// The exception-aware uninitialized-copy/fill helpers are external EASTL boundaries.
static Chain* UninitializedCopy(const Chain* first,const Chain* last,Chain* out,void* descriptor) {
    Chain* result;
    CallGameFunction<void,0x0057BCA0>(&result,first,last,out,descriptor);
    return result;
}
static void Fill(Chain* first,Chain* last,const Chain* value) { CallGameFunction<void,0x0057BE50>(first,last,value); }
static void ConstructFill(Chain* out,uint32_t n,const Chain* value) { CallGameFunction<void,0x0057BD60>(out,n,value); }
void __fastcall InsertChains(Vector<Chain>* self,void*,Chain* pos,uint32_t count,const Chain* value) {
    auto end=self->end;
    if(count<=uint32_t(self->capacity-end)) {
        if(!count) return;
        Chain saved;
        CallGameMethod<void,0x0057BC30>(&saved,value);
        auto tail=uint32_t(end-pos);
        if(count<tail) {
            UninitializedCopy(end-count,end,end,self->descriptor);
            self->end+=count;
            CallGameFunction<Chain*,0x0057BDF0>(pos,end-count,end);
            Fill(pos,pos+count,&saved);
        } else {
            ConstructFill(end,count-tail,&saved);
            self->end+=count-tail;
            UninitializedCopy(pos,end,self->end,self->descriptor);
            self->end+=tail;
            Fill(pos,end,&saved);
        }
        Free(saved.joints.begin);
    } else {
        auto size=uint32_t(end-self->begin);
        auto capacity=std::max(size?2*size:1u,size+count);
        auto data=CallGameFunction<Chain*,0x004E3F90>(uint32_t(sizeof(Chain))*capacity,self->descriptor,0,0,AllocationSource(),0xe7);
        auto out=UninitializedCopy(self->begin,pos,data,self->descriptor);
        ConstructFill(out,count,value);
        out=UninitializedCopy(pos,end,out+count,self->descriptor);
        for(auto p=self->begin;p<end;++p) Free(p->joints.begin);
        Free(self->begin);
        self->begin=data; self->end=out; self->capacity=data+capacity;
    }
}
void __fastcall ResizeChains(Vector<Chain>* self,void*,uint32_t n) {
    auto size=uint32_t(self->end-self->begin);
    if(n>size) {
        Chain value; InitChain(&value,nullptr);
        InsertChains(self,nullptr,self->end,n-size,&value);
        Free(value.joints.begin);
    } else EraseChains(self,nullptr,self->begin+n,self->end);
}
float __fastcall Weight(const void* self,void*) { float result; std::memcpy(&result,static_cast<const unsigned char*>(self)+0x38,4); return result; }
}
