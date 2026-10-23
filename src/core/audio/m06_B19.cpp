#include "ts3/audio/m06_B19.h"
namespace ts3::audio {
B19Services b19Services{};
namespace {
b19* Init(b19* self, unsigned index) {
#if defined(_M_IX86)
    const uintptr_t tables[]={0x01029920,0x010299B0,0x01029A38,0x01029AC0,0x01029B50};
    self->vtable=reinterpret_cast<void**>(tables[index]);
    self->secondary=reinterpret_cast<void*>(0x01029EE0);
    self->reference=reinterpret_cast<void*>(0x01029888);
#else
    self->vtable=b19Services.tables[index];
    self->secondary=b19Services.secondary;
    self->reference=b19Services.reference;
#endif
    self->state=0;
    self->enabled=1;
    self->value=0.0f;
    self->link=nullptr;
    self->owner=nullptr;
    self->nameHash=0;
    return self;
}
uint32_t Name(b19* self,const char* text) {
    if (self->nameHash == 0) {
#if defined(_M_IX86)
        self->nameHash=CallGameFunction<uint32_t,0x004FA590>(text,0x811c9dc5u,1u);
#else
        self->nameHash=b19Services.hash(text,0x811c9dc5u,1u);
#endif
    }
    return self->nameHash;
}
const B19Weather& Weather() {
#if defined(_M_IX86)
    auto adjusted=*reinterpret_cast<uint8_t**>(0x011D8C40);
    // EA checks null only before subtracting, then unconditionally dereferences.
    auto manager=adjusted ? adjusted-0x27BD0 : nullptr;
    return *reinterpret_cast<const B19Weather*>(manager+0x27D48);
#else
    return *b19Services.weather;
#endif
}
float UIntFloat(uint32_t bits) {
#if defined(_M_IX86)
    // Keep EA's signed fild + conditional 2^32 correction under the caller's CW.
    float result;
    const float correction=4294967296.0f;
    __asm {
        fild dword ptr [bits]
        cmp dword ptr [bits], 0
        jge converted
        fadd dword ptr [correction]
    converted:
        fstp dword ptr [result]
    }
    return result;
#else
    return static_cast<float>(bits);
#endif
}
uintptr_t Publish(b19* self,float value) {
#if defined(_M_IX86)
    return CallGameVirtual<uintptr_t,14>(self,value);
#else
    return b19Services.publish(self,value);
#endif
}
}
b19* b19::InitFrozen() { return Init(this,0); }
b19* b19::InitHail() { return Init(this,1); }
b19* b19::InitRain() { return Init(this,2); }
b19* b19::InitSeason() { return Init(this,3); }
b19* b19::InitSnow() { return Init(this,4); }
uint32_t b19::NameFrozen() { return Name(this,"isfrozen"); }
uint32_t b19::NameHail() { return Name(this,"ishail"); }
uint32_t b19::NameRain() { return Name(this,"israin"); }
uint32_t b19::NameSeason() { return Name(this,"isseason"); }
uint32_t b19::NameSnow() { return Name(this,"issnow"); }
uintptr_t b19::EvaluateHail(uint32_t, uint32_t) {
    return Publish(this,Weather().kind == 2 ? 1.0f : 0.0f);
}
uintptr_t b19::EvaluateRain(uint32_t, uint32_t) {
    const auto& w=Weather();
    float value=0.0f;
    if (w.kind == 1 || w.kind == 5) value=UIntFloat(w.intensity);
    return Publish(this,value);
}
uintptr_t b19::EvaluateSeason(uint32_t, uint32_t) {
    float value=0.0f;
    switch (Weather().season) {
    case 2: value=1.0f; break;
    case 4: value=2.0f; break;
    case 8: value=3.0f; break;
    }
    return Publish(this,value);
}
uintptr_t b19::EvaluateSnow(uint32_t, uint32_t) {
    const auto& w=Weather();
    return Publish(this,w.kind == 3 ? UIntFloat(w.intensity) : 0.0f);
}
}
