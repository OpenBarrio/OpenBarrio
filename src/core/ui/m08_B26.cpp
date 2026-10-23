#define TS3_M08_B26_LAYOUTS
#include "ts3/ui/m08_B26.h"
#include "ts3/scratch/clu_4e3f70_Scratch.h"
#include <algorithm>
#include <cstring>
#include <stdexcept>
namespace ts3::ui::b26 {
namespace {
template<uint32_t EA> void* Table() {
#if defined(_M_IX86)
    return reinterpret_cast<void*>(uintptr_t(EA));
#else
    throw std::runtime_error("M08 B26: original table requires x86 image");
#endif
}
Combo* Owner250(ComboInterface* self) {
    return reinterpret_cast<Combo*>(reinterpret_cast<uint8_t*>(self)-offsetof(Combo,interface250));
}
Combo* Owner4(void* self) {
    return reinterpret_cast<Combo*>(reinterpret_cast<uint8_t*>(self)-offsetof(m08::Window,secondary));
}
uint32_t Count(const StringVector& v) {
    const intptr_t bytes=static_cast<intptr_t>(reinterpret_cast<uintptr_t>(v.end)-reinterpret_cast<uintptr_t>(v.begin));
    return static_cast<uint32_t>(bytes >> (sizeof(void*)==4?4:5));
}
void* Cleanup(WideString& s) {
    void* result=s.begin;
    const intptr_t capacity=static_cast<intptr_t>((reinterpret_cast<uintptr_t>(s.cap)-reinterpret_cast<uintptr_t>(s.begin)) & ~uintptr_t(1));
    if(capacity>2 && result) result=ts3::scratch::Free4E3F70(result);
    return result;
}
}
// Adapted from the verified constant leaf FileType; only the immediate differs.
uint32_t __fastcall Type(void*,void*) { return 0xC1; }
int32_t __fastcall Find(ComboInterface* self,void*,const char16_t* text) {
    if(!text) return -1;
    const auto count=Count(self->items);
    const auto* strings=static_cast<const WideString*>(self->items.begin);
    for(uint32_t i=0;i<count;++i) {
        size_t n=0; while(text[n]) ++n;
        const auto& s=strings[i];
        const size_t length=(reinterpret_cast<uintptr_t>(s.end)-reinterpret_cast<uintptr_t>(s.begin))/2;
        if(length==n && (!n || std::memcmp(s.begin,text,n*2)==0)) return static_cast<int32_t>(i);
    }
    return -1;
}
Drawable* __fastcall CreateDrawable(void*,void*,uint32_t,uint32_t allocator) {
    auto* d=CallGameFunction<Drawable*,0x00504E70>(sizeof(Drawable),4u,Table<0x00FBBA00>(),allocator);
    if(!d) return nullptr;
    d->refs=0;
    d->primary=Table<0x00FBB944>();
    d->secondary=Table<0x00FBB92C>();
    d->tertiary=Table<0x00FBB914>();
    d->object10=nullptr; d->object14=nullptr; d->object18=nullptr;
    return d;
}
Combo* __fastcall InitCombo(Combo* self,void*) {
    CallGameMethod<void,0x00515C80>(&self->window);
    auto& c=self->interface250;
    self->window.primary=Table<0x00FBBC18>();
    self->window.secondary=Table<0x00FBBAF0>();
    c.primary=Table<0x00FBBA58>(); c.adapter=Table<0x00FBBA48>();
    c.colors[6]=0xFF718AAD;
    c.enabled=0; c.child=nullptr; c.drawable=nullptr; c.changed=0;
    c.selected=-1; c.previous=-1; c.colors[7]=0xFFFFFFFF;
    c.colors[8]=1; c.colors[9]=1;
    c.items.begin=nullptr; c.items.end=nullptr; c.items.capEnd=nullptr;
    c.items.allocator=Table<0x00FBBA30>(); c.items.field10=nullptr; c.items.field14=nullptr;
    for(auto& metric:c.metrics) metric=2.0f;
    c.metrics[4]=0.0f;
    c.object300=nullptr; c.object304=nullptr;
    for(auto& bound:c.bounds) bound=0.0f;
    c.enabled=1;
    CallGameVirtual<void,0x74/4>(&self->window.secondary,8u,1u);
    std::memset(c.block2AC,0,sizeof(c.block2AC));
    c.colors[4]=0xFFFFFFFF;
    c.colors[1]=0xFFECE9D8; c.colors[2]=0xFFECE9D8;
    c.colors[0]=0xFF000000; c.colors[3]=0xFF737163; c.colors[5]=0xFF000000;
    return self;
}
// Verified QueueNotification adjustor pattern, with this offsets and tail targets changed.
void* __fastcall Query250(ComboInterface* self,void*,uint32_t id) {
    return CallGameMethod<void*,0x0051B220>(Owner250(self),id);
}
void* __fastcall Delete250(ComboInterface* self,void*,uint32_t flags) {
    return CallGameMethod<void*,0x0051D6E0>(Owner250(self),flags);
}
void* __fastcall Query4(void* self,void*,uint32_t id) {
    return CallGameMethod<void*,0x0051B220>(Owner4(self),id);
}
void* __fastcall Delete4(void* self,void*,uint32_t flags) {
    return CallGameMethod<void*,0x0051D6E0>(Owner4(self),flags);
}
void* __fastcall Delete254(void* self,void*,uint32_t flags) {
    auto* c=reinterpret_cast<ComboInterface*>(reinterpret_cast<uint8_t*>(self)-offsetof(ComboInterface,adapter));
    return CallGameMethod<void*,0x0051D6E0>(Owner250(c),flags);
}
// ResourceMapList::ClearVector twin: vector +44, destructor target 73ECC0.
int32_t __fastcall Clear(ComboInterface* self,void*) {
    auto* v=&self->items;
    const auto result=CallGameMethod<int32_t,0x0073ECC0>(v,v->begin,v->end);
    v->end=v->begin;
    return result;
}
void* __fastcall CreateCombo(void*,void*,uint32_t,uint32_t allocator) {
    auto* c=CallGameFunction<Combo*,0x00504E70>(sizeof(Combo),16u,Table<0x00FBBD00>(),allocator);
    if(!c) return nullptr;
    c=InitCombo(c,nullptr);
    return c?&c->window.secondary:nullptr;
}
void* __fastcall Append(ComboInterface* self,void*,const char16_t* text) {
    WideString temporary{nullptr,nullptr,nullptr,Table<0x00FBBD2C>()};
    CallGameMethod<void,0x0040B700>(&temporary,text);
    auto& v=self->items;
    if(reinterpret_cast<uintptr_t>(v.end)<reinterpret_cast<uintptr_t>(v.capEnd)) {
        void* slot=v.end;
        v.end=reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(slot)+sizeof(WideString));
        if(slot) CallGameMethod<void,0x00408EB0>(static_cast<WideString*>(slot),&temporary);
    } else {
        CallGameMethod<void,0x0051D700>(&v,v.end,&temporary);
    }
    return Cleanup(temporary);
}
uint8_t __fastcall Insert(ComboInterface* self,void*,uint32_t index,const char16_t* text) {
    if(index>Count(self->items)) return 0;
    WideString temporary{nullptr,nullptr,nullptr,Table<0x00FBBD2C>()};
    CallGameMethod<void,0x0040B700>(&temporary,text);
    auto* slot=reinterpret_cast<WideString*>(reinterpret_cast<uintptr_t>(self->items.begin)+uintptr_t(index)*sizeof(WideString));
    CallGameMethod<void,0x0051D890>(&self->items,slot,&temporary);
    Cleanup(temporary);
    return 1;
}
ConsoleDrawable* __fastcall InitConsole(ConsoleDrawable* self,void*,void* owner) {
    CallGameMethod<void,0x00538CD0>(self);
    self->owner=owner;
    self->drawable.primary=Table<0x00FBBDF0>();
    self->drawable.secondary=Table<0x00FBBDD8>();
    self->drawable.tertiary=Table<0x00FBBDC0>();
    return self;
}
uint8_t __fastcall Measure(void*,void*,float* out,uint32_t,uint32_t kind) {
    if(kind!=3) return 0;
    out[0]=20.0f; out[1]=12.0f;
    return 1;
}
}
