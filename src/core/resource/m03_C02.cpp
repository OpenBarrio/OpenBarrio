#include "ts3/resource/m03_C02.h"
namespace ts3::resource {
C02Boundary& C02Dependencies() { static C02Boundary b; return b; }
void* C02DestroyRef(void* p,uint32_t flags) {
    return C02Destroy(static_cast<uint8_t*>(p)-3*sizeof(void*),flags);
}
uintptr_t C02DatabaseMethod(void* p) {
    // -8 followed by +8: preserve the incoming database subobject exactly.
    if(auto f=C02Dependencies().databaseMethod) return f(p);
    return CallGameMethod<uintptr_t,0x0072C920>(p);
}
void* C02DestroyDatabase(void* p,uint32_t flags) {
    return C02Destroy(static_cast<uint8_t*>(p)-2*sizeof(void*),flags);
}
void* C02Destroy(void* p,uint32_t flags) {
    // Opaque mutex storage: original x86 displacement is C0. No pointer is truncated.
    void* mutex=static_cast<uint8_t*>(p)+0xC0;
    if(auto f=C02Dependencies().deleteMutex) f(mutex);
    else {
#if defined(_M_IX86)
        using Fn=void(__stdcall*)(void*);
        (*reinterpret_cast<Fn*>(uintptr_t{0x00F95128}))(mutex);
#else
        AbortNotRewritten("ShadowWatcher mutex deletion");
#endif
    }
    void* db=p ? static_cast<uint8_t*>(p)+2*sizeof(void*) : nullptr;
    if(auto f=C02Dependencies().destroyDatabase) f(db);
    else CallGameMethod<void,0x0072CB00>(db);
#if defined(_M_IX86)
    *static_cast<void**>(p)=reinterpret_cast<void*>(uintptr_t{0x00FB9C58});
#else
    static const unsigned char baseVtableToken=0;
    *static_cast<void**>(p)=const_cast<unsigned char*>(&baseVtableToken);
#endif
    if(flags & 1) {
        if(auto f=C02Dependencies().freeResource) f(p);
        else CallGameFunction<void,0x004E4230>(p);
    }
    return p;
}
void* C02_ABI HookC02Ref(void* p,void*,uint32_t f) { return C02DestroyRef(p,f); }
uintptr_t C02_ABI HookC02Method(void* p,void*) { return C02DatabaseMethod(p); }
void* C02_ABI HookC02Database(void* p,void*,uint32_t f) { return C02DestroyDatabase(p,f); }
void* C02_ABI HookC02Destroy(void* p,void*,uint32_t f) { return C02Destroy(p,f); }
}
