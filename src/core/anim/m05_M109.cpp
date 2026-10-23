#include "ts3/anim/m05_M109.h"
#include <cmath>
#include <cstring>
namespace ts3::anim::m109 {
NodeKey* __stdcall Chain(NodeKey* node,const Key* key,uint32_t hash) {
    (void)hash;
    while(node) {
        if(node->key.word[0]==key->word[0] && node->key.word[2]==key->word[2] &&
           node->key.word[3]==key->word[3] && node->key.word[1]==key->word[1]) return node;
        node=node->next;
    }
    return nullptr;
}
Pair* Table::FindLarge(Pair* out,const uint32_t* key) {
    const uint32_t k=*key;
    void** slot=&buckets[k%count];
    auto node=static_cast<NodeLarge*>(*slot);
    while(node && node->key!=k) node=node->next;
    if(!node) { slot=&buckets[count]; node=static_cast<NodeLarge*>(*slot); }
    *out={node,slot};
    return out;
}
Pair* Table::FindSmall(Pair* out,const uint32_t* key) {
    const uint32_t k=*key;
    void** slot=&buckets[k%count];
    auto node=static_cast<NodeSmall*>(*slot);
    while(node && node->key!=k) node=node->next;
    if(!node) { slot=&buckets[count]; node=static_cast<NodeSmall*>(*slot); }
    *out={node,slot};
    return out;
}
Pair* Table::FindKey(Pair* out,const Key* key) {
    const uint32_t hash=key->word[0]^key->word[3];
    void** slot=&buckets[hash%count];
    auto node=Chain(static_cast<NodeKey*>(*slot),key,hash);
    if(!node) { slot=&buckets[count]; node=static_cast<NodeKey*>(*slot); }
    *out={node,slot};
    return out;
}
uintptr_t Callback::Invoke() {
#if defined(_M_IX86)
    auto result=call(context,*argument);
#else
    auto result=call(context,*argument);
#endif
    fired=1;
    return result;
}
bool Transfer(Entity* source,Entity* destination) {
    if(!source || !destination) return false;
    auto a=CallGameVirtual<Shape*,28>(&source->secondary);
    auto b=CallGameVirtual<Shape*,28>(&destination->secondary);
    if(!a || !b) return false;
    alignas(16) Float4Value position,rotation,extent;
    CallGameVirtual<void,12>(source,&position,&rotation);
    auto type=CallGameVirtual<uint32_t,1>(a);
    if(type==0x065bace9) extent=a->extent;
    else if(type==0x065bad09) {
        const float r=a->extent.lane[0],h=a->extent.lane[1];
        extent={{r,h,r,r}};
        position.lane[0]+=0.0f; position.lane[1]+=h; position.lane[2]+=0.0f; position.lane[3]+=0.0f;
    } else {
        float squared; std::memcpy(&squared,reinterpret_cast<unsigned char*>(a)+0x30,4);
        const float radius=std::sqrt(squared);
        extent={{radius,radius,radius,radius}};
    }
    return CallGameMethod<bool,0x006829c0>(b,&extent,&position,&rotation);
}
}
