#include "ts3/anim/m05_M68.h"
#include <intrin.h>
namespace ts3::anim::m68 {
namespace {
Node* Owner(Link* link) { return link ? reinterpret_cast<Node*>(reinterpret_cast<unsigned char*>(link)-offsetof(Node,link)) : nullptr; }
void Retain(Resource* r) { if(r) _InterlockedIncrement(&r->refs); }
void Release(Resource* r) {
    if(r && !_InterlockedDecrement(&r->refs)) {
        _InterlockedExchange(&r->refs,1);
        CallGameVirtual<void,0>(r,1);
    }
}
}
uintptr_t __fastcall Update(Manager* self,void*,float dt) {
    self->flags |= 8;
    Node* sentinel=reinterpret_cast<Node*>(reinterpret_cast<unsigned char*>(self)-offsetof(Node,link));
    bool pending=false;
    self->cursor=Owner(self->link.next);
    while(self->cursor!=sentinel) {
        Node* node=self->cursor;
        self->cursor=Owner(node->link.next);
        Resource* held=node->retained; Retain(held);
        CallGameMethod<void,0x00942C00>(node,dt);
        bool enabled=node->state && node->state->enabled && dt>0.0f;
        if(!CallGameMethod<uint8_t,0x0093E190>(node)) {
            if(enabled) CallGameMethod<void,0x00940430>(node,dt);
        } else if(enabled) {
            CallGameMethod<void,0x00C0FD60>(node); pending=true;
        }
        Release(held);
    }
    if(pending) {
#if defined(_M_IX86)
        const float step=*reinterpret_cast<const float*>(0x010434C4);
        const float minimum=*reinterpret_cast<const float*>(0x00FC25C4);
#else
        const float step=0.03333333507180214f, minimum=0.01666666753590107f;
#endif
        auto pass=[&](float delta) {
            self->cursor=Owner(self->link.next);
            while(self->cursor!=sentinel) {
                Node* node=self->cursor;
                self->cursor=Owner(node->link.next);
                Resource* held=node->retained; Retain(held);
                if(CallGameMethod<uint8_t,0x0093E190>(node) && node->state && node->state->enabled)
                    CallGameMethod<void,0x00940430>(node,delta);
                Release(held);
            }
        };
        while(dt>=step) { dt-=step; pass(step); }
        if(dt>=minimum) pass(dt);
    }
    self->cursor=sentinel;
    CallGameFunction<void,0x00577060>();
    self->flags &= ~8u;
    return CallGameMethod<uintptr_t,0x00571A50>(self);
}
void __fastcall SetName(NameOwner* self,void*,Text* text) {
    if(!text) return;
    auto bytes=reinterpret_cast<uintptr_t>(text->end)-reinterpret_cast<uintptr_t>(text->begin);
    if(!(static_cast<uint32_t>(bytes)&0xfffffffeu)) return;
    if(!self->name) {
#if defined(_M_IX86)
        auto descriptor=reinterpret_cast<const char*>(0x00FC27B8);
        self->name=CallGameFunction<Text*,0x004E3F90>(0x10,descriptor,0,0,0,0);
        if(self->name) self->name=CallGameMethod<Text*,0x00408890>(self->name,&descriptor);
#else
        NotRewritten("M68 name allocation descriptor");
#endif
    }
    if(text!=self->name) CallGameMethod<void,0x00408E20>(self->name,text->begin,text->end);
    self->hash=CallGameFunction<uint64_t,0x0057A470>(text->begin,0);
}
void* __fastcall Create(void*,void*) {
#if defined(_M_IX86)
    void* object=CallGameFunction<void*,0x004E3F90>(0x40,reinterpret_cast<const char*>(0x00FC281C),0,0,0,0);
    return object ? CallGameMethod<void*,0x0057C7F0>(object) : nullptr;
#else
    NotRewritten("M68 resource creation");
#endif
}
uint8_t __stdcall Load(void* object,void* stream,uint32_t arg,uint32_t endian) {
    return CallGameMethod<uint8_t,0x0057C8D0>(object,stream,arg,endian);
}
uint32_t __fastcall Type(void*,void*) { return 0x033260e3u; }
uintptr_t __cdecl RegisterFactories() {
    void* manager=CallGameFunction<void*,0x004AFD20>();
#if defined(_M_IX86)
    void* clip=CallGameFunction<void*,0x004E41F0>(0x10,reinterpret_cast<const char*>(0x00FC2874),0,0,0,0);
    if(clip) clip=CallGameMethod<void*,0x0057A4E0>(clip);
    CallGameVirtual<uintptr_t,10>(manager,1,clip,0);
    void* rig=CallGameFunction<void*,0x004E41F0>(0x10,reinterpret_cast<const char*>(0x00FC285C),0,0,0,0);
    if(rig) rig=CallGameMethod<void*,0x0057B300>(rig);
    CallGameVirtual<uintptr_t,10>(manager,1,rig,0);
    void* mask=CallGameFunction<void*,0x004E41F0>(8,reinterpret_cast<const char*>(0x00FC2840),0,0,0,0);
    if(mask) {
        auto* r=static_cast<Resource*>(mask);
        r->vtable=reinterpret_cast<void**>(0x00FB21E8);
        _InterlockedExchange(&r->refs,0);
        r->vtable=reinterpret_cast<void**>(0x00FC27D8);
    }
    return CallGameVirtual<uintptr_t,10>(manager,1,mask,0);
#else
    (void)manager; NotRewritten("M68 factory registration descriptors");
#endif
}
}
