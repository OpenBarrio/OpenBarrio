#include "ts3/render/m07_M10.h"
#include <cstring>
namespace ts3::render::m07::m10 {
namespace {
template<class T> T Read(const void* p,size_t n) { T x; std::memcpy(&x,static_cast<const uint8_t*>(p)+n,sizeof(x)); return x; }
template<unsigned A> bool Test(void* p) { return CallGameMethod<uint8_t,A>(p)!=0; }
}
uintptr_t __fastcall Fixup(SceneModel* s,void*) {
    auto* resource=s->resource.resolver;
    auto* model=CallGameMethod<Model*,0x006DC5B0>(resource);
    s->resourceState=CallGameMethod<uint32_t,0x0072C920>(resource);
    s->flags &= 0xffffffa5;
    if(Test<0x006DB760>(resource)) s->flags|=0x40;
    bool refresh=false;
    if(Test<0x006DB830>(resource) || (s->options&8)) { s->flags|=2; refresh=true; }
    else refresh=Read<uint32_t>(s->resource.resolver,0x44)!=0 || (s->flags&0x40)!=0;
    if(refresh) CallGameMethod<void,0x006FAC70>(s);
    if(!model) {
        s->count=0; s->records=nullptr;
        auto r=CallGameMethod<uintptr_t,0x006DB7B0>(resource);
        if(uint8_t(r)) s->flags|=0x10;
        return r;
    }
    if(s->options&4) s->flags|=0x10;
    if(s->options2&0x10) s->flags|=0x80;
    if(s->options2&0x20) s->flags|=0x100;
    if(s->options2&0x40) s->flags|=0x200;
    if(s->options2&0x80) s->flags|=0x800;
    auto* info=CallGameMethod<void*,0x006DB800>(resource);
    if(info && Read<uint8_t>(info,48) && !Read<uint8_t>(info,49) && !(s->options3&1)) s->flags|=0x400;
    CallGameVirtual<void,36>(s);
    uint32_t count=CallGameMethod<uint32_t,0x006C93D0>(model);
    s->count=count+1;
    CallGameMethod<void,0x006F7AB0>(&s->vector,count+1);
    s->records=s->vector.begin;
    CallGameVirtual<void,38>(s,model);
    CallGameMethod<void,0x006F7880>(s);
    CallGameVirtual<void,40>(s);
    float opacity=CallGameFunction<uint8_t,0x006DE460>() ? 1.0f : CallGameMethod<float,0x006FCB60>(s);
    bool translucent=opacity>0.001f && opacity<0.99900001f;
    bool low=opacity<0.30000001f;
    bool suppress=s->color[3]*s->opacity<0.001f;
    CallGameMethod<void,0x006FAD10>(s);
    uint16_t offset=uint16_t(Read<uint16_t>(s->resource.resolver,0x28)+Read<uint16_t>(s->resource.resolver,0x2a));
    for(uint32_t i=0;i<count;++i) {
        auto* m=model->materials[i]; auto& out=s->vector.begin[i];
        bool blend=translucent,below=low;
        if(Test<0x0068EF50>(m)) { blend=false; below=false; suppress=false; }
        CallGameMethod<void,0x0068F920>(m,uint32_t(blend?7:0));
        if((blend && Test<0x0068EEF0>(m)) || suppress) { out.flags=0; continue; }
        bool hidden=false;
        if(Test<0x0068EEF0>(m)) {
            if(s->renderFlags&0x40) hidden=true;
            else if(CallGameFunction<void*,0x006E8330>()) {
                if(Test<0x006EB100>(CallGameFunction<void*,0x006E8330>())) hidden=true;
                else if(CallGameMethod<int32_t,0x009158F0>(s)<0 && !Test<0x006E8310>(CallGameFunction<void*,0x006E8330>()) && Test<0x006FAC20>(s)) hidden=true;
            }
        }
        if(hidden) { out.flags=0; continue; }
        out.flags=Test<0x0068EF20>(m)?4:1;
        if(Test<0x0068EE70>(m)) out.flags|=0x100;
        out.offset=offset; out.order=m->order==100?s->order:m->order;
        if(Test<0x0068EE60>(m)) out.flags=(out.flags&0xfffffffa)|0x800;
        if(Test<0x0068EF30>(m)) out.flags=(out.flags&0xffff0000)|0x20;
        if(Test<0x0068EF40>(m)) out.flags=(out.flags&0xfffffffa)|0x1000;
        if(Test<0x0068EEF0>(m)) out.flags=(out.flags&0xffffb3b8)|0x10000;
        else if(Test<0x0068EF80>(m) && (s->resource.flags&4) && !below) out.flags|=0x80;
        if(m->flags1&0x40) out.flags=(out.flags&0xffffbfb8)|0x400;
        else if(m->flags1&0x80) out.flags=(out.flags&0xfffffbb8)|0x4000;
        else if(m->flags2&1) out.flags=(out.flags&0xffffb3ba)|2;
        else if(m->flags2&2) {
            out.flags|=0x40;
            if(!Test<0x0068EE60>(m)) out.flags|=0x8004;
            out.flags&=0xffffbbfc;
        }
        if(s->flags&0x400) out.flags|=m->kind==1?0x40000:0x20000;
    }
    auto* terminal=s->vector.end-1;
    terminal->flags=8;
    std::memcpy(terminal->transform,s->bounds,32);
    terminal->order=100;
    alignas(16) uint8_t matrix[64];
    void* transform=CallGameMethod<void*,0x006FAF40>(s,matrix);
    CallGameMethod<void,0x006FCD80>(&s->flags,transform);
    return CallGameMethod<uintptr_t,0x006FCC50>(s);
}
}

