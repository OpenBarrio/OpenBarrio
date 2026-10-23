#include "ts3/resource/m03_B05.h"
#include "ts3/layout.h"
namespace ts3::resource::b05 {
thread_local const Boundaries* boundaries=nullptr;
namespace {
void* Script() { return boundaries?boundaries->script():CallGameFunction<void*,0x7692B0>(); }
const void* Constant(const void* Boundaries::*field,uintptr_t address) {
    if(boundaries) return boundaries->*field;
#if defined(_M_IX86)
    return reinterpret_cast<const void*>(address);
#else
    (void)address; AbortNotRewritten("M03 B05 native bindings"); return nullptr;
#endif
}
uintptr_t FactoryName(uintptr_t arg,bool assembly) {
    void* factory=CallGameVirtual<void*,95>(Script());
    return assembly?CallGameVirtual<uintptr_t,19>(factory,arg):CallGameVirtual<uintptr_t,18>(factory,arg);
}
uintptr_t Lookup(uintptr_t arg,bool string) {
    void* text=CallGameVirtual<void*,22>(Script(),arg);
    void* locale=boundaries?boundaries->locale():CallGameFunction<void*,0x59ADA0>();
    if(boundaries) return boundaries->lookup(locale,text,string);
    return string?CallGameMethod<uintptr_t,0x59C850>(locale,text):CallGameMethod<uintptr_t,0x59C800>(locale,text);
}
void* Allocate() {
    if(boundaries) return boundaries->allocate();
    return CallGameFunction<void*,0x4E3F90>(4,reinterpret_cast<const char*>(0xFDE6D8),0u,0u,0u,0u);
}
uintptr_t Register(void* service,const void* vtable) {
    void* object=Allocate();
    if(object) *static_cast<const void**>(object)=vtable;
    return CallGameVirtual<uintptr_t,18>(service,object);
}
}
uintptr_t __cdecl DataClassName(uintptr_t arg) { return FactoryName(arg,false); }
uintptr_t __cdecl DataAssemblyName(uintptr_t arg) { return FactoryName(arg,true); }
const void** __fastcall DataTable(void*,void*,const void** out) {
    *out=Constant(&Boundaries::dataTable,0x115DDF0); return out;
}
const void* __fastcall DataName(void*,void*) { return Constant(&Boundaries::dataName,0x1000744); }
uintptr_t __cdecl RegisterData(void* service) { return Register(service,Constant(&Boundaries::dataVtable,0x1000750)); }
uintptr_t __cdecl LocaleUInt64(uintptr_t arg) { return Lookup(arg,false); }
uintptr_t __cdecl LocaleString(uintptr_t arg) { return Lookup(arg,true); }
uintptr_t __cdecl LocaleUpdate(uintptr_t arg) {
    if(boundaries) return boundaries->tail(0,arg);
    return CallGameFunction<uintptr_t,0x59C870>(arg);
}
uintptr_t __cdecl ChangeLocale() {
    if(boundaries) return boundaries->tail(1,0);
    return CallGameFunction<uintptr_t,0x59C890>();
}
uintptr_t __cdecl GetLocale() {
    if(boundaries) return boundaries->tail(2,0);
    return CallGameFunction<uintptr_t,0x59C960>();
}
uintptr_t __cdecl LogoSuffix() {
    if(boundaries) return boundaries->tail(3,0);
    return CallGameFunction<uintptr_t,0x59CA30>();
}
const void** __fastcall LocaleTable(void*,void*,const void** out) {
    *out=Constant(&Boundaries::localeTable,0x1160288); return out;
}
uintptr_t __cdecl RegisterLocale(void* service) { return Register(service,Constant(&Boundaries::localeVtable,0x1001AF8)); }
}
