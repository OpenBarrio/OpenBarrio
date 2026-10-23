#include "ts3/anim/m05_M134.h"
#include "ts3/layout.h"
#include <cstring>
#include <intrin.h>
#include <cfloat>
#include <initializer_list>
namespace ts3::anim::m134 {
namespace {
// These are views of external game objects, not native serialized pointer blobs.
// Only the x86 bridge backend may interpret their observed byte offsets.
template<class T> T& Field(void* p, size_t offset) { return *reinterpret_cast<T*>(static_cast<unsigned char*>(p)+offset); }
#if defined(_M_IX86)
template<class T> T Global(uintptr_t address) { return *reinterpret_cast<T*>(address); }
uint32_t Bits(float f) { uint32_t b; std::memcpy(&b,&f,4); return b; }
float Float(uint32_t b) { float f; std::memcpy(&f,&b,4); return f; }
void Release(void* p,bool atomic) {
    if (!p) return;
    auto* refs=&Field<long>(p,4);
    long remaining=atomic ? _InterlockedDecrement(refs) : --*refs;
    if (!remaining) {
        if (atomic) _InterlockedExchange(refs,1); else *refs=1;
        CallGameVirtual<uintptr_t,0>(p,1);
    }
}
struct Request {
    float time,limit; uint32_t a,b,c; uint8_t d,e; uint16_t pad; void* resource;
};
static_assert(offsetof(Request,resource)==24);
struct Result { void* controller; uint8_t flag; uint8_t pad[3]; };
float Evaluate(void* evaluator,float time,float& duration,bool reverse=false) {
    if (CallGameVirtual<int,1>(evaluator)==1)
        return static_cast<float>(CallGameVirtual<double,2>(evaluator));
    Request request{time,2000.0f,0,0,0,0,0,0,nullptr};
    Result result{nullptr,0,{}};
    float value=FLT_MAX;
    if (CallGameVirtual<uint8_t,3>(evaluator,&request,&result)) {
        alignas(16) unsigned char a[16],b[16];
        value=CallGameFunction<float,0x008492B0>(result.controller,reverse?b:a,reverse?a:b,&duration);
    }
    Release(result.controller,false);
    Release(request.resource,true);
    return value;
}
void MergeDuration(void* body,float& duration) {
    float candidate=Field<float>(body,352)*duration;
    float old=Field<float>(body,360);
    // Not fmax: EA selects with the sign bit of the rounded subtraction.
    uint32_t mask=uint32_t(int32_t(Bits(candidate-old))>>31);
    Field<float>(body,360)=Float((mask&Bits(old))|(~mask&Bits(candidate)));
    duration=old;
}
int32_t Count(void* body) {
    auto begin=reinterpret_cast<uintptr_t>(Field<void*>(body,392));
    auto end=reinterpret_cast<uintptr_t>(Field<void*>(body,396));
    return int32_t(end-begin)/28;
}
void Clear(void* body) {
    CallGameMethod<void,0x00759410>(static_cast<unsigned char*>(body)+392,Field<void*>(body,392),Field<void*>(body,396));
    Field<void*>(body,396)=Field<void*>(body,392);
}
#endif
}
uintptr_t __fastcall Refresh(void* self, void*, uint32_t style, uint32_t flags, uint32_t arg, uint32_t extraFlags, uint8_t a, uint8_t b) {
#if defined(_M_IX86)
    auto* body=static_cast<unsigned char*>(self);
    void* manager=Global<void*>(0x011D9140);
    int32_t count=Count(body);
    bool retained=false;
    if (count) {
        if (Field<uint8_t>(body,383)) return static_cast<uintptr_t>(count);
        retained=true;
        Field<uint8_t>(body,386)=1;
        Field<uint8_t>(body,387)=1;
    }
    Field<float>(body,348)=Global<float>(0x0114F628);
    Field<float>(body,352)=Global<float>(0x0114F62C);
    if (CallGameMethod<uint8_t,0x00844EC0>(manager,style,flags,arg,extraFlags,body+40)) {
        Field<float>(body,360)=0;
        if (Field<uint8_t>(body,141)) {
            Field<uint32_t>(body,348)=981668463;
            Field<float>(body,352)=2;
        }
        Field<float>(body,304)=0;
        Field<float>(body,300)=0;
        Field<float>(body,308)=0;
        Field<float>(body,312)=0;
        for (size_t off:{size_t(316),size_t(320)}) {
            if (Field<void*>(body,off)) {
                CallGameFunction<void,0x004E3F70>(Field<void*>(body,off));
                Field<void*>(body,off)=nullptr;
            }
        }
        if (!retained) Clear(body);
        int selected=CallGameMethod<int,0x00844C10>(manager,Field<uint32_t>(body,120),extraFlags|flags,body+392);
        if (!retained) Field<uint32_t>(body,584)=0;
        Field<uint32_t>(body,588)=0;
        count=Count(body);
        if (count) {
            Field<int32_t>(body,588)=selected;
            uint32_t bytes=uint32_t(count)>0x3fffffffu ? 0xffffffffu : uint32_t(count)*4;
            Field<void*>(body,316)=CallGameFunction<void*,0x004E3F90>(bytes,"Locomotion/AnimatedSimBody/SpeedIntoDistances",0,0,0,0);
            Field<void*>(body,320)=CallGameFunction<void*,0x004E3F90>(bytes,"Locomotion/AnimatedSimBody/SlowOutOfDistances",0,0,0,0);
            alignas(16) unsigned char temporary[48];
            CallGameMethod<void,0x00757690>(temporary,body+40);
            float duration; // EA leaves this scratch untouched on scalar/failed evaluations.
            for (uint32_t i=0;i<uint32_t(Count(body));++i) {
                auto* record=static_cast<unsigned char*>(Field<void*>(body,392))+28*i;
                if (CallGameMethod<uint8_t,0x00844980>(manager,Field<uint32_t>(record,24),Field<uint32_t>(body,124),Field<uint32_t>(body,128),Field<uint32_t>(body,132),Field<uint32_t>(body,136),temporary,1)) {
                    for (unsigned side=0;side<2;++side) {
                        void* evaluator=Field<void*>(temporary,40+side*4);
                        auto* distances=static_cast<float*>(Field<void*>(body,316+side*4));
                        if (!evaluator || CallGameVirtual<uint8_t,4>(evaluator)) distances[i]=side?0.0f:-1.0f;
                        else {
                            distances[i]=Evaluate(evaluator,Field<float>(body,292),duration,side!=0);
                            if (i==Field<uint32_t>(body,588)) {
                                // This loop's duration scratch is NOT replaced by the old maximum.
                                float copy=duration; MergeDuration(body,copy);
                            }
                        }
                    }
                }
            }
            CallGameMethod<void,0x00754060>(temporary);
        } else Clear(body);
        alignas(16) unsigned char combined[124];
        std::memset(combined,0,80);
        Field<void*>(combined,108)=reinterpret_cast<void*>(0x011C7E54);
        Field<void*>(combined,112)=reinterpret_cast<void*>(0x011C7E54);
        Field<void*>(combined,116)=reinterpret_cast<void*>(0x011C7E56);
        Field<const char*>(combined,120)="Locomotion/Manager/AnimationStyleDefinition/mLoopSound";
        CallGameMethod<void,0x0084BC00>(body,combined);
        float duration; // shared scratch, also intentionally stale after scalar evaluation
        void* evaluators[]={Field<void*>(combined,4),Field<void*>(body,48),Field<void*>(combined,12)};
        const size_t targets[]={304,300,308};
        for (unsigned i=0;i<3;++i) if (evaluators[i]) {
            Field<float>(body,targets[i])=Evaluate(evaluators[i],Field<float>(body,292),duration);
            MergeDuration(body,duration);
        }
        if (Field<void*>(body,64)) {
            float ignored;
            float value=Evaluate(Field<void*>(body,64),Field<float>(body,292),ignored);
            Field<float>(body,312)=value==FLT_MAX?0.0f:value;
        }
        CallGameMethod<void,0x00754060>(combined);
    }
    CallGameMethod<void,0x007E4DD0>(body+240,0.0f);
    CallGameMethod<void,0x00626270>(body+240,0.0f);
    float distance=Field<float>(body,308);
    void* overrideEvaluator=Field<void*>(body,56);
    if (overrideEvaluator && !CallGameVirtual<uint8_t,4>(overrideEvaluator)) {
        float ignored;
        distance=Evaluate(Field<void*>(body,56),Field<float>(body,292),ignored);
    } else {
        uint32_t n=Field<uint32_t>(body,588);
        auto* slow=static_cast<float*>(Field<void*>(body,320));
        if (n && slow) do { distance=slow[n]+distance; } while (--n);
    }
    void* resource=CallGameMethod<void*,0x00C93970>(Field<void*>(body,20));
    if (resource) _InterlockedExchangeAdd(&Field<long>(resource,4),1);
    void* entity=CallGameVirtual<void*,3>(resource);
    float additional=0;
    if (CallGameMethod<uint8_t,0x0081C880>(entity,0,0x2000,0)) {
        void* tuning=CallGameFunction<void*,0x005EB7C0>();
        if (tuning) additional=CallGameMethod<float,0x005ECBA0>(tuning,extraFlags|flags);
    }
    Field<float>(body,288)=(distance*Global<float>(0x01166368)+additional)+Global<float>(0x01166364);
    uintptr_t result=reinterpret_cast<uintptr_t>(&Field<long>(resource,4));
    if (!_InterlockedDecrement(&Field<long>(resource,4))) {
        _InterlockedExchange(&Field<long>(resource,4),1);
        result=CallGameVirtual<uintptr_t,0>(resource,1);
    }
    Field<uint8_t>(body,594)=a;
    Field<uint8_t>(body,595)=b;
    Field<uint8_t>(body,596)=0;
    return result;
#else
    (void)self; (void)style; (void)flags; (void)arg; (void)extraFlags; (void)a; (void)b;
    NotRewritten("m05_M134 external animated-body x86 bridge");
#endif
}
}
