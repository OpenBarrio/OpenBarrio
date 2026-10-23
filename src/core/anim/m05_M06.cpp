#include "ts3/anim/m05_M06.h"
#include <cstring>
#include "ts3/debug/clu_6c8680_Debug.h"
namespace ts3::anim::m06 {
void* Query(void* self,uint32_t id) {
    // Adapted from verified PackedFileRecordQueryInterfaceById; only IDs differ.
    return id==0x73deb8c8u || id==0x9419c8abu ? self : nullptr;
}
uintptr_t Save(Node* self,void* stream) {
    CallGameFunction<uintptr_t,0x0054bf70>(self->left,stream);
    return CallGameFunction<uintptr_t,0x0054bf70>(self->right,stream);
}
uintptr_t Load(Node* self,void* stream) {
    CallGameFunction<uintptr_t,0x0054bfc0>(self->owner,&self->left,stream);
    return CallGameFunction<uintptr_t,0x0054bfc0>(self->owner,&self->right,stream);
}
void* Destroy(Node* self,uint32_t flags) {
    // Same deleting-dtor pattern as verified m03::b04::DestroyFactory.
#if defined(_M_IX86)
    self->vtable=reinterpret_cast<void**>(0x00fc1250);
#else
    self->vtable=nullptr;
#endif
    CallGameMethod<void,0x0054c130>(self);
    if(flags&1u) CallGameFunction<void,0x004e3f70>(self);
    return self;
}
static Pose* Temporary(Context* context) {
#if defined(_M_IX86)
    auto* p=CallGameFunction<Pose*,0x005748a0>(0x19b30,reinterpret_cast<const char*>(0xfc10d8),0,0,0,0);
#else
    auto* p=CallGameFunction<Pose*,0x005748a0>(0x19b30,static_cast<const char*>(nullptr),0,0,0,0);
#endif
    if(p) {
        if(context->rig) CallGameVirtual<void,0>(context->rig);
        p=CallGameMethod<Pose*,0x00574f50>(p,context->rig);
        if(p) ++p->refs;
    }
    return p;
}
static uintptr_t Release(Pose* p) {
    auto result=static_cast<uint32_t>(--p->refs);
    if(!result) {
        p->refs=1;
        return CallGameVirtual<uintptr_t,0>(p,1);
    }
    return result;
}
uintptr_t Evaluate(Node* self,Context* context,Pose* output) {
    Pose* left=Temporary(context);
    CallGameVirtual<uintptr_t,2>(self->left,context,left);
    Pose* right=Temporary(context);
    CallGameVirtual<uintptr_t,2>(self->right,context,right);
    float* source=right->weights;
    float* dest=nullptr;
    if(source) {
        CallGameMethod<void,0x005741b0>(output,1.0f);
        dest=output->weights;
    }
    auto skeleton=static_cast<unsigned char*>(context->rig)+0x28;
    uint32_t count=CallGameMethod<uint32_t,0x00575650>(skeleton);
    for(uint32_t i=0;i<count;++i) {
        alignas(16) unsigned char inverse[48],composed[48];
        auto* a=CallGameMethod<unsigned char*,0x00574190>(left)+48*i;
        CallGameFunction<void,0x0054eef0>(a,inverse);
        auto* b=CallGameMethod<unsigned char*,0x00574190>(right)+48*i;
        CallGameFunction<void,0x0054c8d0>(b,inverse,composed);
        auto* out=CallGameMethod<unsigned char*,0x00574190>(output)+48*i;
        std::memcpy(out,composed,48);
        if(source && dest) {
#if defined(_M_IX86)
            // FLD/FSTP quiets signaling NaNs, unlike an SSE bitwise copy.
            const float* from=source+i;
            float* to=dest+i;
            __asm { mov eax, from
                    fld dword ptr [eax]
                    mov eax, to
                    fstp dword ptr [eax] }
#else
            dest[i]=source[i];
#endif
        }
        // The original re-reads the rig and joint count after every bone.
        skeleton=static_cast<unsigned char*>(context->rig)+0x28;
        count=CallGameMethod<uint32_t,0x00575650>(skeleton);
    }
    uintptr_t result=Release(right);
    if(left) result=Release(left);
    return result;
}
void* Segment(void* self,const void* a,const void* b,uint32_t color) {
    return static_cast<ts3::debug::DebugLineBuffer*>(self)->AddSegment(
        static_cast<const ts3::debug::Vec4*>(a),static_cast<const ts3::debug::Vec4*>(b),color,color);
}
}
