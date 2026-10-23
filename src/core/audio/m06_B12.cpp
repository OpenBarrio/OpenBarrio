#include "ts3/audio/m06_B12.h"
#include <cstring>
#include <algorithm>
namespace ts3::audio::m06_B12 {
namespace {
void* Table(uintptr_t address) {
#if defined(_M_IX86)
    return reinterpret_cast<void*>(address);
#else
    (void)address; NotRewritten("m06_B12 game vtable");
#endif
}
void* Secondary(void* object) { return static_cast<uint8_t*>(object)+sizeof(void*); }
int32_t Read(const void* p, size_t offset) { int32_t v; std::memcpy(&v,static_cast<const uint8_t*>(p)+offset,4); return v; }
void __fastcall ManagerCallback(void* object, void*) { CallGameVirtual<void,145>(object); }
}
Service* Construct(Service* self) {
    CallGameMethod<void,0x59ECC0>(self,0x039E8474u,0u);
    self->vtable=Table(0x01029060);
    self->manager=nullptr;
    auto argument=CallGameFunction<void*,0x931020>();
    CallGameMethod<void,0x44CC70>(self->inner,argument);
    return self;
}
const void* TypeId(Service*) {
#if defined(_M_IX86)
    return reinterpret_cast<const void*>(0x01029094);
#else
    static const uint8_t id[16]={0xBB,0xDC,0x75,0x01,0xC5,0xD9,0x75,0x01,0x48,0x7B,0x95,0x02,0,0,0,0};
    return id;
#endif
}
bool Start(Service* self) {
    auto memory=CallGameFunction<void*,0x4E3F90>(0x27D60u,"Audio/Manager",0u,0u,0u,0u);
    self->manager=memory?CallGameMethod<void*,0x868380>(memory):nullptr;
    if (!self->manager) return false;
    CallGameVirtual<void,2>(Secondary(self->manager));
    // 007D99C0 merely sets a stack word to -1; C0FD60 is a bare RET.
    CallGameVirtual<void,1>(self->inner);
    return CallGameVirtual<bool,0>(self->manager,self->inner);
}
uintptr_t Stop(Service* self) {
    if (self->manager) {
        CallGameVirtual<void,1>(self->manager);
        CallGameVirtual<void,3>(Secondary(self->manager));
        self->manager=nullptr;
    }
    return CallGameVirtual<uintptr_t,2>(self->inner);
}
void Idle(Service*, uint32_t, uint32_t) { /* C0FD60 is a bare RET. */ }
uintptr_t Tick(Service* self, uint32_t, float elapsed) {
    CallGameVirtual<void,14>(self->inner);
    // The original FLD/FSTP pair quiets signaling NaNs before forwarding.
    float forwarded;
#if defined(_M_IX86)
    __asm { fld elapsed }
    __asm { fstp forwarded }
#else
    forwarded=elapsed;
#endif
    return CallGameVirtual<uintptr_t,2>(self->manager,forwarded);
}
Service* Destroy(Service* self, uint32_t flags) {
    self->vtable=Table(0x01029060);
    CallGameMethod<void,0x44D500>(self->inner);
    CallGameMethod<void,0x731CF0>(self);
    if (flags&1) CallGameFunction<void,0x4E3F70>(self);
    return self;
}
void RegisterService() {
#if defined(_M_IX86)
    auto registry=*reinterpret_cast<void**>(0x011CE3F4);
#else
    void* registry=nullptr;
    NotRewritten("m06_B12 registry singleton");
#endif
    if (!registry || CallGameMethod<bool,0x59EFE0>(registry,0x039E8474u)) return;
    auto memory=CallGameFunction<Service*,0x4E3F90>(0xD0u,"Audio/Service",0u,0u,0u,0u);
    auto service=memory?Construct(memory):nullptr;
    CallGameMethod<void,0x59F500>(registry,service,3u);
}
void Activate(Service* self) {
    if (!self->manager) return;
    struct Callback { void* vptr; int32_t refs; uint32_t flags; void* object; void* function; void* unused; };
    Callback callback{Table(0x010650B0),0,0,self->manager,reinterpret_cast<void*>(&ManagerCallback),nullptr};
    CallGameMethod<void,0x7D98C0>(&callback,1u,0u);
    CallGameVirtual<void,4>(self->manager,0u);
}
uint32_t SurfaceName(uint32_t surface, void* buffer, uint32_t capacity) {
    auto output=static_cast<char*>(buffer);
    static const uint8_t mapping[39]={0,0,0,0,0,1,2,2,2,2,3,4,4,2,2,3,5,2,2,6,6,7,7,8,8,9,10,10,11,3,12,12,13,6,14,14,14,14,14};
    static const char* names[]={"cment","cpet","gravel","generic","grass","lino","marble","sand","wood","wdeck","marble","metal","cment","grass","puddle"};
    const char* name=surface-1u<39u?names[mapping[surface-1u]]:"generic";
    auto length=static_cast<uint32_t>(std::strlen(name));
    if (capacity) {
        auto count=std::min(length,capacity-1);
        std::memcpy(output,name,count);
        output[count]=0;
    }
    return length;
}
uint8_t DeviceFlag() {
    auto device=CallGameFunction<const uint8_t*,0x6E8330>();
    return device?device[0x270]:0;
}
uint32_t Contains(const void* self, const int8_t* c) {
    return c[0]>=0 && c[0]<Read(self,0x174) && c[1]>=0 && c[1]<Read(self,0x178)
        && c[2]>=Read(self,0x17C) && c[2]<=Read(self,0x180);
}
uint32_t HasFlag(const void* self) {
    return (static_cast<uint32_t>(Read(self,0xD0))&(1u<<12)) || (static_cast<uint32_t>(Read(self,0x9C))&(1u<<6));
}
}
