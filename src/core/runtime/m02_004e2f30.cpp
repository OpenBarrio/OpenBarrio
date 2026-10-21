#include "ts3/runtime/m02_004e2f30.h"
#include <windows.h>
namespace ts3::runtime::m02 {
namespace {
void ReleaseRecord(ThreadRecord* record) {
    // XADD sets ZF from the sum (old-1), not the old value in its source register.
    if(InterlockedDecrement(reinterpret_cast<LONG*>(&record->refs))==0)
        CallGameFunction<void,0x004E2DA0>(record);
}
}
void* LaunchCallback(Thread* self,void* function,void* argument,const ThreadParameters* parameters,void* wrapper) {
#if defined(_M_IX86)
    if(self->record) ReleaseRecord(self->record);
    auto record=CallGameFunction<ThreadRecord*,0x004E2D30>();
    if(record) {
        record->handle=nullptr; record->id=0; record->state=0; record->result=0;
        record->wrapper=nullptr;
        InterlockedExchange(reinterpret_cast<LONG*>(&record->refs),0);
    }
    self->record=record;
    // Deliberately no allocation failure check: the original dereferences null here.
    InterlockedIncrement(reinterpret_cast<LONG*>(&self->record->refs));
    InterlockedIncrement(reinterpret_cast<LONG*>(&self->record->refs));
    self->record->handle=nullptr;
    self->record->function=function; self->record->argument=argument; self->record->wrapper=wrapper;
    auto begin=*reinterpret_cast<BeginThread*>(uintptr_t(0x00F955D0));
    self->record->handle=reinterpret_cast<void*>(begin(nullptr,parameters?parameters->stackSize:0,
        reinterpret_cast<ThreadEntry>(uintptr_t(0x004E2E30)),self->record,4,&self->record->id));
    if(self->record->handle) {
        if(parameters) CallGameMethod<void,0x004E2B70>(self,parameters->name);
        self->record->tlsHandle=self->record->handle;
        if(parameters && parameters->priority) CallGameMethod<void,0x004E2AF0>(self,parameters->priority);
        auto cursor=reinterpret_cast<LONG*>(uintptr_t(0x011CB538));
        int32_t cpu;
        if(parameters && parameters->idealCpu>=0) {
            cpu=parameters->idealCpu;
            if(cpu==*cursor) InterlockedIncrement(cursor);
        } else {
            cpu=*reinterpret_cast<int32_t*>(uintptr_t(0x0114C90C));
            if(cpu<0) cpu=InterlockedIncrement(cursor);
        }
        CallGameMethod<void,0x004E2C50>(self,cpu);
        using Resume=uint32_t(__stdcall*)(void*);
        (*reinterpret_cast<Resume*>(uintptr_t(0x00F95150)))(self->record->handle);
        return self->record->handle;
    }
    ReleaseRecord(self->record);
    ReleaseRecord(self->record);
    self->record=nullptr;
    return nullptr;
#else
    (void)self; (void)function; (void)argument; (void)parameters; (void)wrapper;
    AbortNotRewritten("m02 callback launch game boundaries"); return nullptr;
#endif
}
}
