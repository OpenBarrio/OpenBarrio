#include "ts3/anim/m05_M85.h"
#include <cstring>
namespace ts3::anim::m85 {
uintptr_t Update(RigResource* self) {
    auto* begin=static_cast<Chain*>(self->ikBegin);
    const auto bytes=reinterpret_cast<intptr_t>(self->ikEnd)-reinterpret_cast<intptr_t>(begin);
    const int32_t count=static_cast<int32_t>(bytes/static_cast<intptr_t>(sizeof(Chain)));
    uintptr_t result=static_cast<uint32_t>(count);
    for(int32_t i=0;i<count;++i) {
        auto& c=static_cast<Chain*>(self->ikBegin)[i];
        if(c.length) continue;
        result=reinterpret_cast<uintptr_t>(c.begin);
        if(c.begin==c.end) continue;
        const int32_t root=*c.begin;
        result=CallGameMethod<uintptr_t,0x00575650>(&self->skeleton);
        int32_t joint=static_cast<int32_t>(result)-1;
        while(joint>root) {
            result=CallGameMethod<uintptr_t,0x005756F0>(&self->skeleton,joint,root);
            if(static_cast<uint8_t>(result)) { c.length=joint-root; break; }
            --joint;
        }
    }
    return result;
}
RigResource* Construct(RigResource* self) {
#if defined(_M_IX86)
    self->base.vtable=reinterpret_cast<void*>(0x00FB2C04);
    self->base.atomicRefs=0;
    std::memset(self->base.opaque08,0,sizeof self->base.opaque08);
    self->base.vtable=reinterpret_cast<void*>(0x00FC30F0);
    CallGameMethod<void,0x00575F50>(&self->skeleton);
    self->base.vtable=reinterpret_cast<void*>(0x00FC3310);
    self->skeleton.vtable=reinterpret_cast<void*>(0x00FC330C);
    self->localBegin=self->localEnd=self->localCapacity=nullptr;
    self->localDescriptor=reinterpret_cast<void*>(0x00FC32E8);
    self->worldBegin=self->worldEnd=self->worldCapacity=nullptr;
    self->worldDescriptor=reinterpret_cast<void*>(0x00FC32C4);
    self->ikBegin=self->ikEnd=self->ikCapacity=nullptr;
    self->ikDescriptor=reinterpret_cast<void*>(0x00FC32A4);
    std::memset(self->opaquebc,0xff,sizeof self->opaquebc);
    self->vertexChannelCount=0;
    return self;
#else
    NotRewritten("M85 original skeleton constructor and vtables");
#endif
}
uintptr_t Destroy(RigResource* self) {
#if defined(_M_IX86)
    self->base.vtable=reinterpret_cast<void*>(0x00FC3310);
    self->skeleton.vtable=reinterpret_cast<void*>(0x00FC330C);
#else
    NotRewritten("M85 original destructor vtables");
#endif
    CallGameMethod<void,0x0057B830>(&self->ikBegin);
    if(self->worldBegin) CallGameFunction<void,0x004E3F70>(self->worldBegin);
    if(self->localBegin) CallGameFunction<void,0x004E3F70>(self->localBegin);
    CallGameMethod<void,0x00575FB0>(&self->skeleton);
    return CallGameMethod<uintptr_t,0x00731CF0>(self);
}
RigResource* Delete(RigResource* self,uint32_t flags) {
    Destroy(self);
    if(flags&1) CallGameFunction<void,0x004E3F70>(self);
    return self;
}
RigResource* DeleteSecondary(Skeleton* self,uint32_t flags) {
    return Delete(reinterpret_cast<RigResource*>(reinterpret_cast<unsigned char*>(self)-offsetof(RigResource,skeleton)),flags);
}
RigResource* Create() {
#if defined(_M_IX86)
    auto* self=CallGameFunction<RigResource*,0x004E3F90>(0xf0,reinterpret_cast<const char*>(0x00FC332C),0,0,0,0);
    return self?Construct(self):nullptr;
#else
    NotRewritten("M85 original allocation descriptor");
#endif
}
uintptr_t __fastcall HookUpdate(RigResource* self,void*) { return Update(self); }
RigResource* __fastcall HookConstruct(RigResource* self,void*) { return Construct(self); }
RigResource* __fastcall HookDeleteSecondary(Skeleton* self,void*,uint32_t flags) { return DeleteSecondary(self,flags); }
uintptr_t __fastcall HookDestroy(RigResource* self,void*) { return Destroy(self); }
RigResource* __fastcall HookDelete(RigResource* self,void*,uint32_t flags) { return Delete(self,flags); }
}
