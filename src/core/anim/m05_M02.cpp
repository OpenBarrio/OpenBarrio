#include "ts3/anim/m05_M02.h"
#include <xmmintrin.h>
namespace ts3::anim::m02 {
namespace {
template<unsigned A> void** Table() {
#if defined(_M_IX86)
    return reinterpret_cast<void**>(uintptr_t(A));
#else
    static void* table[11]{}; return table;
#endif
}
SrtValue* Storage(Pose* p) { return CallGameMethod<SrtValue*,0x00574190>(p); }
uint32_t Count(Context* c) { return CallGameMethod<uint32_t,0x00575650>(&c->rig->skeleton); }
__m128 Load(const SrtValue& s,unsigned n) {return _mm_load_ps(reinterpret_cast<const float*>(&s)+4*n);}
void Store(SrtValue& s,unsigned n,__m128 v) {_mm_store_ps(reinterpret_cast<float*>(&s)+4*n,v);}
}
bool SaveSource(Evaluator* p,Cursor* s) { return b01::WriteNode(p->source,s); }
bool LoadSource(Evaluator* p,Cursor* s) { return b01::ReadNode(p->base.host,&p->source,s); }
Evaluator* DeleteBlend(Evaluator* p,uint32_t flags) {
    p->base.vtable=Table<0x00FC10AC>();
    b01::Reset(&p->base);
    if(flags & 1u) CallGameFunction<void,0x004E3F70>(p);
    return p;
}
Evaluator* ConstructAdd(Evaluator* p,void* host) {
    b01::Construct(&p->base,host);
    p->base.vtable=Table<0x00FC1114>();
    return p;
}
Evaluator* DeleteAdd(Evaluator* p,uint32_t flags) {
    p->base.vtable=Table<0x00FC1114>();
    b01::Reset(&p->base);
    if(flags & 1u) CallGameFunction<void,0x004E3F70>(p);
    return p;
}
uintptr_t Evaluate(Evaluator* p,Context* c,Pose* output) {
    auto* sentinel=&p->base.children;
    if(sentinel->previous==sentinel) return reinterpret_cast<uintptr_t>(sentinel);
    auto* first=sentinel->next ? reinterpret_cast<Node*>(reinterpret_cast<unsigned char*>(sentinel->next)-offsetof(Node,link)):nullptr;
    CallGameVirtual<void,2>(first,c,output);
    Pose* temporary=CallGameFunction<Pose*,0x005748A0>(0x19b30,Table<0x00FC10D8>(),0,0,0,0);
    if(temporary) {
        auto* rig=c->rig;
        if(rig) CallGameVirtual<void,0>(rig);
        temporary=CallGameMethod<Pose*,0x00574F50>(temporary,rig);
        if(temporary) ++temporary->refs;
    }
    // EA does not recover from allocation failure: it still dispatches with null.
    CallGameVirtual<void,2>(p->source,c,temporary);
    float* weights=temporary->weights;
    uintptr_t result=Count(c);
    for(uint32_t i=0;i<result;) {
        float weight=first->effective;
        if(weights) weight*=weights[i];
        if(weight>0.0f) {
            SrtValue* source=Storage(temporary)+i;
            SrtValue* dest=Storage(output)+i;
            SrtValue old=*dest, aligned=old;
            alignas(16) SrtValue transformed;
            CallGameFunction<void,0x0054C8D0>(source,&aligned,&transformed);
            dest=Storage(output)+i;
            __m128 w=_mm_set1_ps(weight), inverse=_mm_set1_ps(1.0f-weight);
            Store(*dest,0,_mm_add_ps(_mm_mul_ps(w,Load(transformed,0)),_mm_mul_ps(inverse,Load(old,0))));
            Store(*dest,2,_mm_add_ps(_mm_mul_ps(w,Load(transformed,2)),_mm_mul_ps(inverse,Load(old,2))));
            __m128 q=_mm_add_ps(_mm_mul_ps(w,Load(aligned,1)),_mm_mul_ps(inverse,Load(old,1)));
            __m128 square=_mm_mul_ps(q,q);
            __m128 sum=_mm_add_ps(_mm_shuffle_ps(square,square,0x0e),square);
            sum=_mm_add_ps(_mm_shuffle_ps(sum,sum,1),sum);
            sum=_mm_shuffle_ps(sum,sum,0);
            __m128 r=_mm_rsqrt_ps(sum),one=_mm_set1_ps(1.0f),half=_mm_set1_ps(0.5f);
            r=_mm_add_ps(_mm_mul_ps(_mm_sub_ps(one,_mm_mul_ps(_mm_mul_ps(r,r),sum)),_mm_mul_ps(half,r)),r);
            r=_mm_add_ps(_mm_mul_ps(_mm_sub_ps(one,_mm_mul_ps(_mm_mul_ps(r,r),sum)),_mm_mul_ps(half,r)),r);
            Store(*dest,1,_mm_mul_ps(r,q));
        }
        ++i; result=Count(c);
        if(i>=result) break;
    }
    result=static_cast<uint32_t>(--temporary->refs);
    if(!result) {
        temporary->refs=1;
        result=CallGameVirtual<uintptr_t,0>(temporary,1);
    }
    return result;
}
}
#if defined(_M_IX86)
namespace ts3::anim::m02::hooks {
bool __fastcall SaveSource(Evaluator* p,void*,Cursor* s){return m02::SaveSource(p,s);}
bool __fastcall LoadSource(Evaluator* p,void*,Cursor* s){return m02::LoadSource(p,s);}
Evaluator* __fastcall DeleteBlend(Evaluator* p,void*,uint32_t f){return m02::DeleteBlend(p,f);}
uintptr_t __fastcall Evaluate(Evaluator* p,void*,Context* c,Pose* o){return m02::Evaluate(p,c,o);}
Evaluator* __fastcall ConstructAdd(Evaluator* p,void*,void* h){return m02::ConstructAdd(p,h);}
Evaluator* __fastcall DeleteAdd(Evaluator* p,void*,uint32_t f){return m02::DeleteAdd(p,f);}
}
#endif
