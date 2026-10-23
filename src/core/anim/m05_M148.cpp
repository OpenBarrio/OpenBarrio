#include "ts3/anim/m05_M148.h"
#include <initializer_list>
#include <xmmintrin.h>
#include <intrin.h>
namespace ts3::anim::m148 {
namespace {
using V=__m128;
unsigned char* At(void* p,size_t n) {return static_cast<unsigned char*>(p)+n;}
template<class T> T& Field(void* p,size_t n) {return *reinterpret_cast<T*>(At(p,n));}
#if defined(_M_IX86)
void Table(void* p,size_t n,uintptr_t address) {Field<void*>(p,n)=reinterpret_cast<void*>(address);}
float Constant(uintptr_t address) {return *reinterpret_cast<const float*>(address);}
V Splat(float x) {return _mm_set1_ps(x);}
V Squared(V x) {
    V a=_mm_mul_ps(x,x);
    V sum=_mm_add_ps(_mm_add_ps(_mm_shuffle_ps(a,a,2),_mm_shuffle_ps(a,a,1)),a);
    return _mm_shuffle_ps(sum,sum,0);
}
void* Slave(void* host,int i) {return CallGameMethod<void*,0x00839ca0>(host,i);}
void Notify(void* host,int state,bool finish) {CallGameVirtual<void,20>(host,state,0,0);if(finish) CallGameVirtual<void,17>(host);}
bool HasFlag(void* route) {void* p=CallGameVirtual<void*,3>(route,0,0x2000,0);return CallGameMethod<uint8_t,0x0081c880>(p)!=0;}
#endif
}
void* Construct(void* self,void* host,float speed) {
#if defined(_M_IX86)
    CallGameMethod<void,0x008397e0>(self,host);
    Table(self,0,0x0102770c);Table(self,4,0x01027708);
    CallGameMethod<void,0x0085a7f0>(At(self,0x20));
    CallGameMethod<void,0x0085a900>(At(self,0x40));
    Table(self,0x64,0x0102700c);Field<float>(self,0x70)=Constant(0x010276f0);
    CallGameMethod<void,0x008399e0>(self,At(self,0x20));
    CallGameMethod<void,0x008399e0>(self,At(self,0x40));
    CallGameMethod<void,0x008399e0>(self,At(self,0x64));
    Field<float>(self,0x90)=speed;
    Field<V>(self,0x80)=_mm_setzero_ps();
    Field<float>(self,0x70)=Constant(0x0107a538);
#else
    (void)host;(void)speed;NotRewritten("m148 constructor game dependencies");
#endif
    return self;
}
void* Delete(void* self,uint32_t flags) {
    CallGameMethod<void,0x00858430>(self);
    if(flags&1) CallGameFunction<void,0x004e3f70>(self);
    return self;
}
void DestructVehicle(void* self) {
#if defined(_M_IX86)
    Table(self,0,0x010277d8);Table(self,4,0x010277d4);Table(self,0x18,0x010277c4);
    if(auto p=Field<void*>(self,0xc8)) {
        CallGameVirtual<void,30>(Field<void*>(self,0x14),p,0);
        Field<void*>(self,0xc8)=nullptr;
    }
    CallGameMethod<void,0x00839a10>(self,At(self,0x40));
    CallGameMethod<void,0x00839a10>(self,At(self,0x60));
    for(size_t offset:{size_t(0x94),size_t(0x84)}) {
        auto begin=Field<void*>(self,offset);
        auto end=Field<void*>(self,offset+8);
        int32_t bytes=static_cast<int32_t>(reinterpret_cast<uintptr_t>(end)-reinterpret_cast<uintptr_t>(begin)) & ~1;
        if(bytes>2 && begin) CallGameFunction<void,0x004e3f70>(begin);
    }
    CallGameMethod<void,0x0085a7d0>(At(self,0x60));
    CallGameMethod<void,0x0085a7d0>(At(self,0x40));
    CallGameMethod<void,0x00839750>(self);
#else
    (void)self;NotRewritten("m148 vehicle game dependencies");
#endif
}
void* DeleteSecondary(void* self,uint32_t flags) {
    // The x86 secondary vptr is one pointer after the primary vptr.
    return CallGameMethod<void*,0x0085a260>(At(self,0)-sizeof(void*),flags);
}
void* QuerySecondary(void* self,uint32_t type) {
#if defined(_M_IX86)
    return CallGameMethod<void*,0x00855800>(At(self,0)-0x18,type);
#else
    (void)self;(void)type;NotRewritten("m148 query game dependency");
#endif
}
void Update(void* self,float dt) {
#if defined(_M_IX86)
    auto host=Field<void*>(self,0x14);
    if(Field<int32_t>(host,4)>0) return;
    auto route=CallGameMethod<void*,0x00c93970>(host);
    if(!route) return;
    auto refs=reinterpret_cast<volatile long*>(At(route,4));
    _InterlockedExchangeAdd(refs,1);
    bool group=Field<uint8_t>(host,0x20)!=0;
    void* target=host;
    if(group) {
        _InterlockedExchangeAdd(refs,1);
        target=CallGameMethod<void*,0x0083a1b0>(host,route);
    }
    auto direction=CallGameMethod<V*,0x00580430>(At(self,0x20));
    float margin=0;
    float distance=static_cast<float>(CallGameVirtual<double,75>(route));
    V goal;CallGameVirtual<void,77>(route,&goal);
    auto entity=CallGameVirtual<void*,1>(host,0x14cfd2f8);
    auto id=CallGameVirtual<uint64_t,4>(host);
    float remaining=static_cast<float>(CallGameVirtual<double,41>(route,id));
    if(entity && HasFlag(route)) {
        auto kind=CallGameMethod<uint32_t,0x00953690>(entity);
        double radius=CallGameFunction<double,0x008484b0>(kind);
        margin=static_cast<float>(radius);
        float d=distance-margin;
        if(radius>0 && d>0) {
            distance=d;V scratch;
            goal=*CallGameVirtual<V*,71>(route,&scratch,d,0,0);
        }
    }
    auto broadcast=[&](int state,bool finish) {
        Notify(host,state,finish);
        for(int i=0;i<Field<int32_t>(host,0xc);++i)
            if(auto slave=Slave(host,i)) Notify(slave,state,finish);
    };
    if(CallGameMethod<uint8_t,0x0085a8f0>(At(self,0x40))) {
        if(Field<uint8_t>(self,0x61)) broadcast(2,false);
        else {
            if(CallGameVirtual<uint8_t,79>(route)) {
                V scratch;auto rotation=CallGameVirtual<V*,80>(route,&scratch);
                CallGameVirtual<void,12>(target,&goal,rotation);
            } else CallGameVirtual<void,8>(target,&goal);
            broadcast(3,true);
        }
        CallGameMethod<void,0x0083a650>(host,nullptr,0,0);
    } else if(remaining+margin>=distance && CallGameVirtual<uint8_t,39>(route)) {
        broadcast(3,true);
    } else if(!(_mm_cvtss_f32(Squared(*direction))>0)) {
        CallGameMethod<void,0x0085a8c0>(At(self,0x40),1,0);
    } else {
        float speed=Field<float>(self,0x90);
        if(CallGameMethod<uint8_t,0x0085a8f0>(At(self,0x40))) speed=1;
        V squared=Squared(*direction), inv=_mm_rsqrt_ps(squared);
        for(int n=0;n<2;++n) inv=_mm_add_ps(_mm_mul_ps(_mm_sub_ps(Splat(1),_mm_mul_ps(_mm_mul_ps(inv,inv),squared)),_mm_mul_ps(inv,Splat(.5f))),inv);
        V unit=_mm_mul_ps(inv,*direction),position;
        CallGameVirtual<void,7>(target,&position);
        V delta=_mm_mul_ps(_mm_mul_ps(Splat(speed),unit),Splat(dt));
        float length=_mm_cvtss_f32(_mm_sqrt_ps(Squared(delta)));
        float limit=static_cast<float>(CallGameMethod<double,0x009492f0>(At(self,0x40)));
        if(length>=limit) delta=_mm_sub_ps(goal,position);
        void* current=host;int index=-1;
        do {
            V pos,rotation;CallGameVirtual<void,7>(current,&pos);CallGameVirtual<void,9>(current,&rotation);
            Field<V>(self,0x80)=!(dt>0)?_mm_setzero_ps():_mm_mul_ps(Splat(1.0f/dt),delta);
            V next=_mm_add_ps(pos,delta);
            if(CallGameVirtual<int,6>(current)==1) {alignas(16) float lanes[4];_mm_store_ps(lanes,delta);lanes[1]=0;delta=_mm_load_ps(lanes);}
            if(!group || current==host) {
                if(CallGameFunction<uint8_t,0x00858610>(&delta,&rotation)) CallGameVirtual<void,12>(current,&next,&rotation);
                else CallGameVirtual<void,8>(current,&next);
            } else {
                V masterPos,masterRot;CallGameVirtual<void,7>(host,&masterPos);CallGameVirtual<void,9>(host,&masterRot);
                if(CallGameVirtual<int,6>(current)==2) {
                    V p=_mm_add_ps(_mm_mul_ps(Splat(.76f),unit),masterPos);CallGameVirtual<void,12>(current,&p,&masterRot);
                } else if(CallGameVirtual<int,6>(current)==1) {
                    auto e=CallGameVirtual<void*,1>(host,0x14cfd2f8);
                    if(e) {
                        margin=0;limit=0;
                        // The cdecl radius argument is still on the stack at FSTP:
                        // [esp+14h] is the margin slot, not the slave index.
                        if(HasFlag(route)) {auto k=CallGameMethod<uint32_t,0x00953660>(e,index);margin=static_cast<float>(CallGameFunction<double,0x008484b0>(k));}
                        float offset;
                        if(CallGameMethod<uint8_t,0x00953620>(e,index,1024)) offset=margin+Constant(0x01166370);
                        else if(CallGameMethod<uint8_t,0x00953620>(e,index,1280)) offset=margin+Constant((CallGameMethod<uint32_t,0x00953690>(e)&0x7f)==4?0x01166378:0x01166374);
                        else if(CallGameMethod<uint8_t,0x00953620>(e,index,512)) offset=-0.0f-(margin+Constant(0x0116637c));
                        else if((CallGameMethod<uint32_t,0x00953690>(e)&0xcf00)==0xe00) offset=-0.0f-(margin+Constant(0x01166384));
                        else offset=0;
                        V p=_mm_add_ps(_mm_mul_ps(Splat(offset),unit),masterPos);CallGameVirtual<void,12>(current,&p,&masterRot);
                    }
                }
            }
            CallGameVirtual<void,19>(current);
            if(index+1>=Field<int32_t>(host,0xc) || !group) break;
            current=Slave(host,++index);
        } while(current);
    }
    if(!_InterlockedDecrement(refs)) {_InterlockedExchange(refs,1);CallGameVirtual<void*,0>(route,1);}
#else
    (void)self;(void)dt;NotRewritten("m148 update game dependencies");
#endif
}
#if defined(_M_IX86)
namespace hooks {
void* __fastcall Construct(void* s,void*,void* h,float v){return m148::Construct(s,h,v);}
void* __fastcall Delete(void* s,void*,uint32_t f){return m148::Delete(s,f);}
void __fastcall Update(void* s,void*,float dt){m148::Update(s,dt);}
void __fastcall DestructVehicle(void* s,void*){m148::DestructVehicle(s);}
void* __fastcall DeleteSecondary(void* s,void*,uint32_t f){return m148::DeleteSecondary(s,f);}
void* __fastcall QuerySecondary(void* s,void*,uint32_t t){return m148::QuerySecondary(s,t);}
}
#endif
}
