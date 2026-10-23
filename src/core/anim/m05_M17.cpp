#include "ts3/anim/m05_M17.h"
#include <cstring>
namespace ts3::anim::m17 {
namespace {
void** Table(uint32_t address) {
#if defined(_M_IX86)
    return reinterpret_cast<void**>(address);
#else
    static void* tables[4][11]{};
    return tables[address==0xfc1578?0:address==0xfc15a4?1:address==0xfc1618?2:3];
#endif
}
}
Trans* ConstructTrans(Trans* n,void* host) {
    b01::Construct(&n->base,host); n->base.vtable=Table(0xfc1578);
    n->begin=n->end=n->capacity=nullptr; n->descriptor=Table(0xfc1554); return n;
}
Trans* DestroyTrans(Trans* n,uint32_t flags) {
    n->base.vtable=Table(0xfc1578); n->end=n->begin;
    if(n->begin) CallGameFunction<void,0x004e3f70>(n->begin);
    b01::Reset(&n->base);
    if(flags&1u) CallGameFunction<void,0x004e3f70>(n);
    return n;
}
Constant* ConstructConstant(Constant* n,void* host) {
    b01::Construct(&n->base,host); n->base.vtable=Table(0xfc15a4);
    std::memset(&n->translation,0,16); float q[4]={0,0,0,1}; std::memcpy(&n->rotation,q,16); return n;
}
// Adapted from verified m03_B04::DestroyFactory: only table, base destructor and free target differ.
Constant* DestroyConstant(Constant* n,uint32_t flags) {
    n->base.vtable=Table(0xfc15a4); b01::Reset(&n->base);
    if(flags&1u) CallGameFunction<void,0x004e3f70>(n);
    return n;
}
Blend* ConstructBlend(Blend* n,void* host) {
    b01::Construct(&n->base,host); n->base.vtable=Table(0xfc1618);
    n->a=n->b=n->c=n->d=n->e=n->f=nullptr;
    n->flag54=n->flag55=0; n->f38=n->f3c=n->f48=0; n->f40=n->f58=1;
    uint32_t bits=0xbeaa7efa; std::memcpy(&n->f44,&bits,4); return n;
}
void Evaluate(Trans* n,Context* context,RigPose* pose) {
    auto sentinel=&n->base.children;
    if(sentinel->previous!=sentinel) {
        auto link=sentinel->next;
        auto child=link?reinterpret_cast<b01::Node*>(reinterpret_cast<unsigned char*>(link)-offsetof(b01::Node,link)):nullptr;
        CallGameVirtual<void,2>(child,context,pose);
    } else CallGameMethod<void,0x005743a0>(pose);
    if(!(context->flags&2)) return;
    auto srt=CallGameMethod<SrtValue*,0x00574190>(pose);
    auto weights=CallGameMethod<float*,0x005741b0>(pose,0.0f);
    for(auto p=n->begin;p!=n->end;++p) {
        if(p->bone<0 || static_cast<uint32_t>(p->bone)>=pose->boneCount) continue;
        std::memcpy(&srt[p->bone],&p->translation,32);
        weights[p->bone]=1.0f;
    }
}
}

#if defined(_M_IX86)
namespace ts3::anim::m17::hooks {
void __fastcall Evaluate(Trans* n,void*,Context* c,RigPose* p) { return m17::Evaluate(n,c,p); }
Trans* __fastcall ConstructTrans(Trans* n,void*,void* h) { return m17::ConstructTrans(n,h); }
Trans* __fastcall DestroyTrans(Trans* n,void*,uint32_t f) { return m17::DestroyTrans(n,f); }
Constant* __fastcall ConstructConstant(Constant* n,void*,void* h) { return m17::ConstructConstant(n,h); }
Constant* __fastcall DestroyConstant(Constant* n,void*,uint32_t f) { return m17::DestroyConstant(n,f); }
Blend* __fastcall ConstructBlend(Blend* n,void*,void* h) { return m17::ConstructBlend(n,h); }
}
#endif
