#include "ts3/anim/m05_M39.h"
namespace ts3::anim::m39 {
namespace {
void* Address(uint32_t a) {
#if defined(_M_IX86)
    return reinterpret_cast<void*>(uintptr_t(a));
#else
    (void)a; NotRewritten("M39 game data");
#endif
}
void Tables(BoneController* p) {
    p->base.vtable=Address(0x00fc1df0); p->base.secondary8=Address(0x00fc1de8); p->base.secondaryC=Address(0x00fc1de4);
}
float Fallback() { return *static_cast<float*>(Address(0x0114f594)); }
}
BoneController* Construct(BoneController* p,void* host) {
    b08::Construct(&p->base,host); Tables(p);
    p->map.maxLoad=1.0f; p->map.growth=2.0f; p->map.allocator=Address(0x00fc1e34);
    p->map.size=0; p->map.hash=0; p->map.bucketCount=1; p->map.buckets=static_cast<BoneEntry**>(Address(0x0114c900));
    return p;
}
BoneController* Destroy(BoneController* p,uint32_t flags) {
    Tables(p);
    CallGameMethod<void,0x0056c350>(&p->map,p->map.buckets,p->map.bucketCount);
    p->map.size=0;
    if(p->map.bucketCount>1) CallGameFunction<void,0x004e3f70>(p->map.buckets);
    CallGameMethod<void,0x00565b40>(&p->base);
    if(flags&1) CallGameFunction<void,0x004e3f70>(p);
    return p;
}
intptr_t SetTime(BoneController* p,uint32_t key,float time) {
    auto* rig=CallGameVirtual<void*,8>(p->base.hostSlot);
    if(!rig) return 0;
    auto* skeleton=reinterpret_cast<b21::SkeletonView*>(static_cast<unsigned char*>(rig)+sizeof(ResourcePrefix));
    int32_t index=b21::FindIndex(skeleton,key);
    if(index==-1) return -1;
    InsertResult result;
    // EA defines only the low byte of the unused hint.
    CallGameMethod<void,0x0056c560>(&p->map,&result,&index,0u);
    result.node->time=time;
    result.node->value={{0.0f,0.0f,1.0f,0.0f}};
    return reinterpret_cast<intptr_t>(result.node);
}
float First(ForwardController* p) { return p->child?CallGameVirtual<float,7>(p->child):Fallback(); }
float Second(ForwardController* p) { return p->child?CallGameVirtual<float,8>(p->child):Fallback(); }
float Third(ForwardController* p) { return p->child?CallGameVirtual<float,9>(p->child):Fallback(); }
}
