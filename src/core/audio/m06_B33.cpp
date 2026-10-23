#include "ts3/audio/m06_B33.h"
#include <cmath>
#include <cstring>
#include <emmintrin.h>
namespace ts3::audio::b33 {
namespace {
void* System() { return CallGameFunction<void*,0x008b0a70>(); }
void* RW() {
#if defined(_M_IX86)
    return *reinterpret_cast<void**>(0x011ffc28);
#else
    AbortNotRewritten("m06_B33 RW singleton");
    return nullptr;
#endif
}
void Lock() { if(auto p=RW()) CallGameMethod<void,0x00ed7170>(p); }
void Unlock() { if(auto p=RW()) CallGameMethod<void,0x00ed7190>(p); }
void Flag(Emitter* s,uint32_t mask,bool value) { CallGameMethod<void,0x008a0b60>(s,mask,uint32_t(value)); }
// The original getters use FLD/FSTP, not a bitwise MOVSS copy.
void StoreScalar(float* out, const float* value) {
#if defined(_M_IX86)
    __asm {
        mov eax, value
        fld dword ptr [eax]
        mov eax, out
        fstp dword ptr [eax]
    }
#else
    uint32_t bits;
    std::memcpy(&bits,value,sizeof bits);
    if ((bits & 0x7f800000u)==0x7f800000u && (bits & 0x007fffffu)) bits|=0x00400000u;
    std::memcpy(out,&bits,sizeof bits);
#endif
}
float Product(float a,float b) { return _mm_cvtss_f32(_mm_mul_ss(_mm_set_ss(a),_mm_set_ss(b))); }
}
void* SetCallback(Emitter* s,void* value) { s->callback=value; return value; }
uint32_t SetKey(Emitter* s,const uint32_t* key) {
    // Sequential loads/stores preserve even overlapping input views.
    s->key[0]=key[0]; s->key[1]=key[1]; s->key[2]=key[2];
    uint32_t result=key[3]; s->key[3]=result; s->key[2]=0x01a527db; return result;
}
void SetFlag(Emitter* s,uint32_t value) {
    if(uint8_t(value)) s->flags|=0x100; else s->flags&=~0x100u;
}
uintptr_t Notify(Emitter* s) {
    CallGameVirtual<void,85>(System(),0x03a0fd6eu,s->callback,0u,0u,0u);
    return CallGameVirtual<uintptr_t,6>(s);
}
bool SetRate(Emitter* s,float v) {
    s->overrides&=~4u;
    if(v<0.0f || v>10.0f || !s->rateNode) return false;
    CallGameMethod<void,0x00edaf00>(s->rateNode,0u,v); return true;
}
bool SetChannelRate(Emitter* s,float v) {
    s->overrides&=~8u;
    if(v<0.5f || v>2.0f || !s->gainNode) return false;
    if(s->channels<2) v=1.0f;
    CallGameMethod<void,0x00edaf00>(s->gainNode,0u,v); return true;
}
bool SetPitch(Emitter* s,float v) {
    s->overrides&=~1u;
    if(v<0.0f || v>1.0f || !s->pitchNode) return false;
    CallGameMethod<void,0x00edaf00>(s->pitchNode,0u,v); return true;
}
void Poll(Emitter* s) {
    auto rw=RW();
    if(!rw || !CallGameMethod<bool,0x00ed71b0>(rw,s->handle)) return;
    if(s->flags&2) {
        s->flags&=~2u;
        CallGameVirtual<void,33>(System(),&s->secondary);
    }
    CallGameVirtual<void,1>(s);
}
bool Stop(Emitter* s) {
    if(s->flags&1) { CallGameVirtual<void,6>(s); s->flags&=~1u; }
    if(auto p=s->retained) { s->retained=nullptr; CallGameVirtual<void,1>(p); }
    s->word4e8=0; return true;
}
bool Load(Emitter* s) {
    if(s->flags&0x20) return false;
    if(!CallGameVirtual<bool,22>(s) || !CallGameVirtual<bool,24>(s)) return false;
    if(s->resource) {
        if(CallGameVirtual<bool,6>(s->resource,0x79da6d74u)) {
            uint32_t v[4]{};
            CallGameVirtual<void,13>(s->resource,0x79da6d74u,v); s->property=v[0];
        }
        CallGameMethod<void,0x008aaef0>(reinterpret_cast<uint8_t*>(s)+0x10,s->resource,0x1e4db1ebu);
        CallGameVirtual<void,11>(s->resource,0xed1f36d5u,&s->word3c0);
        uint32_t value;
        if(CallGameVirtual<bool,9>(s->resource,0x12d2a4d4u,&value)) Flag(s,4,uint8_t(value)!=0);
        if(CallGameVirtual<bool,9>(s->resource,0x3c10c7b9u,&value)) Flag(s,0x200,uint8_t(value)!=0);
    }
    return true;
}
bool Play(Emitter* s,double time) {
    auto sys=System(); if(!sys) return false;
    s->flags&=~0x80u; bool playing=(s->flags&0x20)!=0;
    if(playing) { Lock(); CallGameMethod<void,0x00ed5a20>(s->voice,1u,0u); Unlock(); }
    s->time=time;
    if(!(s->flags&2)) { CallGameVirtual<void,32>(sys,&s->secondary); s->flags|=2; }
    CallGameVirtual<void,36>(s);
    if(!playing) {
        s->flags|=0x20;
        CallGameMethod<void,0x008c0f50>(s->voices);
        CallGameVirtual<void,30>(s,0.0f,1u);
        CallGameVirtual<void,85>(sys,0x0617208bu,s->callback,s->key[0],0u,0u);
        if(!*(static_cast<uint8_t*>(s->voices)+0x955)) CallGameMethod<void,0x008c1ca0>(s->voices);
        CallGameVirtual<void,0>(s);
    }
    return true;
}
bool Mute(Emitter* s,uint32_t value) {
    uint8_t v=uint8_t(value);
    if(s->muted==v) return true;
    s->muted=v; s->dirty=1;
    if(!s->voices) return true;
    // VoiceContainer +8 is a pointer, not a fixed-width handle.
    void* voice=*reinterpret_cast<void**>(static_cast<uint8_t*>(s->voices)+2*sizeof(void*));
    if(!voice || ((s->flags&0x400) && (s->flags&8))) return true;
    Lock(); CallGameMethod<void,0x00edb5b0>(voice,s->muted?0.0f:1.0f); Unlock();
    return true;
}
bool SetProperty(Emitter* s,uint32_t id,float v,uint32_t deferred) {
    Lock(); bool result=true; const bool now=uint8_t(deferred)==0;
    switch(id) {
    case 0x04f8f9be: Flag(s,0x800,v!=0.0f); break;
    case 0x12d2a4d4: Flag(s,4,v!=0.0f); break;
    case 0xba134192: Flag(s,0x400,v!=0.0f); break;
    case 0x03593710: s->overrides|=0x20; s->spatial[5]=v; if(now) CallGameVirtual<void,28>(s,s->spatial); break;
    case 0x050c5d65: s->flags|=0x40; s->spatial[2]=v; if(now) CallGameVirtual<void,28>(s,s->spatial); break;
    case 0x050c5d66: s->flags|=0x40; s->spatial[1]=v; if(now) CallGameVirtual<void,28>(s,s->spatial); break;
    case 0x050c5d67: s->flags|=0x40; s->spatial[0]=v; if(now) CallGameVirtual<void,28>(s,s->spatial); break;
    case 0x2406a047: s->overrides|=0x40; s->spatial[3]=v; if(now) CallGameVirtual<void,28>(s,s->spatial); break;
    case 0x0f616b72: s->overrides|=0x10; s->spatial[4]=v; if(now) CallGameVirtual<void,28>(s,s->spatial); break;
    case 0x14f9bdf6: s->value3fc=v; break;
    case 0x6dd08218: if((s->flags&8) && v==0.0f) CallGameVirtual<void,75>(System()); break;
    case 0x49eb68db:
        s->overrides|=0x100; s->pitchScale=v;
        if(now && s->voices) CallGameMethod<void,0x008c1040>(s->voices,id,Product(s->pitch,v));
        break;
    case 0x25df0108: s->overrides|=1; s->pitch=v; if(now) CallGameVirtual<void,27>(s,v); break;
    case 0x6df5822d: CallGameVirtual<void,33>(s,_mm_cvtt_ss2si(_mm_set_ss(v))); break;
    case 0x82beafe0: s->overrides|=8; s->rate=v; if(now) CallGameVirtual<void,29>(s,v); break;
    case 0x71bc3009:
        s->overrides|=4; s->gain=v;
        if(now) CallGameVirtual<void,26>(s,s->channels<2?v:Product(s->rate,v));
        break;
    default: result=s->voices && CallGameMethod<bool,0x008c1040>(s->voices,id,v); break;
    }
    Unlock(); return result;
}
bool GetProperty(Emitter* s,uint32_t id,float* out) {
    switch(id) {
    case 0x03593710: StoreScalar(out,&s->spatial[5]); break;
    case 0x050c5d65: StoreScalar(out,&s->spatial[2]); break;
    case 0x050c5d66: StoreScalar(out,&s->spatial[1]); break;
    case 0x050c5d67: StoreScalar(out,&s->spatial[0]); break;
    case 0x0f616b72: StoreScalar(out,&s->spatial[4]); break;
    case 0x2406a047: StoreScalar(out,&s->spatial[3]); break;
    case 0x12d2a4d4: *out=(s->flags&4)?1.0f:0.0f; break;
    case 0x6dd08218: *out=(s->flags&8)?1.0f:0.0f; break;
    case 0x25df0108: StoreScalar(out,&s->pitch); break;
    case 0x49eb68db: StoreScalar(out,&s->pitchScale); break;
    case 0x82beafe0: StoreScalar(out,&s->rate); break;
    case 0x71bc3009: StoreScalar(out,&s->gain); break;
    case 0x893f8476: *out=CallGameVirtual<float,34>(s); break;
    case 0x8f678117: StoreScalar(out,&s->value408); break;
    case 0xaa4e5a2e: *out=s->enabled?1.0f:0.0f; break;
    case 0xd24d4aff: StoreScalar(out,&s->value400); break;
    case 0xc2d1f579: StoreScalar(out,&s->value3f4); break;
    case 0xba03d0cd: StoreScalar(out,&s->value404); break;
    default: return s->voices && CallGameMethod<bool,0x008c10f0>(s->voices,id,out);
    }
    return true;
}
}
