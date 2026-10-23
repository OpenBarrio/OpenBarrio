#include "ts3/resource/m03_G02.h"
#include "ts3/layout.h"
#include <atomic>
namespace ts3::resource::m03::g02 {
namespace {
void SetVtable(void*& target,uintptr_t address) {
#if defined(_M_IX86)
    target=reinterpret_cast<void*>(address);
#else
    (void)target; (void)address;
    ts3::AbortNotRewritten("G02 original resource vtable");
#endif
}
template<unsigned Address> void RetireHash(b01::Hash& h) {
    ts3::CallGameMethod<void,Address>(&h,h.buckets,h.count);
    h.size=0;
    if(h.count>1) ts3::CallGameVirtual<void,3>(h.allocator.core_allocator(),h.buckets,uint32_t(h.count*4+4));
}
}
uintptr_t __fastcall Destroy(Manager* self,void*) {
    SetVtable(self->vtable,0x00fb2da0);
    ts3::CallGameMethod<void,0x004e1670>(&self->factoryMutex);
    ts3::CallGameMethod<void,0x004e1670>(&self->typeMutex);
    ts3::CallGameMethod<void,0x004e1670>(&self->nameMutex);
    RetireHash<0x004b1710>(self->keyNames);
    RetireHash<0x004b16a0>(self->typeNames);
    auto& v=self->callbacks;
    ts3::CallGameMethod<void,0x004b0e10>(&v,v.begin(),v.end());
    if(v.begin()) {
        const auto bytes=v.capacity()*sizeof(manager::CallbackEntry);
        ts3::CallGameVirtual<void,3>(v.get_allocator().core_allocator(),v.begin(),uint32_t((int32_t(bytes)>>3)*8));
    }
    RetireHash<0x004b2260>(self->factories);
    ts3::CallGameMethod<void,0x004e1670>(&self->mutex);
    ts3::CallGameMethod<void,0x004b0a60>(&self->databases);
    auto result=ts3::CallGameMethod<uintptr_t,0x004b0b00>(&self->filters,self->filters.mAnchor.mpNodeParent);
    SetVtable(self->vtable,0x00fb84d0);
    return result;
}
Record* __fastcall Construct(Record* self,void*,void* owner,void* stream,const Key* key,uint32_t open,uint32_t owned) {
    SetVtable(self->vtable,0x00fb21e8);
    std::atomic_ref<int32_t>(self->refs).exchange(0);
    self->owner=owner;
    self->stream=stream;
    SetVtable(self->vtable,0x00fb2f30);
    self->key=*key;
    self->open=uint8_t(open);
    self->owned=uint8_t(owned);
    self->pending=1;
    return self;
}
}
