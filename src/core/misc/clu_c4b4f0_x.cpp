// Reimplementation of the small resource-index cluster at 0x00C4B4F0-0x00C50200.
#include "ts3/misc/clu_c4b4f0_x.h"

#include <cmath>
#include <cstring>

#if defined(_MSC_VER) && defined(_M_IX86)
#define TS3_MISC_FASTCALL __fastcall
#define TS3_MISC_STDCALL __stdcall
#define TS3_MISC_CDECL __cdecl
#else
#define TS3_MISC_FASTCALL
#define TS3_MISC_STDCALL
#define TS3_MISC_CDECL
#endif

namespace ts3::misc {
namespace {
constexpr ptrdiff_t kEntry = 2076;
constexpr int32_t Page(int32_t n) {
    return static_cast<int32_t>((static_cast<int64_t>(n) + 0x1000000) / 4 - 0x400000);
}
constexpr int64_t FloorDiv4(int64_t n) { return n >= 0 ? n / 4 : -((-n + 3) / 4); }
uint32_t GameGlobal32(uint32_t address) {
#if defined(_M_IX86)
    return *reinterpret_cast<const uint32_t*>(static_cast<uintptr_t>(address));
#else
    (void)address; return 0;
#endif
}
void* GameGlobalPointer(uint32_t address) {
#if defined(_M_IX86)
    return *reinterpret_cast<void**>(static_cast<uintptr_t>(address));
#else
    (void)address; return nullptr;
#endif
}
void* GameCallback(uint32_t address) {
#if defined(_M_IX86)
    return reinterpret_cast<void*>(static_cast<uintptr_t>(address));
#else
    (void)address; return nullptr;
#endif
}
intptr_t PointerDelta(uintptr_t a, uintptr_t b) { return static_cast<intptr_t>(a - b); }
int32_t ReleaseRef(void* object) {
    if (!object) return 0;
    auto** vtable = *reinterpret_cast<void***>(object);
#if defined(_MSC_VER) && defined(_M_IX86)
    using Fn = int32_t(__thiscall*)(void*);
#else
    using Fn = int32_t(*)(void*);
#endif
    return reinterpret_cast<Fn>(vtable[1])(object);
}
int CompareKey(const uint8_t* a, const uint8_t* b) {
    constexpr int order[] = {2, 0, 1, 5, 3, 4};
    for (int i : order) {
        const auto x = static_cast<int8_t>(a[i]), y = static_cast<int8_t>(b[i]);
        if (x < y) return -1;
        if (x > y) return 1;
    }
    return 0;
}
}

// 0x00C4B4F0: combine page-table and entry cursors (EA's reciprocal division retained).
uint32_t C4B4F0(const uintptr_t* a, const uintptr_t* b) {
    const auto a0 = static_cast<int64_t>(PointerDelta(a[0], a[1]));
    const auto a3 = static_cast<int64_t>(PointerDelta(a[3], b[3]));
    const auto b2 = static_cast<int64_t>(PointerDelta(b[2], b[0]));
    return static_cast<uint32_t>(a0 / kEntry + 4 * FloorDiv4(a3) + b2 / kEntry - 4);
}

// 0x00C4B540: move across 2076-byte entries and four-page blocks.
void* TS3_MISC_FASTCALL C4B540(void* p, void*, int32_t delta) {
    auto* s = static_cast<uintptr_t*>(p);
    const int32_t index = static_cast<int32_t>(static_cast<uint32_t>(delta) +
        static_cast<uint32_t>(PointerDelta(s[0], s[1]) / kEntry));
    if (static_cast<uint32_t>(index) < 4) {
        s[0] = static_cast<uintptr_t>(static_cast<intptr_t>(s[0]) + static_cast<int64_t>(delta) * kEntry);
        return p;
    }
    const int32_t page = Page(index);
    auto* table = reinterpret_cast<uintptr_t*>(s[3]);
    s[3] += static_cast<uintptr_t>(page) * sizeof(uintptr_t);
    s[1] = *reinterpret_cast<uintptr_t*>(s[3]);
    s[2] = s[1] + 8304;
    s[0] = s[1] + static_cast<uintptr_t>(index - page * 4) * kEntry;
    (void)table;
    return p;
}

// 0x00C4B5C0
void* TS3_MISC_FASTCALL C4B5C0(void* s, void*, void* out, int32_t n) {
    uintptr_t t[4]; std::memcpy(t, s, sizeof(t));
    C4B540(t, nullptr, static_cast<int32_t>(0u - static_cast<uint32_t>(n)));
    std::memcpy(out, t, sizeof(t)); return out;
}
void* TS3_MISC_FASTCALL C4BA30(void* s, void*, void* out, int32_t n) {
    uintptr_t t[4]; std::memcpy(t, s, sizeof(t)); C4B540(t, nullptr, n); std::memcpy(out, t, sizeof(t)); return out;
}

// 0x00C4B670: copy 24-byte records, forward only.
void* C4B670(void* begin, void* end, const void* value) {
    auto* d = static_cast<uint32_t*>(begin); const auto* e = static_cast<const uint32_t*>(end);
    const auto* src=static_cast<const uint32_t*>(value);
    while (d != e) {
        d[0]=src[0]; d[1]=src[1]; d[2]=src[2]; d[3]=src[3]; d[4]=src[4]; d[5]=src[5];
        d+=6;
    }
    return d;
}

// 0x00C4B730: count links, advancing through the supplied bucket list on null links.
int32_t C4B730(uintptr_t head, const uintptr_t* buckets, uintptr_t end) {
    int32_t count = 0; size_t b = 1;
    while (head != end) { uintptr_t next = *reinterpret_cast<const uintptr_t*>(head + 128);
        while (!next) next = buckets[b++]; head = next; ++count; }
    return count;
}

// 0x00C4B9E0: same page/cursor count as C4B4F0, for the embedded iterator fields.
uint32_t TS3_MISC_FASTCALL C4B9E0(const uintptr_t* s, void*) {
    return static_cast<uint32_t>(static_cast<int64_t>(PointerDelta(s[6],s[7])) / kEntry +
        4 * FloorDiv4(PointerDelta(s[9],s[5])) +
        static_cast<int64_t>(PointerDelta(s[4],s[2])) / kEntry - 4);
}

// 0x00C4BA80: lower-bound tree lookup; comparison order is the game's signed-byte tuple.
void* TS3_MISC_FASTCALL C4BA80(void* self, void*, void* out, const uint8_t* key) {
    auto* t = static_cast<uintptr_t*>(self); uintptr_t node = t[3], result = reinterpret_cast<uintptr_t>(t + 1);
    while (node) {
        auto* n = reinterpret_cast<uint8_t*>(node); const int c = CompareKey(n + 16, key);
        if (c < 0) node = *reinterpret_cast<uintptr_t*>(n);
        else { result = node; node = *reinterpret_cast<uintptr_t*>(n + 4); }
    }
    *static_cast<uintptr_t*>(out) = result; return out;
}

// 0x00C4D120 / 0x00C4D1B0: key searches through distinct next-link offsets.
static uintptr_t FindKey(uintptr_t n, const uint8_t* key, size_t link) {
    while (n && CompareKey(reinterpret_cast<const uint8_t*>(n), key))
        n = *reinterpret_cast<const uintptr_t*>(n + link);
    return n;
}
uintptr_t TS3_MISC_STDCALL C4D120(uintptr_t n, const uint8_t* k, int32_t) { return FindKey(n, k, 16); }
uintptr_t TS3_MISC_STDCALL C4D1B0(uintptr_t n, const uint8_t* k, int32_t) { return FindKey(n, k, 20); }

// 0x00C4D170: return linked nodes to the free-list and clear every bucket.
void TS3_MISC_FASTCALL C4D170(void* self, void*, uintptr_t* buckets, uint32_t count) {
    auto* s = static_cast<uintptr_t*>(self);
    for (uint32_t i=0; i<count; ++i) { uintptr_t n=buckets[i]; while(n) {
        auto* node=reinterpret_cast<uintptr_t*>(n); uintptr_t next=node[5];
        auto* head=reinterpret_cast<uintptr_t*>(s[7]); node[0]=*head; *head=n; n=next;
    } buckets[i]=0; }
}

// 0x00C4EA80: approximate 2D coordinate match and exact third component.
uintptr_t TS3_MISC_STDCALL C4EA80(uintptr_t n, const float* key, int32_t) {
    while (n) { auto* q=reinterpret_cast<const float*>(n);
        if (*reinterpret_cast<const uint32_t*>(n+8)==*reinterpret_cast<const uint32_t*>(key+2) &&
            !(std::fabs(q[0]-key[0]) > .001f) && !(std::fabs(q[1]-key[1]) > .001f) &&
            !std::isnan(q[0]-key[0]) && !std::isnan(q[1]-key[1])) return n;
        n=*reinterpret_cast<const uintptr_t*>(n+16);
    } return 0;
}

// Remaining object-lifecycle and engine-global routines are implemented below as direct
// field operations where they are independent of the engine-owned global service.
int32_t TS3_MISC_FASTCALL C4C1B0(void* self, void*, const uint32_t* value) {
    auto* b=static_cast<uint8_t*>(self);
    int32_t result;
    if (value) {
        std::memcpy(b+8,value,16);
        result=static_cast<int32_t>(reinterpret_cast<uintptr_t>(b+8));
        if (void* manager=GameGlobalPointer(0x011D3C84)) {
            auto** vt=*reinterpret_cast<void***>(manager);
#if defined(_MSC_VER) && defined(_M_IX86)
            using RegisterFn=int32_t(__thiscall*)(void*,void*,uint32_t,void*,uint32_t,uint32_t,void*,uint32_t,uint32_t);
#else
            using RegisterFn=int32_t(*)(void*,void*,uint32_t,void*,uint32_t,uint32_t,void*,uint32_t,uint32_t);
#endif
            result=reinterpret_cast<RegisterFn>(vt[13])(manager,b+8,0xEB54B802u,b+60,0,0,GameCallback(0x00D4B150),0,1);
        }
    } else {
        for (unsigned i=0;i<4;++i) reinterpret_cast<uint32_t*>(b+8)[i]=GameGlobal32(0x011ECB08+4*i);
        result=static_cast<int32_t>(*reinterpret_cast<uint32_t*>(b+16));
    }
    void* old=*reinterpret_cast<void**>(b+56);
    if (old) { *reinterpret_cast<void**>(b+56)=nullptr; result=ReleaseRef(old); }
    b[42]=0; *reinterpret_cast<uint32_t*>(b+104)=0; return result;
}

int32_t TS3_MISC_FASTCALL C4C850(void* self, void*, int32_t index) {
    auto* s=static_cast<uintptr_t*>(self); const int32_t v=index+static_cast<int32_t>(PointerDelta(s[113],s[114])/kEntry)-
        *reinterpret_cast<int32_t*>(s[1]+104); const int32_t page=Page(v);
    auto* table=reinterpret_cast<uintptr_t*>(s[116]); auto* e=reinterpret_cast<uint8_t*>(table[page]+(v-page*4)*kEntry);
    e[2072]=0; return page*4;
}
uintptr_t TS3_MISC_FASTCALL C4C8C0(void* self, void*, int32_t index, int32_t field) {
    auto* s=static_cast<uintptr_t*>(self); const int32_t v=index+static_cast<int32_t>(PointerDelta(s[113],s[114])/kEntry)-
        *reinterpret_cast<int32_t*>(s[1]+104); const int32_t page=Page(v);
    auto* table=reinterpret_cast<uintptr_t*>(s[116]); return table[page]+(v-page*4)*kEntry+296*field;
}

// 0x00C4D540: the static global seed values are engine-owned and initialized at runtime;
// all object-local constants and flags are initialized here while seed ranges are zeroed.
void* TS3_MISC_FASTCALL C4D540(void* self,void*,int32_t a,int32_t b) {
    auto* s=static_cast<uint32_t*>(self); s[0]=a; s[1]=b;
    for (unsigned i=0;i<4;++i) s[2+i]=GameGlobal32(0x011EB8D0+4*i);
    s[6]=GameGlobal32(0x011EB8B0); s[7]=GameGlobal32(0x011EB8B4);
    s[8]=GameGlobal32(0x011EB8B8); s[9]=GameGlobal32(0x011EB8BC);
    *reinterpret_cast<uint16_t*>(s+10)=0xffff; static_cast<uint8_t*>(self)[42]=0;
    s[11]=0xffffffffu; s[14]=s[15]=s[16]=0;
    for (unsigned i=0;i<8;++i) s[18+i]=GameGlobal32(0x011EB8D0+4*(i%4));
    s[26]=s[27]=0;
    auto* q=reinterpret_cast<uint8_t*>(self); q[112]=(q[112]&0x80)|0x20; return self;
}

int32_t TS3_MISC_FASTCALL C4D610(void* self,void*) {
    auto* s=static_cast<uintptr_t*>(self); s[26]=s[27]=0;
    auto clearSmart=[&](uintptr_t* slot, bool invokeForTagged) -> int32_t {
        int32_t result=static_cast<int32_t>(*slot);
        if (*slot && (invokeForTagged || (*slot&3)==0)) {
            if (void* manager=GameGlobalPointer(0x011D3C84)) {
                auto** vt=*reinterpret_cast<void***>(manager);
#if defined(_MSC_VER) && defined(_M_IX86)
                using RemoveFn=int32_t(__thiscall*)(void*,void*,void*);
#else
                using RemoveFn=int32_t(*)(void*,void*,void*);
#endif
                result=reinterpret_cast<RemoveFn>(vt[15])(manager,slot,GameCallback(0x00D4B150));
            }
            if (*slot && (*slot&3)==0) { result=ReleaseRef(reinterpret_cast<void*>(*slot)); *slot=0; }
        }
        return result;
    };
    int32_t result=static_cast<int32_t>(s[15]);
    // The original makes two passes over +0x40: a first normal-ref pass, then a
    // residual/tagged-ref pass after manager cleanup.
    if ((s[16]&3)==0 && s[16]) result=clearSmart(&s[16], false);
    if (s[16]) result=clearSmart(&s[16], true);
    if (s[15]) result=clearSmart(&s[15], true);
    if (s[14]) result=ReleaseRef(reinterpret_cast<void*>(s[14]));
    return result;
}

void* TS3_MISC_FASTCALL C4DDC0(void* self,void*,const void* source) {
    auto* d=static_cast<uint8_t*>(self); const auto* s=static_cast<const uint8_t*>(source);
    std::memcpy(d,s,40);
    std::memcpy(d+40,s+40,2); d[42]=s[42];
    std::memcpy(d+44,s+44,12);
    std::memcpy(d+72,s+72,40);
    // 0x00C4DE23: +0x38 is copied from the source, then AddRef'd (game run 2026-10-14:
    // without this copy the destination's stale +0x38 was AddRef'd and the game crashed).
    *reinterpret_cast<void**>(d+56)=*reinterpret_cast<void* const*>(s+56);
    if (void* object=*reinterpret_cast<void**>(d+56)) {
        auto** vt=*reinterpret_cast<void***>(object);
#if defined(_MSC_VER) && defined(_M_IX86)
        using AddRefFn=void(__thiscall*)(void*);
#else
        using AddRefFn=void(*)(void*);
#endif
        reinterpret_cast<AddRefFn>(vt[0])(object);
    }
    *reinterpret_cast<uintptr_t*>(d+60)=0; *reinterpret_cast<uintptr_t*>(d+64)=0;
    if (void* manager=GameGlobalPointer(0x011D3C84)) {
        auto** vt=*reinterpret_cast<void***>(manager);
#if defined(_MSC_VER) && defined(_M_IX86)
        using CloneFn=int32_t(__thiscall*)(void*,void*,const void*,void*,void*,uint32_t,uint32_t,uint32_t);
#else
        using CloneFn=int32_t(*)(void*,void*,const void*,void*,void*,uint32_t,uint32_t,uint32_t);
#endif
        auto clone=reinterpret_cast<CloneFn>(vt[14]);
        clone(manager,d+60,s+60,GameCallback(0x0056CD70),GameCallback(0x00D4B150),1,0,0);
        clone(manager,d+64,s+64,GameCallback(0x0056CD70),GameCallback(0x00D4B150),1,0,0);
    }
    d[112]=static_cast<uint8_t>((d[112]&0x80)|(s[112]&0x7f)); return self;
}

void* TS3_MISC_FASTCALL C4E5B0(void* self,void*,void* out,const int8_t* key) {
    auto* s=static_cast<uintptr_t*>(self); const uint32_t h=static_cast<uint32_t>(key[1]+key[4]+100*(key[0]+key[3]+200*key[2]));
    auto* buckets=reinterpret_cast<uintptr_t*>(s[1]); const uint32_t n=static_cast<uint32_t>(s[2]);
    auto* slot=&buckets[h%n]; uintptr_t found=C4D1B0(*slot,reinterpret_cast<const uint8_t*>(key),static_cast<int32_t>(h));
    auto* o=static_cast<uintptr_t*>(out); o[0]=found?found:buckets[n]; o[1]=reinterpret_cast<uintptr_t>(found?slot:buckets+n); return out;
}

uint32_t* TS3_MISC_FASTCALL C50200(void* self,void*,int32_t id) {
    auto* s=static_cast<uintptr_t*>(self); auto* b=reinterpret_cast<uint32_t*>(s[144]); auto* e=reinterpret_cast<uint32_t*>(s[145]);
    for (auto* p=b;p!=e;) { if (*p==static_cast<uint32_t>(id)) { std::memmove(p,p+3,static_cast<size_t>(e-(p+3))*4); e-=3; s[145]=reinterpret_cast<uintptr_t>(e); }
        else p+=3; } return e;
}

} // namespace ts3::misc

