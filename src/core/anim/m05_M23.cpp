#include "ts3/anim/m05_M23.h"
#include <cstring>
#include <xmmintrin.h>
namespace ts3::anim::m23 {
namespace {
void Read(b01::Cursor* s,void* out,uint32_t n) {
    s->position=static_cast<int32_t>(static_cast<uint32_t>(s->position)+n);
    if(s->position>=0) CallGameMethod<void,0x00593FF0>(s,n);
    if(s->position<0) std::memcpy(out,s->input+s->position+1-n,n);
}
void* Address(uint32_t ea) {
#if defined(_M_IX86)
    return reinterpret_cast<void*>(ea);
#else
    (void)ea; return nullptr;
#endif
}
}
void Load(Evaluator* self,b01::Cursor* s) {
    Read(s,&self->first,4); Read(s,&self->second,4);
    Read(s,self->key,8); Read(s,self->tail,8);
}
Evaluator* Destroy(Evaluator* self,uint32_t flags) {
    self->base.vtable=static_cast<void**>(Address(0x00FC1948));
    b01::Reset(&self->base);
    if(flags&1u) CallGameFunction<void,0x004E3F70>(self);
    return self;
}
void Evaluate(Subject* subject,void* pose) {
    auto manager=CallGameFunction<void*,0x004DA9D0>();
    auto list=static_cast<void**>(manager);
    if(list[1]!=manager) {
        auto link=static_cast<unsigned char*>(list[0]);
        void* evaluator=link?link-sizeof(void*):nullptr;
        CallGameVirtual<void,2>(evaluator,subject,pose);
    }
    if(!(subject->flags&2)) return;
    auto p=static_cast<unsigned char*>(pose);
    auto joints=CallGameMethod<unsigned char*,0x00574190>(pose);
    __m128 q=_mm_load_ps(reinterpret_cast<float*>(p+0x19b10));
    __m128 t=_mm_load_ps(reinterpret_cast<float*>(p+0x19b20));
    if(_mm_movemask_ps(_mm_cmpeq_ps(_mm_set_ps(1,0,0,0),q))==15 &&
       (_mm_movemask_ps(_mm_cmpeq_ps(_mm_setzero_ps(),t))&7)==7) return;
    auto skeleton=static_cast<unsigned char*>(subject->rig)+0x28;
    int index=CallGameMethod<int,0x00575E30>(skeleton,Address(0x00FC1974));
    if(index<0) return;
    float* joint=reinterpret_cast<float*>(joints+48*index);
    __m128 r=_mm_load_ps(joint+4);
    __m128 a=_mm_mul_ps(_mm_shuffle_ps(r,r,9),_mm_shuffle_ps(q,q,0x12));
    __m128 b=_mm_mul_ps(_mm_shuffle_ps(r,r,0x12),_mm_shuffle_ps(q,q,9));
    __m128 qw=_mm_shuffle_ps(q,q,0xff), rw=_mm_shuffle_ps(r,r,0xff);
    b=_mm_sub_ps(b,a);
    b=_mm_add_ps(b,_mm_mul_ps(r,qw));
    b=_mm_add_ps(b,_mm_mul_ps(rw,q));
    __m128 product=_mm_mul_ps(r,q);
    __m128 dot=_mm_add_ps(_mm_shuffle_ps(product,product,2),_mm_shuffle_ps(product,product,1));
    dot=_mm_add_ps(dot,product); dot=_mm_shuffle_ps(dot,dot,0);
    __m128 w=_mm_sub_ps(_mm_mul_ps(rw,qw),dot);
    __m128 mix=_mm_shuffle_ps(b,w,0x0a);
    b=_mm_shuffle_ps(b,mix,0x84);
    _mm_store_ps(joint+4,b);
    _mm_store_ps(joint,_mm_add_ps(_mm_load_ps(joint),t));
    uint16_t flags; std::memcpy(&flags,p+0x50,2); flags&=0xfffe; std::memcpy(p+0x50,&flags,2);
}
void DestroyController(Controller* self) {
    self->vtable=Address(0x00FC1998); self->secondary8=Address(0x00FC1990); self->secondaryC=Address(0x00FC198C);
    CallGameMethod<void,0x005659D0>(&self->secondaryC);
    self->vtable=Address(0x0107396C);
}
uint32_t Unregister(uint32_t key) {
    auto r=static_cast<b01::Registry*>(Address(0x011CD508));
    auto result=b01::Erase(r,&key);
    return (result&0xffffff00u)|1u;
}
uint32_t Active(Sequence* self) {
    if(self->nodes.next!=&self->nodes || self->pending || self->pending50) return 1;
    return static_cast<uint8_t>(CallGameMethod<uint32_t,0x0056BC80>(self->transition))?1u:0u;
}
}
#if defined(_M_IX86)
namespace ts3::anim::m23::hooks {
void __fastcall Load(Evaluator* p,void*,b01::Cursor* s) {m23::Load(p,s);}
Evaluator* __fastcall Destroy(Evaluator* p,void*,uint32_t f) {return m23::Destroy(p,f);}
void __stdcall Evaluate(Subject* s,void* p) {m23::Evaluate(s,p);}
void __fastcall DestroyController(Controller* p,void*) {m23::DestroyController(p);}
uint32_t __cdecl Unregister(uint32_t k) {return m23::Unregister(k);}
uint32_t __fastcall Active(Sequence* p,void*) {return m23::Active(p);}
}
#endif
