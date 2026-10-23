#include "ts3/anim/m05_M96.h"
#include "ts3/anim/m05_layouts.h"
#if defined(_M_IX86)
#include <intrin.h>
#endif
namespace ts3::anim::m96 {
void SetIdentifier(Entity* self,uint32_t low,uint32_t high) {
    if (low != self->idLow || high != self->idHigh) {
        CallGameMethod<void,0x00581D80>(self);
        self->idLow=low;
        self->idHigh=high;
    }
}
void CopyIdentifier(Entity* self,const Identifier* id) {
    SetIdentifier(self,id ? id->low : 0,id ? id->high : 0);
}
namespace {
void ReadWord(Stream* stream,uint32_t& word) {
    void* operation=CallGameFunction<void*,0x004DD700>(stream->buffer,&word,stream->mode);
    CallGameMethod<void,0x007D77C0>(stream,operation);
}
Entity* AllocateEntity() {
#if defined(_M_IX86)
    auto* e=CallGameFunction<Entity*,0x004E3F90>(0x20,reinterpret_cast<const char*>(0x00FCB86C),0,0,0,0);
    if (!e) return nullptr;
    e->vtable=reinterpret_cast<void**>(0x00FF39F8);
    _InterlockedExchange(reinterpret_cast<volatile long*>(&e->refs),0);
    e->secondary=reinterpret_cast<void**>(0x00FA1B84);
    e->vtable=reinterpret_cast<void**>(0x00FCB7EC);
    e->secondary=reinterpret_cast<void**>(0x00FCB7DC);
    e->flags=0x40000001;
    e->idLow=0;
    e->idHigh=0;
    e->cached=nullptr;
    return e;
#else
    NotRewritten("m96 scene entity allocator");
#endif
}
}
bool LoadEntities(void* self,Stream* stream,void* arg) {
    uint32_t count=0;
    ReadWord(stream,count);
    bool success=true;
    for (uint32_t i=0;i<count;++i) {
        uint32_t name=0,present=0;
        ReadWord(stream,name);
        ReadWord(stream,present);
        Entity* entity=present ? AllocateEntity() : nullptr;
        // EA allocates before noticing a failure from the preceding insertion.
        if (!success) return false;
        if (entity && !CallGameMethod<bool,0x00581AD0>(entity,stream,arg)) return false;
        success=CallGameMethod<bool,0x00582090>(self,name,entity);
    }
    return success;
}
void* ResolveRig(Handle* self) {
    void* result=nullptr;
    if (self->context->provider) result=CallGameVirtual<void*,9>(self->context->provider);
    if (result) return result;
    if (self->context->fallback) return CallGameMethod<void*,0x005775D0>(self->context->fallback);
    return nullptr;
}
uint32_t ResolveJoint(void* object,uint32_t key,uint32_t* status) {
    *status=0;
    void* found=nullptr;
#if defined(_M_IX86)
    if (object) found=CallGameMethod<void*,0x00935770>(object,*reinterpret_cast<void**>(0x011DC484));
#else
    if (object) NotRewritten("m96 object interface lookup");
#endif
    if (!found) { *status=1; return key; }
    auto* iface=static_cast<unsigned char*>(found)+offsetof(Entity,secondary);
    void* rig=CallGameVirtual<void*,8>(iface);
    if (!rig) { *status=2; return key; }
    // The resource base occupies 0x28 bytes on x86; its native scaffold prefix widens pointers.
    auto* skeleton=static_cast<unsigned char*>(rig)+sizeof(ResourcePrefix);
    int32_t index=CallGameMethod<int32_t,0x00575B00>(skeleton,key);
    if (index<0) { *status=4; return key; }
    int32_t parent=CallGameMethod<int32_t,0x005756D0>(skeleton,index);
    uint32_t result=CallGameMethod<uint32_t,0x00575690>(skeleton,parent);
    *status=result ? 0 : 5;
    return result;
}
void* FindEntity(Handle* self) {
    if (!self->context->nameSpace) return nullptr;
    const char* name=nullptr;
    if (self->context->fallback) name=CallGameMethod<const char*,0x00F20360>(self->context->fallback);
#if defined(_M_IX86)
    if (!name || !*name) name=reinterpret_cast<const char*>(0x00FCB9D0);
#else
    if (!name || !*name) name="unknown_namespace";
#endif
    // Capture the namespace before the interning call (which may mutate the context).
    void* nameSpace=self->context->nameSpace;
    uint32_t hash=CallGameFunction<uint32_t,0x007CFE70>(name);
    return CallGameMethod<void*,0x00581810>(nameSpace,hash);
}
}
