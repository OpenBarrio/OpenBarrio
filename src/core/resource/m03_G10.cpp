#include "ts3/resource/m03_G10.h"
namespace ts3::resource::m03::g10 {
#if defined(_M_IX86)
namespace {
void* AllocateStream(const char* name) {
    void* mem=CallGameFunction<void*,0x004E3F90>(44u,name,0u,0u,0u,0u);
    return mem?CallGameMethod<void*,0x004DF0C0>(mem,0u,0u,name):nullptr;
}
void* AllocateRecord(void* stream,const void* key,uint32_t a,uint32_t b,const char* name) {
    void* mem=CallGameFunction<void*,0x004E41F0>(40u,name,0u,0u,0u,0u);
    return mem?CallGameMethod<void*,0x004B3650>(mem,0u,stream,key,a,b):nullptr;
}
void DropStream(void* stream) { CallGameVirtual<void,0>(stream,1u); }
void Release(void* obj) { CallGameVirtual<void,1>(obj); }
void* Manager() { return CallGameFunction<void*,0x004AFD20>(); }
void* Factory(void* manager,uint32_t type) { return CallGameVirtual<void*,11>(manager,type,-1); }
}
#endif
uint8_t __fastcall Materialize(Context* self,void*,uint32_t index) {
#if defined(_M_IX86)
    if(index>=self->count) return 0;
    uintptr_t& slot=self->groups[0].children[index];
    if(!(slot&1u)) return 1;
    slot-=1u;
    auto* child=reinterpret_cast<a06::LoadPrefix*>(slot);
    void* manager=Manager();
    void* stream=AllocateStream("ResourceLoad/AsyncFactory/ChildStream");
    if(!stream) return 0;
    a06::Record16 zero{};
    void* record=AllocateRecord(stream,&zero,0,0,"ResourceLoad/AsyncFactory/ChildRecord");
    if(!record) { DropStream(stream); return 0; }
    CallGameVirtual<void,1>(record);
    uint32_t source=0,size=0;
    a06::GetPair(self,index,&source,&size);
    auto* key=&self->groups[0].records[index]; // EA uses group zero, not the split key accessor.
    CallGameVirtual<void,5>(record,key);
    CallGameMethod<void,0x004DEC90>(stream,reinterpret_cast<void*>(source),size,1u,0u,0u);
    CallGameVirtual<void,10>(stream,0u,0u);
    void* factory=Factory(manager,key->words[2]);
    child->key=*key;
    bool ok=false;
    if(factory) {
        if(CallGameVirtual<uint32_t,6>(factory)==0x01cce04du)
            ok=CallGameVirtual<uint8_t,14>(factory,child,stream,self,self->flags&0xffu)!=0;
        else
            ok=CallGameVirtual<uint8_t,9>(factory,record,child,0u,key->words[2])!=0;
    }
    CallGameVirtual<void,2>(record);
    DropStream(stream);
    return ok?1:0;
#else
    (void)self; (void)index; NotRewritten("m03_G10 Materialize original stream/allocator ABI");
#endif
}
uint8_t __stdcall Resolve(void*,Context* self) {
#if defined(_M_IX86)
    const uint8_t disabled=uint8_t(self->flags>>8);
    void* manager=Manager();
    const uint32_t count=self->count;
    for(uint32_t i=0;i<count;++i) {
        auto* key=i<self->count
            ? (i<self->groups[0].count?&self->groups[0].records[i]:&self->groups[1].records[i-self->groups[0].count])
            : a06::GetRecord(self,0xffffffffu,0);
        if(!key->words[2]) continue;
        void* factory=Factory(manager,key->words[2]);
        if(!factory) return 0;
        if(i<self->count && (self->groups[0].children[i]&~uintptr_t(1))) continue;
        void* resource=nullptr;
        void* provider=CallGameVirtual<void*,22>(manager,key);
        uint32_t owned=0;
        if(provider && CallGameVirtual<uint8_t,6>(provider,key,&resource) && !disabled) owned=1;
        if(!resource && !CallGameVirtual<uint8_t,7>(factory,0u,&resource,0u,key->words[2])) {
            if(resource) Release(resource);
            return 0;
        }
        a06::SetChild(self,i,resource,owned);
        if(resource) Release(resource);
    }
    for(int32_t i=int32_t(self->groups[2].count-1u);i>=0;--i) {
        auto* key=a06::GetRecord(self,uint32_t(i),2);
        void* provider=CallGameVirtual<void*,22>(manager,key);
        void* resource=nullptr;
        if(!provider) return 0;
        CallGameVirtual<void,6>(provider,key,&resource);
        if(!resource) return 0;
        CallGameVirtual<void,0>(resource);
        auto& slot=self->groups[2].children[i];
        if(slot) Release(reinterpret_cast<void*>(slot));
        slot=reinterpret_cast<uintptr_t>(resource);
        Release(resource);
    }
    const uint32_t primary=self->groups[0].count;
    for(int32_t i=int32_t(primary-1u);i>=0;--i) {
        const uint32_t handle=uint32_t(i)+1u, type=handle>>28, index=(handle&0x0fffffffu)-1u;
        if(type>=4u || index>=self->groups[type].count) return 0;
        uintptr_t child=self->groups[type].children[index];
        if(child&1u) {
            const uint32_t flat=type==1u?index+self->groups[0].count:index;
            if(!Materialize(self,nullptr,flat)) return 0;
            child=self->groups[type].children[index];
        }
        if(!child) return 0;
    }
    if(!uint8_t(self->flags>>8)) {
        for(uint32_t i=0;i<primary;++i) {
            void* child=i<self->count?reinterpret_cast<void*>(self->groups[0].children[i]&~uintptr_t(1)):nullptr;
            CallGameVirtual<void,23>(manager,child,1u);
        }
    }
    return 1;
#else
    (void)self; NotRewritten("m03_G10 Resolve original factory ABI");
#endif
}
uint8_t __cdecl CopyRecord(void** slot) {
#if defined(_M_IX86)
    void* stream=AllocateStream("ResourceLoad/AsyncLoader/MemoryStream");
    if(!stream) return 0;
    CallGameVirtual<void,1>(stream);
    CallGameMethod<void,0x004DEAC0>(stream,1u,1.0f);
    void* source=CallGameVirtual<void*,6>(*slot);
    const uint32_t size=CallGameVirtual<uint32_t,7>(source);
    if(!CallGameVirtual<uint8_t,8>(stream,size)) { DropStream(stream); return 0; }
    CallGameMethod<void,0x004DEAC0>(stream,1u,0.0f);
    void* data=nullptr;
    void* buffer=reinterpret_cast<void**>(stream)[1];
    if(buffer) data=reinterpret_cast<void**>(buffer)[2];
    if(!CallGameVirtual<uint32_t,12>(source,data,size)) { DropStream(stream); return 0; }
    // AllocateRecord must not fetch the old key before allocation succeeds.
    void* mem=CallGameFunction<void*,0x004E41F0>(40u,"ResourceLoad/AsyncLoader/RecordGeneric",0u,0u,0u,0u);
    if(!mem) { DropStream(stream); return 0; }
    const void* key=CallGameVirtual<const void*,4>(*slot);
    void* record=CallGameMethod<void*,0x004B3650>(mem,0u,stream,key,1u,1u);
    if(!record) { DropStream(stream); return 0; }
    CallGameVirtual<void,1>(record);
    CallGameVirtual<void,8>(*slot);
    void* old=*slot;
    *slot=record;
    if(old) CallGameVirtual<void,2>(old);
    return 1;
#else
    (void)slot; NotRewritten("m03_G10 CopyRecord original memory stream ABI");
#endif
}
}
