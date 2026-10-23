#include "ts3/anim/m05_B26.h"
#include "ts3/layout.h"
#include <immintrin.h>
#include <cmath>
#include <cstring>
namespace ts3::anim::b26 {
uintptr_t __fastcall InsertPriority(PriorityList* s, void*, Priority* p) {
    if (!p) return 0;
    auto at=s->begin;
    while(at!=s->end && (!*at || !((*at)->priority>p->priority))) ++at;
    return CallGameMethod<uintptr_t,0x00AD3080>(s,at,&p);
}
void __fastcall SetIdentifier(Task* s,void*,const Identifier* p) {
    if(s->flags&1) return;
    s->target->identifierLow=0; s->target->identifierHigh=0;
    if(p) {s->target->identifierLow=p->low;s->target->identifierHigh=p->high;}
}
void __fastcall SetWord(Task* s,void*,uint32_t v) { if(!(s->flags&1)) std::memcpy(s->target->opaque28,&v,4); }
void __fastcall SetAngle2C(Task* s,void*,float v) {if(!(s->flags&1)) s->target->angle2C=v;}
void __fastcall SetAngle30(Task* s,void*,float v) {if(!(s->flags&1)) s->target->angle30=v;}
void __fastcall SetBit0(Task* s,void*,uint32_t v) {if(!(s->flags&1)) {if(uint8_t(v)) s->target->mode|=1;else s->target->mode&=~1u;}}
void __fastcall SetBit1(Task* s,void*,uint32_t v) {if(!(s->flags&1)) {if(uint8_t(v)) s->target->mode|=2;else s->target->mode&=~2u;}}
static __m128 Load(const Float4Value& v) {return _mm_load_ps(v.lane);}
static __m128 Sum(__m128 v,const Matrix44Value& m) {
    return _mm_add_ps(_mm_add_ps(_mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(v,v,0),Load(m.vector[0])),_mm_mul_ps(_mm_shuffle_ps(v,v,85),Load(m.vector[1]))),_mm_mul_ps(_mm_shuffle_ps(v,v,170),Load(m.vector[2]))),_mm_mul_ps(_mm_shuffle_ps(v,v,255),Load(m.vector[3])));
}
static void* Interface(void* object,uintptr_t global) {
#if defined(_M_IX86)
    return CallGameMethod<void*,0x00935770>(object,*reinterpret_cast<void**>(global));
#else
    (void)object;(void)global;AbortNotRewritten("look-at interface global");return nullptr;
#endif
}
Float4Value* __fastcall Position(void* s,void*,Float4Value* out) {
    void* owner=CallGameVirtual<void*,0>(s);
    void* transform=owner?Interface(owner,0x011E9158):nullptr;
    if(!transform) {*out=*CallGameVirtual<Float4Value*,2>(s);return out;}
    Matrix44Value world,local;
    CallGameMethod<void,0x00959060>(transform,&world);
    if(CallGameVirtual<uint32_t,1>(s)) {
        void* provider=Interface(owner,0x011E9114);
        if(provider) {
            void* object=CallGameMethod<void*,0x00951F00>(provider,CallGameVirtual<uint32_t,1>(s));
            if(object && CallGameVirtual<int,3>(object,1)) {
                CallGameVirtual<void,5>(object,&local);
                CallGameVirtual<void,8>(object,&world,&local,0);
                Matrix44Value composed;
                for(int i=0;i<4;++i) _mm_store_ps(composed.vector[i].lane,Sum(Load(local.vector[i]),world));
                world=composed;
            }
        }
    }
    auto v=Load(*CallGameVirtual<Float4Value*,2>(s));
    auto r=_mm_add_ps(_mm_add_ps(_mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(v,v,0),Load(world.vector[0])),Load(world.vector[3])),_mm_mul_ps(_mm_shuffle_ps(v,v,85),Load(world.vector[1]))),_mm_mul_ps(_mm_shuffle_ps(v,v,170),Load(world.vector[2])));
    _mm_store_ps(out->lane,r);return out;
}
void __fastcall Notify(Task* s,void*,int32_t event) {
    if(!s->callback) return;
    void* service=CallGameFunction<void*,0x007692B0>();
    if(!service) return;
#if defined(_M_IX86)
    alignas(16) unsigned char argument[32],result[32];
    *reinterpret_cast<void**>(argument)=reinterpret_cast<void*>(0x00FB96B4);
    std::memset(argument+8,0,4);
    CallGameMethod<void,0x004F5CE0>(argument,&event);
    CallGameVirtual<void,77>(service,result,s->callback,argument,1);
    *reinterpret_cast<void**>(result)=reinterpret_cast<void*>(0x00FB96B4);
    if(result[8]&4) CallGameMethod<void,0x004F55A0>(result,0);
    // MSVC's array destructor helper is stdcall, unlike ordinary engine calls.
    using Destroy=void(__stdcall*)(void*,uint32_t,uint32_t,void*);
    reinterpret_cast<Destroy>(0x00446710)(argument,32,1,reinterpret_cast<void*>(0x004F5660));
#else
    AbortNotRewritten("script value bridge");
#endif
}
static Task* Node(ListLink* link) {return link?reinterpret_cast<Task*>(reinterpret_cast<unsigned char*>(link)-offsetof(Task,link)):nullptr;}
void* __fastcall Rebase(Manager* s,void*) {
    for(auto p=s->list.next;p!=&s->list;p=p->next) Node(p)->start=Node(p)->start-s->time;
    s->time=0;return reinterpret_cast<unsigned char*>(s)-offsetof(Task,link);
}
uintptr_t __fastcall Select(Manager* s,void*) {
    auto temporary=CallGameFunction<Task**,0x004E3F90>(uint32_t(10*sizeof(Task*)),"Animation/LookAtTaskPtr",0,0,0,0);
    unsigned count=0;uint32_t mask=0;
    for(auto p=s->list.next;p!=&s->list;p=p->next) {
        auto t=Node(p);
        if((mask&t->mask)==t->mask) continue;
        if(t->state<4) mask|=t->mask;
        t->flags|=0x10;temporary[count++]=t;
        if(count==10) break;
    }
    // EA uses OR with 1/2/4 as a predicate: all nonnull entries pass the first pass.
    for(unsigned i=0;i<count;++i) s->selected[i]=temporary[i];
    auto result=CallGameFunction<uintptr_t,0x004E3F70>(temporary);
    if(count<10) {s->selected[count]=nullptr;return reinterpret_cast<uintptr_t>(s);}
    return result;
}
void __fastcall Begin(Task* s,void*,float time) {
    Float4Value position;Position(s->target,nullptr,&position);
    auto registry=CallGameFunction<void*,0x00938B50>();
    auto source=registry?CallGameMethod<void*,0x00939560>(registry,s->sourceLow,s->sourceHigh):nullptr;
    auto transform=source?Interface(source,0x011E9158):nullptr;
    if(!transform) {s->flags|=2;return;}
    Matrix44Value world;CallGameMethod<void,0x00959060>(transform,&world);
    auto delta=_mm_sub_ps(Load(position),Load(world.vector[3]));auto sq=_mm_mul_ps(delta,delta);
    auto n=_mm_add_ps(_mm_add_ps(_mm_shuffle_ps(sq,sq,2),_mm_shuffle_ps(sq,sq,1)),sq);n=_mm_shuffle_ps(n,n,0);
    auto inverse=_mm_rsqrt_ps(n);const auto one=_mm_set1_ps(1),half=_mm_set1_ps(.5f);
    for(int i=0;i<2;++i) inverse=_mm_add_ps(_mm_mul_ps(_mm_sub_ps(one,_mm_mul_ps(_mm_mul_ps(inverse,inverse),n)),_mm_mul_ps(half,inverse)),inverse);
    auto zero=_mm_setzero_ps();auto equal=_mm_cmpeq_ps(zero,n);
    auto length=_mm_or_ps(_mm_andnot_ps(equal,_mm_mul_ps(inverse,n)),_mm_and_ps(equal,zero));
    if(_mm_cvtss_f32(length)==0) {s->flags|=2;return;}
    auto dot=_mm_mul_ps(_mm_mul_ps(inverse,delta),Load(world.vector[2]));
    auto sum=_mm_add_ps(_mm_add_ps(_mm_shuffle_ps(dot,dot,2),_mm_shuffle_ps(dot,dot,1)),dot);
    // MAXSS/MINSS return their second operand on unordered comparisons.
    auto clamp=_mm_min_ss(_mm_max_ss(sum,_mm_set_ss(-1)),_mm_set_ss(1));
    float angle=static_cast<float>(std::acos(static_cast<double>(_mm_cvtss_f32(clamp))));
    double speed=CallGameVirtual<double,3>(s->target);
    s->flags|=8;s->progress=0;s->start=time;s->state=2;s->duration=static_cast<float>(angle/speed);
    Notify(s,nullptr,0);
}
void __fastcall Hold(Task* s,void*,float time,float start) {
    uint32_t max=0x7f7fffff;std::memcpy(&s->duration,&max,4);
    s->state=3;s->start=start;s->progress=time-start;Notify(s,nullptr,1);
}
void __fastcall Advance(Task* s,void*,float time) {
    switch(s->state) {
    case 1: Begin(s,nullptr,time);break;
    case 2:
        if(!(s->progress>1)) {float start=s->start;float duration=s->duration;float v=(time-start)/duration;s->progress=v;if(v>1) Hold(s,nullptr,time,start+duration);}
        else Hold(s,nullptr,time,time-((1-s->progress)*s->duration));
        break;
    case 3:s->progress=time-s->start;break;
    case 4:
        if(!(s->start<0)) {
            if(!(s->progress>=1)) {float v=(time-s->start)/s->duration;s->progress=v;if(v>1) s->progress=1;}
            else {s->progress=1;if(!(s->flags&2)) {s->flags|=2;s->state=5;Notify(s,nullptr,3);}}
        } else {s->start=time;Notify(s,nullptr,2);}break;
    }
}
void __fastcall Update(Manager* s,void*,float dt) {
    s->time=s->time+dt;if(s->time>=3600) Rebase(s,nullptr);
    if(s->list.next==&s->list) {s->selected[0]=nullptr;return;}
    for(auto p=s->list.next;p!=&s->list;) {
        Task* t=Node(p);p=p->next;
        if(!(t->flags&2)) {t->flags&=~0x10u;continue;}
        t->link.previous->next=t->link.next;t->link.next->previous=t->link.previous;
        int32_t before=t->refs;t->refs=static_cast<int32_t>(uint32_t(before)-1);
        if(before==1) {t->refs=1;CallGameVirtual<void,0>(t,1);}
    }
    Select(s,nullptr);
    for(auto p=s->list.next;p!=&s->list;p=p->next) {auto t=Node(p);if(t->flags&0x18) Advance(t,nullptr,s->time);}
}
}
