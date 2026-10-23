#include "ts3/audio/m06_B13.h"
#include <cmath>
#include <cstring>
// Avoid the legacy duplicate WideString definition in clu_408e20_Scratch.h.
namespace ts3::scratch { void* __fastcall AssignWide408E20(WideString*, void*, const void*, const void*); }
namespace ts3::audio::b13 {
namespace {
void Lock() { CallGameFunction<void,0x00F02F10>(); }
void Unlock() { CallGameFunction<void,0x00F02F20>(); }
}
uint32_t __fastcall SetFlags(Player* p, void*, uint32_t mask, uint8_t enabled) {
    if (enabled) p->flags |= mask;
    else p->flags &= ~mask;
    return p->flags;
}
uint8_t __fastcall Close(Player* p, void*) {
    if (p->flags & 1) {
        CallGameVirtual<void,6>(p);
        p->flags &= ~1u;
        CallGameMethod<void,0x00F10410>(p->handle);
        p->handle = nullptr;
    }
    return 1;
}
uint8_t __fastcall Stop(Player* p, void*) {
    if (!(p->flags & 0x20)) return 0;
    p->flags = (p->flags & ~0x60u) | 0x400;
    Lock();
    CallGameMethod<void,0x00F01E00>(p->handle);
    Unlock();
    return 1;
}
uint8_t __fastcall SetLoop(Player* p, void*, uint8_t value) {
    const uint8_t old = (p->flags & 0x1000) != 0;
    if (old != value) p->flags |= 0x2000;
    else p->flags &= ~0x2000u;
    if (value) p->flags |= 0x1000;
    else p->flags &= ~0x1000u;
    return 1;
}
uint8_t __fastcall IsLooping(Player* p, void*) { return (p->flags & 0x1000) != 0; }
uint32_t __fastcall SetKey(Player* p, void*, const uint32_t* source) {
    // Adapted from verified ResourceKeyTarget::AssignKey (004AF0E0), fields +20..2c.
    p->key[0] = source[0];
    p->key[1] = source[1];
    p->key[2] = source[2];
    p->key[3] = source[3];
    return p->key[3];
}
void __fastcall SetDeferred(Player* p, void*, uint8_t value) {
    if (value) p->flags |= 0x800;
    else p->flags &= ~0x800u;
}
#if defined(_M_IX86)
// No float store on the return path: callers may consume extended ST0 directly.
__declspec(naked) float __fastcall Attenuation(Player*, void*, float, float, float, float) {
    __asm {
        movss xmm0, dword ptr [esp+12]
        comiss xmm0, dword ptr [esp+4]
        ja unity
        movss xmm0, dword ptr [esp+8]
        xorps xmm1, xmm1
        ucomiss xmm0, xmm1
        jp calculate
        je unity
    calculate:
        fld dword ptr [esp+4]
        fdiv dword ptr [esp+12]
        fld1
        fsub st(1), st(0)
        fxch st(1)
        fmul dword ptr [esp+8]
        fadd st(0), st(1)
        fdivp st(1), st(0)
        ret 16
    unity:
        fld1
        ret 16
    }
}
#else
float __fastcall Attenuation(Player*, void*, float distance, float rolloff, float maximum, float) {
    if (maximum > distance || rolloff == 0.0f) return 1.0f;
    const long double ratio = static_cast<long double>(distance) / maximum;
    return static_cast<float>(1.0L / ((ratio - 1.0L) * rolloff + 1.0L));
}
#endif
uint8_t __fastcall Start(Player* p, void*) {
    if (p->flags & 0x40) return 0;
    uint8_t spatial;
    if (p->resource && CallGameVirtual<uint8_t,9>(p->resource,0x12d2a4d4u,&spatial)) {
        if (spatial) p->flags |= 8;
        else p->flags &= ~8u;
    }
    p->flags |= 0x20;
    return 1;
}
uint8_t __fastcall GetProperty(Player* p, void*, uint32_t key, float* out) {
    switch (key) {
    case 0x03593710: *out = p->maximum; break;
    case 0x050c5d65: *out = p->position[2]; break;
    case 0x050c5d66: *out = p->position[1]; break;
    case 0x050c5d67: *out = p->position[0]; break;
    case 0x0f616b72: *out = p->rolloff; break;
    case 0x12d2a4d4: *out = (p->flags & 8) ? 1.0f : 0.0f; break;
    case 0x2406a047: *out = p->distance; break;
    case 0x25df0108: *out = p->gain; break;
    case 0x49eb68db: *out = p->level; break;
    case 0x647a7836: *out = p->pan; break;
    case 0x6dd08218: *out = (p->flags & 0x10) ? 1.0f : 0.0f; break;
    case 0x71bc3009: *out = p->speed; break;
    case 0xa26a765f: *out = p->pitch; break;
    case 0xaa4e5a2e: *out = 1.0f; break;
    case 0x893f8476: {
        Lock();
        uint32_t state;
        CallGameMethod<void,0x00F02020>(p->handle,&state);
        if (state == 0) CallGameMethod<void,0x00F02090>(p->handle,out);
        else *out = 0.0f;
        Unlock();
        break;
    }
    case 0xc2d1f579:
        Lock(); CallGameMethod<void,0x00F01FF0>(p->handle,out); Unlock(); break;
    case 0xd24d4aff:
        Lock(); CallGameMethod<void,0x00F020C0>(p->handle,out); Unlock(); break;
    default: return 0;
    }
    return 1;
}
uint8_t __fastcall GetPropertyContinuation(Player* p, void*, uint32_t key, float* out) {
    return GetProperty(p,nullptr,key,out);
}
void __fastcall SetResource(Player* p, void*, void* resource) {
    void* old = p->resource;
    if (old != resource) {
        if (resource) CallGameVirtual<void,0>(resource);
        p->resource = resource;
        if (old) CallGameVirtual<void,1>(old);
    }
}
void __fastcall Update(Player* p, void*, uint32_t, uint32_t) {
    if (!(p->flags & 0x40)) return;
    if (p->flags & 8) {
        float listener[3], basis[9];
        auto system = CallGameFunction<void*,0x008B0A70>();
        CallGameVirtual<void,35>(system,0u,listener,basis);
        const float x = p->position[0] - listener[0];
        const float y = p->position[1] - listener[1];
        const float z = p->position[2] - listener[2];
        const float a = (basis[3]*y + basis[6]*z) + basis[0]*x;
        const float b = (basis[1]*x + basis[7]*z) + basis[4]*y;
        const float c = (basis[8]*z + basis[5]*y) + basis[2]*x;
        float gain;
#if defined(_M_IX86)
        float spatialLength;
        __asm {
            fld c
            fmul st(0), st(0)
            fld b
            fmul st(0), st(0)
            faddp st(1), st(0)
            fld a
            fmul st(0), st(0)
            faddp st(1), st(0)
            fsqrt
            fstp spatialLength
            mov ecx, p
            push dword ptr [ecx+58h]
            push dword ptr [ecx+54h]
            push dword ptr [ecx+40h]
            push spatialLength
            mov eax, dword ptr [ecx]
            call dword ptr [eax+50h]
            mov ecx, p
            fmul dword ptr [ecx+3ch]
            fstp gain
        }
#else
        const float length = static_cast<float>(std::sqrt((static_cast<long double>(c)*c + static_cast<long double>(b)*b) + static_cast<long double>(a)*a));
        const float attenuation = CallGameVirtual<float,20>(p,length,p->distance,p->rolloff,p->maximum);
        gain = static_cast<float>(static_cast<long double>(attenuation)*p->gain);
#endif
        Lock(); CallGameMethod<void,0x00F01F10>(p->handle,gain); Unlock();
    }
    if (p->flags & 0x4000) {
        Lock(); CallGameMethod<void,0x00F01FB0>(p->handle,p->pitch); Unlock();
        p->flags &= ~0x4000u;
    }
    if (p->flags & 0x8000) {
        Lock(); CallGameMethod<void,0x00F01F70>(p->handle,p->pan); Unlock();
        p->flags &= ~0x8000u;
    }
    if (p->flags & 0x10000) {
        Lock(); CallGameMethod<void,0x00F01E80>(p->handle,0u,p->level); Unlock();
        p->flags &= ~0x10000u;
    }
    if (p->flags & 0x20000) {
        Lock(); CallGameMethod<void,0x00F01ED0>(p->handle,p->speed); Unlock();
        p->flags &= ~0x20000u;
    }
}
void __cdecl SwapEntries(PlaylistEntry* a, PlaylistEntry* b) {
    PlaylistEntry temporary;
    CallGameMethod<void,0x00408EB0>(&temporary.name,&a->name);
    for (unsigned i=0;i<32;++i) temporary.payload[i]=a->payload[i];
    if (a != b) scratch::AssignWide408E20(&a->name,nullptr,b->name.begin,b->name.cur);
    for (unsigned i=0;i<32;++i) a->payload[i]=b->payload[i];
    scratch::AssignWide408E20(&b->name,nullptr,temporary.name.begin,temporary.name.cur);
    for (unsigned i=0;i<32;++i) b->payload[i]=temporary.payload[i];
    const auto capacity = (reinterpret_cast<uintptr_t>(temporary.name.cap)-reinterpret_cast<uintptr_t>(temporary.name.begin)) & ~uintptr_t(1);
    if (static_cast<intptr_t>(capacity)>2 && temporary.name.begin)
        CallGameFunction<void,0x004E3F70>(temporary.name.begin);
}
PlaylistEntry* __cdecl CopyBackward(const PlaylistEntry* first, const PlaylistEntry* last, PlaylistEntry* out) {
    while (first != last) {
        --last;
        --out;
        if (last != out) scratch::AssignWide408E20(&out->name,nullptr,last->name.begin,last->name.cur);
        for (unsigned i=0;i<32;++i) out->payload[i]=last->payload[i];
    }
    return out;
}
}
