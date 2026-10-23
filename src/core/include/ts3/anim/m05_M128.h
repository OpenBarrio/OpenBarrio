#pragma once
#include "ts3/layout.h"
#include "ts3/math/float_values.h"
namespace ts3::anim::m128_impl {
struct Ref { void** vtable; int32_t refs; };
struct Style { void** vtable; uint32_t opaque[2]; void* host; };
struct Body {
    unsigned char opaque0[0xf0];
    unsigned char state[0x1c]; uint32_t mode;
    unsigned char opaque110[0x6f]; uint8_t flag17f;
    unsigned char opaque180[0xc8]; uint32_t limit, progress;
    uint8_t flag250,flag251;
};
struct Controller {
    Ref base; void** queryVtable;
    unsigned char opaque[0x2c-12];
    Ref* indirect; Controller* child;
};
struct RootController { void** vtable; unsigned char opaque[0x1c-4]; void* model; };
struct alignas(16) Pose { Ref base; ts3::math::Float4Value translation,rotation; };
struct alignas(16) Evaluation {
    float matrix[16]; void* rig; float zero; uint32_t opaque[2]; float duration; uint32_t mode;
    unsigned char rest[0x120];
};
TS3_X86_OFFSET(Style,host,0xc);
TS3_X86_OFFSET(Controller,indirect,0x2c);
TS3_X86_OFFSET(Controller,child,0x30);
TS3_X86_OFFSET(RootController,model,0x1c);
TS3_X86_OFFSET(Evaluation,duration,0x50);
TS3_X86_OFFSET(Body,limit,0x248);
// TS3W.exe 0x008481C0: destroy the style adapter and optionally free it; returns this even after free.
void* DeleteStyle(Style*,uint32_t);
// TS3W.exe 0x008481F0: dispatch style progress callbacks and release the retained style resource.
uintptr_t UpdateStyle(Style*,uint32_t);
// TS3W.exe 0x00848590: test whether the animated body has an unblocked pending progression.
uint32_t CanProgress(Body*);
// TS3W.exe 0x00848A90: find a controller through query interfaces with a signed depth limit, consuming its reference.
Controller** FindController(Controller**,Controller*,int32_t);
// TS3W.exe 0x008492B0: sample end-of-clip root translation/rotation and derive speed, restoring controller time.
double RootMotion(RootController*,ts3::math::Float4Value*,ts3::math::Float4Value*,float*);
// TS3W.exe 0x00849520: sample root motion while discarding the derived speed.
double RootDistance(RootController*,ts3::math::Float4Value*,ts3::math::Float4Value*);
}
// Manifest ABI facade: thiscall methods operate on their original subobject;
// cdecl entries accept opaque pointers from the registration adapter.
namespace ts3::anim {
struct m128 {
    void* DeleteStyle(uint32_t f) { return m128_impl::DeleteStyle(reinterpret_cast<m128_impl::Style*>(this),f); }
    uintptr_t UpdateStyle(uint32_t f) { return m128_impl::UpdateStyle(reinterpret_cast<m128_impl::Style*>(this),f); }
    uint32_t CanProgress() { return m128_impl::CanProgress(reinterpret_cast<m128_impl::Body*>(this)); }
    static double RootMotion(void* p,void* t,void* r,void* s) {
        return m128_impl::RootMotion(static_cast<m128_impl::RootController*>(p),static_cast<math::Float4Value*>(t),static_cast<math::Float4Value*>(r),static_cast<float*>(s));
    }
    static double RootDistance(void* p,void* t,void* r) {
        return m128_impl::RootDistance(static_cast<m128_impl::RootController*>(p),static_cast<math::Float4Value*>(t),static_cast<math::Float4Value*>(r));
    }
};
}
