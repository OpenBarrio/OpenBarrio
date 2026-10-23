#include "ts3/resource/m03_A01.h"
#include "ts3/layout.h"
#include <cstring>
#include <xmmintrin.h>

namespace ts3::resource::m03::a01 {
namespace {
constexpr uint32_t timeout=0xffffffffu;
}
Manager* Singleton() {
#if defined(_M_IX86)
    return *reinterpret_cast<Manager**>(0x011c78d8);
#else
    ts3::NotRewritten("m03 A01 singleton native provider");
#endif
}
uint8_t Initialize(Manager* self) {
    if (!self->initialized) self->initialized=1;
    return 1;
}
void* DelegateDatabase(Manager* self,const Key* key) {
    // Flags alias the STACK COPY of the pointer, not the resource key itself.
    return ts3::CallGameVirtual<void*,16>(self,key,reinterpret_cast<uint32_t*>(&key));
}
int32_t DatabaseLock(Manager* self,uint32_t lock) {
    if (lock & 0xff) return ts3::CallGameMethod<int32_t,0x004e16f0>(&self->mutex,&timeout);
    return ts3::CallGameMethod<int32_t,0x004e17b0>(&self->mutex);
}
uint8_t Control(Manager* self,uint32_t add,void* pointer,uint32_t type) {
    if (!pointer) return 0;
    auto& c=self->controls;
    if (add & 0xff) {
        for (unsigned i=0;i<7;++i) if (!c.pointers[i]) {
            c.pointers[i]=pointer;
            c.types[i]=static_cast<uint16_t>(type);
            return 1;
        }
        return 0;
    }
    for (unsigned i=0;c.pointers[i];++i) if (c.pointers[i]==pointer) {
        // EA keeps the old type array and shifts all remaining POINTER slots,
        // including the reserved eighth sentinel (no explicit clearing).
        std::memmove(c.pointers+i,c.pointers+i+1,(7-i)*sizeof(void*));
        return 1;
    }
    return 0;
}
void* FindControl(Manager* self,uint32_t type) {
    auto& c=self->controls;
    for (unsigned i=0;c.pointers[i];++i)
        if (c.types[i]==static_cast<uint16_t>(type)) return c.pointers[i];
    return nullptr;
}
uint32_t ReadPolicy(Manager* self,float* out) {
    if (out) {
#if defined(_M_IX86)
        float* source=&self->controls.factor;
        __asm { mov eax,source
                fld dword ptr [eax]
                mov eax,out
                fstp dword ptr [eax] }
#else
        // Match FLD/FSTP's observable sNaN quieting without x87 on x64.
        uint32_t bits;
        std::memcpy(&bits,&self->controls.factor,4);
        if ((bits & 0x7f800000u)==0x7f800000u && (bits & 0x007fffffu)) bits |= 0x00400000u;
        std::memcpy(out,&bits,4);
#endif
    }
    return self->controls.policy;
}
uint32_t SetPolicy(Manager* self,uint32_t policy,float factor) {
    self->controls.policy=policy;
    // SSE comparisons intentionally honor ambient DAZ, like UCOMISS. NaNs
    // keep their original payload, and even negative nonzero factors survive.
    const __m128 x=_mm_set_ss(factor),zero=_mm_setzero_ps();
    const bool unordered=(_mm_movemask_ps(_mm_cmpunord_ss(x,zero)) & 1)!=0;
    const bool equal=(_mm_movemask_ps(_mm_cmpeq_ss(x,zero)) & 1)!=0;
    // UCOMISS, unlike CMPLTSS, does not raise invalid for a quiet NaN.
    const bool negative=_mm_ucomilt_ss(x,zero)!=0;
    const uint32_t ah=unordered ? 0x47u : equal ? 0x42u : negative ? 0x03u : 0x02u;
    self->controls.factor=equal ? 0.8f : factor;
    return (policy & 0xffff00ffu) | (ah<<8);
}
void** PointerSlots(Manager* self) { return self->controls.pointers; }
uint8_t ParseName(Manager* self,ts3::misc::ResourceSortKey* out,const char16_t* text,uint32_t type,uint32_t group) {
    return ts3::CallGameFunction<uint8_t,0x004aece0>(out,text,type,group,self,static_cast<void*>(nullptr));
}
uint8_t ContainsDatabase(Manager* self,void* database) {
    DatabaseLock(self,1);
    auto* p=self->databases.begin();
    auto* end=self->databases.end();
    for (;p!=end;++p) if (p->database==database) {
        DatabaseLock(self,0);
        return 1;
    }
    DatabaseLock(self,0);
    return 0;
}
void* FindDatabase(Manager* self,const Key* key,uint32_t* flags) {
    DatabaseLock(self,1);
    auto* p=self->databases.begin();
    auto* end=self->databases.end();
    for (;p!=end;++p) {
        void* database=p->database;
        DatabaseLock(self,0);
        const auto result=ts3::CallGameVirtual<uint32_t,13>(database,key,static_cast<void*>(nullptr),1,6,1,0);
        if (result & 0xff) {
            *flags=p->flags;
            DatabaseLock(self,1);
            DatabaseLock(self,0);
            return database;
        }
        DatabaseLock(self,1);
    }
    DatabaseLock(self,0);
    return nullptr;
}
manager::RawCallback* FindCallback(Manager* self,const uint32_t* type) {
    auto* p=self->callbacks.begin();
    auto* end=self->callbacks.end();
    for (;p!=end;++p) {
        auto* callback=p->callback;
        if (ts3::CallGameVirtual<uint32_t,6>(callback,type,0) & 0xff) return callback;
    }
    return nullptr;
}
}
