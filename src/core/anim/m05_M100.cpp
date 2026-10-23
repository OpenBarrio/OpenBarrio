#include "ts3/anim/m05_M100.h"
#include <utility>
#include <cstring>
#include <xmmintrin.h>
namespace ts3::anim::m100 {
namespace {
void* Address(uint32_t value) {
#if defined(_M_IX86)
    return reinterpret_cast<void*>(uintptr_t(value));
#else
    (void)value; return nullptr;
#endif
}
void Free(void* p) { CallGameFunction<void,0x004e3f70>(p); }
void Release(Controller* p) {
    if(p && --p->refs==0) {
        p->refs=1;
        CallGameVirtual<void,0>(p,1u);
    }
}
}
void Remove(ListLink* list,const TrackKey* key) {
    auto* next=list->next;
    while(next!=list) {
        auto* current=reinterpret_cast<TrackNode*>(next);
        next=next->next;
        if(current->payload!=key->payload) continue;
        // EA selects next->previous *after* advancing the iterator.
        auto* node=reinterpret_cast<TrackNode*>(next->previous);
        node->links.previous->next=node->links.next;
        node->links.next->previous=node->links.previous;
        Release(node->controller);
        Free(node);
    }
}
uint32_t Update(Manager* p,float time,uint8_t* first,uint8_t* second) {
    struct Pending { ListLink links; void* descriptor; } pending;
    pending.links.next=pending.links.previous=&pending.links;
    pending.descriptor=Address(0x00fcba88);
    uint32_t count=0;
    for(auto* link=p->tracks.next;link!=&p->tracks;link=link->next) {
        auto* node=reinterpret_cast<TrackNode*>(link);
        if(node->controller) {
            ++count;
            if(CallGameMethod<uint8_t,0x00565f40>(node->controller)) {
                CallGameVirtual<void,2>(node->controller,time);
                *first|=CallGameVirtual<uint8_t,11>(node->controller);
                *second|=CallGameVirtual<uint8_t,12>(node->controller);
                continue;
            }
        }
        auto* copy=CallGameFunction<TrackNode*,0x004e3f90>(sizeof(TrackNode),pending.descriptor,0,0,Address(0x00f98980),0xe7);
        copy->controller=node->controller;
        if(copy->controller) ++copy->controller->refs;
        copy->payload=node->payload;
        copy->links.next=&pending.links;
        copy->links.previous=pending.links.previous;
        pending.links.previous->next=&copy->links;
        pending.links.previous=&copy->links;
    }
    for(auto* link=pending.links.next;link!=&pending.links;link=link->next)
        Remove(&p->tracks,reinterpret_cast<const TrackKey*>(&reinterpret_cast<TrackNode*>(link)->controller));
    CallGameMethod<void,0x0081f320>(&pending.links);
    return count;
}
Manager* Construct(Manager* p,void* host) {
    p->vtable=Address(0x00fcba70);
    p->descriptor=Address(0x00fcbab8);
    p->tracks.next=p->tracks.previous=&p->tracks;
    p->flag=0;
    p->host=host;
    return p;
}
Manager* Destroy(Manager* p,uint32_t flags) {
    p->vtable=Address(0x00fcba70);
    CallGameMethod<void,0x005827b0>(p);
    CallGameMethod<void,0x0081f320>(&p->tracks);
    if(flags&1) Free(p);
    return p;
}
b20::Pose* Mirror(b20::Pose* p) {
    auto* skeleton=&p->rig->skeleton;
    auto* joints=CallGameMethod<Joint*,0x009f0de0>(skeleton);
    auto* local=b20::Local(p);
    const uint32_t count=p->count;
    auto* service=CallGameFunction<void*,0x005eb7c0>();
    uint32_t mode=service?CallGameMethod<uint32_t,0x005ec9e0>(service,reinterpret_cast<unsigned char*>(p->rig)+8)&0xcf00:0;
    bool special=mode!=0 && mode!=0x100;
    int32_t specialIndex=CallGameMethod<int32_t,0x00575e30>(skeleton,Address(0x00fc1898));
    for(uint32_t i=0;i<count;++i) {
        auto& joint=joints[i];
        if(joint.index54<0) continue;
        auto& value=local[i];
        if(joint.value60&4) std::swap(value.translation,local[joint.index54].translation);
        if(joint.value60&8) std::swap(value.rotation,local[joint.index54].rotation);
        if(joint.value60&16) std::swap(value.scale,local[joint.index54].scale);
        if(joint.value60&1)
            CallGameFunction<void,0x00583eb0>(&value.translation,&value.translation,uint32_t(i!=0 && int32_t(i)!=specialIndex && special));
        if(joint.value60&2) {
            if(special && int32_t(i)==specialIndex) {
                auto q=_mm_load_ps(value.rotation.lane);
                q=_mm_mul_ps(q,_mm_set1_ps(-1.0f));
                _mm_store_ps(value.rotation.lane,_mm_shuffle_ps(q,q,_MM_SHUFFLE(1,0,3,2)));
            } else {
                CallGameFunction<void,0x00583f50>(&value.rotation,&value.rotation,uint32_t(i!=0 && special));
            }
        }
    }
    p->flags &= 0xfffe;
    return p;
}
bool Bind(const SampledClipView* clip,RigResource* rig,int32_t* out) {
    auto* skeleton=&rig->skeleton;
    // This otherwise-unused call is observable (and intentionally unconditional).
    CallGameMethod<Joint*,0x009f0de0>(skeleton);
    auto* records=clip->relative20==-1?nullptr:reinterpret_cast<const unsigned char*>(clip)+clip->relative20;
    for(uint32_t i=0;i<clip->channelCount;++i) {
        uint32_t hash; std::memcpy(&hash,records+12*i+4,4);
        out[i]=hash==0x2af0a8ae?-1:CallGameMethod<int32_t,0x00575b00>(skeleton,hash);
    }
    return true;
}
#if defined(_M_IX86)
void __fastcall HookRemove(ListLink* p,void*,const TrackKey* k) { Remove(p,k); }
uint32_t __fastcall HookUpdate(Manager* p,void*,float t,uint8_t* a,uint8_t* b) { return Update(p,t,a,b); }
Manager* __fastcall HookConstruct(Manager* p,void*,void* h) { return Construct(p,h); }
Manager* __fastcall HookDestroy(Manager* p,void*,uint32_t f) { return Destroy(p,f); }
#endif
}
