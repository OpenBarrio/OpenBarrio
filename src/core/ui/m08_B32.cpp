#include "ts3/ui/m08_B32.h"
#include "ts3/layout.h"
#include "ts3/ui/m08_abi.h"
#include <cmath>
#include <limits>
namespace ts3::ui::b32 {
thread_local Dependencies dependencies;
TS3_X86_OFFSET(Explicit,interface1c,0x1c);
TS3_X86_OFFSET(Explicit,target,0x20);
TS3_X86_OFFSET(Fade,start,0x70);
TS3_X86_OFFSET(Navigator,bindings,0x0c);
TS3_X86_SIZE(Descriptor,16);
namespace {
void* Table(uint32_t address) {
#if defined(_M_IX86)
    return reinterpret_cast<void*>(uintptr_t(address));
#else
    (void)address; return nullptr; // Original image tables have no native ABI.
#endif
}
const char* AllocationName(bool fade) {
#if defined(_M_IX86)
    return reinterpret_cast<const char*>(uintptr_t(fade?0x00FBC82C:0x00FBC674));
#else
    return fade?"UI/UTFWin/EA::UTFWinControls::FadeEffect":"UI/UTFWin/EA::UTFWinControls::ExplicitNavigator";
#endif
}
template<class R,class... A> R Virtual(void* self,size_t slot,A... args) {
    auto table=*static_cast<void***>(self);
    return m08::CallThis(reinterpret_cast<m08::Thiscall<R,void,A...>>(table[slot]),self,args...);
}
void* Back(void* p,size_t n) { return static_cast<char*>(p)-n; }
}
void* __fastcall DestroyExplicit1c(void* self,void*,uint32_t flags) {
    return m08::CallThis(m08::Original(0x0053D0D0,dependencies.destroyExplicit1c),Back(self,offsetof(Explicit,interface1c)),flags);
}
void* __fastcall DestroyExplicit4(void* self,void*,uint32_t flags) {
    return m08::CallThis(m08::Original(0x005351A0,dependencies.destroyExplicit4),Back(self,offsetof(Explicit,secondary)),flags);
}
void* __stdcall CreateExplicit(uint32_t,uint32_t tag) {
    auto* p=static_cast<Explicit*>(m08::Original(0x00504E70,dependencies.allocate)(sizeof(Explicit),4,AllocationName(false),tag));
    if(!p) return nullptr;
    m08::CallThis(m08::Original(0x00534EF0,dependencies.explicitCtor),static_cast<void*>(p));
    p->interface1c=Table(0x00FBEC80);
    p->primary=Table(0x00FBC5D8);p->secondary=Table(0x00FBC5BC);p->interface10=Table(0x00FBE550);
    p->interface1c=Table(0x00FBC5A4);p->target=nullptr;
    return p;
}
uintptr_t __fastcall ApplyFade(Fade* self,void*,void* target,float t) {
    const float start=self->start;
    const uint32_t color=Virtual<uint32_t>(target,12);
    const float end=self->end;
    int64_t alpha;
#if defined(_M_IX86)
    const float scale=-255.0f;
    unsigned short saved,truncate;
    __asm { fnstcw saved }
    truncate=static_cast<unsigned short>(saved|0x0c00);
    __asm {
        fld end
        fld start
        fsub st(1),st(0)
        fxch st(1)
        fmul t
        faddp st(1),st(0)
        fmul scale
        fldcw truncate
        fistp alpha
        fldcw saved
    }
#else
    double x=(double(start)+(double(end)-double(start))*double(t))*(-255.0);
    alpha=(!std::isfinite(x)||x>=9223372036854775808.0||x< -9223372036854775808.0)?std::numeric_limits<int64_t>::min():static_cast<int64_t>(x);
#endif
    return Virtual<uintptr_t>(target,24,(color&0x00ffffffu)-(static_cast<uint32_t>(alpha)<<24));
}
uint32_t __fastcall FadeType(void*,void*) { return 0x6F2B6D2C; }
void* __fastcall DestroyFade10(void* self,void*,uint32_t flags) {
    return m08::CallThis(m08::Original(0x00522650,dependencies.destroyFade),Back(self,offsetof(Fade,interface10)),flags);
}
void* __fastcall DestroyFade4(void* self,void*,uint32_t flags) {
    return m08::CallThis(m08::Original(0x00522650,dependencies.destroyFade),Back(self,offsetof(Fade,secondary)),flags);
}
void* __stdcall CreateFade(uint32_t,uint32_t tag) {
    auto* p=static_cast<Fade*>(m08::Original(0x00504E70,dependencies.allocate)(sizeof(Fade),4,AllocationName(true),tag));
    if(!p) return nullptr;
    m08::CallThis(m08::Original(0x005179E0,dependencies.fadeCtor),static_cast<void*>(p));
    p->start=1.0f;p->primary=Table(0x00FBC748);p->secondary=Table(0x00FBC730);p->interface10=Table(0x00FBC6E0);p->end=0.0f;
    return p;
}
uint32_t __fastcall DescribeFade(void* self,void*,Descriptor* out) {
    out->object=Back(self,offsetof(Fade,secondary));out->table=Table(0x00FBC7F0);out->count=1;
    return out->type=Virtual<uint32_t>(self,5);
}
void* __fastcall QueryNavigator(void* self,void*,uint32_t id) {
    if(id==0x030D54AC) return self;
    if(id==0xEEC58382) return self?static_cast<char*>(self)+offsetof(Navigator,secondary):nullptr;
    return m08::CallThis(m08::Original(0x00504D90,dependencies.queryBase),self,id);
}
uint32_t __fastcall NavigatorType(void*,void*) { return 0x030D54B9; }
Binding* __fastcall SelectBinding(Navigator* self,void*,uint32_t flags) {
    const unsigned kind=flags&7;
    auto* b=self->bindings;
    if(kind>3) return b;
    if(flags&8) {
        unsigned special=4+kind;
        if(b[special].pointer) return b+special;
        if(kind>=2 && b[4].pointer) return b+4;
    }
    if(kind && b[kind].pointer) return b+kind;
    return b;
}
uintptr_t __fastcall ForwardRectangle(void* self,void*,const float* rect) {
    return Virtual<uintptr_t>(self,14,rect[0],rect[1],rect[2],rect[3]);
}
uint32_t __fastcall DescribeNavigator(void* self,void*,Descriptor* out) {
    out->object=Back(self,offsetof(Navigator,secondary));out->table=Table(0x00FBC8E8);out->count=1;
    return out->type=Virtual<uint32_t>(self,5);
}
}
