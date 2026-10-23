#pragma once
#include "ts3/layout.h"
namespace ts3::anim::m129_impl {
struct Ref { void** vtable; int32_t refs; };
struct Controller { void** vtable; int32_t refs; void* interfaces[5]; void* owner; uint32_t id; uint8_t latch; };
struct Link { Link* next; Link* previous; Controller* controller; uint32_t payload; };
struct Body { void* prefix[7]; Link list; };
struct Request { void* source; float values[4]; uint8_t flags[2]; Ref* resource; };
struct Result { Ref* controller; uint8_t flag; };
struct Destination { void* prefix[6]; Ref* map; };
TS3_X86_OFFSET(Controller,owner,0x1c);
TS3_X86_OFFSET(Controller,latch,0x24);
TS3_X86_OFFSET(Body,list,0x1c);
TS3_X86_SIZE(Request,0x1c);
// TS3W.exe 0x00849540: Return the root-motion distance, discarding auxiliary outputs.
double RootDistance(void* controller);
// TS3W.exe 0x00849570: Notify eligible owned controllers, testing only AL from the owner predicate.
void NotifyControllers(Body* self, uint32_t event);
// TS3W.exe 0x00849A30: Query a controller for a motion request and release temporary references.
double QueryMotion(void* source,uint32_t style,void* provider,float amount,int mode,float* distance);
// TS3W.exe 0x00849BF0: Construct and install a namespace map with a scene entity, retaining EA release ordering.
uintptr_t InstallNamespace(Body* self,Destination* destination);
}
namespace ts3::anim {
struct m129 {
    void NotifyControllers(uint32_t event) { m129_impl::NotifyControllers(reinterpret_cast<m129_impl::Body*>(this),event); }
    uint32_t InstallNamespace(void* destination) { return static_cast<uint32_t>(m129_impl::InstallNamespace(reinterpret_cast<m129_impl::Body*>(this),static_cast<m129_impl::Destination*>(destination))); }
};
}
