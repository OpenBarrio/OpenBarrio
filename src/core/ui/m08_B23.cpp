#include "ts3/ui/m08_B23.h"
#include "ts3/ui/m08_abi.h"
namespace ts3::ui {
using Window=b23::Window;
using Style=b23::Style;
using Message=b23::Message;
namespace {
using ts3::ui::m08::Thiscall;
using ts3::ui::m08::CallThis;
using ts3::ui::m08::Original;
template<class R, size_t Slot, class T, class... A> R V(T* self,A... args) {
    auto table=*reinterpret_cast<void***>(self);
    return CallThis(reinterpret_cast<Thiscall<R,T,A...>>(table[Slot]),self,args...);
}
void Invalidate(Window* w) { V<uintptr_t,0x90/4>(&w->secondary); }
int32_t Hit(Window* w,float x,float y) { return V<int32_t,0xb0/4>(w,x,y); }
void Dismiss(Window* w) { V<uintptr_t,1>(&w->delegate,w->popup,uint32_t(1)); }
}
uint8_t b23::Press(Window* w,float x,float y,uint32_t,uint32_t) {
    if(V<uint32_t,0x28/4>(&w->secondary)&2) {
        w->pressed=Hit(w,x,y);
        if(w->pressed!=-1) {
            if(V<void*,0x60/4>(w->manager,uint32_t(1))!=&w->secondary)
                V<uintptr_t,0x64/4>(w->manager,uint32_t(1),static_cast<void*>(&w->secondary),uint32_t(0));
            if(!(w->flags&4)) V<uintptr_t,0xa8/4>(w);
            Invalidate(w);
        }
    }
    return 1;
}
uint8_t b23::Release(Window* w,float x,float y,uint32_t,uint32_t) {
    if(V<void*,0x60/4>(w->manager,uint32_t(1))==&w->secondary)
        V<uintptr_t,0x68/4>(w->manager,uint32_t(1),static_cast<void*>(&w->secondary),uint32_t(0));
    if(w->selecting) { w->pressed=Hit(w,x,y); w->hover=-1; }
    else { w->pressed=-1; w->hover=Hit(w,x,y); }
    Invalidate(w); return 1;
}
uint8_t b23::Move(Window* w,float x,float y,uint32_t) {
    const auto hit=Hit(w,x,y);
    if(w->hover!=hit) { w->hover=hit; Invalidate(w); }
    return 1;
}
uint8_t b23::FocusChanged(Window* w,uint32_t channel,void* target) {
    if(channel==1) {
        if(target!=&w->secondary && w->hover!=-1) { w->hover=-1; Invalidate(w); }
        if(w->flags&4) {
            if(V<void*,0x60/4>(w->manager,uint32_t(1))!=&w->secondary) {
                if(!w->popup && target==&w->secondary && !V<void*,0x60/4>(w->manager,uint32_t(1))) {
                    V<uintptr_t,0xa8/4>(w); return 1;
                }
                if(w->popup && target!=w->popup && target!=&w->secondary) Dismiss(w);
            }
        }
    }
    return 1;
}
uintptr_t b23::Handle(Window* w,const Message* msg) {
    if(msg->kind==0x21 && msg->argument==0 && w->popup) Dismiss(w);
    using Fn=ts3::ui::m08::Thiscall<uintptr_t,Window,const Message*>;
    return ts3::ui::m08::CallThis(ts3::ui::m08::Original<Fn>(0x005045F0),w,msg);
}
uint8_t b23::Activate(Window* w) { V<uintptr_t,0xa4/4>(w); return 1; }
uintptr_t b23::SecondaryReset(void* secondary) {
    auto* w=reinterpret_cast<Window*>(static_cast<uint8_t*>(secondary)-offsetof(Window,secondary));
    V<uintptr_t,0xa0/4>(w);
    using Fn=ts3::ui::m08::Thiscall<uintptr_t,void>;
    return ts3::ui::m08::CallThis(ts3::ui::m08::Original<Fn>(0x00514950),secondary);
}
void b23::Start(Window* w) {
    if(!w->started) { w->started=1; V<uintptr_t,0x74/4>(&w->secondary,uint32_t(8),uint32_t(1)); }
}
uint32_t b23::SetColor(Style* s,uint32_t index,uint32_t value) {
    if(index<6) s->colors[index]=value;
    return index;
}
uint32_t b23::Color(const Style* s,uint32_t index) { return index<6?s->colors[index]:0; }
uint32_t b23::SetValue(Style* s,uint32_t value) { s->value=value; s->dirty=1; return value; }
float* b23::Rectangle(const Style* s,float* a,float* b,float* c,float* d) {
    // Sequential x87 load/store behavior, including aliasing and sNaN quieting.
    volatile double v=s->rectangle[0]; *a=static_cast<float>(v);
    v=s->rectangle[1]; *b=static_cast<float>(v);
    v=s->rectangle[2]; *c=static_cast<float>(v);
    v=s->rectangle[3]; *d=static_cast<float>(v);
    return c;
}
void b23::SetRectangle(Style* s,float a,float b,float c,float d) {
    s->rectangle[0]=a; s->rectangle[1]=b; s->rectangle[2]=c; s->rectangle[3]=d; s->dirty=1;
}
// Twins Get5334C0 / Set0C7E4DD0: only the member offset differs.
float b23::Scale(const Style* s) { return s->scale; }
void b23::SetScale(Style* s,float value) { s->scale=value; }
}
