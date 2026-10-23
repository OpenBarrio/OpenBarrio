#include "ts3/anim/m05_M155.h"
#include <cstring>
#include <emmintrin.h>
#if defined(_M_IX86)
#include <intrin.h>
#endif
namespace ts3::anim::m155 {
namespace {
void* Key(uint32_t address) {
#if defined(_M_IX86)
    return *reinterpret_cast<void**>(address);
#else
    (void)address; NotRewritten("M155 component registry key");
#endif
}
void* Table(uint32_t address) {
#if defined(_M_IX86)
    return reinterpret_cast<void*>(address);
#else
    (void)address; NotRewritten("M155 original vtable");
#endif
}
}
namespace {
// These two original debug helpers have usercall register inputs. Keep the
// marshalling isolated here; the drawing/selection logic is native C++ below.
void Axes(const Matrix44Value* matrix,const Matrix44Value* model,void* renderer,float axisSize) {
#if defined(_M_IX86)
    const uintptr_t target=0x0093f1e0;
    __asm {
        mov eax, matrix
        mov ecx, model
        mov esi, renderer
        movss xmm6, axisSize
        call target
    }
#else
    (void)matrix; (void)model; (void)renderer; (void)axisSize;
    NotRewritten("M155 axes usercall bridge");
#endif
}
void Label(const void* viewport,const Float4Value* point,void* font,const char* text,void* screen,void* handle,uint32_t* color) {
#if defined(_M_IX86)
    const uintptr_t target=0x0093f3d0;
    __asm {
        push color
        push handle
        push screen
        mov eax, viewport
        mov ecx, point
        mov edi, font
        mov esi, text
        call target
        add esp, 12
    }
#else
    (void)viewport; (void)point; (void)font; (void)text; (void)screen; (void)handle; (void)color;
    NotRewritten("M155 label usercall bridge");
#endif
}
}
namespace {
// MSVC may commute ADDPS/MULPS intrinsics even with precise FP. That changes
// which NaN payload wins. Explicit operand order is observable in the debug API.
__m128 OrderedAdd(__m128 a,__m128 b) {
#if defined(_M_IX86)
    __m128 result;
    __asm {
        movaps xmm0, a
        addps xmm0, b
        movaps result, xmm0
    }
    return result;
#else
    __m128 both=_mm_and_ps(_mm_cmpunord_ps(a,a),_mm_cmpunord_ps(b,b));
    __m128 first=_mm_or_ps(a,_mm_castsi128_ps(_mm_set1_epi32(0x00400000)));
    __m128 sum=_mm_add_ps(a,b);
    return _mm_or_ps(_mm_and_ps(both,first),_mm_andnot_ps(both,sum));
#endif
}
__m128 OrderedMul(__m128 a,__m128 b) {
#if defined(_M_IX86)
    __m128 result;
    __asm {
        movaps xmm0, a
        mulps xmm0, b
        movaps result, xmm0
    }
    return result;
#else
    __m128 both=_mm_and_ps(_mm_cmpunord_ps(a,a),_mm_cmpunord_ps(b,b));
    __m128 first=_mm_or_ps(a,_mm_castsi128_ps(_mm_set1_epi32(0x00400000)));
    __m128 product=_mm_mul_ps(a,b);
    return _mm_or_ps(_mm_and_ps(both,first),_mm_andnot_ps(both,product));
#endif
}
Float4Value Transform(const Float4Value& p,const Matrix44Value& m,bool yFirst=false) {
    __m128 v=_mm_load_ps(p.lane);
    __m128 x=OrderedMul(_mm_shuffle_ps(v,v,0),_mm_load_ps(m.vector[0].lane));
    __m128 y=OrderedMul(_mm_shuffle_ps(v,v,85),_mm_load_ps(m.vector[1].lane));
    __m128 z=OrderedMul(_mm_shuffle_ps(v,v,170),_mm_load_ps(m.vector[2].lane));
    __m128 w=OrderedMul(_mm_shuffle_ps(v,v,255),_mm_load_ps(m.vector[3].lane));
    Float4Value result;
    _mm_store_ps(result.lane,OrderedAdd(OrderedAdd(yFirst?OrderedAdd(y,x):OrderedAdd(x,y),z),w));
    return result;
}
bool Near(const Matrix44Value& a,const Matrix44Value& b) {
    __m128 mask=_mm_castsi128_ps(_mm_set1_epi32(-1));
    for(unsigned i=0;i<4;++i) {
        __m128 d=_mm_sub_ps(_mm_load_ps(a.vector[i].lane),_mm_load_ps(b.vector[i].lane));
        __m128 magnitude=_mm_max_ps(_mm_mul_ps(d,_mm_set1_ps(-1.0f)),d);
        mask=_mm_and_ps(mask,_mm_cmple_ps(magnitude,_mm_set1_ps(0x1p-16f)));
    }
    return _mm_movemask_ps(mask)==15;
}
Float4Value Position(const Matrix44Value& m) {
    Float4Value p=m.vector[3]; p.lane[3]=1.0f; return p;
}
}
void DrawPose(void* object,void* renderer,const Matrix44Value* model,uint32_t allLabels,void* screen,const void* camera,int32_t limit,const uint32_t* flags,const uint32_t* colors,const uint32_t* textColors,const char* const* labels,const Matrix44Value* comparison,const Matrix44Value* staleAxes) {
    if(!renderer || !object) return;
    auto state=CallGameMethod<RenderState*,0x006f4a50>(object);
    if(!state) return;
    if(!CallGameMethod<Matrix44Value*,0x00580420>(state)) {
        void* storage=CallGameFunction<void*,0x005748a0>(105264,Table(0x00fc10d8),0,0,0,0);
        Ref* pose=nullptr;
        if(storage) {
            RigResource* copy;
            CallGameMethod<void,0x0074b950>(&copy,state->rig);
            pose=CallGameMethod<Ref*,0x00574f50>(storage,copy);
        }
        Ref* retained=pose;
        if(pose) ++pose->refs; // deliberately non-atomic, as in EA
        CallGameMethod<void,0x005743a0>(pose);
        CallGameMethod<void,0x00574990>(pose);
        CallGameMethod<void,0x00573330>(state,pose);
        CallGameMethod<void,0x0055cfa0>(&retained);
    }
    auto matrices=CallGameMethod<Matrix44Value*,0x00580420>(state);
    if(!matrices || !state->rig) return;
    alignas(16) uint32_t viewport[4];
    void* service=CallGameFunction<void*,0x0060e2c0>();
    CallGameMethod<void,0x0060ffb0>(service,viewport);
    alignas(16) unsigned char font[0x278];
    CallGameMethod<void,0x0045ee80>(font);
    *reinterpret_cast<float*>(font+0x200)=8.0f;
    *reinterpret_cast<uint32_t*>(font+0x244)=1;
    void* factory=CallGameFunction<void*,0x0044d890>(1);
    void* handle=CallGameVirtual<void*,8>(factory,font,0,0,0xffff,-1,1);
    auto skeleton=&state->rig->skeleton;
    uint32_t count=CallGameMethod<uint32_t,0x00575650>(skeleton);
    if(limit>0 && static_cast<uint32_t>(limit)<count) count=static_cast<uint32_t>(limit);
    for(uint32_t i=0;i<count;++i) {
        uint32_t f=flags?flags[i]:(static_cast<uint8_t>(allLabels)?29:1);
        if(!f) continue;
        bool compared=comparison && (f&32);
        bool same=compared && Near(matrices[i],comparison[i]);
        bool hasLabel=(f&16) && labels && labels[i];
        uint32_t color=colors?colors[i]:0xffffff00;
        uint32_t textColor=textColors?textColors[i]:0xffffffff;
        int32_t parent=CallGameMethod<int32_t,0x005756b0>(skeleton,i);
        Matrix44Value bone=matrices[i];
        Float4Value start=Position(matrices[parent]);
        Float4Value end=Position(bone);
        if(model) { start=Transform(start,*model); end=Transform(end,*model); }
        if(f&1) CallGameMethod<void,0x006c8680>(renderer,&start,&end,color,color);
        if(f&2) Axes(&bone,model,renderer,0.03f);
        if(!screen || !camera || (!(f&4) && !(f&8) && !hasLabel)) continue;
        char text[256]; text[0]=0;
        if(f&4) CallGameFunction<void,0x004f2ce0>(text,"%d",i);
        if(f&8) {
            if(f&4) std::strcat(text," : ");
            std::strcat(text,CallGameMethod<const char*,0x00575670>(skeleton,i));
        }
        if(hasLabel && (!compared || same)) {
            if(f&(4|8)) std::strcat(text," : (");
            std::strcat(text,labels[i]);
            if(f&(4|8)) std::strcat(text,")");
        }
        // EA terminates byte 254 only after concatenation (not a bounded append).
        text[254]=0;
        auto projection=reinterpret_cast<const Matrix44Value*>(static_cast<const char*>(camera)+0x130);
        Float4Value projected=Transform(end,*projection,true);
        Label(viewport,&projected,font,text,screen,handle,&textColor);
        if(compared) {
            text[0]=0;
            if(hasLabel && !same) {
                CallGameFunction<void,0x004f2ce0>(text,"%s",labels[i]);
                text[254]=0;
            }
            Float4Value alternate=Position(comparison[i]);
            if(model) alternate=Transform(alternate,*model,true);
            uint32_t dim=((color>>1)&0x7f7f7f)-0x1000000;
            CallGameMethod<void,0x006c8680>(renderer,&end,&alternate,dim,dim);
            Axes(staleAxes,model,renderer,0.015f);
            projected=Transform(alternate,*projection,true);
            Label(viewport,&projected,font,text,screen,handle,&textColor);
        }
    }
    CallGameVirtual<void,3>(handle);
}

bool Update(Component* self,uintptr_t) {
    if(self->pending) CallGameMethod<void,0x0093f510>(self);
    if((self->host && CallGameMethod<void*,0x00935770>(self->host,Key(0x011e90fc))) || (self->flags & 0x80000000u))
        CallGameVirtual<void,12>(self);
    if(self->transform && self->host) {
        void* source=CallGameMethod<void*,0x00935770>(self->host,Key(0x011e9158));
        if(source) {
            void* translation=CallGameMethod<void*,0x007f1620>(source);
            _mm_store_ps(static_cast<float*>(self->transform),_mm_load_ps(static_cast<float*>(translation)));
            void* rotation=CallGameMethod<void*,0x00580430>(source);
            _mm_store_ps(static_cast<float*>(self->transform)+4,_mm_load_ps(static_cast<float*>(rotation)));
            _mm_store_ps(static_cast<float*>(self->transform)+8,_mm_load_ps(static_cast<float*>(self->transform)+4));
        }
    }
    return true;
}
uintptr_t Invalidate(Component* self) {
    self->flags |= 0x800;
    return CallGameMethod<uintptr_t,0x00940310>(self);
}
LocalPose* DeletePose(LocalPose* self,uint32_t flags) {
    void** weights=self->weights;
    self->vtable=static_cast<void**>(Table(0x0104348c));
    if(weights) {
        if(*weights) CallGameFunction<void,0x004e3f70>(*weights);
        CallGameFunction<void,0x004e3f70>(weights);
    }
    self->vtable=static_cast<void**>(Table(0x0107396c));
    if(flags & 1) CallGameFunction<void,0x004e3f70>(self);
    return self;
}
uintptr_t UpdateRender(uint32_t kind,uint32_t id,void* key,Ref* retained) {
#if defined(_M_IX86)
    if(retained) _InterlockedExchangeAdd(&retained->refs,1);
#else
    if(retained) NotRewritten("M155 interlocked game refs");
#endif
    uintptr_t result=CallGameFunction<uintptr_t,0x006fde10>();
    if(result) {
        result=CallGameMethod<uintptr_t,0x006fe3b0>(reinterpret_cast<void*>(result),kind,id);
        if(result) {
            result=CallGameVirtual<uintptr_t,3>(reinterpret_cast<void*>(result),Table(0x017f1d88));
            if(result) {
                void* owner=reinterpret_cast<void*>(result);
                void* render=CallGameMethod<void*,0x006f4a50>(owner);
                if(!render) {
                    void* rig=nullptr;
                    CallGameFunction<void,0x0054c010>(key,&rig);
                    if(rig) {
                        void* storage=CallGameFunction<void*,0x004e3f90>(80,Table(0x00ff98ac),0,0,0,0);
                        render=storage ? CallGameMethod<void*,0x00574100>(storage) : nullptr;
                        void* copy;
                        // The virtual consumes the by-value retained-pointer wrapper.
                        CallGameMethod<void,0x005a8cd0>(&copy,&rig);
                        CallGameVirtual<void,0>(render,copy);
                        CallGameMethod<void,0x006f5ca0>(owner,rig);
                        CallGameMethod<void,0x006f50e0>(owner,render);
                        if(rig) CallGameVirtual<void,1>(rig);
                    }
                }
                CallGameMethod<void,0x005740b0>(render,retained);
                CallGameMethod<void,0x00573320>(render);
                result=CallGameMethod<uintptr_t,0x00573e80>(render);
            }
        }
    }
    if(retained) {
        result=reinterpret_cast<uintptr_t>(&retained->refs);
#if defined(_M_IX86)
        if(_InterlockedExchangeAdd(&retained->refs,-1)==1) {
            _InterlockedExchange(&retained->refs,1);
            return CallGameVirtual<uintptr_t,0>(retained,1);
        }
#endif
    }
    return result;
}
}
#if defined(_M_IX86)
namespace ts3::anim::m155 {
// Reproduce only the original frame geometry, not its algorithm. In particular
// [aligned(entry ESP-4)-0x2c0] was never initialized by EA before the overlay
// axes helper read it. Reserving the complete old frame keeps that input alive
// while the native implementation runs below it.
__declspec(naked) void DebugPose(void*,void*,const Matrix44Value*,uint32_t,void*,const void*,int32_t,const uint32_t*,const uint32_t*,const uint32_t*,const char* const*,const Matrix44Value*) {
    __asm {
        push ebp
        mov ebp, esp
        and esp, -16
        sub esp, 0x534
        push ebx
        push esi
        push edi
        lea eax, [esp+0x280]
        push eax
        push dword ptr [ebp+52]
        push dword ptr [ebp+48]
        push dword ptr [ebp+44]
        push dword ptr [ebp+40]
        push dword ptr [ebp+36]
        push dword ptr [ebp+32]
        push dword ptr [ebp+28]
        push dword ptr [ebp+24]
        push dword ptr [ebp+20]
        push dword ptr [ebp+16]
        push dword ptr [ebp+12]
        push dword ptr [ebp+8]
        call m155::DrawPose
        add esp, 52
        pop edi
        pop esi
        pop ebx
        mov esp, ebp
        pop ebp
        ret
    }
}

}
namespace ts3::anim::m155::hooks {
bool __fastcall Update(Component* s,void*,uintptr_t a) { return m155::Update(s,a); }
uintptr_t __fastcall Invalidate(Component* s,void*) { return m155::Invalidate(s); }
LocalPose* __fastcall DeletePose(LocalPose* s,void*,uint32_t f) { return m155::DeletePose(s,f); }
}
#endif
