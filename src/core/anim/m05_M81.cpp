#include "ts3/anim/m05_M81.h"
#include "ts3/layout.h"
#include <cstring>
#include <initializer_list>
namespace ts3::anim::m81 {
uint32_t Resize(JointRange* s,uint32_t n) {
    auto count=static_cast<uint32_t>((s->end-s->begin)/0x70);
    if(n>count) {
        struct Temporary { unsigned char* begin; unsigned char* end; unsigned char* capacity; unsigned char storage[0x60]; } t;
        t.begin=t.end=t.storage; t.capacity=t.storage+0x60; t.storage[0]=0;
        return CallGameMethod<uint32_t,0x5783f0>(s,s->end,n-count,&t);
    }
    auto newEnd=s->begin+n*0x70;
    CallGameFunction<void,0x568c00>(s->end,s->end,newEnd);
    auto delta=static_cast<int32_t>(newEnd-s->end);
    s->end+=delta;
    return static_cast<uint32_t>(delta);
}
bool Bind(ClipIkInfo* s,uint32_t a,uint32_t b,void* c,void* d) {
    return CallGameMethod<bool,0x578650>(s,a,b,0u,c,d);
}
void Destruct(ClipIkInfo* s) {
#if defined(_M_IX86)
    s->vtable=reinterpret_cast<void*>(0xfc2f80);
#else
    AbortNotRewritten("m81 IK vtable");
#endif
    if(s->hashBegin) CallGameFunction<void,0x4e3f70>(s->hashBegin);
    if(s->namespaceBegin) CallGameFunction<void,0x4e3f70>(s->namespaceBegin);
    if(s->slotBegin) CallGameFunction<void,0x4e3f70>(s->slotBegin);
}
void Exchange(Range* a,Range* b) {
    struct Temporary { Range range; void* descriptor; unsigned char storage[0xaa0]; } t;
    t.range.begin=t.range.end=t.storage; t.range.capacity=t.storage+sizeof t.storage;
    // The original passes an uninitialized allocator tag. The callee's inline-capacity
    // paths do not inspect it; use null rather than introducing a C++ uninitialized read.
    CallGameMethod<void,0x569840>(&t.range,a->begin,a->end,static_cast<void*>(nullptr));
    if(a!=b) {
        a->end=a->begin;
        CallGameMethod<void,0x569840>(a,b->begin,b->end,static_cast<void*>(nullptr));
    }
    b->end=b->begin;
    CallGameMethod<void,0x569840>(b,t.range.begin,t.range.end,static_cast<void*>(nullptr));
}
bool ResolveGroups(Group* a,Group* b) {
    Exchange(&a->records,&b->records);
    uint8_t ok=1;
    for(auto group: {a,b}) {
        uint32_t i=0;
        while(i<static_cast<uint32_t>((group->records.end-group->records.begin)/0x110)) {
            auto result=CallGameMethod<uint8_t,0x577ea0>(group->records.begin+i*0x110,&ok);
            ok=result && ok;
            ++i;
        }
    }
    return ok!=0;
}
bool Resolve(ClipIkInfo* s) {
    uint32_t count;
    std::memcpy(&count,s->opaquec0+8,4);
    if(!count || !(s->valueCC&1)) return false;
    if(s->valueCC&8) return true;
    s->valueCC|=8;
    auto groups=static_cast<Group*>(s->slotBegin);
    bool first=ResolveGroups(groups,groups+1);
    bool second=ResolveGroups(groups+2,groups+3);
    return second && first;
}
}
