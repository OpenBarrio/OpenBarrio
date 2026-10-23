#pragma once
#include "ts3/anim/m05_layouts.h"
namespace ts3::anim::m136 {
// Fields established by this batch; embedded bridge-only objects remain opaque.
struct Style { void* controller; unsigned char rest[0x78]; };
struct Progression { void *begin,*end,*capacity,*descriptor,*inlinePointer; uint32_t tail[2]; };
struct Body {
    void *vtable,*secondaryBase; unsigned char opaque08[0xc]; void* entity;
    void* secondary; ListLink controllers; void* controllerDescriptor;
    Style style; ListLink effects; void* effectsDescriptor; Style* slaveStyles;
    void **slaveBegin,**slaveEnd,**slaveCapacity; void* slaveDescriptor; void* master;
    int32_t slaveCount; uint8_t slaveFlag; unsigned char padCD[3];
    unsigned char stateD0[0x20],stateF0[0x24];
    void* state114Vtable; unsigned char opaque118[8]; float time; void* host;
    unsigned char opaque128[0x14]; void *buffer13C,*buffer140;
    unsigned char opaque144[0x24]; float scale; unsigned char opaque16C[0xc];
    void* retained; unsigned char opaque17C[8]; uint8_t mirrored; unsigned char pad185[3];
    Progression *styleBegin,*styleEnd,*styleCapacity; void* styleDescriptor; void* inlinePointer;
    unsigned char styleStorage[0xa8]; void* slaveProgressions; uint32_t styleKeyLow,styleKeyHigh;
    uint8_t flags[5];
};
TS3_X86_OFFSET(Body,scale,0x168);
TS3_X86_OFFSET(Body,host,0x124);
TS3_X86_OFFSET(Body,slaveStyles,0xb0);
TS3_X86_OFFSET(Body,styleBegin,0x188);
TS3_X86_OFFSET(Body,slaveProgressions,0x244);
// TS3W.exe 0x0084F270: Read the body's scale, returning through ST0.
float GetScale(const Body* self);
// TS3W.exe 0x0084F280: Adjust the secondary interface back by one pointer and delete the body.
Body* DeleteSecondary(void* self,uint32_t flags);
// TS3W.exe 0x0084F290: Count the controller list by following its circular sentinel.
uint32_t CountControllers(const Body* self);
// TS3W.exe 0x0084F2B0: Tear down body ownership and detach controller delegates in original order.
uintptr_t Destroy(Body* self);
// TS3W.exe 0x0084F560: Adopt slave styles and bind actors, preserving incidental EAX returns.
uintptr_t SetSlaves(Body* self,Style* styles,const uint64_t* ids,int32_t count,uint8_t flag);
// TS3W.exe 0x0084F890: Build and install the body and slave controllers, returning an AL boolean.
bool BuildControllers(Body* self);
}
namespace ts3::anim::m136 {
struct Hooks {
    float Scale() { return GetScale(reinterpret_cast<const Body*>(this)); }
    uint32_t Delete(uint32_t flags) { return static_cast<uint32_t>(reinterpret_cast<uintptr_t>(DeleteSecondary(this,flags))); }
    uint32_t Count() { return CountControllers(reinterpret_cast<const Body*>(this)); }
    uint32_t Destruct() { return static_cast<uint32_t>(Destroy(reinterpret_cast<Body*>(this))); }
    uint32_t Slaves(void* styles,void* ids,int32_t count,uint32_t flag) { return static_cast<uint32_t>(SetSlaves(reinterpret_cast<Body*>(this),static_cast<Style*>(styles),static_cast<const uint64_t*>(ids),count,static_cast<uint8_t>(flag))); }
    bool Build() { return BuildControllers(reinterpret_cast<Body*>(this)); }
};
}
