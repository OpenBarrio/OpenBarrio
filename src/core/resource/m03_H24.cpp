#include "ts3/resource/m03_H24.h"
#include <cstring>
namespace ts3::resource::m03::h24 {
namespace {
thread_local Dispatch* dispatch=nullptr;
template<unsigned Slot> uintptr_t Call(void* p) {
    if(dispatch) return dispatch->call(p,Slot);
    return CallGameVirtual<uintptr_t,Slot>(p);
}
void* Allocate(unsigned family,uint32_t size) {
    if(dispatch) return dispatch->allocate(family,size);
#if defined(_M_IX86)
    if(family==0) {
        void* allocator=CallGameFunction<void*,0x00931020>();
        return CallGameVirtual<void*,2>(allocator,size,"ResourceLoad/AsyncFactory/RawData",0u);
    }
    if(family==1) return CallGameFunction<void*,0x004E3F90>(size,"ResourceLoad/AsyncFactory/MemoryStream",0u,0u,0u,0u);
    return CallGameFunction<void*,0x004E41F0>(size,"ResourceLoad/AsyncFactory/RecordGeneric",0u,0u,0u,0u);
#else
    NotRewritten("H24 native allocator dispatch");
#endif
}
uintptr_t Size(void* stream) {
    if(dispatch) return dispatch->size(stream);
    return CallGameVirtual<uintptr_t,7>(stream);
}
uintptr_t Finish(Context& local,void* owner,uint8_t low) {
    if(dispatch) dispatch->clear(&local);
    else CallGameMethod<void,0x007263E0>(&local);
    uintptr_t result=reinterpret_cast<uintptr_t>(local.begin);
    if(local.begin) result=dispatch?dispatch->free(local.begin):CallGameFunction<uintptr_t,0x004E3F70>(local.begin);
    if(owner) result=Call<2>(owner);
    return (result&~uintptr_t(255))|low;
}
}
Dispatch* SetDispatch(Dispatch* p) { auto* old=dispatch; dispatch=p; return old; }
uintptr_t __fastcall Load(void* self,void*,void* record,void* resource,Context* context,uint32_t type) {
    if(type!=Call<16>(self)) return Call<8>(record)&~uintptr_t(255);
    // 0x00726DB7 is reached only with the parent's stack frame in place.
    const void* key=reinterpret_cast<void*>(Call<4>(record));
    void* owner=record;
    Call<1>(record);
    void* stream=reinterpret_cast<void*>(Call<6>(record));
    if(Call<3>(stream)!=0x347223d2u) {
        const uint32_t size=static_cast<uint32_t>(Call<7>(stream));
        void* bytes=Allocate(0,size);
        if(dispatch) dispatch->read(stream,bytes,size);
        else CallGameVirtual<void,12>(stream,bytes,size);
        void* memory=Allocate(1,44);
        if(memory) memory=dispatch?dispatch->memoryStream(memory,bytes,size):CallGameMethod<void*,0x004DF120>(memory,bytes,size,1u,1u,0u,"ResourceLoad/AsyncFactory/MemoryStream");
        void* replacement=Allocate(2,40);
        if(replacement) {
            void* recordKey=reinterpret_cast<void*>(Call<4>(record));
            replacement=dispatch?dispatch->record(replacement,memory,recordKey):CallGameMethod<void*,0x004B3650>(replacement,0u,memory,recordKey,1u,1u);
        }
        if(replacement!=record) {
            if(replacement) Call<1>(replacement);
            owner=replacement;
            Call<2>(record);
        }
        Call<8>(record);
        stream=memory;
    }
    // Only these fields are initialized by EA. Parse supplies count/group fields.
    Context local;
    local.begin=local.end=local.capacity=nullptr;
    local.allocator=const_cast<char*>("ResourceLoad/AsyncFactory/Context");
    local.childCount=0; local.pairs=nullptr;
    reinterpret_cast<uint8_t*>(&local.flags)[0]=0; local.extra=nullptr;
    if(!context) {
        void* block=*reinterpret_cast<void**>(static_cast<uint8_t*>(stream)+sizeof(void*));
        void* bytes=block?*reinterpret_cast<void**>(static_cast<uint8_t*>(block)+2*sizeof(void*)):nullptr;
        uintptr_t size=Size(stream);
        uintptr_t parsed=dispatch?dispatch->parse(&local,bytes,size):CallGameMethod<uintptr_t,0x00726BA0>(&local,bytes,size,0u);
        if(!(parsed&255)) { Call<8>(record); return Finish(local,owner,0); }
        context=&local;
    }
    if(context->count>0) {
        uintptr_t* child=context->groups[0].children;
        void* old=reinterpret_cast<void*>(*child&~uintptr_t(1));
        if(resource) Call<0>(resource);
        if(old) Call<1>(old);
        *child=reinterpret_cast<uintptr_t>(resource)+1;
    }
    std::memcpy(static_cast<uint8_t*>(resource)+sizeof(void*)+4,key,16);
    uintptr_t built=dispatch?dispatch->build(self,stream,context):CallGameMethod<uintptr_t,0x007266F0>(self,stream,context);
    Call<8>(record);
    return Finish(local,owner,static_cast<uint8_t>(built));
}
}
