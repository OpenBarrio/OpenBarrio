#include "ts3/audio/m06_B42.h"
namespace ts3::audio {
namespace {
template<unsigned Head, unsigned Pool, unsigned Size> void* Allocate(uint32_t size) {
#if defined(_M_IX86)
    if (size == Size) {
        auto& head = *reinterpret_cast<void**>(Head);
        for (;;) {
            void* p = head;
            if (p) { head = *static_cast<void**>(p); return p; }
            if (!CallGameMethod<uint8_t, 0x004E42F0>(reinterpret_cast<void*>(Pool), 0u, 0u)) return nullptr;
        }
    }
#endif
    return CallGameFunction<void*, 0x005C32D0>(size);
}
}
int32_t __fastcall B42Release(B42Value* p, void*) {
    // Verified MemoryRelease twin; this family uses refs+10 and deleting slot 2.
    if (p->refs > 1) return --p->refs;
    CallGameVirtual<void, 2>(p, 1u);
    return 0;
}
void* __cdecl B42AllocateValue(uint32_t n) { return Allocate<0x011DA00C,0x011D9FFC,0x18>(n); }
B42Value* __fastcall B42Construct(B42Value* p, void*, void* owner) {
    p->state = 0; p->refs = 0;
#if defined(_M_IX86)
    p->table = reinterpret_cast<void*>(0x0103A424);
    p->value = *reinterpret_cast<const float*>(0x0116A7A0);
#else
    p->table = nullptr; p->value = 1.0f;
#endif
    p->enabled = 1; p->owner = owner;
    return p;
}
B42Value* __fastcall B42Delete(B42Value* p, void*, uint32_t flags) {
#if defined(_M_IX86)
    p->table = reinterpret_cast<void*>(0x00FA1B78);
    if (flags & 1) {
        auto& head = *reinterpret_cast<void**>(0x011DA00C);
        p->table = head; head = p;
    }
#else
    p->table = nullptr;
    if (flags & 1) NotRewritten("B42 value pool");
#endif
    return p;
}
uint32_t __fastcall B42Reset(B42Scene* p, void*) {
    const float points[] = {0,0,1,1};
    auto r = B42SetCurve(p, nullptr, points, 2);
    return (static_cast<uint32_t>(r) & 0xffffff00u) | 1;
}
float __fastcall B42ValueGet(B42Scene* p, void*) { return p->value; }
void* __fastcall B42Context(B42Scene* p, void*) { return p->context; }
uint8_t __fastcall B42Enabled(B42Scene* p, void*) { return p->enabled; }
uint8_t __fastcall B42SetEnabled(B42Scene* p, void*, uint32_t v) { return p->enabled = static_cast<uint8_t>(v); }
void __fastcall B42Register(B42Scene* p, void*, uint32_t v) {
    if (static_cast<uint8_t>(v)) {
        auto* system = CallGameFunction<void*,0x008B0A70>();
        CallGameVirtual<void,116>(system,p);
        p->registered = 1;
    } else if (p->registered) {
        auto* system = CallGameFunction<void*,0x008B0A70>();
        CallGameVirtual<void,117>(system,p);
        p->registered = 0;
    }
}
uintptr_t __fastcall B42SetCurve(B42Scene* p, void*, const float* points, int32_t count) {
    return CallGameMethod<uintptr_t,0x008AAAD0>(&p->curve,points,count);
}
void* __cdecl B42AllocateScene(uint32_t n) { return Allocate<0x011DA038,0x011DA028,0x388>(n); }
uint8_t __fastcall B42Stop(B42Scene* p, void*) {
    CallGameVirtual<void,12>(p,0u);
    B42Scene* linked = p->linked;
    if (linked) {
        auto type = CallGameVirtual<uint32_t,13>(p);
        if (CallGameVirtual<uint32_t,13>(linked) == type) CallGameVirtual<void,5>(p->linked);
        linked = p->linked;
        if (linked) {
            p->linked = nullptr;
            CallGameVirtual<void,1>(linked);
        }
    }
    return 1;
}
void __fastcall B42Update(B42Scene* p, void*, float value, uint32_t stamp) {
    if (p->stamp && p->stamp == stamp) return;
    p->stamp = stamp;
    if (!p->linked) return;
    CallGameVirtual<void,11>(p->linked,value,stamp);
    if (CallGameVirtual<uint8_t,9>(p->linked)) CallGameVirtual<void,10>(p,1u);
    if (CallGameVirtual<uint8_t,9>(p)) {
        float input = CallGameVirtual<float,8>(p->linked);
        p->value = CallGameMethod<float,0x008AA6B0>(&p->curve,input);
    }
}
void __fastcall B42Link(B42Scene* p, void*, B42Scene* linked) {
    B42Scene* old = p->linked;
    if (old == linked) return;
    if (linked) CallGameVirtual<void,0>(linked);
    p->linked = linked;
    if (old) CallGameVirtual<void,1>(old);
}
}
