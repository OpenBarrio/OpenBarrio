#include "ts3/anim/m05_M119.h"
namespace ts3::anim::m119 {
Result* M119_CALL Insert(Table* self, void*, Result* out, const Input* value, const void*) {
    const uint32_t key=value->key;
    uint32_t index=key % self->bucketCount;
    Node** bucket=self->buckets+index;
    for(Node* node=*bucket; node; node=node->next) {
        if(node->key==key) { out->node=node; out->bucket=bucket; out->inserted=0; return out; }
    }
    struct Growth { uint8_t grow; unsigned char pad[3]; uint32_t count; } growth;
    CallGameMethod<void*,0x004e09b0>(self->policy,&growth,self->bucketCount,self->count,1u);
    Node* node=CallGameMethod<Node*,0x008422f0>(self,value);
    if(growth.grow) {
        index=key % growth.count;
        CallGameMethod<void,0x0083ad50>(self,growth.count);
    }
    bucket=self->buckets+index;
    node->next=*bucket;
    *bucket=node;
    ++self->count;
    out->node=node; out->bucket=bucket; out->inserted=1;
    return out;
}
LargeNode* M119_CALL CreateLarge(Table* self, void*, const Input* value) {
#if defined(_M_IX86)
    const char* source=reinterpret_cast<const char*>(0x00f98980);
#else
    const char* source=nullptr;
#endif
    auto* node=CallGameFunction<LargeNode*,0x004e3f90>(sizeof(LargeNode),self->allocator,0u,0u,source,0xe7u);
    if(node) {
        node->key=value->key;
        node->begin=node->end=node->storage;
        node->capacity=node->storage+sizeof(node->storage);
        CallGameMethod<void,0x0083f2a0>(&node->begin,value->begin,value->end,value);
    }
    node->next=nullptr;
    return node;
}
}
