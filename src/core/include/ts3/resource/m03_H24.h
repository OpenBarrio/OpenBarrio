#pragma once
#include "ts3/resource/m03_A06.h"
namespace ts3::resource::m03::h24 {
using Context = a06::Context;
// Native dependency injection; null dispatch uses the x86 original bridges.
struct Dispatch {
    uintptr_t (*call)(void*,unsigned)=nullptr;
    uintptr_t (*size)(void*)=nullptr;
    void (*read)(void*,void*,uint32_t)=nullptr;
    void* (*allocate)(unsigned,uint32_t)=nullptr;
    void* (*memoryStream)(void*,void*,uint32_t)=nullptr;
    void* (*record)(void*,void*,void*)=nullptr;
    uintptr_t (*parse)(Context*,void*,uintptr_t)=nullptr;
    uintptr_t (*build)(void*,void*,Context*)=nullptr;
    void (*clear)(Context*)=nullptr;
    uintptr_t (*free)(void*)=nullptr;
};
Dispatch* SetDispatch(Dispatch*);
// TS3W.exe 0x00726D90: validates factory type and consumes a record to populate a resource, retaining cleanup EAX high bits.
uintptr_t __fastcall Load(void* self,void*,void* record,void* resource,Context* context,uint32_t type);
// TS3W.exe 0x00726DB7: internal continuation of Load, not independently callable or hookable; retains inverse ownership tags.
// This extent is implemented by the matching-type path of Load above.
}
