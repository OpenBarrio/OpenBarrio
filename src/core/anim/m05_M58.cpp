#include "ts3/anim/m05_M58.h"
#include "ts3/anim/m05_B08.h"
#include "ts3/anim/m05_M40.h"
namespace ts3::anim::m58 {
static void* Allocate(uint32_t size) {
#if defined(_M_IX86)
    void* descriptor=reinterpret_cast<void*>(0x00fc24e8);
#else
    void* descriptor=nullptr;
#endif
    return CallGameFunction<void*,0x004e3f90>(size,descriptor,0u,0u,0u,0u);
}
void* CreateScratch(void* host) {
    void* p=Allocate(0x88);
    if(!p) return nullptr;
    return CallGameMethod<void*,0x0056d560>(p,host);
}
void* CreateTransform(void* host) {
    void* p=Allocate(0x60);
    if(!p) return nullptr;
    return m40::Construct(static_cast<m40::TransformController*>(p),host);
}
void* CreateSequence(void* host) {
    void* p=Allocate(0x70);
    if(!p) return nullptr;
    return CallGameMethod<void*,0x00567480>(p,host,0xffffffffu);
}
void* CreateAnimation(void* host) {
    void* p=Allocate(0x2c);
    if(!p) return nullptr;
    return CallGameMethod<void*,0x0056fe00>(p,host);
}
Controller* Construct(Controller* p,void* host) {
    b08::Construct(p,host);
#if defined(_M_IX86)
    p->vtable=reinterpret_cast<void*>(0x00fc2548);
    p->secondary8=reinterpret_cast<void*>(0x00fc253c);
    p->secondaryC=reinterpret_cast<void*>(0x00fc2538);
#else
    NotRewritten("M58 game interface tables");
#endif
    return p;
}
uint32_t Type(void*) { return 0x074a5f9du; }
}
