#include "ts3/runtime/m02_C1.h"
namespace ts3::runtime::m02::c1 {
namespace {
template<unsigned Address> void* Global() {
#if defined(_M_IX86)
    return *reinterpret_cast<void**>(Address);
#else
    NotRewritten("M02 C1 image global");
#endif
}
StopWatch* Lookup(uint32_t handle) {
    auto scripting=CallGameFunction<void*,0x007692B0>();
    return CallGameVirtual<StopWatch*,51>(scripting,handle);
}
}
void* __cdecl DefaultWrapper() { return Global<0x011CB530>(); }
void* __cdecl AlternateWrapper() { return Global<0x011CB534>(); }
void* __cdecl SchedulerSingleton() { return Global<0x011CE2CC>(); }
void* __cdecl ServiceSingleton() { return Global<0x011CE2C8>(); }
void* __fastcall NativeHandle(Thread* self,void*) {
    return self->record ? self->record->handle : nullptr;
}
void* __fastcall DeleteThreadBase(void* self,void*,uint32_t flags) {
#if defined(_M_IX86)
    *static_cast<void***>(self)=reinterpret_cast<void**>(0x00FB89C8);
#else
    static void* nativeVtable[]={reinterpret_cast<void*>(&DeleteThreadBase)};
    *static_cast<void***>(self)=nativeVtable;
#endif
    if(flags & 1) CallGameFunction<void,0x004E4230>(self);
    return self;
}
void __fastcall ReplaceCallback(CallbackOwner* self,void*,void* incoming) {
    if(incoming) CallGameVirtual<void,0>(incoming);
    if(self->callback) CallGameVirtual<void,1>(self->callback);
    self->callback=incoming;
}
uint32_t __fastcall SetSecondaryConfig(void* secondary,void*,uint32_t value) {
    auto* owner=reinterpret_cast<JobService*>(static_cast<uint8_t*>(secondary)-offsetof(JobService,secondary));
    owner->config=value;
    return value;
}
const void* __fastcall RegistrationName(void*,void*) {
#if defined(_M_IX86)
    return reinterpret_cast<const void*>(0x0100C484);
#else
    static const char name[]="StopWatch";
    return name;
#endif
}
uint32_t __cdecl Stop(uint32_t handle) {
    uint32_t tag=handle & 0xFF000000u;
    if(tag!=0x10000000u) return tag;
    auto* watch=Lookup(handle);
    if(!watch) return 0;
    return CallGameMethod<uint32_t,0x004F3220>(watch);
}
uint32_t __cdecl Elapsed(uint32_t handle) {
    if((handle & 0xFF000000u)!=0x10000000u) return 0;
    auto* watch=Lookup(handle);
    if(!watch) return 0;
    return CallGameMethod<uint32_t,0x004F32C0>(watch);
}
uint32_t __cdecl Start(uint32_t handle) {
    uint32_t tag=handle & 0xFF000000u;
    if(tag!=0x10000000u) return tag;
    auto* watch=Lookup(handle);
    if(!watch) return 0;
    return CallGameMethod<uint32_t,0x00408700>(watch);
}
uint32_t __cdecl Restart(uint32_t handle) {
    uint32_t tag=handle & 0xFF000000u;
    if(tag!=0x10000000u) return tag;
    auto* watch=Lookup(handle);
    if(!watch) return 0;
    return CallGameMethod<uint32_t,0x004E3380>(watch);
}
const void** __fastcall RegistrationTable(void*,void*,const void** output) {
#if defined(_M_IX86)
    *output=reinterpret_cast<const void*>(0x01161838);
#else
    NotRewritten("M02 native stopwatch binding table");
#endif
    return output;
}
}
