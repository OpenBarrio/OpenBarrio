#include "ts3/resource/m03_G20.h"
#include <cstring>
namespace ts3::resource::m03::g20 {
#if defined(_M_IX86)
namespace {
using B=uint8_t;
B* A(void* p,size_t n) { return static_cast<B*>(p)+n; }
template<class T> T& F(void* p,size_t n) { return *reinterpret_cast<T*>(A(p,n)); }
void*& P(void* p,size_t n) { return F<void*>(p,n); }
struct Vec { void* first=nullptr; void* last=nullptr; void* cap=nullptr; void* allocator=nullptr; };
struct Iter { void* current=nullptr; void* first=nullptr; void* last=nullptr; void* node=nullptr; };
void Free(void* p) { CallGameFunction<void,0x004E3F70>(p); }
}
#endif
uintptr_t __fastcall Configure(void* self,void*) {
#if defined(_M_IX86)
    CallGameMethod<void,0x0058B880>(self);
    uintptr_t result=CallGameFunction<uintptr_t,0x0058AAD0>(reinterpret_cast<void*>(0x00FFDD10),reinterpret_cast<void*>(0x00FFDD24),0u);
    if(!(result&255)) return result;
    F<B>(self,0xc)=CallGameFunction<uint8_t,0x0058AAD0>(reinterpret_cast<void*>(0x00FFDD10),reinterpret_cast<void*>(0x00FFDCEC),1u);
    B ranges[20];
    CallGameMethod<void,0x0072A9D0>(ranges);
    if(CallGameMethod<uint8_t,0x0072AD90>(ranges,reinterpret_cast<void*>(0x00FFDD10),reinterpret_cast<void*>(0x00FFDCD8)) && P(ranges,4)!=P(ranges,8)) {
        void* manager=CallGameFunction<void*,0x004AFD20>();
        Vec keys; keys.allocator=reinterpret_cast<void*>(0x00FFDCBC);
        CallGameVirtual<void,8>(manager,&keys,ranges,0u);
        CallGameFunction<void,0x004AFCD0>(&keys);
        Iter out;
        auto count=(reinterpret_cast<intptr_t>(keys.last)-reinterpret_cast<intptr_t>(keys.first))>>4;
        CallGameMethod<void,0x00731A20>(A(self,0x5c),static_cast<uint32_t>(count),&out);
        Iter dest{P(self,0x64),P(self,0x68),P(self,0x6c),P(self,0x70)};
        CallGameFunction<void,0x00730340>(&out,keys.first,keys.last,dest);
        if(keys.first) Free(keys.first);
    }
    Vec explicitKeys; explicitKeys.allocator=reinterpret_cast<void*>(0x00FFDBD8);
    if(CallGameMethod<uint8_t,0x00731760>(&explicitKeys,reinterpret_cast<void*>(0x00FFDD10),reinterpret_cast<void*>(0x00FFDCA0))) {
        for(B* key=static_cast<B*>(explicitKeys.first);key!=explicitKeys.last;key+=16) {
            void* current=P(self,0x74);
            void* next=A(current,16);
            if(next==P(self,0x7c)) CallGameMethod<void,0x0072FFF0>(A(self,0x5c),key);
            else { P(self,0x74)=next; if(current) std::memcpy(current,key,16); }
        }
    }
    if(explicitKeys.first) Free(explicitKeys.first);
    return CallGameMethod<uintptr_t,0x0072A9F0>(ranges);
#else
    (void)self; NotRewritten("m03_G20 Configure");
#endif
}
uintptr_t __fastcall Destroy(void* self,void*) {
#if defined(_M_IX86)
    P(self,0)=reinterpret_cast<void*>(0x00FFE2F0);
    P(self,8)=reinterpret_cast<void*>(0x00FFE250);
    P(self,0x1d8)=reinterpret_cast<void*>(0x00FFE244);
    void* service=CallGameFunction<void*,0x005A00A0>();
    if(service) CallGameVirtual<void,18>(service,reinterpret_cast<void*>(0x00FFE218));
    CallGameMethod<void,0x00730480>(A(self,0x768));
    auto list=[&](size_t offset) {
        void* sentinel=A(self,offset); void* node=P(sentinel,0);
        while(node!=sentinel) { void* old=node; node=P(node,0); Free(old); }
    };
    auto string=[&](size_t offset,size_t inlineOffset) {
        void* first=P(self,offset);
        int32_t span=static_cast<int32_t>(reinterpret_cast<uintptr_t>(P(self,offset+8))-reinterpret_cast<uintptr_t>(first))&~1;
        if(span>2 && first && (!inlineOffset || first!=P(self,inlineOffset))) Free(first);
    };
    list(0x758);
    string(0x744,0);
    CallGameMethod<void,0x00594EC0>(A(self,0x734),P(self,0x734),P(self,0x738));
    if(P(self,0x734)) Free(P(self,0x734));
    CallGameMethod<void,0x00409040>(A(self,0x724),P(self,0x724),P(self,0x728));
    if(P(self,0x724)) Free(P(self,0x724));
    string(0x508,0x518); string(0x2ec,0x2fc);
    list(0x2e0);
    if(P(self,0x2b8)) Free(P(self,0x2b8));
    if(P(self,0x2a8)) Free(P(self,0x2a8));
    CallGameMethod<void,0x004E1670>(A(self,0x278));
    CallGameMethod<void,0x004E1670>(A(self,0x248));
    CallGameMethod<void,0x004E1670>(A(self,0x218));
    if(P(self,0x204)) Free(P(self,0x204));
    if(P(self,0x1f4)) Free(P(self,0x1f4));
    P(self,0x1e8)=reinterpret_cast<void*>(0x00FB2CBC);
    P(self,0x1d8)=reinterpret_cast<void*>(0x00FB2CBC);
    return CallGameMethod<uintptr_t,0x004B30B0>(A(self,8));
#else
    (void)self; NotRewritten("m03_G20 Destroy");
#endif
}
}
