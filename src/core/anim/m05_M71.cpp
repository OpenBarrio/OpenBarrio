#include "ts3/anim/m05_M71.h"
#include "ts3/layout.h"
namespace ts3::anim::m71 {
WeightedRecord* SumWeights(WeightSum* self) {
    self->total=0.0f;
    auto p=self->begin;
    while(p!=self->end) { self->total=p->weight+self->total; ++p; }
    return p;
}
void* AcquirePose() {
#if defined(_M_IX86)
    using LockFn=void(__stdcall*)(void*);
    auto enter=*reinterpret_cast<LockFn*>(0x00f9512c);
    auto leave=*reinterpret_cast<LockFn*>(0x00f95130);
    auto lock=reinterpret_cast<void*>(0x011cd8d8);
    auto& head=*reinterpret_cast<void**>(0x011cd8c4);
    enter(lock);
    while(!head) {
        if(!CallGameMethod<uint8_t,0x004e42f0>(reinterpret_cast<void*>(0x011cd8b4),0,0)) {
            leave(lock); return nullptr;
        }
    }
    void* result=head;
    head=*static_cast<void**>(result);
    leave(lock);
    return result;
#else
    AbortNotRewritten("m71 global pose pool"); return nullptr;
#endif
}
uintptr_t AssignRig(Pose* self,RigResource* rig) {
    auto old=self->rig;
    if(rig!=old) {
        if(rig) CallGameVirtual<uintptr_t,0>(rig);
        self->rig=rig;
        if(old) CallGameVirtual<uintptr_t,1>(old);
    }
    // Deliberately dereferences the current rig even when the argument is null.
    auto& skeleton=self->rig->skeleton;
    self->boneCount=static_cast<uint16_t>((reinterpret_cast<intptr_t>(skeleton.end)-reinterpret_cast<intptr_t>(skeleton.begin))/0x70);
    self->flags=0;
    self->workspace=self->inlineWorkspace;
    if(rig) return CallGameVirtual<uintptr_t,1>(rig);
    return reinterpret_cast<uintptr_t>(self->inlineWorkspace);
}
void ClearWeights(Pose* self) {
    if(self->weights) for(auto p=self->weights->begin;p!=self->weights->end;++p) *p=0.0f;
}
void EnsureWorld(Pose* self) {
    if(!(self->flags&1)) {
        CallGameMethod<void,0x00574500>(self,0,static_cast<int>(self->boneCount));
        self->flags|=1;
    }
}
SrtValue* GetWorld(Pose* self) {
    if(!self->world) { self->flags&=0xfffe; self->world=self->inlineWorld; }
    EnsureWorld(self);
    return self->world;
}
}
