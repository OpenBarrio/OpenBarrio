#include "ts3/anim/m05_B27.h"
#include <cstring>
namespace ts3::anim::b27 {
void SetSpeed(Task* p,uint32_t value) { if (!(p->flags & 1)) p->speed=value; }
void SetTime(Task* p,uint32_t value) { if (!(p->flags & 1)) p->time=value; }
void SetEnabled(Task* p,uint32_t value) {
    if (uint8_t(value)) p->flags |= 4;
    else { p->flags &= ~4u; p->time=0; }
}
LookAtTarget* SetAngle34(Task* p,float value) { auto* t=p->target; t->angle34=value; return t; }
LookAtTarget* SetAngle38(Task* p,float value) { auto* t=p->target; t->angle38=value; return t; }
LookAtTarget* SetGain(Task* p,float value) { auto* t=p->target; t->gain=value; return t; }
LookAtTarget* SetValue40(Task* p,float value) { auto* t=p->target; t->value40=value; return t; }
LookAtTarget* SetValue44(Task* p,float value) { auto* t=p->target; t->value44=value; return t; }
LookAtTarget* SetAngle48(Task* p,float value) { auto* t=p->target; t->angle48=value; return t; }
Iterator* InitIterator(Iterator* p,Slots* slots) {
    p->slots=slots; p->index=slots && slots->entries[0] ? -1 : -2; return p;
}
uint32_t Next(Iterator* p) {
    uint32_t result=uint32_t(p->index);
    if (p->index < -1) return result & 0xffffff00u;
    p->index=int32_t(uint32_t(p->index)+1); result=1;
    while (uint32_t(p->index)<10 && p->slots->entries[p->index] && p->slots->entries[p->index]->kind==5)
        ++p->index;
    if (uint32_t(p->index)>=10 || !p->slots->entries[p->index]) { p->index=-2; result=0; }
    return result;
}
void* Resolve(LookAtTarget* p) {
    if (!(p->identifierLow | p->identifierHigh)) return nullptr;
    auto* world=CallGameFunction<void*,0x00938B50>();
    if (!world) return nullptr;
    auto* found=CallGameMethod<void*,0x00939560>(world,p->identifierLow,p->identifierHigh);
    if (!found) p->identifierLow=p->identifierHigh=0;
    return found;
}
// Verified DDFRecordIndex twin: the sole difference is field +28 instead of +240.
void* GetIndex(IndexOwner* p) { return p->index; }
static Task* Owner(ListLink* p) { return p ? reinterpret_cast<Task*>(reinterpret_cast<unsigned char*>(p)-offsetof(Task,links)) : nullptr; }
uintptr_t Insert(ListLink* p,Task* task) {
    Task* sentinel=Owner(p);
    Task* current=Owner(p->next);
    if(current!=sentinel) {
        while(task->priority < current->priority) {
            current=Owner(current->links.next);
            if(current==sentinel) break;
        }
        if(current!=sentinel && task->priority==current->priority) {
            Task* scan=current;
            while(scan->priority==task->priority) {
                if(scan==task) return reinterpret_cast<uintptr_t>(scan) & ~uintptr_t(255);
                scan=Owner(scan->links.next);
                if(scan==sentinel) break;
            }
        }
    }
    ListLink* previous=current->links.previous;
    current->links.previous=&task->links;
    previous->next=&task->links;
    task->links.previous=previous;
    ++task->refs;
    task->links.next=&current->links;
    return 1;
}
uint8_t Activate(Task* p) {
    if(p->flags & 1) return 1;
    // EA's (argument | 7) == 0 guard is impossible, not an argument check.
    auto* world=CallGameFunction<void*,0x00938B50>();
    if(!world) return 0;
    auto* entity=CallGameMethod<void*,0x00939560>(world,p->sourceLow,p->sourceHigh);
    if(!entity) return 0;
#if defined(_M_IX86)
    auto key=*reinterpret_cast<uint32_t*>(0x011DC484);
#else
    AbortNotRewritten("LookAtTask world component key"); uint32_t key=0;
#endif
    auto* component=CallGameMethod<IndexOwner*,0x00935770>(entity,key);
    if(!component || !component->index) return 0;
    if(!uint8_t(Insert(static_cast<ListLink*>(component->index),p))) return 0;
    p->flags |= 1; p->active=1;
    return 1;
}
#if defined(_M_IX86)
void __fastcall HookSetSpeed(Task* p, void*, uint32_t value) { return SetSpeed(p, value); }
LookAtTarget* __fastcall HookSetAngle34(Task* p, void*, float value) { return SetAngle34(p, value); }
LookAtTarget* __fastcall HookSetAngle38(Task* p, void*, float value) { return SetAngle38(p, value); }
LookAtTarget* __fastcall HookSetGain(Task* p, void*, float value) { return SetGain(p, value); }
LookAtTarget* __fastcall HookSetValue40(Task* p, void*, float value) { return SetValue40(p, value); }
LookAtTarget* __fastcall HookSetValue44(Task* p, void*, float value) { return SetValue44(p, value); }
LookAtTarget* __fastcall HookSetAngle48(Task* p, void*, float value) { return SetAngle48(p, value); }
void __fastcall HookSetTime(Task* p, void*, uint32_t value) { return SetTime(p, value); }
void __fastcall HookSetEnabled(Task* p, void*, uint32_t value) { return SetEnabled(p, value); }
Iterator* __fastcall HookInitIterator(Iterator* p, void*, Slots* slots) { return InitIterator(p, slots); }
uint32_t __fastcall HookNext(Iterator* p, void*) { return Next(p); }
void* __fastcall HookResolve(LookAtTarget* p, void*) { return Resolve(p); }
void* __fastcall HookGetIndex(IndexOwner* p, void*) { return GetIndex(p); }
uintptr_t __fastcall HookInsert(ListLink* p, void*, Task* task) { return Insert(p, task); }
uint8_t __fastcall HookActivate(Task* p, void*) { return Activate(p); }
#endif
}
