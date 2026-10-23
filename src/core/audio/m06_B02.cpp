#include "ts3/audio/m06_B02.h"
#include <cmath>
#include <cstring>
#include <limits>
namespace ts3::audio::b02impl {
namespace {
void* injected;
void* System() {
    if(injected) return injected;
#if defined(_M_IX86)
    return CallGameFunction<void*,0x008B0A70>();
#else
    return nullptr;
#endif
}
template<class R,int Slot,class... A> R V(void* p,A... args) {
#if defined(_M_IX86)
    return CallGameVirtual<R,Slot>(p,args...);
#else
    using F=R(*)(void*,A...);
    return reinterpret_cast<F>((*static_cast<void***>(p))[Slot])(p,args...);
#endif
}
void Word(void* s,uint32_t id,uint32_t v) { V<void,19>(s,id,v); }
void Float(void* s,uint32_t id,float v) { V<void,18>(s,id,v); }
void Header(void* s,uint32_t lo,uint32_t hi,uint32_t handle) {
    V<void,17>(s,0x03475365u);
    V<void,20>(s,0x03475381u,lo,hi);
    Word(s,0x03475385u,handle);
}
uint32_t Truncate(float f) {
    // x87 FISTP qword indefinite is 0x8000000000000000: its low word is zero.
    if(!std::isfinite(f) || double(f)>=9223372036854775808.0 || double(f)<-9223372036854775808.0) return 0;
    return static_cast<uint32_t>(static_cast<int64_t>(f));
}
}
void SetCommandSystem(void* p) { injected=p; }
void Play(uint32_t lo,uint32_t hi,uint32_t h,float a,float b,float c) {
    if(void* s=System()) { Header(s,lo,hi,h); Float(s,0x03475391u,a); Float(s,0x03475395u,b); Float(s,0x03475398u,c); V<void,26>(s); }
}
void PlaySimple(uint32_t lo,uint32_t hi,uint32_t h) {
    if(void* s=System()) { Header(s,lo,hi,h); V<void,26>(s); }
}
void Stop(uint32_t h,uint32_t mode) {
    if(void* s=System()) if(h) { V<void,17>(s,0x0347536bu); Word(s,0x03475385u,h); Word(s,0x034753a0u,mode); V<void,26>(s); }
}
void Reset(Ambience* p) {
    p->initialized=0;
    void* registry=V<void*,3>(System());
    if(registry) V<void,11>(registry,&p->token,0x03a127dcu,0xffffd8f1u);
    for(uint32_t i=0;i<p->count;++i) if(p->handles[i]) {
        Stop(p->handles[i],0);
        if(void* s=System()) V<void,13>(s,p->handles[i]);
        p->handles[i]=0;
    }
    p->used=0;
}
void TickAuto(Ambience* p,float) {
    if(!p->enabled && p->autoStart) V<void,15>(p);
}
void Forward(Ambience* p) { V<void,14>(p); }
uint32_t Query(Ambience* p,uint32_t id,float* out) {
    if(id==0x12f4ae88u) { *out=p->enabled?1.0f:0.0f; return (id&0xffffff00u)|1; }
    if(id==0xbaf7f4f9u) { *out=static_cast<float>(p->count); return (p->count&0xffffff00u)|1; }
    return id&0xffffff00u;
}
void StopAll(Ambience* p) {
    if(p->used>0) for(uint32_t i=0;i<p->count;++i) if(p->handles[i]) Stop(p->handles[i],0);
}
void Update(Ambience* p,float dt) {
    if(!p->enabled) { if(p->autoStart) V<void,15>(p); if(!p->enabled) return; }
    p->elapsed=p->elapsed+dt;
    if(p->interval>p->elapsed || p->used>=p->count) return;
    for(uint32_t i=0;i<p->count;++i) if(!p->handles[i]) {
        void* s=System(); uint32_t h=s?V<uint32_t,11>(s):0;
        PlaySimple(p->key[0],p->key[1],h);
        p->handles[i]=h; ++p->used; p->elapsed=0; break;
    }
}
uint32_t Configure(Ambience* p,void* r) {
    uint32_t key[4]={};
    uint32_t result=V<uint32_t,13>(r,0x25238ae2u,key);
    if(result&255) { p->key[0]=key[0]; p->key[1]=key[1]; result=key[0]; }
    if(!(p->key[0]|p->key[1])) return result&0xffffff00u;
    result=V<uint32_t,12>(r,0x15525baau,&p->interval);
    if(!(result&255)) return result&0xffffff00u;
    V<uint32_t,9>(r,0x37054b49u,&p->autoStart);
    result=V<uint32_t,11>(r,0xbaf7f4f9u,&p->count);
    if(!(result&255)) {
        float count=1;
        result=V<uint32_t,12>(r,0xbaf7f4f9u,&count);
        if(!(result&255)) return result;
        p->count=Truncate(count);
    }
    if(p->count>32) p->count=32;
    void* registry=V<void*,3>(System());
    if(registry) V<void,9>(registry,&p->token,0x03a127dcu,0u,0u);
    p->used=0;
    for(auto& h:p->handles) h=0;
    p->initialized=1;
    return 1;
}
void Evaluate(void* secondary,uint32_t,uint32_t) {
    auto* p=reinterpret_cast<Ambience*>(static_cast<uint8_t*>(secondary)-sizeof(void*));
    if(!p->evaluating) return;
    auto n=p->end-p->begin;
    for(decltype(n) i=0;i<n;++i) {
        auto b=p->begin[i];
        if(V<uint8_t,9>(b.evaluator)) {
            float value=V<float,8>(b.evaluator,1u);
            // EA loads the saved vector count, not the saved binding ID.
            V<void,3>(p,static_cast<uint32_t>(n),value);
        }
    }
}
uint8_t Complete(void* tokenView,uint32_t,void* message) {
    auto* p=reinterpret_cast<Ambience*>(static_cast<uint8_t*>(tokenView)-offsetof(Ambience,token));
    auto* m=static_cast<uint32_t*>(message);
    if(m[2]==0x03a129eeu) for(uint32_t i=0;i<p->count;++i) if(p->handles[i]==m[4]) {
        if(void* s=System()) V<void,13>(s,m[4]);
        p->handles[i]=0; --p->used; break;
    }
    return 0;
}
void Set(Ambience* p,uint32_t id,float value,uint32_t) {
    if(id==0xbaf7f4f9u) { p->count=Truncate(std::fabs(value)); if(p->count>32) p->count=32; }
    else if(id==0x12f4ae88u) p->enabled=(value!=0.0f); // unordered compares enable too
}
}
