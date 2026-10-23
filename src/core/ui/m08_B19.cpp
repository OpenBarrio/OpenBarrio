#include "ts3/ui/m08_B19.h"
#include "ts3/ui/m08_abi.h"
#include <xmmintrin.h>
namespace ts3::ui::b19 {
namespace {
using namespace ts3::ui::m08;
template<class R,size_t Slot,class T,class... A> R Virtual(T* self,A... args) {
    auto table=*reinterpret_cast<void***>(self);
    return CallThis(reinterpret_cast<Thiscall<R,T,A...>>(table[Slot]),self,args...);
}
template<uint32_t EA,class R,class T,class... A> R Method(T* self,A... args) {
    return CallThis(Original<Thiscall<R,T,A...>>(EA),self,args...);
}
Button* FromSecondary(void* p) { return reinterpret_cast<Button*>(static_cast<uint8_t*>(p)-offsetof(Button,secondary)); }
Button* FromController(void* p) { return reinterpret_cast<Button*>(static_cast<uint8_t*>(p)-offsetof(Button,controller)); }
uintptr_t SetLow(uintptr_t word,bool on) { return (word&~uintptr_t(255))|uintptr_t(on); }
uint8_t Size(Drawable* self,float* out,float scale) {
    auto* resource=static_cast<Resource*>(self->resource);
    if(!resource) return 0;
    __m128 width=_mm_cvtsi32_ss(_mm_setzero_ps(),resource->width);
    width=_mm_mul_ss(width,_mm_set_ss(scale));
    _mm_store_ss(out,width);
    // Reload the resource after the first store: output may alias the object.
    resource=static_cast<Resource*>(self->resource);
    _mm_store_ss(out+1,_mm_cvtsi32_ss(_mm_setzero_ps(),resource->height));
    return 1;
}
}
uint8_t Notify(Button* self,uint32_t channel,void* target) {
    void* secondary=self?&self->secondary:nullptr;
    void* controller=&self->controller;
    if(channel==1) {
        if(target==secondary) {
            Virtual<uintptr_t,10>(controller,uint32_t(8),uintptr_t(1));
            // EA saves the controller table BEFORE querying focus (reentrancy matters).
            auto table=*reinterpret_cast<void***>(controller);
            void* focus=Virtual<void*,24>(self->manager,uint32_t(1));
            CallThis(reinterpret_cast<ts3::ui::m08::Thiscall<uintptr_t,void,uint32_t,uintptr_t>>(table[10]),controller,uint32_t(2),SetLow(reinterpret_cast<uintptr_t>(focus),focus==secondary));
        } else {
            Virtual<uintptr_t,10>(controller,uint32_t(8),uintptr_t(0));
            Virtual<uintptr_t,10>(controller,uint32_t(2),uintptr_t(0));
        }
    } else {
        Virtual<uintptr_t,10>(controller,uint32_t(16),SetLow(reinterpret_cast<uintptr_t>(secondary),target==secondary));
        if(target!=secondary) Virtual<uintptr_t,10>(controller,uint32_t(2),uintptr_t(0));
    }
    return 1;
}
uint8_t InvokeAction(Button* self) {
    Virtual<uintptr_t,42>(self);
    return 1;
}
// Adapted from verified FileType/constant leaves; only immediate differs.
uint32_t ButtonType(Button*) { return 0xEEEFE2C3u; }
uintptr_t InitializeSecondary(void* secondary) {
    auto* self=FromSecondary(secondary);
    if(!self->initialized) {
        self->initialized=1;
        Virtual<uintptr_t,29>(secondary,uint32_t(8),uint32_t(1));
    }
    return Method<0x00514950,uintptr_t>(secondary);
}
void* QueryDrawable(Drawable* self,uint32_t id) {
    if(id==0x2F02135Cu) return self?&self->interface00C:nullptr;
    if(id==0xEEC58382u) return self?&self->secondary:nullptr;
    return Method<0x00504D90,void*>(self,id);
}
void SetBinding(Binding* self,void* object,uint8_t flag) {
    if(object!=self->object) {
        if(self->object) Virtual<uintptr_t,1>(self->object);
        self->object=object;
        if(object) Virtual<uintptr_t,0>(object);
    }
    self->flag=flag;
}
uint32_t DrawableType(Drawable*) { return 0x4F03465Eu; }
uint8_t StandardSize(Drawable* self,float* out,uint32_t,uint32_t) { return Size(self,out,0.25f); }
uint8_t RadioSize(Drawable* self,float* out,uint32_t,uint32_t) { return Size(self,out,0x1.555556p-3f); }
uint32_t MakeHandle(Button* self,Handle* out) {
    out->table=ts3::ui::m08::Original<void**>(0x00FBB110);
    out->object=self;
    out->count=1;
    uint32_t type=Virtual<uint32_t,5>(self);
    out->type=type;
    return type;
}
uintptr_t InitializePrimary(Button* self) {
    if(!self->initialized) {
        self->initialized=1;
        Virtual<uintptr_t,29>(&self->secondary,uint32_t(8),uint32_t(1));
    }
    return Method<0x00514340,uintptr_t>(self);
}
// Adapted from verified DDFRecordIndex; receiver field moves from 240 to 26C.
void* Index(Button* self) { return self->index; }
// Adapted from QueueNotification::DeletingDestruct; target/adjustment differ.
void* DeleteSecondary(void* secondary,uint32_t flags) {
    return Method<0x0051A290,void*>(FromSecondary(secondary),flags);
}
void* QueryController(void* controller,uint32_t id) {
    return Method<0x00517B60,void*>(FromController(controller),id);
}
#if defined(_M_IX86)
uint8_t __fastcall HookNotify(Button* s,void*,uint32_t c,void* t) { return Notify(s,c,t); }
uint8_t __fastcall HookInvokeAction(Button* s,void*) { return InvokeAction(s); }
uint32_t __fastcall HookButtonType(Button* s,void*) { return ButtonType(s); }
uintptr_t __fastcall HookInitializeSecondary(void* s,void*) { return InitializeSecondary(s); }
void* __fastcall HookQueryDrawable(Drawable* s,void*,uint32_t id) { return QueryDrawable(s,id); }
void __fastcall HookSetBinding(Binding* s,void*,void* o,uint8_t f) { SetBinding(s,o,f); }
uint32_t __fastcall HookDrawableType(Drawable* s,void*) { return DrawableType(s); }
uint8_t __fastcall HookStandardSize(Drawable* s,void*,float* o,uint32_t a,uint32_t b) { return StandardSize(s,o,a,b); }
uint8_t __fastcall HookRadioSize(Drawable* s,void*,float* o,uint32_t a,uint32_t b) { return RadioSize(s,o,a,b); }
uint32_t __fastcall HookMakeHandle(Button* s,void*,Handle* o) { return MakeHandle(s,o); }
uintptr_t __fastcall HookInitializePrimary(Button* s,void*) { return InitializePrimary(s); }
void* __fastcall HookIndex(Button* s,void*) { return Index(s); }
void* __fastcall HookDeleteSecondary(void* s,void*,uint32_t f) { return DeleteSecondary(s,f); }
void* __fastcall HookQueryController(void* s,void*,uint32_t id) { return QueryController(s,id); }
#endif
}
