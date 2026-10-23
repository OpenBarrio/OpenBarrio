#pragma once
#include "ts3/layout.h"
namespace ts3::anim::M158 {
struct Steering {
    void* vtable; void* host;
    unsigned char entity[52]; // +08, external entity subobject
    void* secondary; uint32_t state64, state68;
    unsigned char path[16];
    void* route; void* routeData; int32_t routeCount; uint8_t routeFlag; uint8_t pad101[3];
    void* ownedNext; void* ownedPrevious; const char* ownedDescriptor;
    float weight; int32_t ids[3]; uint32_t state132,state136;
    uint8_t flags[8]; uint32_t mode; void* retained;
    unsigned char settings[32]; uint32_t argument; float speed, acceleration;
    uint8_t status[4]; const char* nameBegin; const char* nameEnd; const char* nameCapacity; const char* nameDescriptor;
    uint8_t debug; uint8_t pad221[3]; float p224; float p228; void* p232;
    uint8_t b236,b237; uint8_t pad238[2]; void* p240; void* p244; uint32_t state248; uint16_t state252; uint8_t pad254[2];
    void* p256; int8_t railFirst,railLast; int16_t railIndex; float railTime; void* avoidance;
};
TS3_X86_OFFSET(Steering,secondary,60); TS3_X86_OFFSET(Steering,ownedNext,104);
TS3_X86_OFFSET(Steering,settings,156); TS3_X86_OFFSET(Steering,avoidance,268);
// TS3W.exe 0x00956850: replace mismatched body/brain and refresh steering; preserves signed rail limits and temporary releases.
bool Refresh(Steering*, int32_t body, int32_t brain);
// TS3W.exe 0x00956E60: attach entity and retained interface, then update host flags and optional transform.
bool Attach(Steering*, void* entity, void* retained, uint32_t unused4, uint32_t unused5, uint32_t argument, uint8_t transform);
// TS3W.exe 0x00957000: construct steering and allocate its avoidance field, leaving unspecified bytes untouched.
Steering* Construct(Steering*, void* host);
// TS3W.exe 0x00957470: construct only when storage is non-null; forwards the constructor return.
void* ConstructIfPresent(void* storage, void* host);
// TS3W.exe 0x009EEFC0: allocate and publish the animation manager, including a null allocation result.
bool CreateManager();
}

namespace ts3::anim {
// ABI-only member facade for generated thiscall replacement wrappers.
struct m158 : M158::Steering {
    bool Refresh(int32_t body,int32_t brain) { return M158::Refresh(this,body,brain); }
    bool Attach(void* entity,void* retained,uint32_t a,uint32_t b,uint32_t argument,int8_t transform) { return M158::Attach(this,entity,retained,a,b,argument,uint8_t(transform)); }
    void* Construct(void* host) { return M158::Construct(this,host); }
    static void* ConstructIfPresent(void* storage,void* host) { return M158::ConstructIfPresent(storage,host); }
    static bool CreateManager() { return M158::CreateManager(); }
};
}
