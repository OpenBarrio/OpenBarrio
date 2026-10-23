#include "ts3/anim/m05_M22.h"
#include "ts3/anim/m05_B20.h"
#include <cstring>
#include <cmath>
#include <xmmintrin.h>
namespace ts3::anim::m22 {
static void Table(Evaluator* self,uintptr_t address) {
#if defined(_M_IX86)
    self->base.vtable=reinterpret_cast<void**>(address);
#else
    (void)address; self->base.vtable=nullptr;
#endif
}
Evaluator* ConstructScale(Evaluator* self,void* host) {
    b01::Construct(&self->base,host);
    self->keyLow=self->keyHigh=self->selector=0;
#if defined(_M_IX86)
    self->amount=*reinterpret_cast<float*>(0x0114f54c);
#else
    self->amount=1.f;
#endif
    Table(self,0x00fc18f0); return self;
}
Evaluator* DestroyScale(Evaluator* self,uint32_t flags) {
    Table(self,0x00fc18f0);
    b01::Reset(&self->base);
    if(flags&1) CallGameFunction<void,0x004e3f70>(self);
    return self;
}
Evaluator* ConstructPair(Evaluator* self,void* host) {
    b01::Construct(&self->base,host);
    self->keyLow=self->keyHigh=self->selector=self->extraLow=self->extraHigh=0;
    self->amount=0.f;
    Table(self,0x00fc1948); return self;
}
void* QueryPair(void* self,uint32_t id) {
    if(id==0x0bff764e || id==0x73deb8c8) return self;
    return nullptr;
}
uintptr_t SavePair(Evaluator* self,b01::Cursor* s) {
    uintptr_t eax=0;
    const void* fields[]={&self->amount,&self->selector,&self->keyLow,&self->extraLow};
    const unsigned sizes[]={4,4,8,8};
    for(unsigned i=0;i<4;++i) {
        unsigned char value[8]; std::memcpy(value,fields[i],sizes[i]);
        s->position=static_cast<int32_t>(static_cast<uint32_t>(s->position)+sizes[i]);
        if(s->position>=0) eax=CallGameMethod<uintptr_t,0x00594140>(s,sizes[i]);
        if(s->position<0) {
            std::memcpy(s->output+s->position-sizes[i]+1,value,sizes[i]);
            eax=reinterpret_cast<uintptr_t>(s->output);
        }
    }
    return eax;
}
uintptr_t EvaluateScale(Evaluator* self,Context* context,void* pose) {
    auto& sentinel=self->base.children;
    if(sentinel.previous!=&sentinel) {
        auto link=sentinel.next;
        auto child=link?reinterpret_cast<b01::Node*>(reinterpret_cast<unsigned char*>(link)-offsetof(b01::Node,link)):nullptr;
        CallGameVirtual<uintptr_t,2>(child,context,pose);
    }
    uintptr_t result=self->selector&0xcf00;
    if(result==0x100 || !(context->flags&2)) return result;
    auto buffer=reinterpret_cast<unsigned char*>(b20::Local(static_cast<b20::Pose*>(pose)));
    void* service=CallGameFunction<void*,0x005eb7c0>();
    float x,y,rate;
    if(service) {
        x=static_cast<float>(CallGameMethod<double,0x005ecce0>(service,self->selector));
        y=static_cast<float>(CallGameMethod<double,0x005ecd30>(service,self->selector));
        rate=static_cast<float>(CallGameMethod<double,0x005ecc90>(service,self->selector));
    } else { x=y=0.08f; rate=1.f; }
    float amplitude=self->amount*rate;
    float phase=context->phase;
    if(phase==0.f) {
        double random=CallGameMethod<double,0x004fa870>(reinterpret_cast<void*>(uintptr_t(0x011cd4f0)));
        phase=static_cast<float>(random*6.283185482025146484375);
    }
    float wave;
#if defined(_M_IX86)
    // Preserve x87 divide/multiply/subtract/FSIN rounding; amplitude is overwritten by this wave in EA's stack slot.
    __asm {
        fld dword ptr ds:[0x00fc18e4]
        fdiv amplitude
        fmul dword ptr ds:[0x011cdda8]
        fsub phase
        fsin
        fadd dword ptr ds:[0x00f9a5ac]
        fstp wave
    }
#else
    wave=static_cast<float>(std::sin(6.283185482025146484375/double(amplitude)-phase)+0.5);
#endif
    context->updated=1; context->savedPhase=phase;
    auto skeleton=static_cast<unsigned char*>(context->rig)+0x28;
#if defined(_M_IX86)
    auto key=*reinterpret_cast<uint32_t*>(0x0114f550);
#else
    uint32_t key=0;
#endif
    int32_t index=CallGameMethod<int32_t,0x00575e30>(skeleton,key);
    result=static_cast<uint32_t>(index);
    if(index>0) {
        __m128 value=_mm_set_ps(1.f, wave*y+1.f, wave*x+1.f,1.f);
        _mm_store_ps(reinterpret_cast<float*>(buffer+48u*static_cast<uint32_t>(index)+0x20),value);
        result=reinterpret_cast<uintptr_t>(buffer);
    }
    return result;
}
}
#if defined(_M_IX86)
namespace ts3::anim::m22::hooks {
Evaluator* __fastcall ConstructScale(Evaluator* s,void*,void* h) {return m22::ConstructScale(s,h);}
Evaluator* __fastcall DestroyScale(Evaluator* s,void*,uint32_t f) {return m22::DestroyScale(s,f);}
uintptr_t __fastcall EvaluateScale(Evaluator* s,void*,Context* c,void* p) {return m22::EvaluateScale(s,c,p);}
Evaluator* __fastcall ConstructPair(Evaluator* s,void*,void* h) {return m22::ConstructPair(s,h);}
void* __fastcall QueryPair(void* s,void*,uint32_t id) {return m22::QueryPair(s,id);}
uintptr_t __fastcall SavePair(Evaluator* s,void*,b01::Cursor* c) {return m22::SavePair(s,c);}
}
#endif
