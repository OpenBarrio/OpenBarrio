#include "ts3/ui/m08_B22.h"
#include "ts3/ui/m08_abi.h"
#include "ts3/ui/clu_50b5f0_UI.h"
#include <cstddef>
namespace ts3::ui::b22 {
namespace {
using namespace ts3::ui::m08;
template<class R,size_t S,class... A> R V(void* p,A... a) {
    return CallThis(reinterpret_cast<Thiscall<R,void,A...>>((*static_cast<void***>(p))[S]),p,a...);
}
template<uint32_t EA,class R,class T,class... A> R M(T* p,A... a) { return CallThis(Original<Thiscall<R,T,A...>>(EA),p,a...); }
template<uint32_t EA,class R,class... A> R F(A... a) { return Original<Cdecl<R,A...>>(EA)(a...); }
void* Allocate(uint32_t n,const char* name) { return F<0x004E3F90,void*>(n,name,0u,0u,0u,0u); }
const char* Name() {
#if defined(_M_IX86)
    return reinterpret_cast<const char*>(0x00FBB7C4);
#else
    return "UI/UTFWin/CircularAllocator";
#endif
}
Window* Parent(void* p) { return reinterpret_cast<Window*>(static_cast<uint8_t*>(p)-offsetof(Window,control)); }
void* Secondary(Window* w) { return w?&w->secondary:nullptr; }
uint32_t Flags(Window* w) { return V<uint32_t,5>(&w->control); }
}
void* __stdcall CreateEffect(uint32_t,uint32_t tag) {
#if defined(_M_IX86)
    const char* name=reinterpret_cast<const char*>(0x00FBB798);
#else
    const char* name="UI/UTFWin/EA::UTFWinControls::CascadeEffect";
#endif
    void* p=F<0x00504E70,void*>(0x48u,8u,name,tag);
    return p?M<0x0051ACC0,void*>(p):nullptr;
}
uintptr_t __fastcall Resize(Circular* p,void*,uint32_t n,const char* name) {
    uintptr_t result=reinterpret_cast<uintptr_t>(p->begin);
    if(uintptr_t(p->end)-uintptr_t(p->begin)==n) return result;
    if(p->begin) {
        result=F<0x004E3F70,uintptr_t>(p->begin);
        p->begin=p->end=p->read=p->write=nullptr;
    }
    if(n) {
        p->begin=static_cast<uint8_t*>(Allocate(n,name?name:Name()));
        p->end=reinterpret_cast<uint8_t*>(uintptr_t(p->begin)+n);
        p->read=p->write=p->begin; result=uintptr_t(p->begin);
    }
    return result;
}
uintptr_t __fastcall Available(Circular* p,void*,uint32_t alignment) {
    const uint32_t a=((alignment-1u)|7u)+1u;
    const uintptr_t mask=~uintptr_t(a-1u),w=uintptr_t(p->write),r=uintptr_t(p->read);
    if(r>w) return r-((w+a)&mask);
    return (uintptr_t(p->end)&mask)-((w+a-1u)&mask)-((uintptr_t(p->begin)+a-1u)&mask)+r;
}
Circular* __fastcall Init(Circular* p,void*,uint32_t n) {
    p->begin=p->end=p->read=p->write=nullptr;
    if(n) { p->begin=static_cast<uint8_t*>(Allocate(n,Name()));p->end=reinterpret_cast<uint8_t*>(uintptr_t(p->begin)+n);p->read=p->write=p->begin; }
    return p;
}
uint8_t __fastcall FocusChanged(Listener* p,void*,void* source,uint32_t,uint32_t,uint32_t,uint32_t) {
    void* manager=F<0x0050AB70,void*>();
    if(manager && (Flags(p->owner)&4)) {
        if(V<void*,21>(manager,1u)!=source) return 1;
        Window* w=p->owner;
        manager=F<0x0050AB70,void*>();
        V<uintptr_t,32>(manager,source,w?&w->focus:nullptr);
    }
    return 0;
}
uint8_t __fastcall Transition(Listener* p,void*,void* source,uint32_t mode,void* target) {
    if(Flags(p->owner)&4) {
        Window* w=p->owner;
        if(w->active && mode==1 && target!=Secondary(w) && target!=source) {
            V<uintptr_t,1>(&w->focus,source,1u);return 0;
        }
        if(w->active && mode==1 && target==Secondary(w)) V<uintptr_t,45>(p->owner,p->owner->selection);
    } else if(mode==0 && p->owner->active && target!=source && target!=Secondary(p->owner)) {
        if(!V<uint8_t,63>(source,target)) {
            void* parent=V<void*,5>(source);
            if(!V<uint8_t,33>(parent,source,1u)) V<uintptr_t,1>(&p->owner->focus,source,1u);
        }
    }
    return 0;
}
uint8_t __fastcall Activate(Listener* p,void*,void* source,uint32_t,uint32_t,uint32_t) {
    if(p->owner->active) {
        void* parent=V<void*,5>(source);
        if(!V<uint8_t,33>(parent,source,1u)) V<uintptr_t,1>(&p->owner->focus,source,1u);
    }
    return 1;
}
uintptr_t __fastcall MessageReceived(Listener* p,void*,void* source,Message* msg) {
    if((Flags(p->owner)&4) && p->owner->active && msg->kind==0x9A1552D6 && msg->payload->kind==5)
        V<uintptr_t,45>(p->owner,msg->payload->value);
    return M<0x005049B0,uintptr_t>(p,source,msg);
}
uint8_t __fastcall Present(Window* p,void*) {
    V<uintptr_t,40>(p);
    return static_cast<uint8_t>(reinterpret_cast<ts3::ui::UiTransform*>(p)->PresentFromLayout());
}
void* __fastcall Query(Window* p,void*,uint32_t id) {
    if(id==0x2F5528D9) return p?&p->control:nullptr;
    return M<0x00513600,void*>(p,id);
}
uint32_t __fastcall Type(void*,void*) { return 0xAF552C4B; }
uintptr_t __fastcall SetFlags(void* p,void*,uint32_t flags) {
    Window* w=Parent(p); w->flags=flags;return V<uintptr_t,40>(w);
}
uintptr_t __fastcall ModifyFlags(void* p,void*,uint32_t mask,uint32_t set) {
    uint32_t flags=Parent(p)->flags;
    flags=static_cast<uint8_t>(set)?flags|mask:flags&~mask;
    return V<uintptr_t,6>(p,flags);
}
uintptr_t __fastcall Select(void* p,void*,uint32_t value) {
    Window* w=Parent(p);V<uintptr_t,45>(&w->secondary,value);return V<uintptr_t,40>(w);
}
uint8_t __fastcall State(void* p,void*) { return Parent(p)->state; }
}
