#include "ts3/anim/m05_M73.h"
#include <cstring>
#include <cctype>
#if defined(_M_IX86)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#endif
namespace ts3::anim::m73 {
uintptr_t Append(Records* s,const Record* r) {
    auto p=s->end;
    if(reinterpret_cast<uintptr_t>(p)>=reinterpret_cast<uintptr_t>(s->capacity))
        return CallGameMethod<uintptr_t,0x005749F0>(s,p,r);
    s->end=reinterpret_cast<Record*>(reinterpret_cast<uintptr_t>(p)+sizeof(Record));
    if(p) {
        std::memcpy(p->vectors,r->vectors,32);
        std::memcpy(p->words,r->words,16);
        // FLD/FSTP quiets a signaling NaN; it is not a whole-record memcpy.
        volatile double weight=r->weight; p->weight=static_cast<float>(weight);
    }
    return reinterpret_cast<uintptr_t>(p);
}
void AddWeighted(Records* s,const Record* r) {
    if(r->weight>0.0f) { Append(s,r); s->total=s->total+r->weight; }
}
Pose* Delete(Pose* s,uint32_t flags) {
    CallGameMethod<void,0x00574BE0>(s);
    if(flags&1) {
#if defined(_M_IX86)
        auto lock=reinterpret_cast<CRITICAL_SECTION*>(0x011CD8D8);
        auto head=reinterpret_cast<void**>(0x011CD8C4);
        EnterCriticalSection(lock); s->vtable=static_cast<void**>(*head); *head=s; LeaveCriticalSection(lock);
#else
        AbortNotRewritten("scratch pool global");
#endif
    }
    return s;
}
Pose* Construct(Pose* s,RigResource* rig) {
#if defined(_M_IX86)
    s->vtable=reinterpret_cast<void**>(0x00FC2D4C);
#else
    s->vtable=nullptr;
#endif
    s->refs=0;
    s->translation={{0,0,0,0}}; s->rotation={{0,0,0,1}};
    s->rig=rig;
    if(rig) CallGameVirtual<void,0>(rig);
    s->srt=nullptr; s->auxiliary38=nullptr; s->auxiliary3c=nullptr;
    s->auxiliary40=nullptr; s->workspace=nullptr; s->weights=nullptr;
    s->flags=0; s->boneCount=0;
    for(auto& v:s->ones) v=1.0f;
    s->one=1.0f;
    for(auto& w:s->work) {
        w.begin=w.end=w.storage; w.capacity=w.storage+sizeof(w.storage);
        w.scalar=0.0f;
    }
    s->finalRotation={{0,0,0,1}}; s->finalTranslation={{0,0,0,0}};
    s->boneCount=static_cast<uint16_t>((reinterpret_cast<uintptr_t>(rig->skeleton.end)-reinterpret_cast<uintptr_t>(rig->skeleton.begin))/sizeof(Joint));
    if(rig) CallGameVirtual<void,1>(rig);
    return s;
}
void SetWeights(Pose* s,void* source) {
    if(source) { CallGameMethod<void,0x00574CD0>(s); CallGameMethod<void,0x00810DF0>(s->weights,source); }
}
uint32_t Hash(void*,const char* text) {
    uint32_t result=0x811c9dc5;
    for(auto p=reinterpret_cast<const unsigned char*>(text);*p;++p)
        result=result*0x01000193u ^ static_cast<uint32_t>(std::tolower(*p));
    return result;
}
}
