#include "ts3/audio/m06_B52.h"
#include <cstring>
#include <initializer_list>
namespace ts3::audio::b52 {
namespace {
float X87Store(float value) {
#if defined(_M_IX86)
    float stored;
    __asm { fld value }
    __asm { fstp stored }
    return stored;
#else
    // x87 load/store quiets signaling NaNs even without arithmetic.
    uint32_t bits; std::memcpy(&bits,&value,4);
    if((bits&0x7F800000u)==0x7F800000u && (bits&0x007FFFFFu)) bits|=0x00400000u;
    std::memcpy(&value,&bits,4); return value;
#endif
}
Iterator Find(Map* m,const void* key) {
    Iterator it{};
    CallGameMethod<void,0x00D38B00>(m,&it,key);
    return it;
}
template<class F> void Each(Map& m,F fn) {
    Node** bucket=m.buckets;
    while(!*bucket) ++bucket;
    Node* node=*bucket;
    Node* end=m.buckets[m.count];
    while(node!=end) {
        fn(node);
        node=node->next;
        if(!node) { do { ++bucket; } while(!*bucket); node=*bucket; }
    }
}
void Send(Sound* s,uint32_t key,float value) {
    CallGameVirtual<void,13>(s->target,key,X87Store(value),uint32_t(1));
}
Map* Construct(Map* m,uint32_t n,const Source* source,uint32_t bytes) {
    m->count=m->size=0; m->load=1.f; m->growth=2.f; m->threshold=0;
    m->pool.first=nullptr;
#if defined(_M_IX86)
    m->pool.label=reinterpret_cast<const void*>(0x00FA99E8);
#else
    m->pool.label="audio hash pool";
#endif
    CallGameMethod<void,0x004E07A0>(&m->pool,source->begin,bytes,uint32_t(12),uint32_t(4),uint32_t(0));
    m->pool.begin=source->begin;
    m->pool.end=static_cast<uint8_t*>(source->begin)+bytes;
    m->pool.stride=12; m->pool.allocator=source->allocator;
    if(n<2) {
        m->size=m->threshold=0; m->count=1;
#if defined(_M_IX86)
        m->buckets=reinterpret_cast<Node**>(0x0114C900);
#else
        static Node* empty[2]={nullptr,reinterpret_cast<Node*>(~uintptr_t(0))}; m->buckets=empty;
#endif
    } else {
        m->count=CallGameMethod<uint32_t,0x004E0870>(&m->load,n);
        auto buckets=static_cast<Node**>(m->pool.allocator);
        std::memset(buckets,0,m->count*sizeof(Node*));
        buckets[m->count]=reinterpret_cast<Node*>(~uintptr_t(0));
        m->buckets=buckets;
    }
    return m;
}
}
Map* Construct240(Map* m,uint32_t n,uintptr_t,uintptr_t,uintptr_t,uintptr_t,uintptr_t,const Source* a) { return Construct(m,n,a,240); }
Map* Construct480(Map* m,uint32_t n,uintptr_t,uintptr_t,uintptr_t,uintptr_t,uintptr_t,const Source* a) { return Construct(m,n,a,480); }
InsertResult* Insert(Map* m,InsertResult* out,const uint32_t* key,const void*) {
    uint32_t hash=*key, bucket=hash%m->count;
    Node** slot=m->buckets+bucket;
    for(Node* n=*slot;n;n=n->next) if(n->key==hash) { out->node=n; out->bucket=slot; out->inserted=0; return out; }
    struct Growth { uint8_t grow; uint8_t pad[3]; uint32_t count; } growth{};
    CallGameMethod<void,0x004E09B0>(&m->load,&growth,m->count,m->size,uint32_t(1));
    Node* node=CallGameMethod<Node*,0x008B38E0>(m,key);
    if(growth.grow) { bucket=hash%growth.count; CallGameMethod<void,0x008AD420>(m,growth.count); }
    node->next=m->buckets[bucket]; m->buckets[bucket]=node; ++m->size;
    out->node=node; out->bucket=m->buckets+bucket; out->inserted=1; return out;
}
void Publish(Sound* s) {
    if(!s->target) return;
    Each(s->values,[&](Node* n){Send(s,n->key,n->value);});
    if(!(s->flags&8) && s->channels>=2) {
        if(s->propertyFlags&0x8000) Send(s,0x647A7836,s->f8c);
        if(s->propertyFlags&0x10000) Send(s,0xA26A765F,s->f90);
        if(s->propertyFlags&0x20000) Send(s,0x49EB68DB,s->f88);
    }
    for(unsigned i=0;i<3;++i) if(s->flags&(1u<<i)) Send(s,0x050C5D67-i,s->alternate?s->alternatePosition[i]:s->position[i]);
    Send(s,0xBA134192,s->alternate?1.f:0.f);
    if(s->flags&8) { Send(s,0x0F616B72,s->f68); Send(s,0x03593710,s->f6c); }
    Send(s,0x6DF5822D,static_cast<float>(s->channels));
    if(s->stringBegin!=s->stringEnd)
        CallGameVirtual<void,15>(s->target,uint32_t(0x2FE09C83),s->stringBegin,uint32_t(s->stringEnd-s->stringBegin));
}
void SetData(Sound* s,uint32_t key,const void* data,uint32_t bytes) {
    switch(key) {
    case 0x14043549: CallGameVirtual<void,47>(s,data); return;
    case 0x7CB81A35: case 0xF58CA0BE: CallGameVirtual<void,46>(s,data); return;
    case 0x2FE09C83: {
        CallGameMethod<void,0x00405B30>(&s->stringBegin,data);
        auto system=CallGameFunction<void*,0x008B0A70>();
        s->resolved=CallGameVirtual<void*,69>(system,data); break;
    }
    case 0x173C2710: CallGameMethod<void,0x008AAFE0>(s->curveC,data,uint32_t(0)); break;
    case 0xBDC98CE5: CallGameMethod<void,0x008AAFE0>(s->curveA,data,uint32_t(0)); break;
    case 0x8ADCE22F: CallGameMethod<void,0x008AAFE0>(s->curveB,data,uint32_t(0)); break;
    }
    if(s->target && data) CallGameVirtual<void,15>(s->target,key,data,bytes);
}
bool Get(Sound* s,uint32_t key,float* out) {
    switch(key) {
    case 0x050C5D65: *out=X87Store(s->position[2]); return true;
    case 0x050C5D66: *out=X87Store(s->position[1]); return true;
    case 0x050C5D67: *out=X87Store(s->position[0]); return true;
    case 0x03593710: *out=X87Store(s->f6c); return true;
    case 0x0F616B72: *out=X87Store(s->f68); return true;
    case 0x222B8006: *out=s->enabled?1.f:0.f; return true;
    case 0x12D2A4D4: *out=(s->flags&8)?1.f:0.f; return true;
    case 0x2406A047: *out=X87Store(s->f64); return true;
    case 0x25DF0108: *out=X87Store(s->f70); return true;
    case 0x29E8B9F8: *out=X87Store(s->f334); return true;
    case 0x393F7F7D: *out=X87Store(s->f84); return true;
    case 0x49EB68DB: *out=X87Store(s->f88); return true;
    case 0x6325C989: *out=X87Store(s->f78); return true;
    case 0x647A7836: *out=X87Store(s->f8c); return true;
    case 0x6DF5822D: *out=static_cast<float>(s->channels); return true;
    case 0xA26A765F: *out=X87Store(s->f90); return true;
    case 0xB1F42539: if(s->propertyFlags&0x400) { *out=X87Store(s->fac8); return true; } break;
    case 0xBAF7F4F9: *out=static_cast<float>(s->u33c); return true;
    case 0xC2D1F579: *out=X87Store(s->f7c); return true;
    }
    auto it=Find(&s->values,&key);
    if(it.node==s->values.buckets[s->values.count]) return false;
    *out=X87Store(it.node->value); return true;
}
bool Contains(void* secondary,uint32_t key) {
    auto s=reinterpret_cast<Sound*>(static_cast<uint8_t*>(secondary)-offsetof(Sound,secondary));
    Node* end=s->values.buckets[s->values.count];
    return Find(&s->values,&key).node!=end;
}
bool Prepare(Sound* s) {
    for(uint32_t key:{0x66EF9ABAu,0xE4DD1818u,0x0CD928A9u})
        if(!CallGameVirtual<bool,5>(&s->secondary,key)) return false;
    if(!CallGameVirtual<bool,5>(&s->secondary,uint32_t(0x07F28ACC))) {
        s->fallbackKey[0]=0x556FEA66; s->fallbackKey[1]=0;
    }
    uint32_t resourceKey[4]={s->key[0],s->key[1],0x03055F61,0x001407EC};
    auto system=CallGameFunction<void*,0x008B0A70>();
    if(s->resource) { auto old=s->resource; s->resource=nullptr; CallGameVirtual<void,1>(old); }
    if(!CallGameVirtual<bool,112>(system,resourceKey,&s->resource)) return false;
    float property;
    CallGameVirtual<bool,51>(s,uint32_t(0xE4DD1818),&property);
    // cvttss2si, including the indefinite result for NaN and overflow.
    auto trunc=[](float f)->int32_t {
        if(!(f>=-2147483648.f && f<2147483648.f)) return INT32_MIN;
        return static_cast<int32_t>(f);
    };
    int32_t end=trunc(property);
    CallGameVirtual<bool,51>(s,uint32_t(0x66EF9ABA),&property);
    int32_t begin=trunc(property);
    CallGameVirtual<bool,51>(s,uint32_t(0x0CD928A9),&property);
    float scalar=property*0.001f;
    struct Metadata { float position[3],percent,gain,frame; uint32_t unused,stamp; } metadata;
    if(!CallGameMethod<bool,0x008AB690>(s->resource,begin,end,&metadata)) return false;
    float duration=CallGameMethod<float,0x008AB1E0>(s->resource,begin,trunc(metadata.frame));
    float gain=1.f;
#if defined(_M_IX86)
    auto it=Find(&s->curves,reinterpret_cast<void*>(0x0103A888));
#else
    ts3::NotRewritten("B52 channel-curve key provider"); Iterator it{};
#endif
    if(it.node!=s->curves.buckets[s->curves.count]) {
        auto curve=it.node->object;
        CallGameVirtual<void,11>(curve,0.f,uint32_t(0));
        gain=CallGameVirtual<float,8>(curve);
        if(CallGameVirtual<bool,9>(curve)) CallGameVirtual<void,44>(s,uint32_t(0x25DF0108),gain,uint32_t(1));
    }
    float volume=static_cast<float>(end)*0.007874015718698502f;
    volume=volume*gain;
    CallGameVirtual<void,3>(&s->secondary,uint32_t(0x25DF0108),volume,uint32_t(1));
    CallGameVirtual<void,3>(&s->secondary,uint32_t(0x71BC3009),duration,uint32_t(1));
    CallGameVirtual<void,3>(&s->secondary,uint32_t(0xB1F42539),scalar,uint32_t(1));
    CallGameVirtual<void,3>(&s->secondary,uint32_t(0xE8AB99B9),metadata.gain,uint32_t(1));
    s->stamp[0]=metadata.stamp; s->stamp[1]=0;
    for(unsigned i=0;i<3;++i) s->preparedPosition[i]=metadata.position[i];
    s->preparedGain=metadata.gain;
    s->preparedPercent=1.f-metadata.percent*0.01f;
    s->preparedFlag=1; s->preparedZero=0.f; s->preparedState=1;
    system=CallGameFunction<void*,0x008B0A70>();
    CallGameVirtual<void,75>(system); return true;
}
void Copy(Sound* s,Sound* dest) {
#if defined(_M_IX86)
    auto it=Find(&s->releaseCurves,reinterpret_cast<void*>(0x0103A888));
#else
    ts3::NotRewritten("B52 channel-curve key provider"); Iterator it{};
#endif
    if(it.node!=s->releaseCurves.buckets[s->releaseCurves.count]) {
        float value=CallGameVirtual<float,8>(it.node->object);
        CallGameVirtual<void,3>(&dest->secondary,uint32_t(0x25DF0108),value,uint32_t(1));
    }
    Each(s->values,[&](Node* n){CallGameVirtual<void,3>(&dest->secondary,n->key,X87Store(n->value),uint32_t(1));});
}
void Clear(Sound* s) {
    Each(s->releaseCurves,[](Node* n){CallGameVirtual<void,1>(n->object);});
    CallGameMethod<void,0x008B3780>(&s->releaseCurves,s->releaseCurves.buckets,s->releaseCurves.count);
    s->releaseCurves.size=0;
}
void* FindOrInsert(Map* m,void* out,const void* key) {
    // Verified twin m03_A08::Find: only the typed insert callee differs.
    CallGameMethod<void,0x008BC3D0>(m,out,key,uint32_t(0)); return out;
}
ListNode* Append(Links* list,void* const* value) {
#if defined(_M_IX86)
    auto node=CallGameFunction<ListNode*,0x004E3F90>(uint32_t(12),list->allocator,uint32_t(0),uint32_t(0),reinterpret_cast<void*>(0x00F98980),uint32_t(231));
#else
    ts3::NotRewritten("B52 slave-list allocator"); ListNode* node=nullptr;
#endif
    node->value=*value; node->next=list; node->prev=list->prev;
    list->prev->next=reinterpret_cast<Links*>(node); list->prev=reinterpret_cast<Links*>(node); return node;
}
void Attach(Sound* s,Sound* slave) {
    if(!slave || slave==s || s->master || slave->master) return;
    CallGameVirtual<void,60>(slave,s);
    void* value=slave; Append(&s->slaves,&value);
    CallGameVirtual<void,0>(slave);
}
void SetCurve(void* secondary,uint32_t key,float value,uint32_t flag) {
    value=X87Store(value);
    auto s=reinterpret_cast<Sound*>(static_cast<uint8_t*>(secondary)-offsetof(Sound,secondary));
    auto system=CallGameFunction<void*,0x008B0A70>();
    uint64_t resource=CallGameVirtual<uint64_t,71>(system,key);
    if(!resource) { CallGameVirtual<void,44>(s,key,value,flag); return; }
    auto it=Find(&s->releaseCurves,&key);
    if(it.node!=s->releaseCurves.buckets[s->releaseCurves.count]) {
        CallGameMethod<void,0x008A6E10>(it.node->object,value); return;
    }
    void* memory=CallGameFunction<void*,0x008A6E80>(uint32_t(24));
    void* curve=memory?CallGameMethod<void*,0x008A6EC0>(memory,s):nullptr;
    CallGameVirtual<void,0>(curve);
    // Verified 0x00534E60 is the word directly after the vptr.
    *reinterpret_cast<uint32_t*>(static_cast<uint8_t*>(curve)+sizeof(void*))=key;
    CallGameMethod<void,0x008A6E10>(curve,value);
    struct Pair {uint32_t key; void* value;} pair{key,curve};
    Iterator output{}; FindOrInsert(&s->releaseCurves,&output,&pair);
    CallGameVirtual<void,6>(secondary,key,curve);
}
}
