#include "ts3/anim/m05_M130.h"
#include <intrin.h>
#include <cstring>
#include <cmath>
#include <xmmintrin.h>
namespace ts3::anim::m130 {
namespace {
void Add(Reference* p) { if(p) _InterlockedIncrement(reinterpret_cast<volatile long*>(&p->refs)); }
void Drop(Reference* p) {
    if(p && !_InterlockedDecrement(reinterpret_cast<volatile long*>(&p->refs))) {
        _InterlockedExchange(reinterpret_cast<volatile long*>(&p->refs),1);
        CallGameVirtual<void,0>(p,1);
    }
}
void DropController(Reference* p) {
    if(p && --p->refs==0) { p->refs=1; CallGameVirtual<void,0>(p,1); }
}
void Assign(Reference*& dst,Reference* src) {
    if(dst!=src) { Add(src); auto* old=dst;dst=src;Drop(old); }
}
void AssignGame(Reference*& dst,Reference* src) {
    CallGameMethod<void,0x00873360>(&dst,&src);
}
struct Scene {
    void** vtable; volatile int32_t refs; void** secondary; uint32_t flags;
    void *field10,*field14,*field18; uint32_t untouched1c;
};
TS3_X86_SIZE(Scene,32);
Scene* NewScene(const char* name) {
    auto* p=CallGameFunction<Scene*,0x004e3f90>(sizeof(Scene),name,0,0,0,0);
    if(p) {
#if defined(_M_IX86)
        p->vtable=reinterpret_cast<void**>(0x00ff39f8);
        _InterlockedExchange(reinterpret_cast<volatile long*>(&p->refs),0);
        p->secondary=reinterpret_cast<void**>(0x00fa1b84);
        p->vtable=reinterpret_cast<void**>(0x00fcb7ec);
        p->secondary=reinterpret_cast<void**>(0x00fcb7dc);
#else
        NotRewritten("0x00849D40 native scene vtables");
#endif
        p->flags=0x40000001;p->field10=p->field14=p->field18=nullptr;
        CallGameVirtual<void,1>(p);
    }
    return p;
}
void SetId(Scene* p,void* entity) {
    auto id=CallGameVirtual<uint64_t,4>(entity);
    CallGameMethod<void,0x00581f40>(p,uint32_t(id),uint32_t(id>>32));
}
void* Slave(Body* self,int i) { return CallGameMethod<void*,0x00839ca0>(self->host,i); }
void Copy(Style& dst,const Style& src) { CallGameMethod<void,0x00757690>(&dst,&src); }
void Destroy(Style& s) { CallGameMethod<void,0x00754060>(&s); }
void Select(void* manager,uint32_t key,Style& s) {
    CallGameMethod<bool,0x00844980>(manager,key,s.values[1],s.values[2],s.values[3],s.values[4],&s,1);
}
void* Manager() {
#if defined(_M_IX86)
    return *reinterpret_cast<void**>(0x011d9140);
#else
    NotRewritten("0x00849D40 manager");
#endif
}
float Constant(uint32_t address) {
#if defined(_M_IX86)
    return *reinterpret_cast<float*>(address);
#else
    (void)address;NotRewritten("0x00849D40 animation tuning globals");
#endif
}
float Sub(float a,float b) { return _mm_cvtss_f32(_mm_sub_ss(_mm_set_ss(a),_mm_set_ss(b))); }
float Plus(float a,float b) { return _mm_cvtss_f32(_mm_add_ss(_mm_set_ss(a),_mm_set_ss(b))); }
// The original tests the sign bit of (-0 - x), not an ordered max(x,0).
float Clamp(float x) {
    float neg=Sub(-0.0f,x);uint32_t bits,n;std::memcpy(&bits,&x,4);std::memcpy(&n,&neg,4);
    bits &= uint32_t(int32_t(n)>>31);std::memcpy(&x,&bits,4);return x;
}
void AdjustTime(Body* self,int i,Request& request) {
    void* slave=Slave(self,i);
    const bool special=CallGameVirtual<int,6>(slave)==2;
    const uint32_t kind=self->styles[0].values[3];
    float bias=0;bool reverse=false;
    if(special) bias=Constant(0x0116636c);
    else if(kind==512) {bias=Constant((1u<<(self->styles[0].values[1]&31))==4?0x01166380:0x0116637c);reverse=true;}
    else if(kind==1024 || kind==1280) {
        bias=Constant(kind==1024?0x01166370:((1u<<(self->mode&31))==4?0x01166378:0x01166374));
    } else if(self->kind==3584) {bias=Constant(0x01166384);reverse=true;}
    else if(self->kind==17920) return;
    else bias=Constant(0x0107a538);
    auto* host=CallGameMethod<Reference*,0x00c93970>(self->host);Add(host);
    // Fetch the slave again: these lookups/callbacks can be stateful.
    uint64_t id=CallGameVirtual<uint64_t,4>(Slave(self,i));
    float from=static_cast<float>(CallGameVirtual<double,41>(host,uint32_t(id),uint32_t(id>>32)));
    id=CallGameVirtual<uint64_t,4>(self->host);
    float to=static_cast<float>(CallGameVirtual<double,41>(host,uint32_t(id),uint32_t(id>>32)));
    if(special) {
        const float difference=Sub(from,to);
        float value=Clamp(Sub(request.time,Sub(difference,bias)));
        const float absolute=std::fabs(difference);
        // Upper bound uses x87 extended addition; lower bound is rounded SSE.
        if(double(absolute)>double(bias)+double(Constant(0x00faa20c)) || Sub(bias,Constant(0x00faa20c))>absolute)
            request.time=value;
    } else if(reverse) request.time=Clamp(Plus(Sub(Sub(to,from),bias),request.time));
    else request.time=Clamp(Sub(request.time,Sub(Sub(from,to),bias)));
    if(special) Drop(host);
    else CallGameMethod<void,0x007460f0>(&host);
}
}
bool Build(Body* self,Reference*** outputs,int action,void* slave,float time,uintptr_t arg6,uintptr_t arg7,uintptr_t arg8,uintptr_t arg9,Reference* nameSpace,bool option) {
    void* manager=Manager();
    Request request{slave,time,arg6,arg7,arg8,arg9,nameSpace,uint8_t(option)};
    auto* master=NewScene("Locomotion/AnimatedSimBody/MasterSceneEntity");SetId(master,self->host);
    auto finish=[&](bool value) {if(master) CallGameVirtual<void,2>(master);Drop(request.nameSpace);return value;};
    for(int32_t i=0;i<self->count;++i) {
        if(!self->slaves[i]) continue;
        request.slave=self->slaves[i];
        const bool direct=self->direct || (action==4 && self->styles[i].flags[5]);
        auto* memory=CallGameFunction<void*,0x004e3f90>(24,"Locomotion/AnimatedSimBody/NamespaceMap",0,0,0,0);
        auto* map=memory?CallGameMethod<Reference*,0x00582010>(memory):nullptr;Add(map);
        auto* entity=NewScene("Locomotion/AnimatedSimBody/SlaveSceneEntity");SetId(entity,Slave(self,i));
        auto bind=[&](uint32_t a,uint32_t b) {
            CallGameMethod<void,0x00582090>(map,a,entity);
            CallGameMethod<void,0x00582090>(map,b,master);
        };
        if(direct) {
            if(self->kind==17920) {bind(0x050c5d67,0x977486ef);Assign(request.nameSpace,map);}
            else if(self->kind==3584) {bind(0x050c5d67,0xaf93abb8);AssignGame(request.nameSpace,map);}
        } else {
            switch(self->kind) {
            case 512:bind(0x050c5d66,0x050c5d67);Assign(request.nameSpace,map);break;
            case 3840:bind(0x050c5d67,0xb00de1e1);AssignGame(request.nameSpace,map);break;
            case 3328:bind(0x050c5d67,0x8ef0723d);AssignGame(request.nameSpace,map);break;
            case 17920:bind(0x050c5d67,0x977486ef);AssignGame(request.nameSpace,map);break;
            }
        }
        if(entity) CallGameVirtual<void,2>(entity);Drop(map);
        Style style;Copy(style,self->styles[i]);
        Reference* selected=nullptr;bool adjust=false;
        if(self->progression && (self->progression[i].end-self->progression[i].begin)!=0)
            Select(manager,self->progression[i].begin[0].key,style);
        auto select=[&](Reference* p){AssignGame(selected,p);};
        bool valid=true;
        switch(action) {
        case 0:select(self->styles[i].controllers[0]);break;
        case 1:select(style.controllers[7]);break;
        case 2:select(style.controllers[1]);adjust=true;break;
        case 3:select(self->styles[i].controllers[2]);adjust=true;break;
        case 4:
            select(style.controllers[3]);adjust=true;
            if(option) {
                Style second;Copy(second,style);
                if(self->progression && self->current<CallGameMethod<uint32_t,0x0074b990>(&self->progression[i]))
                    Select(manager,self->progression[i].begin[self->current].key,second);
                CallGameFunction<void,0x00848760>(self->primary,1.0f,int(!second.flags[1]));
                if(second.controllers[4] && !CallGameVirtual<bool,4>(second.controllers[4])) select(second.controllers[4]);
                else if(second.controllers[3] && !CallGameVirtual<bool,4>(second.controllers[3])) select(second.controllers[3]);
                else if(style.controllers[4] && !CallGameVirtual<bool,4>(style.controllers[4])) select(style.controllers[4]);
                CallGameFunction<void,0x00848760>(self->slaves[i],1.0f,int(!style.flags[1]));
                if(style.controllers[4] && !CallGameVirtual<bool,4>(style.controllers[4])) select(style.controllers[4]);
                Destroy(second);
            }
            break;
        case 5:select(self->styles[i].controllers[7]);break;
        case 7:select(self->styles[i].controllers[6]);break;
        case 8:select(self->styles[i].controllers[5]);break;
        case 9:case 10: {
            uint32_t index=self->current;
            if(action==9) {index+=1;if(self->target<=self->current) valid=false;}
            else if(!CallGameMethod<bool,0x00848590>(self)) valid=false;
            if(valid && (self->target>=uint32_t(self->end-self->begin) || !self->progression)) valid=false;
            if(valid && index>=CallGameMethod<uint32_t,0x0074b990>(&self->progression[i])) valid=false;
            if(valid) {
                Style second;Copy(second,self->styles[i]);
                Select(manager,self->progression[i].begin[index].key,second);
                select(second.controllers[action==9?10:11]);adjust=true;Destroy(second);
            }
            break;
        }
        default:break;
        }
        if(!valid) {Destroy(style);CallGameMethod<void,0x007460f0>(&selected);return finish(false);}
        if(direct && adjust && self->count==1) AdjustTime(self,i,request);
        struct Result {Reference* controller;uint8_t flag; } result{nullptr,0};
        bool ok=selected && CallGameVirtual<bool,3>(selected,&request,&result);
        if(!ok || !result.controller) {
            for(int j=0;j<i;++j) if((*outputs)[j] && self->slaves[j]) {DropController((*outputs)[j]);(*outputs)[j]=nullptr;}
            DropController(result.controller);Destroy(style);Drop(selected);return finish(false);
        }
        if(direct) {(*outputs)[i]=result.controller;++(*outputs)[i]->refs;}
        else {
            void* raw=CallGameFunction<void*,0x004e3f90>(96,"Animation/RootMotionController",0,0,0,0);
            auto* root=raw?CallGameMethod<Reference*,0x0056caa0>(raw,self->slaves[i],result.controller):nullptr;
            if(root) ++root->refs;
            (*outputs)[i]=root;
            // Deliberately no null check: EA faults on allocation failure here.
            ++(*outputs)[i]->refs;
            DropController(root);
        }
        DropController(result.controller);Destroy(style);Drop(selected);
    }
    return finish(true);
}
bool RunNoActive(Body* self,Reference* ns) {return Build(self,nullptr,0,nullptr,0,0,0,0,0,ns,false);}
#if defined(_M_IX86)
bool __fastcall HookBuild(Body* s,void*,Reference*** out,int action,void* slave,float time,uintptr_t a6,uintptr_t a7,uintptr_t a8,uintptr_t a9,Reference* ns,bool option) {
    return Build(s,out,action,slave,time,a6,a7,a8,a9,ns,option);
}
#endif
}