#include <xmmintrin.h>
namespace ts3::render::m07::m10 {
namespace {
struct alignas(16) Matrix { __m128 rows[4]; };
Matrix Multiply(const Matrix& a,const Matrix& b,bool reverse=false) {
    Matrix out;
    for(int i=0;i<4;++i) {
        auto x=_mm_mul_ps(_mm_shuffle_ps(a.rows[i],a.rows[i],0),b.rows[0]);
        auto y=_mm_mul_ps(_mm_shuffle_ps(a.rows[i],a.rows[i],85),b.rows[1]);
        auto z=_mm_mul_ps(_mm_shuffle_ps(a.rows[i],a.rows[i],170),b.rows[2]);
        auto w=_mm_mul_ps(_mm_shuffle_ps(a.rows[i],a.rows[i],255),b.rows[3]);
        out.rows[i]=_mm_add_ps(_mm_add_ps(reverse?_mm_add_ps(y,x):_mm_add_ps(x,y),z),w);
    }
    return out;
}
void* InstanceMatrices(SceneModel* s) { return s->instance?CallGameMethod<void*,0x00580420>(s->instance):nullptr; }
void InitDescriptor(Descriptor& d,SceneModel* s,uint32_t index) {
    CallGameMethod<void,0x00697D90>(&d);
    d.index=index;d.one=1;d.matrices=InstanceMatrices(s);
    d.data30=s->instance?s->instance->data30:nullptr;
    d.data2c=s->instance?s->instance->data2c:nullptr;
    d.state=s->state2a4;
}
uint32_t ImageWord(uint32_t address) {
#if defined(_M_IX86)
    return *reinterpret_cast<const uint32_t*>(uintptr_t(address));
#else
    (void)address; AbortNotRewritten("m07_M10 renderer budget global"); return 0;
#endif
}
uintptr_t QueryFallback(Material* material,void** value,void** extra) {
#if defined(_M_IX86)
    uintptr_t result;
    // 006F4B50 takes its material in ESI and its second output in EDI.
    __asm {
        push esi
        push edi
        mov esi,material
        mov edi,extra
        mov eax,value
        push eax
        mov eax,006F4B50h
        call eax
        add esp,4
        mov result,eax
        pop edi
        pop esi
    }
    return result;
#else
    (void)material;(void)value;(void)extra;AbortNotRewritten("m07_M10 EDI-output original bridge");return 0;
#endif
}
__declspec(noinline) uintptr_t __fastcall SubmitBody(SceneModel* s,void*,uint32_t index,Packet* p,uintptr_t incoming) {
    if(s->flags&0x1000) return incoming;
    const bool global=CallGameFunction<uint8_t,0x006DE460>()!=0;
    uintptr_t result=reinterpret_cast<uintptr_t>(p);
    auto* resource=s->resource.resolver;
    if(p->flags&8) {
        if(!(s->renderFlags&0x20000)) {
            auto* gate=CallGameMethod<void*,0x006FB3A0>(s,uint32_t(0));result=reinterpret_cast<uintptr_t>(gate);
            if(gate) {
                result=Read<uint32_t>(gate,468);
                if(result!=2 && result!=1) {
                    result=CallGameMethod<uintptr_t,0x006F4BC0>(s);
                    if(!uint8_t(result)) return result;
                }
            }
        }
        if(s->fade) return result;
        // COMISS/JB deliberately admits unordered values here.
        if(s->color[3]*s->opacity<=0.01f && !global) return result;
        auto* model=CallGameMethod<Model*,0x006DBA50>(resource);
        result=reinterpret_cast<uintptr_t>(model);
        alignas(16) Descriptor descriptor;
        if(model) {
            const uint32_t count=CallGameMethod<uint32_t,0x006C93D0>(model);
            Matrix transform;result=reinterpret_cast<uintptr_t>(CallGameMethod<Matrix*,0x006FAF40>(s,&transform));
            for(uint32_t i=0;i<count;++i) {
                auto* material=model->materials[i];
                void* special=CallGameMethod<void*,0x00F20360>(material);
                if(special) {
                    void* source=CallGameMethod<void*,0x0068EC50>(material);
                    void* data=CallGameMethod<void*,0x00F1D820>(source);
                    struct Property { void* handle; uint8_t opaque[28]; } property;
                    CallGameMethod<void,0x006DE120>(&property,data);
                    auto position=CallGameMethod<uint32_t,0x00798DE0>(&property,uint32_t(2031009580));
                    auto scale=CallGameMethod<uint32_t,0x00798DE0>(&property,uint32_t(1215711461));
                    struct Value { float* data; uint8_t opaque[28]; } value;
                    CallGameMethod<void,0x00798E60>(&property,position,&value);
                    __m128 translation=_mm_set_ps(1.0f,value.data[2],value.data[1],value.data[0]);
                    CallGameMethod<void,0x00798E60>(&property,scale,&value);
                    float* scales=value.data;
                    if(s->instance && CallGameMethod<uint32_t,0x0068ED40>(material)==1) {
                        void* matrices=InstanceMatrices(s);
                        Matrix bone;
                        if(matrices && CallGameMethod<uint8_t,0x0068F7A0>(material,uint32_t(0),matrices,&bone)) transform=Multiply(bone,transform,true);
                    }
                    auto x=_mm_mul_ps(_mm_shuffle_ps(translation,translation,0),transform.rows[0]);
                    auto y=_mm_mul_ps(_mm_shuffle_ps(translation,translation,85),transform.rows[1]);
                    auto z=_mm_mul_ps(_mm_shuffle_ps(translation,translation,170),transform.rows[2]);
                    auto w=_mm_mul_ps(_mm_shuffle_ps(translation,translation,255),transform.rows[3]);
                    transform.rows[3]=_mm_add_ps(_mm_add_ps(_mm_add_ps(x,y),z),w);
                    for(int k=0;k<3;++k) transform.rows[k]=_mm_mul_ps(_mm_set1_ps(scales[k]),transform.rows[k]);
                    CallGameMethod<uintptr_t,0x006FAEC0>(p->collector1c,special,&transform);
                } else {
                    InitDescriptor(descriptor,s,i);
                    CallGameVirtual<uintptr_t,37>(s,model,&descriptor,p);
                }
                result=i+1;
            }
            return result;
        }
        model=CallGameMethod<Model*,0x006DB810>(resource);result=reinterpret_cast<uintptr_t>(model);
        if(!model) return result;
        uint32_t count=s->count-1;
        for(uint32_t i=0;i<count;++i) {
            auto* material=model->materials[i];
            result=CallGameMethod<uintptr_t,0x0068EE50>(material);
            if(uint8_t(result)) {
                void* value;void* extra;
                result=QueryFallback(material,&value,&extra);
                if(value) {
                    InitDescriptor(descriptor,s,i);descriptor.lightmap=extra;descriptor.fade=uint32_t(uintptr_t(value));
                    result=CallGameVirtual<uintptr_t,37>(s,model,&descriptor,p);
                }
            }
        }
        return result;
    }
    auto* model=CallGameMethod<Model*,0x006DB810>(resource);
    void* info=CallGameMethod<void*,0x006DB800>(resource);result=reinterpret_cast<uintptr_t>(info);
    if(!info || !model) return result;
    if(s->resource.begin==s->resource.end && !p->lighting) return CallGameMethod<uintptr_t,0x006FAD50>(s);
    void* lighting=p->lighting;bool different=false;
    if(!lighting) {
        void* first=s->resource.begin[0];lighting=first;
        if(uintptr_t(s->resource.end-s->resource.begin)>1) {
            if(model->materials[index]->kind>0) lighting=s->resource.begin[1];
            uint8_t second=CallGameMethod<uint8_t,0x006F4800>(s->resource.begin[1]);
            different=CallGameMethod<uint8_t,0x006F4800>(first)!=second;
        }
    }
    CallGameMethod<void,0x006B8B30>(lighting);
    auto* lightState=CallGameFunction<void*,0x006C3550>();
    CallGameMethod<void,0x006C2370>(lightState,uint32_t(0));
    bool noLight=!CallGameMethod<uint8_t,0x006F4800>(lighting);
    if(s->state1f0) noLight=true;
    alignas(16) float color[4];std::memcpy(color,s->color,16);
    if(global) color[3]=1.0f;
    else if(p->flag13 || p->flag14) color[3]=s->color[3]*s->opacity;
    else color[3]=CallGameMethod<float,0x006FCB60>(s);
    auto* material=model->materials[index];
    if(color[3]<0.001f) {
        result=CallGameMethod<uintptr_t,0x0068EF50>(material);
        if(!uint8_t(result)) return result;
    }
    alignas(16) Descriptor descriptor;
    InitDescriptor(descriptor,s,index);
    if(s->instance) std::memcpy(descriptor.constants,s->instance->constants,16);
    else std::memset(descriptor.constants,0,16);
    descriptor.noLight=uint8_t(noLight);descriptor.different=uint8_t(different);
    descriptor.lightState=s->resource.light?CallGameMethod<uint8_t,0x006AC560>(s->resource.light):0;
    std::memcpy(descriptor.color,color,16);
    descriptor.data18=p->data18;descriptor.option=uint8_t((s->resource.flags>>1)&1);
    descriptor.inverse=1.0f-Read<float>(s->pad1e0,0);
    descriptor.translucent=uint8_t(color[3]<1.0f);descriptor.fade=s->state2ac;
    descriptor.fadeValue=float(double(s->fade)*-0.000000059604652f);
    descriptor.lightingState=CallGameMethod<uint8_t,0x006B7360>(lighting);
    descriptor.modelState=uint8_t(CallGameMethod<int32_t,0x009158F0>(s)<0 || *CallGameMethod<uint32_t*,0x006FAC60>(s)!=0);
    if(p->flags&0x100) {
        void* properties=CallGameMethod<void*,0x0068ECF0>(material);
        descriptor.lightmap=CallGameMethod<void*,0x0079A550>(properties,"CASLightMap");
    } else if(p->flags&0x10000) descriptor.lightmap=nullptr;
    if(CallGameMethod<uint8_t,0x0068EEF0>(material)==1) {
        bool allowed=(s->renderFlags&0x400)==0;
        uint8_t hasInfo=Read<uint8_t>(info,48);
        if(allowed && s->secondary) allowed=(CallGameMethod<uint32_t,0x006C93D0>(s->secondary)&0x400)==0;
        void* special=CallGameMethod<void*,0x00F20360>(material);
        if(special && p->special) {
            if(allowed && s->instance && !hasInfo) {
                auto* matrices=static_cast<Matrix*>(InstanceMatrices(s));
                if(matrices) {
                    Matrix bone=matrices[s->bone];Matrix scratch;
                    auto* transform=CallGameMethod<Matrix*,0x006FAF40>(s,&scratch);
                    Matrix out=Multiply(bone,*transform);
                    return CallGameMethod<uintptr_t,0x006FAEC0>(p->special,special,&out);
                }
            }
            Matrix scratch;void* transform=CallGameMethod<void*,0x006FAF40>(s,&scratch);
            return CallGameMethod<uintptr_t,0x006FAEC0>(p->special,special,transform);
        }
    }
    if(p->reflection && (s->renderFlags&0x1000000)) CallGameFunction<void,0x00725370>(model,p->reflection,&descriptor);
    if(!p->flag15 && p->tick!=0xffffffff) {
        if(p->tick-s->tick>ImageWord(0x0115B1F0)) {
            if(!(ImageWord(0x0115B1E8)&s->renderFlags) && p->budget>ImageWord(0x0115B1EC)) return p->tick;
            ++p->budget;
        }
        s->tick=p->tick;
    }
    return CallGameVirtual<uintptr_t,37>(s,model,&descriptor,p);
}
}
#if defined(_M_IX86)
__declspec(naked) uintptr_t __fastcall Submit(SceneModel*,void*,uint32_t,Packet*) {
    __asm {
        push eax
        push dword ptr [esp+12]
        push dword ptr [esp+12]
        call SubmitBody
        ret 8
    }
}
#else
uintptr_t __fastcall Submit(SceneModel* s,void* edx,uint32_t index,Packet* p) { return SubmitBody(s,edx,index,p,0); }
#endif
}
