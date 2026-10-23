#include "ts3/audio/m06_B36.h"
#if defined(_M_IX86)
#include <intrin.h>
#endif
#include "ts3/misc/cls_4553a0_x.h"
#include <cstring>
namespace ts3::audio::b36 {
namespace {
struct Iterator { Node* node; Node** bucket; };
struct Pair { uint32_t key; void* value; };
Node* End(Map& m) { return m.buckets[m.bucketCount]; }
Iterator Find(Map& m, uint32_t key) {
#if defined(_M_IX86)
    Iterator it{};
    CallGameMethod<Iterator*,0x00D38B00>(&m, &it, &key);
    return it;
#else
    for(uint32_t i=0;i<m.bucketCount;++i)
        for(Node* p=m.buckets[i];p;p=p->next) if(p->key==key) return {p,m.buckets+i};
    return {End(m),m.buckets+m.bucketCount};
#endif
}
Iterator Begin(Map& m) {
    Node** b=m.buckets;
    while(!*b) ++b;
    return {*b,b};
}
void Next(Iterator& i) {
    i.node=i.node->next;
    while(!i.node) i.node=*++i.bucket;
}
template<class F> void Each(Map& m,F f) {
    for(auto i=Begin(m);i.node!=End(m);Next(i)) f(i.node->value);
}
void* Secondary(void* p) { return static_cast<uint8_t*>(p)+sizeof(void*); }
void Insert(Map& m,uint32_t key,void* value) {
    Pair pair{key,value};
    CallGameVirtual<void,0>(value);
    uint8_t out[3*sizeof(void*)]{};
    CallGameMethod<void,0x008ADB20>(&m,out,&pair,uint32_t(0));
    if(pair.value) CallGameVirtual<void,1>(pair.value);
}
void Erase(Map& m,Iterator i) {
    Iterator out{};
    CallGameMethod<void,0x008B6F20>(&m,&out,i);
}
uint64_t Resolve(uint32_t key) {
    auto* system=CallGameFunction<void*,0x008B0A70>();
    return CallGameVirtual<uint64_t,71>(system,key);
}
void* Recycle(Map& m,void* p) {
    if(p==m.special) return p;
    if(reinterpret_cast<uintptr_t>(p)>=reinterpret_cast<uintptr_t>(m.poolBegin) &&
       reinterpret_cast<uintptr_t>(p)<reinterpret_cast<uintptr_t>(m.poolEnd)) {
        *static_cast<void**>(p)=m.freeList;
        m.freeList=p;
    } else {
        return CallGameFunction<void*,0x004E3F70>(p);
    }
    return p;
}
void DestroyMap(Map& m) {
    ClearBuckets(&m,m.buckets,m.bucketCount);
    m.count=0;
    if(m.bucketCount>1) Recycle(m,m.buckets);
}
}
bool ReadValue(Emitter* self,uint32_t key,float* out) {
    auto i=Find(self->values,key);
    if(i.node==End(self->values)) return false;
    *out=CallGameVirtual<float,8>(i.node->value);
    return true;
}
bool HasValue(Emitter* self,uint32_t key) {
    Node* end=End(self->values); // EA captures the sentinel before lookup.
    return Find(self->values,key).node!=end;
}
void UpdateState(Emitter* self,uint32_t key,uint32_t state) {
    auto i=Find(self->states,key);
    if(i.node==End(self->states)) return;
    void* value=i.node->value;
    auto* secondary=Secondary(value);
    CallGameVirtual<void,5>(secondary,state);
    if(CallGameVirtual<uint32_t,6>(secondary)) return;
    CallGameVirtual<void,12>(value,uint32_t(0));
    Each(self->voices,[&](void* voice){CallGameVirtual<void,7>(voice,key,value);});
}
void ClearBuckets(Map* self,Node** buckets,uint32_t count) {
    for(uint32_t n=0;n<count;++n) {
        auto* p=buckets[n];
        while(p) {
            auto* old=p;
            void* value=old->value;
            p=p->next; // fetch before Release: callback may alter the node.
            if(value) CallGameVirtual<void,1>(value);
            Recycle(*self,old);
        }
        buckets[n]=nullptr;
    }
}
bool AddVoice(Emitter* self,void* voice,void* callerStackWord) {
    uint32_t key=CallGameVirtual<uint32_t,2>(voice);
    if(Find(self->voices,key).node!=End(self->voices)) return false;
    CallGameVirtual<void,0>(voice);
    key=CallGameVirtual<uint32_t,2>(voice);
    Insert(self->voices,key,voice);
    CallGameVirtual<void,1>(voice);
    Each(self->states,[&](void* state){
        if(CallGameVirtual<uint32_t,6>(Secondary(state))) {
            uint32_t id=CallGameVirtual<uint32_t,6>(state,state);
#if defined(_M_IX86)
            using Callback = void(__thiscall*)(void*,uint32_t);
            reinterpret_cast<Callback>(*static_cast<void**>(callerStackWord))(voice,id);
#else
            AbortNotRewritten("B36 caller-stack __thiscall callback");
#endif
        }
    });
    Each(self->values,[&](void* value){
        uint32_t id=CallGameVirtual<uint32_t,6>(value);
        if(!Resolve(id)) {
            float f=CallGameVirtual<float,8>(value,uint32_t(1));
            // EA reads [entry ESP+8], although this entry returns with ret 4.
            CallGameVirtual<void,3>(voice,uint32_t(reinterpret_cast<uintptr_t>(callerStackWord)),f);
        }
    });
    if(self->enabled) CallGameVirtual<void,3>(voice,uint32_t(0xb85523e5),1.0f,uint32_t(1));
    return true;
}
bool RemoveVoice(Emitter* self,void* voice) {
    uint32_t key=CallGameVirtual<uint32_t,2>(voice);
    auto i=Find(self->voices,key);
    if(i.node==End(self->voices)) return false;
    return RemoveVoiceFound(self,voice,i.node,i.bucket);
}
bool RemoveVoiceFound(Emitter* self, void* voice, Node* node, Node** bucket) {
    // Raw entry has EBX=self, EBP=voice, ESI=voices and ECX=node; the bucket is in its parent frame.
    Erase(self->voices,Iterator{node,bucket});
    Each(self->states,[&](void* state){
        uint32_t id=CallGameVirtual<uint32_t,6>(state,state);
        CallGameVirtual<void,7>(voice,id);
    });
    if(self->enabled) CallGameVirtual<void,3>(voice,uint32_t(0xb85523e5),0.0f,uint32_t(1));
    return true;
}
bool ReplaceVoice(Emitter* self,uint32_t key,void* voice) {
    auto i=Find(self->voices,key);
    if(i.node==End(self->voices)) return false;
    if(voice) CallGameVirtual<void,0>(voice);
    uint32_t newKey=CallGameVirtual<uint32_t,2>(voice);
    Insert(self->voices,newKey,voice);
    CallGameVirtual<void,1>(voice);
    Erase(self->voices,i);
    return true;
}
void SetValue(Emitter* self, uint32_t key, float value, uint32_t transition) {
    (void)transition; // EA pops the third argument without using it.
    uint32_t bits;
    std::memcpy(&bits, &value, sizeof bits);
    if ((bits & 0x7f800000u) == 0x7f800000u) return;
    if (key == 0xb85523e5u) {
        CallGameMethod<void,0x008A2F40>(self, uint32_t(value != 0.0f));
        return;
    }
    auto i = Find(self->values, key);
    if (i.node != End(self->values)) {
        CallGameMethod<void,0x008A6E10>(i.node->value, value);
        return;
    }
    void* allocation = CallGameFunction<void*,0x008A6E80>(uint32_t(24));
    void* parameter = allocation ? CallGameMethod<void*,0x008A6EC0>(allocation, self) : nullptr;
    CallGameVirtual<void,4>(parameter);
#if defined(_M_IX86)
    reinterpret_cast<ts3::misc::GlyphMetricsMap*>(parameter)->SetBaseAllocator(key);
#else
    *reinterpret_cast<uint32_t*>(static_cast<uint8_t*>(parameter)+sizeof(void*))=key;
#endif
    CallGameMethod<void,0x008A6E10>(parameter, value);
    CallGameVirtual<void,0>(parameter);
    Insert(self->values, key, parameter);
    CallGameVirtual<void,1>(parameter);
    if (Resolve(key)) {
        CallGameVirtual<void,6>(self, key, parameter);
    } else {
        Each(self->voices,[&](void* voice) {
            float f = CallGameVirtual<float,8>(parameter, uint32_t(1));
            // Insertion zeroed the low byte of the caller's value stack slot.
            CallGameVirtual<void,3>(voice, bits & 0xffffff00u, f);
        });
    }
}
bool SetState(Emitter* self, uint32_t key, uint32_t state) {
    const uint64_t resource = Resolve(key);
    if (!resource) return false;
    auto i = Find(self->states, key);
    void* node = nullptr;
    if (i.node == End(self->states)) {
        void* allocation = CallGameFunction<void*,0x008A6590>(uint32_t(112));
        if (allocation) {
            node = CallGameMethod<void*,0x008A69D0>(allocation, self);
            if (node) CallGameVirtual<void,0>(node);
        }
        CallGameVirtual<void,4>(node); // preserve null-allocation dereference
        CallGameMethod<void,0x008A64D0>(node, key);
        CallGameMethod<void,0x008A6530>(node, uint32_t(resource), uint32_t(resource >> 32));
        CallGameVirtual<void,4>(Secondary(node), state);
        Insert(self->states, key, node);
    } else {
        node = i.node->value;
        if (node) CallGameVirtual<void,0>(node);
        CallGameVirtual<void,4>(Secondary(node), state);
    }
    if (CallGameVirtual<uint32_t,6>(Secondary(node)) == 1) {
        CallGameVirtual<void,12>(node, uint32_t(1));
        Each(self->voices,[&](void* voice) { CallGameVirtual<void,6>(voice, key, node); });
    }
    CallGameVirtual<void,1>(node);
    return true;
}
FixedMap* ConstructMap(FixedMap* self, uint32_t policy, uint32_t option) {
    struct Pool { void* free; const char* label; void* begin; void* end; uint32_t stride; void* special; };
    Pool pool{};
#if defined(_M_IX86)
    pool.label=reinterpret_cast<const char*>(0x00fa99e8);
#endif
    CallGameMethod<void,0x004E07A0>(&pool, static_cast<void*>(nullptr), uint32_t(600), uint32_t(12), uint32_t(4), uint32_t(0));
    pool.begin=nullptr;
    pool.end=reinterpret_cast<void*>(uintptr_t(600));
    pool.stride=12;
    pool.special=self->inlineBuckets;
    uint32_t prime=CallGameFunction<uint32_t,0x004E0850>(uint32_t(51));
    CallGameMethod<void,0x008A2FF0>(&self->map, prime, policy, &option, &option, option, &option, &pool);
    struct Policy { float load, growth; uint32_t spare; } p;
    uint32_t loadBits=1176256512u;
    std::memcpy(&p.load,&loadBits,sizeof loadBits);
    p.growth=2.0f; p.spare=0;
    self->map.load=p.load; self->map.growth=p.growth; self->map.spare=0;
    uint32_t size=CallGameMethod<uint32_t,0x004E08F0>(&p,self->map.count);
    if(size>self->map.bucketCount) CallGameMethod<void,0x008AD420>(&self->map,size);
#if defined(_M_IX86)
    self->map.poolOwner=reinterpret_cast<void*>(0x00fa9aa4);
#endif
    CallGameMethod<void,0x004E07A0>(&self->map.freeList, self->inlineNodes, uint32_t(600), uint32_t(12), uint32_t(4), uint32_t(0));
    self->map.poolBegin=self->inlineNodes;
    self->map.poolEnd=self->inlineNodes+50;
    self->map.stride=12;
    return self;
}
bool Reset(Emitter* self) {
    auto i=Begin(self->voices);
    while(i.node!=End(self->voices)) {
        void* voice=i.node->value;
        Next(i); // advance before the callback erases this node.
        CallGameVirtual<void,11>(self,voice);
    }
    Each(self->states,[&](void* state){
        if(CallGameVirtual<void*,13>(state)==self) CallGameVirtual<void,5>(state);
    });
    ClearBuckets(&self->states,self->states.buckets,self->states.bucketCount);
    self->states.count=0;
    Each(self->values,[](void* value){CallGameVirtual<void,5>(value);});
    ClearBuckets(&self->values,self->values.buckets,self->values.bucketCount);
    self->values.count=0;
    return true;
}
void* Destroy(Emitter* self) {
#if defined(_M_IX86)
    self->primary=reinterpret_cast<void**>(0x01031680);
    self->secondary=reinterpret_cast<void**>(0x0103167c);
#endif
    DestroyMap(self->states);
    DestroyMap(self->values);
    // EAX on the final path is the bucket pointer, except an external deallocation.
    auto* result=self->voices.buckets;
    ClearBuckets(&self->voices,result,self->voices.bucketCount);
    self->voices.count=0;
    void* returned=result;
    if(self->voices.bucketCount>1) returned=Recycle(self->voices,result);
#if defined(_M_IX86)
    self->secondary=reinterpret_cast<void**>(0x0107396c);
#endif
    return returned;
}
void* DestroySecondary(void* self,uint32_t flags) {
    // Adapted from ResourceBaseDestroySecondary, the verified near-00409770 twin.
    return CallGameMethod<void*,0x008A3E00>(static_cast<uint8_t*>(self)-sizeof(void*),flags);
}

#if defined(_M_IX86)
bool __fastcall HookReadValue(Emitter* s, void*, uint32_t k, float* o) { return ReadValue(s,k,o); }
bool __fastcall HookHasValue(Emitter* s, void*, uint32_t k) { return HasValue(s,k); }
void __fastcall HookUpdateState(Emitter* s, void*, uint32_t k, uint32_t a) { return UpdateState(s,k,a); }
void __fastcall HookClearBuckets(Map* s, void*, Node** b, uint32_t n) { return ClearBuckets(s,b,n); }
bool __fastcall HookAddVoice(Emitter* s, void*, void* v) { return AddVoice(s,v,reinterpret_cast<void**>(_AddressOfReturnAddress())[2]); }
bool __fastcall HookRemoveVoice(Emitter* s, void*, void* v) { return RemoveVoice(s,v); }
bool __fastcall HookReplaceVoice(Emitter* s, void*, uint32_t k, void* v) { return ReplaceVoice(s,k,v); }
void __fastcall HookSetValue(Emitter* s, void*, uint32_t k, float f, uint32_t t) { return SetValue(s,k,f,t); }
bool __fastcall HookSetState(Emitter* s, void*, uint32_t k, uint32_t a) { return SetState(s,k,a); }
FixedMap* __fastcall HookConstructMap(FixedMap* s, void*, uint32_t a, uint32_t b) { return ConstructMap(s,a,b); }
bool __fastcall HookReset(Emitter* s, void*) { return Reset(s); }
void* __fastcall HookDestroy(Emitter* s, void*) { return Destroy(s); }
void* __fastcall HookDestroySecondary(void* s, void*, uint32_t flags) { return DestroySecondary(s,flags); }
#endif
}
