#include "ts3/anim/m05_M51.h"
namespace ts3::anim::m51 {
uint8_t Save(ControllerState* c,b08::StreamState* s) {
    auto result=CallGameFunction<uint32_t,0x4DDBD0>(s->handle,1u,s->extra);
    CallGameMethod<int32_t,0x7D77C0>(s,result);
    if(!s->good) return 0;
    b08::SaveOptional(c->retained,s);
    return s->good;
}
static void Tables(ControllerState* c) {
#if defined(_M_IX86)
    c->base.vtable=reinterpret_cast<void*>(0xFC2178);
    c->base.secondary8=reinterpret_cast<void*>(0xFC2170);
    c->base.secondaryC=reinterpret_cast<void*>(0xFC216C);
#else
    (void)c; NotRewritten("M51 original vtables");
#endif
}
ControllerState* Construct(ControllerState* c,void* host) {
    b08::Construct(&c->base,host);
    Tables(c);
    c->retained=nullptr;
    return c;
}
uint32_t Type(void*) { return 0x07A11BA8u; }
ControllerState* Query(void* s,uint32_t id) {
    return id==0x07A11BA8u ? reinterpret_cast<ControllerState*>(static_cast<char*>(s)-offsetof(Controller,secondary8)) : nullptr;
}
ControllerState* DeleteInterface(void* s,uint32_t flags) {
    auto* c=reinterpret_cast<ControllerState*>(static_cast<char*>(s)-offsetof(Controller,secondaryC));
    return CallGameMethod<ControllerState*,0x56FFD0>(c,flags);
}
ControllerState* ConstructRetained(ControllerState* c,void* host,Controller* retained) {
    b08::Construct(&c->base,host);
    Tables(c);
    c->retained=retained;
    if(retained) retained->refs=static_cast<int32_t>(static_cast<uint32_t>(retained->refs)+1u);
    return c;
}
}
