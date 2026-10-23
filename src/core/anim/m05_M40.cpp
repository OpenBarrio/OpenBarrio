#include "ts3/anim/m05_M40.h"
#include <xmmintrin.h>
namespace ts3::anim::m40 {
namespace {
float Global(uint32_t address) {
#if defined(_M_IX86)
    return *reinterpret_cast<const float*>(address);
#else
    (void)address; NotRewritten("m05_M40 game float global");
#endif
}
}
float Time(TransformController* p) {
    if(p->child) return CallGameVirtual<float,10>(p->child);
    return Global(0x0114f594);
}
bool Nonidentity(const TransformController* p) {
    auto sign=_mm_set1_ps(Global(0x00faa210));
    auto tolerance=_mm_set1_ps(Global(0x00fc1e58));
    auto t=_mm_load_ps(reinterpret_cast<const float*>(&p->translation));
    auto absolute=_mm_max_ps(_mm_mul_ps(t,sign),t);
    if((_mm_movemask_ps(_mm_cmple_ps(absolute,tolerance))&7)!=7) return true;
    auto identity=_mm_set_ps(Global(0x0107a538),0,0,0);
    auto delta=_mm_sub_ps(_mm_load_ps(reinterpret_cast<const float*>(&p->rotation)),identity);
    auto magnitude=_mm_max_ps(_mm_mul_ps(sign,delta),delta);
    return _mm_movemask_ps(_mm_cmple_ps(magnitude,tolerance))!=15;
}
bool Active(TransformController* p) {
    return p->child?CallGameVirtual<bool,12>(p->child):false;
}
void Notify(TransformController* p) {
    if(p->child) CallGameVirtual<void,1>(p->child);
}
uint8_t Save(TransformController* p,Cursor* c) {
    auto result=CallGameFunction<uint8_t,0x004ddbd0>(c->stream,1,c->mode);
    CallGameMethod<void,0x007d77c0>(c,result);
    if(!c->good) return 0;
    CallGameMethod<void,0x007d7a20>(c,&p->rotation);
    if(!c->good) return 0;
    CallGameMethod<void,0x007d7960>(c,&p->translation);
    if(!c->good) return 0;
    CallGameFunction<void,0x005657a0>(p->child,c);
    return c->good;
}
TransformController* Construct(TransformController* p,void* host) {
    CallGameMethod<Controller*,0x00565a80>(&p->base,host);
#if defined(_M_IX86)
    p->base.vtable=reinterpret_cast<void*>(0x00fc1e80);
    p->base.secondary8=reinterpret_cast<void*>(0x00fc1e74);
    p->base.secondaryC=reinterpret_cast<void*>(0x00fc1e70);
    p->secondary28=reinterpret_cast<void*>(0x00fc1e5c);
#endif
    _mm_store_ps(reinterpret_cast<float*>(&p->translation),_mm_setzero_ps());
    _mm_store_ps(reinterpret_cast<float*>(&p->rotation),_mm_set_ps(Global(0x0107a538),0,0,0));
    p->child=nullptr;
    return p;
}
}
