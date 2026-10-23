#include "ts3/anim/m05_M67.h"
namespace ts3::anim::m67 {
Link* InsertActive(Manager* self,Node* node) {
    Link* link=&node->link;
    link->next=self->active.next;
    link->previous=&self->active;
    self->active.next=link;
    link->next->previous=link;
    return link;
}
Link* InsertPending(Manager* self,Node* node) {
    Link* link=&node->link;
    link->next=self->pending.next;
    link->previous=&self->pending;
    self->pending.next=link;
    link->next->previous=link;
    return link;
}
Node* Remove(Manager* self,Node* node) {
    if(node->link.previous || node->link.next) {
        if(node==self->current) {
            self->current=node->link.next ? reinterpret_cast<Node*>(reinterpret_cast<unsigned char*>(node->link.next)-offsetof(Node,link)) : nullptr;
        }
        node->link.previous->next=node->link.next;
        node->link.next->previous=node->link.previous;
        node->link.previous=nullptr;
        node->link.next=nullptr;
    }
    return node;
}
namespace {
uint32_t InterfaceId() {
#if defined(_M_IX86)
    return *reinterpret_cast<const uint32_t*>(0x011dc484);
#else
    AbortNotRewritten("m67 animation interface id"); return 0;
#endif
}
}
void ResetObject(void* object) {
    if(object) {
        void* animation=CallGameMethod<void*,0x00935770>(object,InterfaceId());
        if(animation) CallGameMethod<void,0x00941bc0>(animation);
    }
}
Resource* GetResource(Manager* self) {
    Resource* cached=self->resource;
    if(cached && cached->key.low==self->key.low && cached->key.high==self->key.high) return cached;
    void* manager=CallGameFunction<void*,0x004afd20>();
    struct ResourceKey { Key id; uint32_t type, group; } key{self->key,0x6b20c4f3,0};
    if(self->resource) {
        Resource* old=self->resource;
        self->resource=nullptr;
        CallGameVirtual<void,1>(old);
    }
    CallGameVirtual<void,3>(manager,&key,&self->resource,0u,0u,0u,0u);
    return self->resource;
}
uintptr_t Drain(Manager* self) {
    uintptr_t result=reinterpret_cast<uintptr_t>(self->begin);
    while(self->begin!=self->end) {
        Key id=self->end[-1];
        result=reinterpret_cast<uintptr_t>(self->end);
        --self->end;
        if(id.low || id.high) {
            CallGameFunction<void,0x00798d40>(id.low,id.high);
            void* world=CallGameFunction<void*,0x00938b50>();
            result=reinterpret_cast<uintptr_t>(world);
            if(world) {
                void* object=CallGameMethod<void*,0x00939560>(world,id.low,id.high);
                ResetObject(object);
                result=CallGameMethod<uintptr_t,0x00939420>(world,id.low,id.high);
            }
        }
    }
    return result;
}
}
#if defined(_M_IX86)
namespace ts3::anim::m67::hooks {
Link* __fastcall InsertActive(Manager* s,void*,Node* n) { return m67::InsertActive(s,n); }
Link* __fastcall InsertPending(Manager* s,void*,Node* n) { return m67::InsertPending(s,n); }
Node* __fastcall Remove(Manager* s,void*,Node* n) { return m67::Remove(s,n); }
void __stdcall ResetObject(void* o) { m67::ResetObject(o); }
Resource* __fastcall GetResource(Manager* s,void*) { return m67::GetResource(s); }
uintptr_t __fastcall Drain(Manager* s,void*) { return m67::Drain(s); }
}
#endif
