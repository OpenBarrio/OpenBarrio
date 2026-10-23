#include "ts3/anim/m05_M141.h"
#include <cstring>
#include <cmath>
#include <emmintrin.h>
#include <intrin.h>
namespace ts3::anim::m141 {
MotionRecord* CopyRecord(MotionRecord* dst,const MotionRecord* src) {
    // Separate loads/stores, not memcpy: the original also permits self assignment.
    for(unsigned i=0;i<10;++i) dst->words[i]=src->words[i];
    for(unsigned i=0;i<4;++i) dst->vectors[i]=src->vectors[i];
    for(unsigned i=0;i<4;++i) dst->tail[i]=src->tail[i];
    return dst;
}
namespace {
float Float(uint32_t b) { float f; std::memcpy(&f,&b,4); return f; }
void Scalar(void* target,float a,float b,float c,float d) {
    CallGameVirtual<void,10>(target,a); CallGameVirtual<void,14>(target,b);
    CallGameVirtual<void,12>(target,c); CallGameVirtual<void,16>(target,d);
}
void Rotation(void* target,uint32_t angle) {
    __m128 half=_mm_div_ps(_mm_set1_ps(Float(angle)),_mm_set1_ps(2.0f));
    float arg=_mm_cvtss_f32(half),s,c;
#if defined(_M_IX86)
    __asm {
        fld arg
        fsin
        fstp s
        fld arg
        fcos
        fstp c
    }
#else
    s=static_cast<float>(std::sin(static_cast<double>(arg)));
    c=static_cast<float>(std::cos(static_cast<double>(arg)));
#endif
    // Multiplications preserve the sign of the zero lanes for negative angles.
    __m128 axis=_mm_castsi128_ps(_mm_set_epi32(0,0,0x3f800000,0));
    __m128 x=_mm_mul_ps(_mm_shuffle_ps(axis,axis,0),_mm_set1_ps(s));
    __m128 y=_mm_mul_ps(_mm_shuffle_ps(axis,axis,0x55),_mm_set1_ps(s));
    __m128 z=_mm_mul_ps(_mm_shuffle_ps(axis,axis,0xaa),_mm_set1_ps(s));
    __m128 q=_mm_shuffle_ps(_mm_shuffle_ps(x,y,0x10),_mm_shuffle_ps(z,_mm_set1_ps(c),0x32),0x88);
    CallGameVirtual<void,7>(target,&q);
}
}
uint8_t RefreshMotion(MotionState* self) {
    if(!self->host) return 1;
    CallGameVirtual<void,10>(self->host);
    auto& r=self->record;
    if(self->mode==8) {
        MotionRecord old;
        CopyRecord(&old,&r);
        r.words[6]=old.words[7]; r.words[7]=old.words[6];
        r.words[4]=(r.words[4]!=1);
        r.vectors[0]=old.vectors[2]; r.vectors[1]=old.vectors[3];
        r.vectors[2]=old.vectors[0]; r.vectors[3]=old.vectors[1];
        void* service=nullptr;
#if defined(_M_IX86)
        service=*reinterpret_cast<void**>(0x011d9140);
#endif
        if(CallGameMethod<uint8_t,0x00844be0>(service,r.tail[0],r.tail[1],r.tail[2],r.tail[3],r.words[4],reinterpret_cast<unsigned char*>(self)+0x1c)==1) {
            void* target=CallGameVirtual<void*,1>(self->host,1419465867);
            if(target) { Scalar(target,0,0,0,1); Rotation(target,0x40490fdb); }
            int32_t n=static_cast<int32_t>(r.words[9]-uint32_t(self->remaining)-1);
            self->remaining=n<0?0:n;
            CallGameVirtual<void,10>(self->host);
        }
        return 1;
    }
    void* target=CallGameVirtual<void*,1>(self->host,1419465867);
    if(!target) return 1;
    Scalar(target,0,0,1,1);
    bool active=CallGameFunction<uint8_t,0x0055da30>(self->controller)!=0;
    uint32_t side=r.words[4];
    bool outer=side==0 || side==2 || side==3;
    if(active || !outer) {
        uint32_t z=0x3e800000, y=1044348142;
        if(outer) {
            z=self->mode==1?1058591998:1048576000;
            y=0xbe3f7cee; // -0.18700001 (subtraction, not a sign-bit rewrite below)
            float fy=-0.0f-Float(1044348142); std::memcpy(&y,&fy,4);
            if(self->mode==4) {z=0; float f=-0.0f-0.0f;std::memcpy(&y,&f,4);}
        } else {
            if(self->mode==1) { z=active?1056461292:1040254501; y=0; }
            if(self->mode==4 || (!active && (self->mode==2 || self->mode==3))) {
                CallGameVirtual<void,8>(self->transform,&r.vectors[2]); z=0;y=0;
            }
        }
        __m128 out=_mm_castsi128_ps(_mm_set_epi32(0,z,y,0));
        CallGameVirtual<void,6>(target,&out);
        return 1;
    }
    float x=0, y=Float(1044348142), z=self->mode==1?Float(1058591998):0.25f;
    bool sampled=false;
    __m128 point=_mm_setzero_ps();
    if(self->mode==2 || self->mode==3 || self->mode==4) {
        __m128 sample;
        if(CallGameVirtual<uint8_t,18>(target,&sample)) {
            __m128 delta=_mm_sub_ps(r.vectors[2],r.vectors[0]);
            delta=_mm_and_ps(delta,_mm_castsi128_ps(_mm_set_epi32(-1,-1,0,-1)));
            __m128 q;
            CallGameFunction<void,0x00860460>(&delta,&q);
            auto square=_mm_mul_ps(q,q);
            auto sum=_mm_add_ps(_mm_shuffle_ps(square,square,14),square);
            sum=_mm_add_ps(_mm_shuffle_ps(sum,sum,1),sum);
            auto norm=_mm_shuffle_ps(sum,sum,0);
            auto rec=_mm_rcp_ps(norm),two=_mm_set1_ps(2.0f);
            rec=_mm_mul_ps(_mm_sub_ps(two,_mm_mul_ps(rec,norm)),rec);
            __m128 sign=_mm_castsi128_ps(_mm_set_epi32(0,int32_t(0x80000000),int32_t(0x80000000),int32_t(0x80000000)));
            auto inv=_mm_mul_ps(_mm_mul_ps(_mm_sub_ps(two,_mm_mul_ps(rec,norm)),rec),_mm_xor_ps(sign,q));
            auto relative=_mm_sub_ps(r.vectors[2],sample);
            auto a=_mm_shuffle_ps(inv,inv,18), b=_mm_shuffle_ps(inv,inv,9);
            auto cross=_mm_add_ps(_mm_sub_ps(_mm_mul_ps(_mm_shuffle_ps(relative,relative,18),b),_mm_mul_ps(_mm_shuffle_ps(relative,relative,9),a)),_mm_mul_ps(_mm_shuffle_ps(inv,inv,255),relative));
            point=_mm_add_ps(_mm_mul_ps(_mm_sub_ps(_mm_mul_ps(_mm_shuffle_ps(cross,cross,18),b),_mm_mul_ps(_mm_shuffle_ps(cross,cross,9),a)),two),relative);
            sampled=true;
        }
    }
    if((self->mode==2 || self->mode==3) && sampled) {
        x=_mm_cvtss_f32(point); z=_mm_cvtss_f32(_mm_shuffle_ps(point,point,0xaa));
        y=0.0f-_mm_cvtss_f32(_mm_shuffle_ps(point,point,0x55));
    }
    if(self->mode==4) {
        y=self->flag84?Float(1052736750):self->flag83?Float(1044348142):0;
        if(side==2 || side==3) {
            uint32_t offset=side==3?0xbe87ae14:0x3e87ae14;
            auto mixed=_mm_shuffle_ps(point,_mm_add_ps(_mm_set1_ps(Float(offset)),point),5);
            point=_mm_shuffle_ps(mixed,point,226);
            Rotation(target,side==3?0xbfc90fdb:0x3fc90fdb);
        }
        if(sampled) {
            x=_mm_cvtss_f32(point); z=_mm_cvtss_f32(_mm_shuffle_ps(point,point,0xaa));
            y=0.0f-_mm_cvtss_f32(_mm_shuffle_ps(point,point,0x55));
        } else z=0;
    }
    // EA duplicates X into W rather than supplying zero or one.
    __m128 out=_mm_set_ps(x,z,-0.0f-y,x);
    CallGameVirtual<void,6>(target,&out);
    return 1;
}

namespace {
Body* Parent(void* secondary) {
    return reinterpret_cast<Body*>(static_cast<unsigned char*>(secondary)-offsetof(Body,secondary));
}
void* Service() {
#if defined(_M_IX86)
    return *reinterpret_cast<void**>(0x011d9140);
#else
    return nullptr;
#endif
}
void Release(ControllerRef* ref,bool atomic=false) {
    if(!ref) return;
    int32_t count=atomic?_InterlockedDecrement(reinterpret_cast<volatile long*>(&ref->refs)):--ref->refs;
    if(!count) {
        if(atomic) _InterlockedExchange(reinterpret_cast<volatile long*>(&ref->refs),1);
        else ref->refs=1;
        CallGameVirtual<void,0>(ref,uint32_t(1));
    }
}
void Fade(Body* body) {
    if(body->actor) CallGameFunction<void,0x00848760>(body->actor,1.0f,uint32_t(body->style.flags[1]==0));
    if(body->slaveCount>0 && body->slaveStyles) {
        for(int32_t i=0;i<body->slaveCount;++i) {
            if(body->slaves[i]) CallGameFunction<void,0x00848760>(body->slaves[i],1.0f,uint32_t(body->slaveStyles[i].flags[1]==0));
        }
    }
}
void StopEffects(Body* body) {
    auto mode=body->style.values[3];
    if((mode==0x4100 || mode==0x4200 || mode==0x4300) && body->actor) CallGameVirtual<void,10>(body->actor);
    void* p=CallGameVirtual<void*,1>(body->host,uint32_t(349164280));
    if(p) {
        p=CallGameMethod<void*,0x00955ef0>(p);
        if(p) CallGameMethod<void,0x0085b430>(p);
    }
}
void Rebuild(Body* body) {CallGameMethod<void,0x00852520>(body);}
void Fallback(Body* body) {
    if(!CallGameMethod<uint8_t,0x00850b90>(body)) CallGameMethod<void,0x0084b0c0>(body,uint32_t(0));
}
void Transition(Body* body,uint32_t a,uint32_t b) {
    if(!CallGameMethod<uint8_t,0x00851710>(body,a,b)) Fallback(body);
}
}
uintptr_t Complete(void* secondary,ControllerRef* const* controller,int32_t event) {
    Body* body=Parent(secondary);
    if(body) ++body->refs;
    if(*controller) CallGameMethod<void,0x00566590>(reinterpret_cast<unsigned char*>(*controller)+3*sizeof(void*),body?secondary:nullptr);
    auto* sentinel=&body->controllers;
    for(auto* link=sentinel->next;link!=sentinel;link=link->next) {
        auto* node=reinterpret_cast<OwnedNode*>(link);
        if(node->controller==*controller) {
            link->previous->next=link->next;
            link->next->previous=link->previous;
            Release(node->controller);
            CallGameFunction<void,0x004e3f70>(node);
            break;
        }
    }
    bool dispatch=true;
    if(body->defer && body->slaveCount>0 && sentinel->next!=sentinel) {
        uint32_t n=0;
        for(auto* link=sentinel->next;link!=sentinel;link=link->next) ++n;
        dispatch=(n==0); // Retain the original wrapping counter behavior.
    }
    if(dispatch) {
        if(!CallGameMethod<void*,0x00c93970>(body->host)) CallGameMethod<void,0x0085a8c0>(body->embeddedF0,uint32_t(0),uint32_t(0));
        switch(event) {
        case 1:
            StopEffects(body);
            if(body->enabled) Transition(body,1,1);
            break;
        case 2:
        case 3: {
            if(event==3) ++body->completions;
            Rebuild(body);
            if(body->actor) {
                auto* p=CallGameVirtual<unsigned char*,1>(body->actor,uint32_t(1956407960));
                auto* flags=reinterpret_cast<uint32_t*>(p+44);
                if(*flags&0x80000) *flags&=~0x200000u;
            }
            break;
        }
        case 4: {
            Fade(body);
            if(body->slaveCount>0 && body->slaveStyles && body->handle) {
                if(body->master) CallGameVirtual<void,30>(body->master,body->handle,uint32_t(0));
                body->handle=0;
            }
            uint64_t key=CallGameVirtual<uint64_t,4>(body->master);
            auto* resource=CallGameMethod<ControllerRef*,0x00c93970>(body->host);
            if(resource) _InterlockedExchangeAdd(reinterpret_cast<volatile long*>(&resource->refs),1);
            float distance=static_cast<float>(CallGameFunction<double,0x00849810>(resource,key,body->style.values[1],body->style.values[3],uint32_t(1)));
            float bound=_mm_cvtss_f32(_mm_mul_ss(_mm_set_ss(body->multiplier),_mm_set_ss(body->rate)));
            uint32_t bits;std::memcpy(&bits,&body->rate,4);
            if(!(distance>bound) || Float(bits&0x7fffffff)<0x1p-16f) {
                if(!CallGameMethod<uint8_t,0x0084c590>(body)) {
                    uint32_t enable=0;
                    if(CallGameMethod<void*,0x00c93970>(body->host)) {
                        void* object=CallGameMethod<void*,0x00c93970>(body->host);
                        enable=CallGameVirtual<uint8_t,79>(object)==0;
                    }
                    Transition(body,enable,0);
                }
                break;
            }
            // The style is 0x7c bytes on x86, including twenty retained resource slots.
            m107::Style style;
            CallGameMethod<void,0x007513f0>(&style);
            CallGameMethod<void,0x0084bc00>(body,&style);
            CallGameVirtual<void,6>(body);
            if(style.values[0]!=body->style.values[0]) {
                CallGameMethod<void,0x007578b0>(&body->style,&style);
                auto* provider=body->style.resources[2];
                float weight;
                if(CallGameVirtual<int32_t,1>(provider)==1) weight=static_cast<float>(CallGameVirtual<double,2>(provider));
                else {
                    struct Request {void* actor;float maximum;uint32_t zeros[3];uint8_t a,b,padding[2];ControllerRef* held;};
                    struct Output {ControllerRef* ref;uint8_t flag,padding[3];};
                    Request request{body->actor,2000.0f,{0,0,0},0,0,{0,0},nullptr};
                    Output output{nullptr,0,{0,0,0}};
                    if(CallGameVirtual<uint8_t,3>(provider,&request,&output)) {
                        __m128 a,b;void* unused;
                        weight=static_cast<float>(CallGameFunction<double,0x008492b0>(output.ref,&a,&b,&unused));
                    } else weight=Float(0x7f7fffff);
                    Release(output.ref);Release(request.held,true);
                }
                body->selectedWeight=weight;
                void* service=Service();
                for(int32_t i=0;i<body->slaveCount;++i) {
                    m107::Style copy;
                    m107::CopyStyle(&copy,&body->slaveStyles[i]);
                    auto* range=body->ranges?&body->ranges[i]:nullptr;
                    if(range && (range->end-range->begin)/28) {
                        uint32_t tag;std::memcpy(&tag,range->begin+24,4);
                        if(CallGameMethod<uint8_t,0x00844980>(service,tag,copy.values[1],copy.values[2],copy.values[3],copy.values[4],&copy,uint32_t(1)))
                            CallGameMethod<void,0x007578b0>(&body->slaveStyles[i],&copy);
                    }
                    m107::ReleaseStyle(&copy);
                }
            }
            int32_t value=CallGameMethod<int32_t,0x0084af20>(body);
            CallGameMethod<void,0x00852700>(body,value);
            m107::ReleaseStyle(&style);
            break;
        }
        case 5: StopEffects(body);Transition(body,0,0);break;
        case 7: Fade(body);Fallback(body);break;
        case 8: CallGameMethod<void,0x0084b0c0>(body,uint32_t(0));break;
        case 9: ++body->nesting;body->flag251=0;Rebuild(body);break;
        case 10: --body->nesting;body->flag250=0;Rebuild(body);break;
        default: break;
        }
    }
    uintptr_t result=static_cast<uint32_t>(--body->refs);
    if(!body->refs) {
        body->refs=1;
        result=CallGameVirtual<uintptr_t,0>(&body->refVtable,uint32_t(1));
    }
    return result;
}
#if defined(_M_IX86)
uintptr_t __fastcall HookComplete(void* p,void*,ControllerRef* const* c,int32_t e) {return Complete(p,c,e);}
MotionRecord* __fastcall HookCopyRecord(MotionRecord* p,void*,const MotionRecord* q) {return CopyRecord(p,q);}
uint8_t __fastcall HookRefreshMotion(MotionState* p,void*) {return RefreshMotion(p);}
#endif
}
