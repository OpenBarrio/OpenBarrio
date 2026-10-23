#include "ts3/anim/m05_M113.h"
#include <intrin.h>
#include <cstring>
#include <cmath>
namespace ts3::anim::m113 {
namespace {
uint32_t Bits(float f) { uint32_t b; std::memcpy(&b,&f,4); return b; }
float Float(uint32_t b) { float f; std::memcpy(&f,&b,4); return f; }
int Sign(float f) { return Bits(f)==0x80000000u || !(f>=0.0f) ? -1:1; }
void Add(Ref* p) { _InterlockedIncrement(reinterpret_cast<volatile long*>(&p->refs)); }
void Release(Ref* p) { if(p && !_InterlockedDecrement(reinterpret_cast<volatile long*>(&p->refs))) { _InterlockedExchange(reinterpret_cast<volatile long*>(&p->refs),1); CallGameVirtual<void,0>(p,1); } }
void ReleaseLocal(Ref* p) { if(p && !--p->refs) { p->refs=1; CallGameVirtual<void,0>(p,1); } }
void* Allocate(uint32_t n,const char* tag,int line) {
    const char* file=nullptr;
#if defined(_M_IX86)
    if(line) file=reinterpret_cast<const char*>(0x00f98980);
    else tag=reinterpret_cast<const char*>(n==192?0x01023250:0x01023228);
#endif
    return CallGameFunction<void*,0x004e3f90>(n,tag,0,0,file,line);
}
}
uint8_t __cdecl Create(Entry* out,const float* phase) {
    out->phase=*phase;
    auto* p=static_cast<Ref*>(Allocate(192,"Locomotion/ControllerBuilderBase",0));
    if(p) {
#if defined(_M_IX86)
        p->vtable=reinterpret_cast<void**>(0x0107396c);
        _InterlockedExchange(reinterpret_cast<volatile long*>(&p->refs),0);
        p->vtable=reinterpret_cast<void**>(0x010231bc);
        CallGameMethod<void,0x0083d960>(reinterpret_cast<unsigned char*>(p)+sizeof(Ref));
#else
        NotRewritten("ControllerBuilderBase vtable");
#endif
    }
    auto* old=out->value;
    if(p!=old) { if(p) Add(p); out->value=p; Release(old); }
    return 1;
}
uint8_t __fastcall Build(Builder* self,void*,void** context,Result* out) {
    float phase; CallGameVirtual<void,10>(self,&phase,context);
    auto* sentinel=&self->sentinel; auto* first=sentinel->next; auto* selected=sentinel;
    for(auto* p=first;p!=sentinel;p=p->next) {
        if(!(phase>=p->phase) || Sign(phase)<Sign(p->phase)) break;
        selected=p;
    }
    if(selected==sentinel && first!=sentinel) selected=!(first->phase>phase) || Sign(first->phase)<Sign(phase)?sentinel->previous:first;
    if(selected==sentinel) return 0;
    auto* next=selected->next;
    if(next==sentinel) return CallGameVirtual<uint8_t,3>(selected->child,context,out);
    auto* check=selected->phase<0?next:selected;
    if(check->child && !CallGameVirtual<uint8_t,6>(check->child)) return CallGameVirtual<uint8_t,3>(selected->child,context,out);
    float delta=next->phase-selected->phase;
    float weight=0;
    if(delta>0) {
        float w=(phase-selected->phase)/delta;
        w=Float(Bits(w)&static_cast<uint32_t>(static_cast<int32_t>(Bits(-0.0f-w))>>31));
        uint32_t mask=static_cast<uint32_t>(static_cast<int32_t>(Bits(1.0f-w))>>31);
        weight=Float((mask&0x3f800000u)|(~mask&Bits(w)));
    }
    if(std::fabs(weight)<0.00001f) { CallGameVirtual<void,3>(selected->child,context,out); return 1; }
    if(std::fabs(weight-1.0f)<0.00001f) { CallGameVirtual<void,3>(next->child,context,out); return 1; }
    Result a{},b{};
    CallGameVirtual<void,3>(selected->child,context,&a); CallGameVirtual<void,3>(next->child,context,&b);
    void* memory=Allocate(60,"Animation/PhaseMatchedBlendController",0);
    Ref* controller=memory?CallGameMethod<Ref*,0x0056a350>(memory,*context,a.value,b.value,weight):nullptr;
    CallGameMethod<void,0x00cf3c70>(out,controller);
    out->flag=a.flag||b.flag;
    ReleaseLocal(b.value); ReleaseLocal(a.value); return 1;
}
uint8_t __fastcall Update(Builder* self,void*,void** context,uint32_t cache) {
    float phase; int mode=CallGameVirtual<int,9>(self,context,&phase);
    if(static_cast<uint8_t>(cache)) self->cached=phase;
    uint8_t accepted=0;
    auto* sentinel=&self->sentinel;
    for(auto* p=sentinel->next;p!=sentinel;p=p->next) {
        bool match=mode==2 || (mode==1 && Sign(phase)==Sign(p->phase)) || (mode==0 && phase==p->phase && Sign(phase)==Sign(p->phase));
        if(match) accepted|=CallGameVirtual<uint8_t,5>(p->child,context,0);
    }
    if(accepted) return 1;
    Entry entry{}; Create(&entry,&phase);
    auto* child=entry.value;
    if(!CallGameVirtual<uint8_t,5>(child,context,0)) { Release(child); return 0; }
    auto* pos=sentinel->next;
    while(pos!=sentinel && !(pos->phase>entry.phase && Sign(pos->phase)>=Sign(entry.phase))) pos=pos->next;
    auto** freeList=reinterpret_cast<Node**>(&sentinel->child);
    Node* node=*freeList; if(node) *freeList=node->next;
    if(reinterpret_cast<uintptr_t>(node)!=uintptr_t(0)-2*sizeof(void*)) { node->child=child; Add(child); node->phase=entry.phase; }
    node->next=pos; node->previous=pos->previous; pos->previous->next=node; pos->previous=node;
    Release(child); return 1;
}
uintptr_t __fastcall Insert(Range* self,void*,unsigned char* pos,unsigned char* value) {
    if(self->end!=self->capacity) {
        auto* source=value;
        if(value>=pos && value<self->end) source+=40;
        if(self->end) CallGameMethod<void,0x0083b480>(self->end,self->end-40);
        CallGameFunction<void,0x0083bcf0>(pos,self->end-40,self->end);
        auto ret=CallGameMethod<uintptr_t,0x0083ba80>(pos,source); self->end+=40; return ret;
    }
    int32_t count=static_cast<int32_t>(reinterpret_cast<uintptr_t>(self->end)-reinterpret_cast<uintptr_t>(self->begin))/40;
    uint32_t capacity=count?uint32_t(count)*2:1;
    auto* data=capacity?static_cast<unsigned char*>(Allocate(capacity*40,self->descriptor,231)):nullptr;
    auto* cursor=pos;
    CallGameFunction<void,0x0083b680>(&cursor,self->begin,pos,data,pos);
    auto* middle=cursor;
    if(middle) CallGameMethod<void,0x0083b480>(middle,value);
    CallGameFunction<void,0x0083b680>(&cursor,pos,self->end,middle+40,middle);
    CallGameMethod<void,0x0083ba40>(self,self->begin,self->end);
    if(self->begin) CallGameFunction<void,0x004e3f70>(self->begin);
    self->begin=data; self->end=cursor; self->capacity=data+40*capacity; return capacity;
}
uintptr_t __fastcall Assign(Range* self,void*,unsigned char* first,unsigned char* last,uintptr_t) {
    // EA forwards last as the copy helper's allocator argument, ignoring the caller's allocator.
    uint32_t n=static_cast<int32_t>(reinterpret_cast<uintptr_t>(last)-reinterpret_cast<uintptr_t>(first))/20;
    uint32_t capacity=static_cast<int32_t>(reinterpret_cast<uintptr_t>(self->capacity)-reinterpret_cast<uintptr_t>(self->begin))/20;
    if(n>capacity) {
        auto* end=last;
        CallGameFunction<void,0x00912a90>(&end,first,last,static_cast<void*>(nullptr),reinterpret_cast<uintptr_t>(last));
        CallGameMethod<void,0x00912b60>(self,self->begin,self->end);
        self->begin=nullptr; self->end=self->capacity=reinterpret_cast<unsigned char*>(uintptr_t(n*20)); return n*20;
    }
    uint32_t size=static_cast<int32_t>(self->end-self->begin)/20;
    if(n>size) {
        auto* split=first+size*20;
        CallGameFunction<void,0x0083bdc0>(first,split,self->begin);
        auto* end=last;
        auto ret=CallGameFunction<uintptr_t,0x00912a90>(&end,split,last,self->end,reinterpret_cast<uintptr_t>(last));
        self->end=end; return ret;
    }
    auto* end=CallGameFunction<unsigned char*,0x0083bdc0>(first,last,self->begin);
    auto ret=CallGameMethod<uintptr_t,0x00912b60>(self,end,self->end); self->end=end; return ret;
}
uintptr_t __fastcall Destroy(Owner* self,void*) {
    CallGameMethod<void,0x0083ba40>(&self->records,self->records.begin,self->records.end);
    if(self->records.begin) CallGameFunction<void,0x004e3f70>(self->records.begin);
    auto ret=CallGameMethod<uintptr_t,0x0083dde0>(self,self->data,self->mode);
    self->cleared=0;
    if(self->mode!=0 && self->mode!=1) return CallGameFunction<uintptr_t,0x004e3f70>(self->data);
    return ret;
}
}
