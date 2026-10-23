#include "ts3/render/m07_B27.h"
#include <xmmintrin.h>
#include <cstring>
namespace ts3::render::m07::b27 {
Dependencies dependencies;
namespace {
template<class T> T Data(uint32_t ea,T injected) {
    if(injected) return injected;
#if defined(_M_IX86)
    return reinterpret_cast<T>(uintptr_t(ea));
#else
    throw std::runtime_error("m07_B27 requires native data injection");
#endif
}
__m128 Load(const float* p) { return _mm_loadu_ps(p); }
void Store(float* p,__m128 v) { _mm_storeu_ps(p,v); }
__m128 Splat(__m128 v,int lane) {
    switch(lane) { case 1:return _mm_shuffle_ps(v,v,0x55);case 2:return _mm_shuffle_ps(v,v,0xaa);default:return _mm_shuffle_ps(v,v,0); }
}
__m128 Length(__m128 v) {
    auto q=_mm_mul_ps(v,v);
    auto sum=_mm_add_ps(_mm_add_ps(_mm_shuffle_ps(q,q,2),_mm_shuffle_ps(q,q,1)),q);
    return _mm_sqrt_ps(_mm_shuffle_ps(sum,sum,0));
}
float Distance(Owner* s,const float* p) {
    alignas(16) float t[16];
    auto* result=CallThis(Original(0x006FAF40,dependencies.transform),s,t);
    auto delta=_mm_sub_ps(Load(result+12),Load(p));
    auto absolute=_mm_max_ps(_mm_mul_ps(_mm_set1_ps(-1.0f),delta),delta);
    auto half=_mm_mul_ps(_mm_sub_ps(Load(s->box+4),Load(s->box)),_mm_set1_ps(0.5f));
    auto distance=_mm_max_ps(_mm_sub_ps(absolute,half),_mm_setzero_ps());
    return _mm_cvtss_f32(Length(distance));
}
void FillUpload(Instance* current,uint32_t count,uint8_t* output) {
    uint8_t* cursor=output;
    for(uint32_t i=0;i<count;++i,cursor+=28) {
        alignas(16) float temp[4];
        auto* p=CallThis(Original(0x00693020,dependencies.convert3),static_cast<void*>(temp),static_cast<const float*>(current[i].data+12));
        // These three copies use x87 in EA and quiet signaling NaNs.
        for(int j=0;j<3;++j) {
            uint32_t bits;std::memcpy(&bits,p+j,4);
            if((bits&0x7f800000u)==0x7f800000u && (bits&0x007fffffu)) bits|=0x00400000u;
            std::memcpy(cursor+j*4,&bits,4);
        }
        p=CallThis(Original(0x00693020,dependencies.convert3),static_cast<void*>(temp),static_cast<const float*>(current[i].data+16));
        std::memcpy(cursor+12,p,12);
        float enabled=current[i].enabled?1.0f:0.0f;std::memcpy(cursor+24,&enabled,4);
    }
}
bool Upload(Instance* current,uint32_t count,uint8_t* output) {
#if defined(_M_IX86)
    __try { FillUpload(current,count,output); return true; }
    __except(1) { return false; }
#else
    try { FillUpload(current,count,output);return true; } catch(...) {return false;}
#endif
}
Owner* Primary(void* p) { return reinterpret_cast<Owner*>(static_cast<uint8_t*>(p)-offsetof(Owner,secondary)); }
void CopyBounds(Owner* s,float* box,float* matrix) {
    Store(box,Load(s->box));Store(box+4,Load(s->box+4));
    for(int i=0;i<16;i+=4) Store(matrix+i,Load(s->matrix+i));
}
}
uintptr_t __fastcall Distance4(Owner* s,void*,void*,void* b,const float* p,void* d) {
    return CallThis(Original(0x006F7300,dependencies.distance4),s,Distance(s,p),b,p,d);
}
uintptr_t __fastcall Distance3(Owner* s,void*,void*,const float* p,void* d) {
    return CallThis(Original(0x006F7410,dependencies.distance3),s,Distance(s,p),p,d);
}
Owner* __fastcall SelectBounds(Owner* s,void*,float* box,float* matrix) {
    // Keep the load/store ordering: outputs may alias the model's own fields.
    Store(box,Load(s->box)); Store(box+4,Load(s->box+4));
    Store(matrix,Load(s->matrix)); auto row0=Load(matrix);
    Store(matrix+4,Load(s->matrix+4)); Store(matrix+8,Load(s->matrix+8));
    auto row3=Load(s->matrix+12); Store(matrix+12,row3);
    auto center=_mm_mul_ps(_mm_add_ps(Load(box+4),Load(box)),_mm_set1_ps(0.5f));
    alignas(16) float point[4];
    Store(point,_mm_add_ps(_mm_add_ps(_mm_add_ps(_mm_mul_ps(Splat(center,0),row0),row3),_mm_mul_ps(Splat(center,1),Load(matrix+4))),_mm_mul_ps(Splat(center,2),Load(matrix+8))));
    auto* result=CallThis(Original(0x006F82B0,dependencies.select),s,static_cast<const float*>(point));
    if(result) CopyBounds(result,box,matrix);
    return result;
}
uintptr_t __fastcall Refresh(Owner* s,void*) {
    CallThis(Original(0x006F7B40,dependencies.baseRefresh),s);
    if(!s->ready) CallThis(Original(0x006F8670,dependencies.fixup),s);
    auto result=CallThis(Original(0x006DB810,dependencies.pending),static_cast<void*>(s->callbacks));
    if(!result) {
        auto fn=reinterpret_cast<Thiscall<uintptr_t,Owner,uint32_t>>(s->table[38]);
        return CallThis(fn,s,0u);
    }
    return result;
}
uintptr_t __fastcall ProxyName(void*,void*,void* output) {
    const char* name="SceneModelArrayFixupProxy";
    return CallThis(Original(0x00405790,dependencies.name),output,name,name+25);
}
uintptr_t __fastcall Draw(Owner* s,void*,void* target,const void* input,const uint32_t* flags) {
    uintptr_t result=dependencies.initialReturn;
#if defined(_M_IX86)
    if(!dependencies.initialReturn) { __asm { mov eax,fs:[0] } __asm { mov result,eax } }
#endif
    if(s->flags40&0x1000) return result;
    void* system=*Data(0x011D1530,dependencies.systemSlot);
    void* device=Original(0x0060E2C0,dependencies.device)();
    if(!s->ready) CallThis(Original(0x006F8670,dependencies.fixup),s);
    uint32_t shifted=flags[1]>>3;
    result=(shifted&0xffffff00u)|(shifted&1u);
    bool alternate=(shifted&1)!=0;
    alignas(16) uint8_t descriptor[96];
    if(!(s->flags40&0x10)) {
        result=CallThis(Original(0x0060F1C0,dependencies.supported),*Data(0x011CECC8,dependencies.hostSlot));
        if(uint8_t(result)) {
            auto* current=s->begin;auto* end=s->end;bool subsequent=false;
            while(current!=end) {
                uint32_t count=uint32_t((reinterpret_cast<intptr_t>(end)-reinterpret_cast<intptr_t>(current))/96);
                uint8_t* output;uint32_t offset;
                int32_t status=CallThis(Original(0x006E0F00,dependencies.allocate),system,28u,&count,&s->revision,&output,&offset);
                if(status==2) return 0;
                bool valid=true;
                if(status==0) {
                    valid=Upload(current,count,output);
                    CallThis(Original(0x006E0FE0,dependencies.unlock),system,28u*count);
                }
                if((status==0 && valid) || status==1) {
                    // Only the original renderer-system buffer pointer at +104 is read.
                    struct SystemView {uint8_t prefix[0x104];void* buffer;};
                    void* buffer=static_cast<SystemView*>(system)->buffer;
                    CallThis(Original(0x0060F020,dependencies.stream),device,1u,buffer,offset,28u);
                    CallThis(Original(0x006F8490,dependencies.descriptor),static_cast<void*>(descriptor),input);
                    uint32_t index=s->indices[alternate?3:1];std::memcpy(descriptor+36,&index,4);std::memcpy(descriptor+40,&count,4);
                    CallThis(Original(0x006DAC50,dependencies.submit),target,static_cast<const void*>(descriptor));
                    current+=count;
                    CallThis(Original(0x0060F020,dependencies.stream),device,1u,static_cast<void*>(nullptr),0u,0u);
                }
                if(subsequent) s->revision=0;
                subsequent=true;
            }
            return reinterpret_cast<uintptr_t>(current);
        }
    }
    if(s->instancer) {
        uint32_t count=uint32_t((reinterpret_cast<intptr_t>(s->end)-reinterpret_cast<intptr_t>(s->begin))/96);
        CallThis(Original(0x00696B80,dependencies.resize),s->instancer,count);
        uint32_t i=0;
        for(auto* p=s->begin;p!=s->end;++p,++i) {
            CallThis(Original(0x00696C30,dependencies.position),s->instancer,i,static_cast<const float*>(p->data+12));
            alignas(16) float color[4];Store(color,Load(p->data+16));color[3]=p->enabled?1.0f:0.0f;
            CallThis(Original(0x00696C60,dependencies.color),s->instancer,i,static_cast<const float*>(color));
        }
        CallThis(Original(0x006F8490,dependencies.descriptor),static_cast<void*>(descriptor),input);
        uint32_t index=s->indices[alternate?3:1];std::memcpy(descriptor+36,&index,4);
        return CallThis(Original(0x00696C90,dependencies.fallbackSubmit),s->instancer,static_cast<const void*>(descriptor));
    }
    return result;
}
uintptr_t __stdcall ArrayName(void* output) {
    const char* name="SceneModelArray";
    return CallThis(Original(0x00405790,dependencies.name),output,name,name+15);
}
uintptr_t __fastcall Destroy(Owner* s,void*) {
    s->table=Data(0x00FF99D8,dependencies.primaryTable);s->secondary=Data(0x00FF99B8,dependencies.secondaryTable);
    CallThis(Original(0x006DD530,dependencies.unsubscribe),static_cast<void*>(s->callbacks),0x0588D61Cu);
    if(s->instancer) {
        CallThis(Original(0x006972A0,dependencies.instanceDestroy),s->instancer);
        Original(0x004E3F70,dependencies.free)(s->instancer); s->instancer=nullptr;
    }
    auto table=Data(0x00FA1B78,dependencies.baseTable); s->callbackTable=table;
    CallThis(Original(0x005ED140,dependencies.listDestroy),static_cast<void*>(&s->sentinel));
    if(s->begin) Original(0x004E3F70,dependencies.free)(s->begin);
    s->secondary=table;
    return CallThis(Original(0x006F5C40,dependencies.baseDestroy),s);
}
uintptr_t __fastcall QuerySecondary(void* s,void*,uint32_t id) {
    return CallThis(Original(0x006F8070,dependencies.query),Primary(s),id);
}
uintptr_t __fastcall AddRefSecondary(void* s,void*) {
    return CallThis(Original(0x00692720,dependencies.addref),static_cast<void*>(Primary(s)));
}
uintptr_t __fastcall ReleaseSecondary(void* s,void*) {
    return CallThis(Original(0x005044C0,dependencies.release),static_cast<void*>(Primary(s)));
}
bool __fastcall RemoveCallback(Owner* s,void*,void* key) {
    Node* found;
    Original(0x006F92F0,dependencies.find)(&found,s->sentinel.next,&s->sentinel,key);
    if(found==&s->sentinel) return false;
    Original(0x006DD270,dependencies.erase)(&found,found,&s->sentinel);
    return true;
}
Owner* __fastcall Construct(Owner* s,void*,void* a,void* b) {
    CallThis(Original(0x006F5B40,dependencies.baseConstruct),s,a,b);
    s->secondary=Data(0x00FF99B8,dependencies.secondaryTable);s->table=Data(0x00FF99D8,dependencies.primaryTable);
    s->begin=s->end=s->capacity=nullptr;s->tag=Data(0x00FF9AA8,dependencies.instanceTag);s->ready=0;
    s->sentinel.value=const_cast<char*>(Data(0x00FF9A7C,dependencies.callbackTag));s->sentinel.next=s->sentinel.prev=&s->sentinel;
    s->callbackOwner=&s->secondary;s->callbackTable=Data(0x00FF9970,dependencies.callbackVtable);s->revision=0;
    CallThis(Original(0x0068BB50,dependencies.emptyBox),static_cast<void*>(s->instanceBox));
    CallThis(Original(0x006DD600,dependencies.subscribe),static_cast<void*>(s->callbacks),static_cast<void*>(&s->callbackTable),0x0588D61Cu,1u);
    s->flags44|=0x80;s->instancer=nullptr;
    s->indices[0]=0;s->indices[1]=1;s->indices[2]=2;s->indices[3]=3;
    return s;
}
uintptr_t __fastcall RebuildBounds(Owner* s,void*,void* model) {
    auto empty=Original(0x0068BB50,dependencies.emptyBox);
    auto include=Original(0x0068BAC0,dependencies.includePoint);
    auto includeBox=Original(0x0068BA30,dependencies.includeBox);
    auto maxScale=_mm_setzero_ps();
    CallThis(empty,static_cast<void*>(s->instanceBox));
    for(auto* p=s->begin;p!=s->end;++p) {
        CallThis(include,static_cast<void*>(s->instanceBox),static_cast<const float*>(p->data+4));
        maxScale=_mm_max_ps(maxScale,Splat(Load(p->data+8),1));
    }
    auto half=_mm_set1_ps(0.5f);
    auto center=_mm_mul_ps(_mm_add_ps(Load(s->instanceBox+4),Load(s->instanceBox)),half);
    alignas(16) float matrix[16];
    Store(matrix,_mm_set_ps(0,0,0,1));
    Store(matrix+8,_mm_set_ps(0,1,0,0));
    // The +B4 stack store follows a PUSH: relative to the matrix it is +10.
    Store(matrix+4,_mm_set_ps(0,0,1,0));
    Store(matrix+12,center); matrix[15]=1.0f;
    Store(s->instanceBox,_mm_sub_ps(Load(s->instanceBox),center));
    Store(s->instanceBox+4,_mm_sub_ps(Load(s->instanceBox+4),center));
    CallThis(Original(0x006F51C0,dependencies.setTransform),s,static_cast<const float*>(matrix));
    alignas(16) float box[8],point[4];
    CallThis(empty,static_cast<void*>(box));
    uint32_t count=model?CallThis(Original(0x006C93D0,dependencies.lodCount),model):0;
    for(uint32_t i=0;i<count;++i) {
        // The model's LOD pointer vector is at +40 in the original layout.
        auto** lods=*reinterpret_cast<void***>(static_cast<uint8_t*>(model)+0x40);
        auto* source=CallThis(Original(0x0068EE20,dependencies.lodBox),lods[i]);
        auto* target=s->lodBoxes+16*i;
        Store(target,Load(source));Store(target+4,Load(source+4));
        if(target[4]>=target[0]) {
            auto low=Load(target),delta=_mm_sub_ps(Load(target+4),low);
            auto expand=_mm_mul_ps(_mm_sub_ps(Length(delta),delta),_mm_set_ps(0.5f,0.5f,0.0f,0.5f));
            auto wide=_mm_add_ps(_mm_mul_ps(_mm_mul_ps(Splat(delta,0),half),maxScale),expand);
            auto adjustment=_mm_shuffle_ps(expand,wide,0x50);
            adjustment=_mm_shuffle_ps(adjustment,expand,0xe8);
            Store(target,_mm_add_ps(_mm_sub_ps(Load(s->instanceBox),adjustment),low));
            Store(target+4,_mm_add_ps(_mm_add_ps(adjustment,Load(s->instanceBox+4)),Load(target+4)));
            CallThis(includeBox,static_cast<void*>(box),static_cast<const float*>(target));
        }
    }
    float epsilon=*Data(0x0115B230,dependencies.epsilon);
    Store(point,_mm_set1_ps(epsilon));CallThis(include,static_cast<void*>(box),static_cast<const float*>(point));
    Store(point,_mm_sub_ps(_mm_setzero_ps(),_mm_set1_ps(epsilon)));CallThis(include,static_cast<void*>(box),static_cast<const float*>(point));
    CallThis(Original(0x006FCE80,dependencies.setBox),s,static_cast<const float*>(box));
    alignas(16) float transform[16];auto* t=CallThis(Original(0x006FAF40,dependencies.transform),s,transform);
    CallThis(Original(0x006FCD80,dependencies.updateBounds),static_cast<void*>(&s->flags40),static_cast<const float*>(t));
    return CallThis(Original(0x006FCC50,dependencies.finalize),s);
}
}
