#include "ts3/audio/m06_B32.h"
#include <algorithm>
namespace ts3::audio::b32_detail {
TS3_X86_OFFSET(Composite, first, 0xC);
TS3_X86_OFFSET(Composite, second, 0x10);
TS3_X86_OFFSET(Collection, root, 0xC);
namespace {
template<int Slot, class... A> uint32_t Forward(Composite* p, A... args) {
    if (p->first && uint8_t(CallGameVirtual<uint32_t,Slot>(p->first,args...))) return 1;
    if (p->second && uint8_t(CallGameVirtual<uint32_t,Slot>(p->second,args...))) return 1;
    return 0;
}
uint32_t& Flags(void* p) { return *reinterpret_cast<uint32_t*>(static_cast<uint8_t*>(p)+0x424); }
}
uint32_t Forward14(Composite* p, uint32_t a, uint32_t b) { return Forward<14>(p,a,b); }
uint32_t Forward15(Composite* p, uint32_t a, uint32_t b, uint32_t c) { return Forward<15>(p,a,b,c); }
uint32_t Forward18(Composite* p, uint32_t a, uint32_t b, uint32_t c) { return Forward<18>(p,a,b,c); }
uint32_t Forward19(Composite* p, uint32_t a, uint32_t b, uint32_t c) { return Forward<19>(p,a,b,c); }
uint32_t Forward20(Composite* p, uint32_t a, uint32_t b, uint32_t c) { return Forward<20>(p,a,b,c); }
uint32_t Forward16(Composite* p, uint32_t a) {
    uint32_t result=0;
    if (p->first) result=CallGameVirtual<uint32_t,16>(p->first,a);
    if (!result && p->second) result=CallGameVirtual<uint32_t,16>(p->second,a);
    return result;
}
uint32_t Forward17(Composite* p, uint32_t a) {
    uint32_t result=0;
    if (p->first) result=CallGameVirtual<uint32_t,17>(p->first,a);
    if (!result && p->second) result=CallGameVirtual<uint32_t,17>(p->second,a);
    return result;
}
Composite* Destroy(Composite* p, uint32_t flags) {
    if (p->second) CallGameVirtual<void,1>(p->second);
    if (p->first) CallGameVirtual<void,1>(p->first);
#if defined(_M_IX86)
    p->vtable=reinterpret_cast<void*>(0x0107396C);
    if (flags&1) {
        auto& head=*reinterpret_cast<Composite**>(0x011D9D54);
        p->next=head; head=p;
    }
#else
    if(flags&1) AbortNotRewritten("audio composite pool");
#endif
    return p;
}
void Collect(Composite* p, Collection* out, uint8_t clear) {
    if(clear) {
        if(out->root) CallGameMethod<void,0x006A0850>(out,out->root);
        out->left=&out->left; out->right=&out->left; out->root=nullptr;
        out->flag=0; out->count=0;
    }
    if(p->first) CallGameVirtual<void,3>(p->first,out,uint32_t(0));
    if(p->second) CallGameVirtual<void,3>(p->second,out,uint32_t(0));
}
uint32_t ReadBits(BitReader* p, uint32_t count) {
    uint32_t result=0;
    while(count) {
        uint32_t offset=p->position&7;
        uint32_t take=std::min(8-offset,count);
        uint32_t part=(p->data[p->position>>3]>>(8-offset-take))&((1u<<take)-1);
        p->position+=take;
        result=(result<<take)|part;
        count-=take;
    }
    return result;
}
uint32_t SetFlags(void* p, uint32_t mask, uint8_t on) {
    auto& flags=Flags(p);
    if(on) flags|=mask; else flags&=~mask;
    return flags;
}
uint8_t Start(void* p) {
    auto& flags=Flags(p);
    if(flags&1) return 0;
    flags|=1;
    auto system=CallGameFunction<void*,0x008B0A70>();
    CallGameVirtual<void,110>(system,static_cast<uint8_t*>(p)+0x3C4,uint32_t(0),uint32_t(0));
    return 1;
}
uint8_t Resume(void* p) {
    if(!(Flags(p)&0x10)) return 0;
    CallGameVirtual<void,25>(p);
    Flags(p)=(Flags(p)&~0x20u)|0x80;
    return 1;
}
uint8_t GetFlag(const void* p) { return *(static_cast<const uint8_t*>(p)+0x3C8); }
}
