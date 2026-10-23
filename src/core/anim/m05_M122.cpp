#include "ts3/anim/m05_M122.h"
#include <cstring>
namespace ts3::anim::m122 {
namespace {
uint32_t Abs(uint32_t u) { return (u & 0x80000000u) ? 0u-u : u; }
int32_t Signed(uint32_t u) { int32_t r; std::memcpy(&r,&u,4); return r; }
}
bool __cdecl SelectImpl(Manager* self,uint32_t fallback,int32_t first,int32_t second,int32_t third,int32_t category,Style* out,bool raw) {
    unsigned char found=0;
    bool prior=false;
    if(fallback) prior=Select(self,nullptr,0,first,second,third,category,out,false);
    if(!first) first=5;
    if(!second) second=12;
    if(!third) third=256;
    if(!raw) {
        first=CallGameFunction<int32_t,0x005af020>(first);
        second=CallGameFunction<int32_t,0x005af020>(second);
    }
    Style best{};
#if defined(_M_IX86)
    best.sound.begin=best.sound.end=reinterpret_cast<void*>(0x011c7e54);
    best.sound.capacity=reinterpret_cast<void*>(0x011c7e56);
    best.sound.descriptor=reinterpret_cast<void*>(0x010043e0);
#endif
    int32_t score=0x0fffffff, tie=0;
    auto* range=CallGameMethod<Range*,0x00844870>(reinterpret_cast<unsigned char*>(self)+sizeof(Manager),&fallback);
    if(range->begin!=range->end) {
        for(auto* p=range->begin;p!=range->end;++p) {
            uint32_t distance=Abs(uint32_t(first)-uint32_t(p->first));
            if(p->first<first) distance*=2;
            distance=distance*2+Abs(uint32_t(second)-uint32_t(p->second))+8*Abs(uint32_t(third)-uint32_t(p->third));
            int32_t d=Signed(distance);
            if(p->category==category && (d<score || (d==score && first>tie))) {
                CallGameMethod<void,0x007578b0>(&best,p);
                score=d; tie=first; found=1;
            }
        }
        if(found) CallGameMethod<void,0x0083be10>(out,&best);
    }
    CallGameMethod<void,0x00754060>(&best);
    return prior || found!=0;
}
bool __fastcall Select(Manager* p,void*,uint32_t f,int32_t a,int32_t b,int32_t c,int32_t d,Style* o,bool raw) {
    return SelectImpl(p,f,a,b,c,d,o,raw);
}
bool __fastcall SelectMapped(Manager* p,void*,uint32_t f,int32_t a,int32_t b,int32_t c,int32_t d,Style* o) {
    return Select(p,nullptr,f,a,b,c,d,o,false);
}
bool __fastcall SelectDefault(Manager* p,void*,uint32_t f,int32_t a,int32_t b,int32_t c,Style* o) {
    return Select(p,nullptr,f,a,b,c,-1,o,false);
}
Node* __fastcall Append(List* self,void*,const void* value) {
#if defined(_M_IX86)
    auto* file=reinterpret_cast<void*>(0x00f98980);
#else
    void* file=nullptr;
#endif
    auto* node=CallGameFunction<Node*,0x004e3f90>(sizeof(Node),self->descriptor,0,0,file,0xe7);
    if(reinterpret_cast<uintptr_t>(&node->value)!=0) CallGameMethod<void,0x00844b90>(&node->value,value);
    node->next=reinterpret_cast<Node*>(self);
    Node* old=self->previous;
    node->previous=old;
    old->next=node;
    self->previous=node;
    return old;
}
}
