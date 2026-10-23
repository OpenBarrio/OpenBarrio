#include "ts3/anim/m05_B30.h"
namespace ts3::anim::b30 {
namespace {
void** Table(uint32_t address) {
#if defined(_M_IX86)
    return reinterpret_cast<void**>(static_cast<uintptr_t>(address));
#else
    // Native layout has no original image; keep distinct, pointer-sized table tokens.
    static void* state[1]{}; static void* entity[1]{}; static void* secondary[1]{};
    return address==0x01022dc8 ? state : address==0x01022e0c ? entity : secondary;
#endif
}
}
void InitState(State* self) { self->vtable=Table(0x01022dc8); }
uint32_t StateType(State*) { return 0x34cec21c; }
State* QueryState(State* self,uint32_t type) { return type==0x34cec21c ? self : nullptr; }
State* ConstructState(State* self,void* payload) { InitState(self); self->payload=payload; return self; }
State* DeleteState(State* self,uint32_t flags) {
    InitState(self);
    if(flags & 1) CallGameFunction<void,0x004e3f70>(self);
    return self;
}
float ForwardTime(Entity* self) { return CallGameVirtual<float,5>(self); }
float DefaultTime(Entity*) { return 5.0f; }
uint32_t ControllerType(Entity*) { return 0x54cfb94f; }
Entity* QueryController(Entity* self,uint32_t type) { return type==0x54cfb94f ? self : nullptr; }
uintptr_t Release(Entity** wrapper) {
    Entity* entity=*wrapper;
    if(!entity) return 0;
    // Unsigned arithmetic deliberately retains wraparound for zero and INT_MIN refs.
    uint32_t refs=entity->refs-1u;
    entity->refs=refs;
    if(refs) return refs;
    entity->refs=1;
    return CallGameVirtual<uintptr_t,0>(&entity->secondary,uint32_t(1));
}
Link* ResetEntity(Entity* self) {
    self->vtable=Table(0x01022e0c);
    self->secondary=Table(0x01022e08);
    self->payload=nullptr;
    self->links.previous=&self->links;
    self->links.next=&self->links;
    self->secondary=Table(0x0107396c);
    return &self->links;
}
uint32_t EntityType(Entity*) { return 0x94cfa9a4; }
Entity* QueryEntity(Entity* self,uint32_t type) { return type==0x94cfa9a4 ? self : nullptr; }
Entity* DeleteSecondary(void* self,uint32_t flags) {
    return DeleteEntity(reinterpret_cast<Entity*>(static_cast<unsigned char*>(self)-offsetof(Entity,secondary)),flags);
}
Entity* DeleteEntity(Entity* self,uint32_t flags) {
    ResetEntity(self);
    if(flags & 1) CallGameFunction<void,0x004e3f70>(self);
    return self;
}
}
