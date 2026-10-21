#include "ts3/runtime/m02_004e31a0.h"
#include "ts3/runtime/m02_004e2d30.h"
#include "ts3/runtime/m02_004e2da0.h"
#include "ts3/runtime/m02_T2.h"
#define NOMINMAX
#include <windows.h>
namespace ts3::runtime::m02 {
namespace {
thread_local VirtualLaunchDispatch* dispatch=nullptr;
uint32_t Add(uint32_t* p,uint32_t n) {
    return uint32_t(InterlockedExchangeAdd(reinterpret_cast<volatile LONG*>(p),LONG(n)));
}
void Release(ThreadRecord* p) {
    // XADD's JNE tests the SUM, not the returned old value.
    if(Add(&p->refs,UINT32_MAX)==1) {
        if(dispatch) dispatch->recycle(dispatch->context,p);
        else RecycleThreadRecord(p);
    }
}
}
VirtualLaunchDispatch* SetVirtualLaunchDispatch(VirtualLaunchDispatch* value) {
    auto old=dispatch; dispatch=value; return old;
}
void* LaunchVirtualThread(Thread* self,void* object,void* argument,const ThreadParameters* params,void* wrapper) {
    if(self->record) Release(self->record);
    auto record=dispatch?dispatch->reserve(dispatch->context):ReserveThreadRecord();
    if(record) {
        record->handle=nullptr; record->id=0; record->state=0; record->result=0;
        record->wrapper=nullptr;
        InterlockedExchange(reinterpret_cast<volatile LONG*>(&record->refs),0);
    }
    self->record=record;
    // EA has no allocation-failure guard here: even a null record is dereferenced.
    Add(&record->refs,1);
    Add(&self->record->refs,1);
    record=self->record;
    record->handle=nullptr;
    record->function=object; record->argument=argument; record->wrapper=wrapper;
    const unsigned stack=params?params->stackSize:0;
    if(dispatch) {
        record->handle=reinterpret_cast<void*>(dispatch->begin(dispatch->context,stack,dispatch->entry,record,4,&record->id));
    } else {
#if defined(_M_IX86)
        auto begin=*reinterpret_cast<BeginThread*>(0x00F955D0);
        record->handle=reinterpret_cast<void*>(begin(nullptr,stack,reinterpret_cast<ThreadEntry>(0x004E30A0),record,4,&record->id));
#else
        AbortNotRewritten("m02 virtual launch requires native dispatch");
#endif
    }
    if(self->record->handle) {
        if(params) {
            if(dispatch) dispatch->name(dispatch->context,self,params->name);
            else t2::DebugName(self,params->name);
        }
        self->record->tlsHandle=self->record->handle;
        int32_t* defaultCpu;
        uint32_t* cursor;
        if(dispatch) { defaultCpu=dispatch->defaultCpu; cursor=dispatch->cpuCursor; }
        else {
#if defined(_M_IX86)
            defaultCpu=reinterpret_cast<int32_t*>(0x0114C90C);
            cursor=reinterpret_cast<uint32_t*>(0x011CB538);
#else
            AbortNotRewritten("m02 virtual launch CPU globals require dispatch");
#endif
        }
        int32_t cpu;
        if(params && params->priority!=0) {
            if(dispatch) dispatch->priority(dispatch->context,self,params->priority);
            else t2::Priority(self,params->priority);
        }
        if(params && params->idealCpu>=0) {
            cpu=params->idealCpu;
            if(uint32_t(cpu)==*cursor) Add(cursor,1);
        } else {
            cpu=*defaultCpu;
            if(cpu<0) cpu=int32_t(Add(cursor,1)+1);
        }
        if(dispatch) {
            dispatch->ideal(dispatch->context,self,cpu);
            dispatch->resume(dispatch->context,self->record->handle);
        } else {
            t2::Ideal(self,cpu);
#if defined(_M_IX86)
            using Resume=uint32_t(__stdcall*)(void*);
            (*reinterpret_cast<Resume*>(0x00F95150))(self->record->handle);
#endif
        }
        return self->record->handle;
    }
    Release(self->record);
    Release(self->record);
    self->record=nullptr;
    return nullptr;
}
void* __fastcall LaunchVirtualThreadEntry(Thread* p,void*,void* object,void* argument,const ThreadParameters* params,void* wrapper) {
    return LaunchVirtualThread(p,object,argument,params,wrapper);
}
}
