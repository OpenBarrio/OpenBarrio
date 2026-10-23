#include "ts3/render/m07_M12.h"
#include "ts3/shader/clu_71b550_Shader.h"
#include <cstring>
#include <intrin.h>
namespace ts3::render::m07::m12 {
namespace {
template<class T> T& F(void* p, size_t n) { return *reinterpret_cast<T*>(static_cast<uint8_t*>(p)+n); }
#if defined(_M_IX86)
template<class T> T& G(uint32_t a) { return *reinterpret_cast<T*>(uintptr_t(a)); }
void* Image(uint32_t a) { return reinterpret_cast<void*>(uintptr_t(a)); }
uintptr_t Yes(uintptr_t v) { return (v & ~uintptr_t(255)) | 1; }
float Sin(float x) { float r;
    __asm { fld x }
    __asm { fsin }
    __asm { fstp r }
    return r;
}
float Cos(float x) { float r;
    __asm { fld x }
    __asm { fcos }
    __asm { fstp r }
    return r;
}
void* Find(void* system, uint32_t lo,uint32_t hi) {
    return CallGameFunction<void*,0x006DF0C0>(CallGameMethod<void*,0x006FE3B0>(system,lo,hi));
}
uintptr_t Refresh(void* system,uint32_t lo,uint32_t hi) { return CallGameMethod<uintptr_t,0x00703430>(system,lo,hi); }
void Clear(void* object) {
    alignas(16) uint8_t state[144]; CallGameMethod<void,0x006FAE00>(state); F<uint32_t>(state,0)=0;
    CallGameMethod<void,0x006FC860>(object,0,state);
}
void* Allocate(uint32_t bytes,uint32_t tag) { return CallGameFunction<void*,0x004E3F90>(bytes,Image(tag),0,0,0,0); }
#endif
}
uintptr_t __fastcall Dispatch(void* self,void*,uint32_t message,void* data) {
#if defined(_M_IX86)
    uintptr_t result=message;
    if(message==0x038E3C0E || message==0x038E3C6C || message==0x9B8E0967) {
        void* p=G<void*>(0x011D2E50);
        if(message==0x038E3C0E) result=0;
        if(message==0x038E3C6C) result=0;
        if(p) { p=CallGameVirtual<void*,25>(p); result=uintptr_t(p);
            if(p) result=CallGameVirtual<uintptr_t,4>(p,uint32_t(message==0x038E3C6C?4:0)); }
        return Yes(result);
    }
    if(message==0xAE1CFE73) return Yes(result);
    if(message==0xE73A6138) return Yes(CallGameMethod<uintptr_t,0x00718840>(static_cast<uint8_t*>(data)+16,F<uint32_t>(data,8),F<uint32_t>(data,12)));
    if(message==0x9EC35DDD) return Yes(CallGameMethod<uintptr_t,0x00718890>(static_cast<uint8_t*>(data)+16,F<uint32_t>(data,8),F<uint32_t>(data,12)));
    if(message==0xB390246D) {
        void* p=CallGameFunction<void*,0x004E3FB0>(704,16,0,Image(0x00FFC468),0,0,0,0);
        if(p) p=CallGameMethod<void*,0x00718B70>(p,F<uint32_t>(data,8),F<uint32_t>(data,12));
        void* holder; result=CallGameMethod<uintptr_t,0x0074B950>(&holder,p);
        if(holder) {
            void* system=CallGameFunction<void*,0x006FDE10>();
            if(system && CallGameMethod<uint8_t,0x00700CB0>(system,holder) && !CallGameMethod<uint8_t,0x00718960>(holder,F<uint32_t>(data,16),F<uint32_t>(self,12)))
                Refresh(system,F<uint32_t>(data,8),F<uint32_t>(data,12));
            result=CallGameVirtual<uintptr_t,1>(holder);
        }
        return Yes(result);
    }
    if(message==0x5488D5D3) {
        result=uintptr_t(data); void* source=F<void*>(data,16);
        if(!source || F<uint32_t>(data,8)) return Yes(result);
        void* id=CallGameVirtual<void*,26>(source,9,0); result=uintptr_t(id); if(!id) return Yes(result);
        uint32_t lo=F<uint32_t>(id,0),hi=F<uint32_t>(id,4);
        void* system=CallGameFunction<void*,0x006FDE10>(); result=uintptr_t(system); if(!system) return Yes(result);
        void* object=Find(system,lo,hi); result=uintptr_t(object); if(!object) return Yes(result);
        uint32_t kind=F<uint32_t>(object,540);
        if(kind) {
            void* service=CallGameFunction<void*,0x007692C0>();
            if(service && kind<=3) {
                void* msg=Allocate(24,0x00FFC480);
                if(msg) {
                    F<uint32_t>(msg,16)=kind==1?106840347:(kind==2?236351822:236354871);
                    F<void*>(msg,0)=Image(0x0100DF80);
                    _InterlockedExchange(&F<volatile long>(msg,4),0);
                    F<void*>(msg,0)=Image(0x0100DF80);
                }
                void* retained=nullptr; CallGameMethod<void,0x005C9100>(&retained,msg);
                F<uint32_t>(retained,8)=lo; F<uint32_t>(retained,12)=hi;
                CallGameVirtual<void,7>(service,F<uint32_t>(retained,16),retained,0,0);
                CallGameVirtual<void,2>(retained);
            }
        }
        if(F<uint8_t>(object,532)) { Clear(object); result=Refresh(system,lo,hi); }
        return Yes(result);
    }
    // Rotation messages build the transform even if the renderer singleton is absent.
    alignas(16) uint8_t state[144];
    if(message==0x4D1010B9) {
        void* src=F<void*>(data,16); CallGameMethod<void,0x006FAE00>(state);
        F<uint32_t>(state,0)=F<uint32_t>(src,16);
        float sx=Sin(F<float>(src,48)),cx=Cos(F<float>(src,48));
        float sy=Sin(F<float>(src,52)),cy=Cos(F<float>(src,52));
        float sz=Sin(F<float>(src,56)),cz=Cos(F<float>(src,56));
        float* m=reinterpret_cast<float*>(state+16);
        m[0]=cz*cy; m[1]=sz*cy; m[2]=0.0f-sy; m[3]=0;
        m[4]=(cz*sx)*sy-sz*cx; m[5]=(sz*sx)*sy+cz*cx; m[6]=cy*sx; m[7]=0;
        m[8]=(cz*cx)*sy+sz*sx; m[9]=(sz*cx)*sy-cz*sx; m[10]=cy*cx; m[11]=0;
        std::memcpy(state+64,static_cast<uint8_t*>(src)+32,16);
        if(F<uint32_t>(state,0)==4) std::memcpy(state+80,static_cast<uint8_t*>(src)+64,64);
        else F<uint32_t>(state,80)=F<uint32_t>(src,20);
    }
    switch(message) {
    case 0x0D9046E6: case 0x0F9F4229: case 0x4D1010B9: case 0x735DAC73:
    case 0x9422103A: case 0xA6D1F26C: case 0xB35DAF22: case 0xCBC7FF16: break;
    default:
        if(message<=0x0D9046E6) result=message-0x038E3C6C;
        return result & ~uintptr_t(255);
    }
    void* system=CallGameFunction<void*,0x006FDE10>(); result=uintptr_t(system); if(!system) return Yes(result);
    if(message==0x0F9F4229 && !data) return Yes(result);
    void* object=Find(system,F<uint32_t>(data,8),F<uint32_t>(data,12)); result=uintptr_t(object); if(!object) return Yes(result);
    switch(message) {
    case 0x0D9046E6: result=CallGameMethod<uintptr_t,0x00718A20>(object,F<uint32_t>(data,16)); break;
    case 0x0F9F4229: F<uint32_t>(object,552)=F<uint32_t>(data,24); F<uint32_t>(object,556)=F<uint32_t>(data,28); break;
    case 0x4D1010B9: {void* src=F<void*>(data,16); result=CallGameMethod<uintptr_t,0x00718C40>(object,F<uint32_t>(src,8),F<uint32_t>(src,12),state); break;}
    case 0x735DAC73: result=CallGameMethod<uintptr_t,0x00719D40>(object,F<uint32_t>(data,16),F<uint32_t>(data,20)); break;
    case 0x9422103A: result=CallGameMethod<uintptr_t,0x0071A0F0>(object,uint32_t(F<uint32_t>(data,16)!=0)); break;
    case 0xB35DAF22: result=CallGameMethod<uintptr_t,0x007189E0>(object,F<uint32_t>(data,16)); break;
    case 0xCBC7FF16: result=CallGameMethod<uintptr_t,0x00719DF0>(object,F<uint32_t>(data,16)); break;
    case 0xA6D1F26C:
        result=CallGameVirtual<uintptr_t,4>(F<void*>(object,528));
        if(uint8_t(result)) F<uint8_t>(object,532)=1;
        else { Clear(object); result=Refresh(system,F<uint32_t>(data,8),F<uint32_t>(data,12)); }
        break;
    }
    return Yes(result);
#else
    (void)self;(void)message;(void)data; NotRewritten("m07_M12 Dispatch image dependencies");
#endif
}
uint8_t __fastcall AddRecord(void* self,void*,uint32_t key,void* data,uint32_t* index) {
#if defined(_M_IX86)
    if(F<uint8_t>(data,16)) return 0;
    uint32_t extra[2]={}; void* metadata=F<void*>(data,36);
    if(metadata && F<uint16_t>(metadata,150)==2) std::memmove(extra,static_cast<uint8_t*>(F<void*>(metadata,152))+4*F<uint16_t>(metadata,148),8);
    void* resource=nullptr; void* particle=nullptr;
    if(F<uint8_t>(data,13)==16) CallGameVirtual<void,3>(F<void*>(self,0x38B4),F<uint32_t>(data,0),F<uint32_t>(data,4),&particle);
    else {
        void* provider=F<void*>(data,8);
        if(provider) {
            void* p=CallGameVirtual<void*,3>(provider,uint32_t(0x00B3D7A7));
            if(p) resource=reinterpret_cast<void*>(uintptr_t(p)+40);
            if(resource) CallGameVirtual<void,1>(resource);
            else {
                p=CallGameVirtual<void*,3>(provider,uint32_t(0x01BE8CA6));
                if(p) { if(!F<void*>(p,4)) CallGameVirtual<void,4>(p); resource=F<void*>(p,4); }
                if(resource) CallGameVirtual<void,1>(resource);
            }
        } else if((F<uint32_t>(data,0)&F<uint32_t>(data,4))!=0xFFFFFFFF) {
            if(F<uint32_t>(data,0)==0x7F000000 && !F<uint32_t>(data,4)) {
                void* p=CallGameMethod<void*,0x00722F20>(static_cast<uint8_t*>(self)+0x3898);
                if(!p) return 0; resource=reinterpret_cast<void*>(uintptr_t(p)+40); if(!resource) return 0;
                CallGameVirtual<void,1>(resource);
            } else {
                uint32_t group=(F<uint32_t>(data,24)&F<uint32_t>(data,28))==0xFFFFFFFF?0:F<uint32_t>(data,24);
                CallGameVirtual<void,1>(F<void*>(self,0x38B4),F<uint32_t>(data,0),F<uint32_t>(data,4),group,&resource);
            }
        }
        if(!resource) {
            if(!F<uint8_t>(self,0x3894)) return 0;
            void* p=CallGameMethod<void*,0x00722F20>(static_cast<uint8_t*>(self)+0x3898);
            if(!p) return 0; resource=reinterpret_cast<void*>(uintptr_t(p)+40); if(!resource) return 0;
            CallGameVirtual<void,1>(resource);
        }
    }
    // The original deliberately does not initialize every field of these stack records.
    ts3::shader::ShaderParamRecord record;
    auto& r=record;
    r.d00=key; r.b04=F<uint8_t>(data,13)<18?F<uint8_t>(data,13):0; r.b05=F<uint8_t>(data,12);
    r.d08=uint32_t(uintptr_t(resource)); r.d20=F<uint32_t>(data,0);r.d24=F<uint32_t>(data,4);r.d28=uint32_t(uintptr_t(particle));
    r.w5C=F<uint16_t>(data,18);r.d68=extra[0];r.d6C=extra[1];
    uint16_t flags=F<uint16_t>(data,14);
    r.b70=uint8_t(flags)>>7;r.b71=flags&1;r.b72=(flags&32)==0;r.b73=(flags&64)!=0;r.b74=(flags>>8)&1;
    r.f2C=G<float>(0x0115B5C4);r.f30=r.f2C;r.f34=r.f2C;
    r.f38=-0.0f-r.f2C;r.f3C=r.f38;r.f40=r.f38;r.b44=1;
    std::memcpy(&r.f58,static_cast<uint8_t*>(data)+32,4);std::memcpy(&r.f60,static_cast<uint8_t*>(data)+20,4);
    r.d18=0xFFFFFFFF;
    if(!(G<uint32_t>(0x011D2F74)&1)) { G<uint32_t>(0x011D2F74)|=1;G<uint32_t>(0x011D2F70)=CallGameFunction<uint32_t,0x007CFE70>(Image(0x00FFC530)); }
    if(!(G<uint32_t>(0x011D2F74)&2)) { G<uint32_t>(0x011D2F74)|=2;G<uint32_t>(0x011D2F6C)=CallGameFunction<uint32_t,0x007CFE70>(Image(0x00FFC520)); }
    r.index=0;
    if(particle) {
        if(r.b05==17) {
            if(!F<void*>(self,200)) {
                void* vs=Allocate(20,0x00FFC50C);if(vs) vs=CallGameMethod<void*,0x00799760>(vs);
                uint8_t ok=CallGameVirtual<uint8_t,3>(vs,Image(0x0115B7B0))&1;
                CallGameMethod<void,0x0079ABE0>(F<void*>(F<void*>(G<void*>(0x011D753C),76),0),vs);
                G<uint32_t>(0x0115B758)=F<uint16_t>(vs,4);
                void* ps=Allocate(20,0x00FFC4F8);if(ps) ps=CallGameMethod<void*,0x007997E0>(ps);
                ok &= CallGameVirtual<uint8_t,3>(ps,Image(0x0115B5E8));
                CallGameMethod<void,0x0079ABE0>(F<void*>(F<void*>(G<void*>(0x011D753C),76),0),ps);
                G<uint32_t>(0x0115B75C)=F<uint16_t>(ps,4);
                if(ok) {
                    uint8_t stream[44], scratch[24];
                    CallGameMethod<void,0x004DF120>(stream,Image(0x0115B738),uint32_t(0x74),0,1,0,0);
                    CallGameMethod<void,0x007D91B0>(scratch,stream,1);
                    void* shader=Allocate(56,0x00FFC4E8);if(shader) shader=CallGameMethod<void*,0x0079A290>(shader,uint32_t(0xFFFF),0);
                    F<void*>(self,200)=shader;
                    CallGameMethod<void,0x0079A690>(shader,scratch,0);
                    void* pair=F<void*>(G<void*>(0x011D753C),76);
                    CallGameMethod<void,0x0079A570>(shader,F<void*>(pair,0),F<void*>(pair,4));
                    CallGameMethod<void,0x007D7760>(scratch);CallGameMethod<void,0x004559D0>(stream);
                }
                if(!F<void*>(self,200)) F<void*>(self,200)=CallGameMethod<void*,0x008AB2B0>(particle);
            }
            r.d0C=uint32_t(uintptr_t(F<void*>(self,200)));
        } else r.d0C=uint32_t(uintptr_t(CallGameMethod<void*,0x008AB2B0>(particle)));
        if(void* p=F<void*>(particle,84)) r.index=CallGameMethod<uint32_t,0x0071F4D0>(self,CallGameMethod<uint32_t,0x00444710>(p));
        r.d14=0;
    } else {
        uint32_t name=G<uint32_t>((flags&1)?0x011D2F70:0x011D2F6C);
        r.d0C=uint32_t(uintptr_t(CallGameMethod<void*,0x0079B790>(G<void*>(0x011D753C),name,0,1)));
        r.index=(flags&64)?0:CallGameMethod<uint32_t,0x0071F4D0>(self,name);
        bool alternate=r.b04==8 || r.b04==9;
        r.d14=r.b04==11?4:(r.b05==0?uint32_t(alternate)+1:(r.b05==1 && alternate?3:0));
        if(r.b74) r.d14+=5;
    }
    if(F<void*>(self,232)==F<void*>(self,236)) {
        ts3::shader::ShaderParamRecord blank;
        blank.f2C=G<float>(0x0115B5C4);blank.f30=blank.f2C;blank.f34=blank.f2C;
        blank.f38=-0.0f-blank.f2C;blank.f3C=blank.f38;blank.f40=blank.f38;
        auto* end=F<ts3::shader::ShaderParamRecord*>(self,220);
        if(uintptr_t(end)<uintptr_t(F<void*>(self,224))) {
            F<ts3::shader::ShaderParamRecord*>(self,220)=end+1;
            if(end) end->Assign71C240(blank);
        } else CallGameMethod<void,0x0071EE00>(static_cast<uint8_t*>(self)+216,end,&blank);
        *index=uint32_t((int32_t(uintptr_t(F<void*>(self,220))-uintptr_t(F<void*>(self,216)))/120)-1);
    } else {
        auto*& end=F<uint32_t*>(self,236);*index=end[-1];--end;
    }
    auto* destination=F<ts3::shader::ShaderParamRecord*>(self,216)+*index;
    destination->Assign71B550(r);
    return 1;
#else
    (void)self;(void)key;(void)data;(void)index;NotRewritten("m07_M12 AddRecord image dependencies");
#endif
}
}
