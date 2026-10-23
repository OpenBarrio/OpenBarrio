#include "ts3/anim/m05_M03.h"
#include "ts3/layout.h"
#include <xmmintrin.h>
namespace ts3::anim::m03 {
namespace {
template<unsigned A> void* Table() {
#if defined(_M_IX86)
    return reinterpret_cast<void*>(uintptr_t(A));
#else
    AbortNotRewritten("M03 evaluator table"); return nullptr;
#endif
}
}
Node* ConstructReset(Node* p,void* arg) {
    CallGameMethod<void,0x0054C1D0>(p,arg);
    p->table=Table<0x00FC1140>(); return p;
}
Node* ConstructFirst(Node* p,void* arg) {
    CallGameMethod<void,0x0054C1D0>(p,arg);
    p->table=Table<0x00FC116C>(); return p;
}
Node* ConstructSecond(Node* p,void* arg) {
    CallGameMethod<void,0x0054C1D0>(p,arg);
    p->table=Table<0x00FC1198>(); return p;
}
Node* DeleteReset(Node* p,uint32_t flags) {
    p->table=Table<0x00FC1140>();
    CallGameMethod<void,0x0054C130>(p);
    if(flags&1u) CallGameFunction<void,0x004E3F70>(p);
    return p;
}
uintptr_t Reset(Node*,RigPose*,RigPose* out) {
    CallGameMethod<void,0x005743A0>(out);
    return CallGameMethod<uintptr_t,0x005741B0>(out,0.0f);
}
uintptr_t AddDeltas(Node* p,RigPose* input,RigPose* out) {
    uintptr_t result=reinterpret_cast<uintptr_t>(&p->children);
    // EA tests the sentinel's previous link, then evaluates only its first child.
    if(p->children.previous==&p->children) return result;
    Node* child=p->children.next ? reinterpret_cast<Node*>(reinterpret_cast<char*>(p->children.next)-offsetof(Node,siblings)):nullptr;
    result=CallGameVirtual<uintptr_t,2>(child,input,out);
    if(!input->vertexWeights) return result;
    auto* values=CallGameMethod<SrtValue*,0x00574190>(out);
    result=reinterpret_cast<uintptr_t>(values);
    auto* delta=static_cast<SrtValue*>(input->vertexWeights);
    for(unsigned i=0;i<out->boneCount;++i) {
        auto* dst=reinterpret_cast<float*>(values+i);
        auto* src=reinterpret_cast<const float*>(delta+i);
        // EAX is the current delta address, not the output pose or a success flag.
        result=reinterpret_cast<uintptr_t>(delta+i);
        _mm_store_ps(dst,_mm_add_ps(_mm_load_ps(dst),_mm_load_ps(src)));
        __m128 q=_mm_add_ps(_mm_load_ps(src+4),_mm_load_ps(dst+4));
        __m128 n=_mm_mul_ps(q,q);
        __m128 t=_mm_add_ps(_mm_shuffle_ps(n,n,0x0e),n);
        n=_mm_add_ps(_mm_shuffle_ps(t,t,1),t);
        n=_mm_shuffle_ps(n,n,0);
        // COMISS/JBE skips the store for zero and unordered lengths: retain old q.
        if(_mm_cvtss_f32(n)>0.0f) {
            __m128 r=_mm_rsqrt_ps(n);
            __m128 correction=_mm_sub_ps(_mm_set1_ps(1.0f),_mm_mul_ps(_mm_mul_ps(r,r),n));
            __m128 scale=_mm_add_ps(_mm_mul_ps(correction,_mm_mul_ps(_mm_set1_ps(0.5f),r)),r);
            _mm_store_ps(dst+4,_mm_mul_ps(scale,q));
        }
        _mm_store_ps(dst+8,_mm_add_ps(_mm_load_ps(src+8),_mm_load_ps(dst+8)));
    }
    out->flags &= 0xfffe;
    return result;
}
}
