#include "ts3/anim/m05_B38.h"
#include <cstring>
#include <xmmintrin.h>
namespace ts3::anim::b38 {
namespace {
void Store(State* s,size_t offset,float v) { std::memcpy(s->bytes+offset,&v,4); }
}
void __fastcall Stop(TurnState* s,void*) {
    if(s->child) {
        auto* child=s->child;
        s->child=nullptr;
        if(--child->refcount==0) {
            child->refcount=1;
            CallGameVirtual<void,0>(child,1u);
        }
    }
    if(s->host) CallGameVirtual<void,6>(s->host);
}
void __fastcall Clear(routemanager::RouteManagerObject* s,void*) {
    s->field128=0;
}
float* __fastcall Accumulate(void*,void*,float* out,const float* q) {
    const __m128 v=_mm_set_ps(1.8f,0.0f,0.0f,1.8f);
    const __m128 r=_mm_load_ps(q);
    const __m128 a=_mm_shuffle_ps(r,r,9);
    const __m128 b=_mm_shuffle_ps(r,r,18);
    __m128 c=_mm_sub_ps(_mm_mul_ps(_mm_shuffle_ps(v,v,18),a),_mm_mul_ps(_mm_shuffle_ps(v,v,9),b));
    c=_mm_add_ps(c,_mm_mul_ps(_mm_shuffle_ps(r,r,255),v));
    __m128 d=_mm_sub_ps(_mm_mul_ps(_mm_shuffle_ps(c,c,18),a),_mm_mul_ps(_mm_shuffle_ps(c,c,9),b));
    d=_mm_mul_ps(d,_mm_set1_ps(2.0f));
    d=_mm_add_ps(d,v);
    d=_mm_add_ps(d,_mm_load_ps(out));
    _mm_store_ps(out,d);
    return out;
}
uint32_t __fastcall RouteType(void*,void*) { return 0x0f612d91u; }
uint32_t __fastcall RouteSubType(void* s,void*) {
    return RouteType(static_cast<unsigned char*>(s)-offsetof(routemanager::RouteManagerObject,vtableSub18),nullptr);
}
routemanager::RouteManagerObject* __fastcall DestroySub(void* s,void*,uint32_t flags) {
    auto* base=reinterpret_cast<routemanager::RouteManagerObject*>(static_cast<unsigned char*>(s)-offsetof(routemanager::RouteManagerObject,vtableSub04));
    return base->Destruct(flags);
}
uint32_t __fastcall VehicleType(void*,void*) { return 0xf4cfbaecu; }
void* __fastcall Query(void* s,void*,uint32_t id) {
    if(id==0x54cfb94fu) return s;
    if(id==0xf4cfbaecu) return s;
    return nullptr;
}
float __fastcall Weight(State* s,void*) {
    float value; std::memcpy(&value,s->bytes+0x90,4); return value;
}
float* __fastcall Vector(State* s,void*,float* out) {
    _mm_store_ps(out,_mm_load_ps(reinterpret_cast<float*>(s->bytes+0x80)));
    return out;
}
void __fastcall ResetVehicle(State* s,void*) {
    Store(s,0x4c,0.0f);
    Store(s,0x50,0.1f);
    _mm_store_ps(reinterpret_cast<float*>(s->bytes+0x80),_mm_setzero_ps());
}
uint32_t __fastcall TurnType(void*,void*) { return 0xf4da7c8du; }
void __fastcall ResetTurn(State* s,void*) {
    Store(s,0xb4,4.0f);
    Store(s,0xb8,2.0f);
    Store(s,0xbc,2.0f);
    Store(s,0xc0,1.0f);
    const uint32_t mode=2;
    std::memcpy(s->bytes+0xc4,&mode,4);
}
void __fastcall ResetMotion(State* s,void*) {
    Store(s,0x6c,0.0f);
    Store(s,0x70,0.5f);
    Store(s,0xac,0.0f);
    Store(s,0xb0,0.0f);
}
void __fastcall SetMotion(State* s,void*,float v) {
    Store(s,0xa8,v);
}
}
