#include "ts3/anim/m05_B29.h"
namespace ts3::anim::b29 {
namespace {
void* Address(uintptr_t value) {
#if defined(_M_IX86)
    return reinterpret_cast<void*>(value);
#else
    (void)value;
    return nullptr;
#endif
}
void Release(Ref* p) {
    if (p && --p->refs == 0) {
        p->refs = 1;
        CallGameVirtual<void,0>(p,uint32_t{1});
    }
}
}
uint32_t __fastcall Append(Group* self,void*,Ref* controller,uint32_t first,uint32_t second,float time,float weight,uint32_t flag1,uint32_t flag2) {
    auto* node=CallGameFunction<Entry*,0x004E3F90>(uint32_t(sizeof(Entry)),self->descriptor,uint32_t{0},uint32_t{0},Address(0x00F98980),uint32_t{0xe7});
    node->controller=nullptr;
    node->link.next=&self->list;
    node->link.previous=self->list.previous;
    self->list.previous->next=&node->link;
    self->list.previous=&node->link;
    // Reload through the tail: the original does not retain an allocation cursor.
    auto* tail=reinterpret_cast<Entry*>(self->list.previous);
    auto* old=tail->controller;
    if (controller != old) {
        if (controller) ++controller->refs;
        tail->controller=controller;
        Release(old);
    }
    tail->first=first;
    tail->second=second;
    tail->time=time;
    tail->weight=weight;
    tail->flags[0]=static_cast<uint8_t>(flag1);
    tail->flags[1]=static_cast<uint8_t>(flag2);
    return (first & 0xffffff00u) | static_cast<uint8_t>(flag2);
}
Group* __fastcall Sequence(Group* self,void*) {
    self->vtable=Address(0x00FCB890);
    self->descriptor=Address(0x00FCB898);
    self->list.next=self->list.previous=&self->list;
    return self;
}
Group* __fastcall DeleteSequence(Group* self,void*,uint32_t flags) {
    self->vtable=Address(0x00FCB890);
    CallGameMethod<void,0x0081F320>(&self->list);
    if (flags & 1) CallGameFunction<void,0x004E3F70>(self);
    return self;
}
Group* __fastcall Synchronize(Group* self,void*) {
    self->vtable=Address(0x00FCB894);
    self->descriptor=Address(0x00FCB8C8);
    self->list.next=self->list.previous=&self->list;
    self->master=nullptr;
    return self;
}
Group* __fastcall DeleteSynchronized(Group* self,void*,uint32_t flags) {
    self->vtable=Address(0x00FCB894);
    Release(self->master);
    CallGameMethod<void,0x0081F320>(&self->list);
    if (flags & 1) CallGameFunction<void,0x004E3F70>(self);
    return self;
}
uint32_t __fastcall Forward(void* self,void*,uintptr_t a,uintptr_t b,uintptr_t c,uintptr_t d,uintptr_t e) {
    return CallGameMethod<uint32_t,0x0095A360>(self,a,b,c,d,e,uint32_t{0},uint32_t{0});
}
uint32_t __fastcall ForwardEnabled(void* self,void*,uintptr_t a,uintptr_t b,uintptr_t c,uintptr_t d,uintptr_t e) {
    return CallGameMethod<uint32_t,0x0095A360>(self,a,b,c,d,e,uint32_t{1},uint32_t{0});
}
void __fastcall Notify(Observer* self,void*) {
    if (self->target) CallGameVirtual<void,0>(self->target,&self->payload);
}
void* __cdecl Find(const void* owner) {
    if (!owner) return nullptr;
#if defined(_M_IX86)
    const uint32_t mask=*reinterpret_cast<const uint32_t*>(0x011E90FC);
    return CallGameMethod<void*,0x00935770>(owner,mask);
#else
    AbortNotRewritten("B29 selected binding mask");
    return nullptr;
#endif
}
uint8_t __fastcall Any11(Group* self,void*) {
    for (auto* n=self->list.next;n!=&self->list;n=n->next) {
        auto* c=reinterpret_cast<Entry*>(n)->controller;
        if (c && static_cast<uint8_t>(CallGameVirtual<uint32_t,11>(c))) return 1;
    }
    return 0;
}
uint8_t __fastcall Any12(Group* self,void*) {
    for (auto* n=self->list.next;n!=&self->list;n=n->next) {
        auto* c=reinterpret_cast<Entry*>(n)->controller;
        if (c && static_cast<uint8_t>(CallGameVirtual<uint32_t,12>(c))) return 1;
    }
    return 0;
}
uint32_t __fastcall Count(const Group* self,void*) {
    uint32_t count=0;
    for (auto* n=self->list.next;n!=&self->list;n=n->next) ++count;
    return count;
}
}
