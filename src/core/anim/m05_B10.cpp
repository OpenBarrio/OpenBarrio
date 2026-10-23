#include "ts3/anim/m05_B10.h"
namespace ts3::anim::b10 {
namespace {
void Retain(Controller* p) { if(p) p->refs=static_cast<int32_t>(static_cast<uint32_t>(p->refs)+1u); }
uintptr_t Release(Controller* p) {
    if(!p) return 0;
    auto refs=static_cast<uint32_t>(p->refs)-1u;
    p->refs=static_cast<int32_t>(refs);
    if(refs) return refs;
    p->refs=1;
    return CallGameVirtual<uintptr_t,0>(p,1u);
}
Node* Allocate(Nodes* list) {
    return static_cast<Node*>(CallGameFunction<void*,0x004e3f90>(sizeof(Node),list->descriptor,0u,0u,reinterpret_cast<const char*>(uintptr_t(0xf98980)),231u));
}
void Link(Node* before,Node* node) {
    node->next=before; node->previous=before->previous;
    before->previous->next=node; before->previous=node;
}
}
Controller* Find(Sequence* self,uint32_t id) {
    if(self->current&&self->current->uniqueId==id) return self->current;
    if(self->pending&&self->pending->uniqueId==id) return self->pending;
    auto end=reinterpret_cast<Node*>(&self->nodes);
    for(auto p=self->nodes.next;p!=end;p=p->next)
        if(p->value.controller&&p->value.controller->uniqueId==id) return p->value.controller;
    return nullptr;
}
float Duration(Sequence* self) {
    float total=0;
    if(self->pending) {
        CallGameVirtual<void*,1>(&self->pending->secondary8,0xb489fc2cu);
        total=CallGameVirtual<float,9>(self->pending);
        if(self->overlap>0) { float d=total-self->overlap; total=d>0?d:0; }
    }
    float absent;
#if defined(_M_IX86)
    absent=*reinterpret_cast<const float*>(0x0114f594);
#else
    absent=-1.0f;
#endif
    auto end=reinterpret_cast<Node*>(&self->nodes);
    for(auto p=self->nodes.next;p!=end;p=p->next) {
        float duration=CallGameVirtual<float,10>(p->value.controller);
        // LAHF/test AH,44h/JP selects zero only on ordered equality;
        // unordered has BOTH ZF and PF set, hence even parity and keeps the NaN.
        if(duration==absent) duration=0;
        total=total+duration;
        if(p->value.overlap>0) { float d=total-p->value.overlap; total=d>0?d:0; }
    }
    return total;
}
uintptr_t ClearFlags(void* secondary,uint32_t tag,uint32_t unused) {
    (void)unused;
    auto self=reinterpret_cast<Sequence*>(static_cast<unsigned char*>(secondary)-offsetof(Sequence,secondary28));
    Retain(&self->base);
    auto end=reinterpret_cast<Node*>(&self->nodes);
    for(auto p=self->nodes.next;p!=end;p=p->next)
        if(p->value.tag==tag) p->value.flags&=~4u;
    return Release(&self->base);
}
bool Remove(DelegateList* self,void* key) {
    auto end=reinterpret_cast<DelegateNode*>(&self->links);
    for(auto p=reinterpret_cast<DelegateNode*>(self->links.next);p!=end;p=p->next) {
        if(p&&p->key==key) {
            if(!self->depth) {
                p->previous->next=p->next; p->next->previous=p->previous;
                CallGameFunction<void,0x004e3f70>(p);
            } else { p->key=nullptr; p->payload=0; }
            return true;
        }
    }
    return false;
}
Value* Assign(Value* self,const Value* value) {
    if(self->controller!=value->controller) {
        auto next=value->controller; Retain(next);
        auto old=self->controller; self->controller=next; Release(old);
    }
    self->tag=value->tag; self->flags=value->flags;
    self->weight=value->weight; self->overlap=value->overlap;
    return self;
}
void RemoveAll(Sequence* self,void* key) {
    if(self->current) Remove(reinterpret_cast<DelegateList*>(&self->current->secondaryC),key);
    if(self->pending) Remove(reinterpret_cast<DelegateList*>(&self->pending->secondaryC),key);
    auto end=reinterpret_cast<Node*>(&self->nodes);
    for(auto p=self->nodes.next;p!=end;p=p->next)
        if(p->value.controller) Remove(reinterpret_cast<DelegateList*>(&p->value.controller->secondaryC),key);
}
Node* Append(Nodes* self) {
    auto p=Allocate(self); p->value={};
    Link(reinterpret_cast<Node*>(self),p); return p;
}
void InsertRange(Nodes* self,Node* before,Node* first,Node* last,uint32_t unused) {
    (void)unused;
    for(auto p=first;p!=last;p=p->next) {
        auto node=Allocate(self);
        node->value.controller=p->value.controller; Retain(node->value.controller);
        node->value.tag=p->value.tag; node->value.flags=p->value.flags;
        node->value.weight=p->value.weight; node->value.overlap=p->value.overlap;
        Link(before,node);
    }
}
uint32_t Count(Nodes* self) {
    uint32_t count=0; auto end=reinterpret_cast<Node*>(self);
    for(auto p=self->next;p!=end;p=p->next) ++count;
    return count;
}
uint32_t Type(void*) { return 0x067c5cab; }
void* Query(void* self,uint32_t id) {
    if(id==0x067c5cab||id==0x7489d72b) return static_cast<unsigned char*>(self)-8;
    return nullptr;
}
uint32_t Write(Writer* self,uint32_t value) {
    auto result=CallGameFunction<uint32_t,0x004ddcc0>(self->stream,value,self->order);
    CallGameMethod<void,0x007d77c0>(self,result);
    return result;
}
}
