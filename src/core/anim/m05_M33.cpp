#include "ts3/anim/m05_M33.h"
namespace ts3::anim {
void* M33Query(void* secondary, uint32_t id) {
    if (id == 0x34f49f94 || id == 0x7489d72b)
        return static_cast<unsigned char*>(secondary)-2*sizeof(void*);
    return nullptr;
}
void* M33Destroy(void* secondary, uint32_t flags) {
    return CallGameMethod<void*,0x0056A730>(static_cast<unsigned char*>(secondary)-3*sizeof(void*),flags);
}
M33Controller* M33Construct(M33Controller* self, void* host, Controller* a, Controller* b, float blend) {
    CallGameMethod<void*,0x00565A80>(&self->base,host);
#if defined(_M_IX86)
    self->base.vtable=reinterpret_cast<void*>(0x00fc1bb0);
    self->base.secondary8=reinterpret_cast<void*>(0x00fc1ba8);
    self->base.secondaryC=reinterpret_cast<void*>(0x00fc1ba4);
    self->observer=reinterpret_cast<void*>(0x00fc1e5c);
#endif
    self->first=a;
    if (a) ++a->refs;
    self->second=b;
    if (b) ++b->refs;
    self->blend=blend;
    float first=CallGameVirtual<float,8>(a);
    double second=CallGameVirtual<float,8>(b);
    self->duration=static_cast<float>(static_cast<double>(first)+(second-static_cast<double>(first))*blend);
    CallGameMethod<void,0x00566170>(b ? &b->secondaryC : nullptr,&self->observer,-1);
    return self;
}
uintptr_t M33SetTime(M33Controller* self, float time) {
    ++self->base.refs;
    double a=CallGameVirtual<float,8>(self->first);
    CallGameVirtual<void,2>(self->first,static_cast<float>((a/self->duration)*time));
    double b=CallGameVirtual<float,8>(self->second);
    CallGameVirtual<void,2>(self->second,static_cast<float>((b/self->duration)*time));
    uint32_t refs=static_cast<uint32_t>(--self->base.refs);
    if (refs == 0) {
        self->base.refs=1;
        return CallGameVirtual<uintptr_t,0>(self,1u);
    }
    return refs;
}
void* M33Evaluate(M33Controller* self, void* context) {
    void* a=CallGameVirtual<void*,3>(self->first,context);
    void* b=CallGameVirtual<void*,3>(self->second,context);
    if (!a || !b) return nullptr;
    void* factory=CallGameMethod<void*,0x0054C770>(context,0xd3dfc020u);
    void* result=CallGameVirtual<void*,1>(factory,0xd3dfc020u);
    CallGameMethod<void,0x0054BF40>(a,1.0f-self->blend);
    CallGameMethod<void,0x0054BF40>(b,self->blend);
    CallGameVirtual<void,3>(result,a);
    CallGameVirtual<void,3>(result,b);
    return result;
}
namespace {
struct LoadStream { void* vtable; void* opaque4; void* source; uint32_t opaqueC; uint32_t value; uint8_t ok; };
}
bool M33Load(M33Controller* self, void* input) {
    auto* stream=static_cast<LoadStream*>(input);
    uint32_t version=0xffffffff;
    auto token=CallGameFunction<uint32_t,0x004DD700>(stream->source,&version,stream->value);
    CallGameMethod<void,0x007D77C0>(stream,token);
    if (!stream->ok || version > 1) return false;
    auto a=CallGameFunction<Controller*,0x00565C80>(self->base.hostSlot,stream);
    CallGameMethod<void,0x00CF3C70>(&self->first,a);
    if (!self->first) return false;
    auto b=CallGameFunction<Controller*,0x00565C80>(self->base.hostSlot,stream);
    CallGameMethod<void,0x00CF3C70>(&self->second,b);
    if (!self->second) return false;
    CallGameMethod<void,0x00565700>(stream,&self->blend);
    if (!stream->ok) return false;
    CallGameMethod<void,0x00565700>(stream,&self->duration);
    if (!stream->ok) return false;
    if (self->second) CallGameMethod<void,0x00566170>(&self->second->secondaryC,&self->observer,-1);
    return stream->ok != 0;
}
}
#if defined(_M_IX86)
namespace ts3::anim::m33hooks {
void* __fastcall Query(void* s,void*,uint32_t i){return M33Query(s,i);}
void* __fastcall Destroy(void* s,void*,uint32_t f){return M33Destroy(s,f);}
M33Controller* __fastcall Construct(M33Controller* s,void*,void* h,Controller* a,Controller* b,float f){return M33Construct(s,h,a,b,f);}
uintptr_t __fastcall SetTime(M33Controller* s,void*,float f){return M33SetTime(s,f);}
void* __fastcall Evaluate(M33Controller* s,void*,void* c){return M33Evaluate(s,c);}
bool __fastcall Load(M33Controller* s,void*,void* c){return M33Load(s,c);}
}
#endif
