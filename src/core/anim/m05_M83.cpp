#include "ts3/anim/m05_M83.h"
#include "ts3/layout.h"
#include <cstring>
#include <atomic>
namespace ts3::anim::m83_detail {
using namespace ts3;
namespace {
void* Address(uint32_t a) {
#if defined(_M_IX86)
    return reinterpret_cast<void*>(a);
#else
    (void)a; AbortNotRewritten("M83 game static data"); return nullptr;
#endif
}
}
uint64_t Identifier(const char16_t* text,uint32_t seed) {
    if(!text) return 0;
    char converted[256];
    CallGameFunction<void,0x004F4260>(converted,256u,text,-1);
    return CallGameFunction<uint64_t,0x0057A400>(converted,seed);
}
void* Factory(void* self) {
    CallGameMethod<void,0x0072F190>(self,0x6b20c4f3u,0x6b20c4f3u);
    *static_cast<void**>(self)=Address(0xFC3058);
    return self;
}
void* DeleteFactory(void* self,uint32_t flags) {
    *static_cast<void**>(self)=Address(0xFC3058);
    CallGameMethod<void,0x0072F180>(self);
    if(flags&1u) CallGameFunction<void,0x004E4230>(self);
    return self;
}
Resource* Construct(Resource* self) {
    auto& s=self->clip;
    s.base.vtable=Address(0xFB2C04);
    reinterpret_cast<std::atomic<int32_t>*>(&s.base.atomicRefs)->exchange(0);
    std::memset(s.base.opaque08,0,sizeof(s.base.opaque08));
    s.base.vtable=Address(0xFC3130);
    s.loaded=0;
    // Native pointer-width map view of the scaffold's otherwise opaque region.
    auto* m=&s.map;
    m->left=m; m->right=m; m->count=0; m->color=0;
    m->extra=nullptr; m->descriptor=Address(0xFC3108);
    s.ik=nullptr; s.view=nullptr; s.rawBuffer=nullptr;
    s.eventBegin=nullptr; s.eventEnd=nullptr; s.eventCapacity=nullptr;
    s.eventDescriptor=Address(0xFC30D8); self->allocator=-2;
    return self;
}
uintptr_t Load(Clip* s,void* stream) {
    s->view=nullptr; s->loaded=0;
    CallGameFunction<void,0x004E3F70>(s->rawBuffer);
    s->rawBuffer=nullptr;
    if(s->ik) CallGameVirtual<void,0>(s->ik,1u);
    s->ik=nullptr;
    uint32_t size=CallGameVirtual<uint32_t,7>(stream);
    if(size>0) s->rawBuffer=CallGameFunction<void*,0x004E3F90>(size,Address(0xFC30C0),0u,0u,0u,0u);
    if(s->rawBuffer) {
        CallGameVirtual<void,12>(stream,s->rawBuffer,size);
        CallGameMethod<void,0x00731CC0>(s,size);
    }
    auto* ik=CallGameFunction<ClipIkInfo*,0x004E3F90>(0xe0u,Address(0xFC309C),0u,0u,0u,0u);
    s->ik=ik?CallGameMethod<ClipIkInfo*,0x00578AB0>(ik):nullptr;
    s->view=nullptr;
    uintptr_t result=reinterpret_cast<uintptr_t>(s->rawBuffer);
    if(!s->rawBuffer) return result;
    result=CallGameFunction<uintptr_t,0x00579690>(s->rawBuffer);
    if(!(result&255u)) return result;
    s->view=s->rawBuffer;
    CallGameMethod<void,0x005790A0>(s->ik,s->rawBuffer,10u);
    // The validated on-disk header contains relocated native pointers on x86.
    struct Events { uint32_t unknown; void* type; uint32_t count; void* cursor; void* strings; };
    auto* events=*reinterpret_cast<Events**>(static_cast<unsigned char*>(s->view)+0x18);
    result=reinterpret_cast<uintptr_t>(s->view);
    if(events) {
        auto* cursor=events->cursor; auto* strings=events->strings;
        for(uint32_t i=0;i<events->count;++i) {
            auto* event=CallGameFunction<void*,0x0057F670>(events->type,s,&strings,&cursor,0u);
            if(!event) break;
            CallGameMethod<void,0x00580000>(&s->eventBegin,event);
        }
        result=CallGameMethod<uintptr_t,0x0057FEA0>(&s->eventBegin);
    }
    s->loaded=1;
    return result;
}
bool Open(void* source,Clip* s) {
    void* stream=CallGameVirtual<void*,6>(source);
    if(stream) CallGameVirtual<void,1>(stream);
    Load(s,stream);
    bool loaded=s->loaded!=0;
    if(stream) CallGameVirtual<void,2>(stream);
    return loaded;
}
}
