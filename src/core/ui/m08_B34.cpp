#include "ts3/ui/m08_B34.h"
#include "ts3/layout.h"
#include <cstring>
namespace ts3::ui::b34 {
namespace {
const FactoryServices* services=nullptr;
template<class T> T& field(Grid* s,size_t offset) { return *reinterpret_cast<T*>(reinterpret_cast<uint8_t*>(s)+offset+sizeof(void*)-4); }
uint32_t alOne(uint32_t v) { return (v&0xffffff00u)|1; }
template<class... A> uint32_t slot(void* s,size_t n,A... a) {
    using F=uint32_t(__fastcall*)(void*,void*,A...);
    return reinterpret_cast<F>((*static_cast<void***>(s))[n])(s,nullptr,a...);
}
void copyFloat(volatile float* src,float* dst) {
#if defined(_M_IX86)
    // FLD/FSTP quiets signalling NaNs and does not apply SSE DAZ/FTZ.
    __asm { mov eax, src }
    __asm { fld dword ptr [eax] }
    __asm { mov eax, dst }
    __asm { fstp dword ptr [eax] }
#else
    volatile double v=*src; *dst=static_cast<float>(v);
#endif
}
uint32_t rect(Grid* s,size_t off,float* a,float* b,float* c,float* d) {
    copyFloat(&field<volatile float>(s,off),a); copyFloat(&field<volatile float>(s,off+4),b);
    copyFloat(&field<volatile float>(s,off+8),c); copyFloat(&field<volatile float>(s,off+12),d);
    return alOne(static_cast<uint32_t>(reinterpret_cast<uintptr_t>(c)));
}
}
void SetFactoryServices(const FactoryServices* s) { services=s; }
uint32_t __fastcall Register(void* s,void*,Record* r) {
    r->owner=static_cast<uint8_t*>(s)-sizeof(void*);
#if defined(_M_IX86)
    r->descriptor=reinterpret_cast<void*>(0x00FBC9B4);
#else
    static const uint32_t descriptor=0x00FBC9B4; r->descriptor=&descriptor;
#endif
    r->count=1;
    return r->type=slot(s,5);
}
void* __stdcall Create(uint32_t, uint32_t tag) {
    Glide* p;
    if(services) p=static_cast<Glide*>(services->allocate(sizeof(Glide),4,"UI/UTFWin/EA::UTFWinControls::GlideEffect",tag));
    else p=static_cast<Glide*>(CallGameFunction<void*,0x00504E70>(0x80,4,reinterpret_cast<const char*>(uintptr_t(0x00FBCB1C)),tag));
    if(!p) return nullptr;
    if(services) services->construct(p); else CallGameMethod<void,0x005179E0>(p);
    if(services) {
        p->primary=static_cast<void**>(const_cast<void*>(services->tables[0]));
        p->secondary=static_cast<void**>(const_cast<void*>(services->tables[1]));
        p->effect=static_cast<void**>(const_cast<void*>(services->tables[2]));
        p->tail=static_cast<void**>(const_cast<void*>(services->tables[3]));
    } else {
#if defined(_M_IX86)
        p->primary=reinterpret_cast<void**>(0x00FBCA78); p->secondary=reinterpret_cast<void**>(0x00FBCA60);
        p->effect=reinterpret_cast<void**>(0x00FBCA10); p->tail=reinterpret_cast<void**>(0x00FBC9F0);
#else
        AbortNotRewritten("GlideEffect native tables");
#endif
    }
    p->xyz[0]=p->xyz[1]=p->xyz[2]=0.0f; return p;
}
void* __fastcall Query(void* s,void*,uint32_t id) {
    if(id==0xaf1ee902u) return s?static_cast<uint8_t*>(s)+0x250:nullptr;
    return CallGameMethod<void*,0x00513600>(s,id);
}
uint32_t __fastcall Type(void*,void*) { return 0x6f1f1aa6u; }
uint32_t __fastcall Modify(Grid* s,void*,uint32_t mask,uint32_t set) {
    uint32_t value=static_cast<uint8_t>(set)?(s->flags|mask):(s->flags&~mask);
    return slot(s,10,value);
}
uint32_t __fastcall Read(Grid* s,void*,uint32_t* out) { uint32_t v=field<uint32_t>(s,0x160); *out=v; return alOne(v); }
uint32_t __fastcall SetEntry(Grid* s,void*,uint32_t v,uint32_t i) {
    if(i>=8) return i&0xffffff00u;
    field<uint32_t>(s,0x164+i*4)=v; return alOne(i);
}
uint32_t __fastcall GetEntry(Grid* s,void*,uint32_t* out,uint32_t i) {
    if(i>=8) return i&0xffffff00u;
    uint32_t v=field<uint32_t>(s,0x164+i*4); *out=v; return alOne(v);
}
uint32_t __fastcall Enabled(Grid* s,void*) { return slot(s,12,0x800u); }
uint32_t __fastcall RectA(Grid* s,void*,float* a,float* b,float* c,float* d) { return rect(s,0x10,a,b,c,d); }
uint32_t __fastcall RectB(Grid* s,void*,float* a,float* b,float* c,float* d) { return rect(s,0x20,a,b,c,d); }
uint32_t __fastcall RectC(Grid* s,void*,float* a,float* b,float* c,float* d) { return rect(s,0x30,a,b,c,d); }
uint32_t __fastcall SetMode(Grid* s,void*,uint32_t enable,uint32_t v) { slot(s,11,0x200u,enable); field<uint32_t>(s,0x80)=v; return v; }
uint32_t __fastcall SetValue(Grid* s,void*,uint32_t v) { field<uint32_t>(s,0x168)=v; return alOne(v); }
static_assert(sizeof(void*)!=4 || sizeof(Glide)==0x80,"glide layout");
}
