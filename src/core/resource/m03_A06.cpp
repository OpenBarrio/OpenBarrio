#include "ts3/resource/m03_A06.h"
#include <cstring>
namespace ts3::resource::m03::a06 {
namespace {
Dispatch* dispatch=nullptr;
uintptr_t Ref(void* p,unsigned slot) {
    if(dispatch && dispatch->ref) return dispatch->ref(p,slot);
    return slot==0 ? CallGameVirtual<uintptr_t,0>(p) : CallGameVirtual<uintptr_t,1>(p);
}
uintptr_t FreePairs(void* p) {
    if(dispatch && dispatch->freePairs) return dispatch->freePairs(p);
#if defined(_M_IX86)
    return CallGameMethod<uintptr_t,0x004E5920>(*reinterpret_cast<void**>(0x011CB864),p);
#else
    NotRewritten("A06 pair allocator");
#endif
}
uintptr_t FreeCore(void* p) {
    if(dispatch && dispatch->freeCore) return dispatch->freeCore(p);
    return CallGameFunction<uintptr_t,0x004E3F70>(p);
}
void* Allocate(size_t n,void* allocator) {
    if(dispatch && dispatch->allocate) return dispatch->allocate(n,allocator);
#if defined(_M_IX86)
    return CallGameFunction<void*,0x004E3F90>(n,allocator,0,0,reinterpret_cast<const char*>(0x00F98980),0xe7);
#else
    NotRewritten("A06 core allocation");
#endif
}
Record16* Fallback(bool typed) {
#if defined(_M_IX86)
    auto* flags=reinterpret_cast<uint32_t*>(typed?0x011D3C80:0x011D3C68);
    auto* record=reinterpret_cast<Record16*>(typed?0x011D3C70:0x011D3C58);
#else
    static uint32_t flagsNative[2]{};
    static Record16 records[2]{};
    auto* flags=&flagsNative[typed]; auto* record=&records[typed];
#endif
    if(!(*flags&1)) { *flags|=1; *record={}; }
    return record;
}
}
Dispatch* SetDispatch(Dispatch* d) { auto* old=dispatch; dispatch=d; return old; }
void SetChild(Context* s,uint32_t i,void* p,uint32_t owned) {
    if(i>=s->count) return;
    void* old=reinterpret_cast<void*>(s->groups[0].children[i]&~uintptr_t(1));
    if(p) Ref(p,0);
    if(old) Ref(old,1);
    s->groups[0].children[i]=reinterpret_cast<uintptr_t>(p)+(static_cast<uint8_t>(owned)==0);
}
uintptr_t GetPair(Context* s,uint32_t i,uint32_t* a,uint32_t* b) {
    if(i>=s->count) return uintptr_t(i)&~uintptr_t(255);
    *a=s->pairs[i].first; *b=s->pairs[i].second;
    return (reinterpret_cast<uintptr_t>(b)&~uintptr_t(255))|1;
}
uintptr_t GetTypedPair(Context* s,uint32_t h,uint32_t* a,uint32_t* b) {
    uint32_t t=h>>28, i=(h&0x0fffffffu)-1;
    if(t>=4 || i>=s->groups[t].count) return 0;
    if(t==0) return GetPair(s,i,a,b);
    if(t==1) return GetPair(s,s->groups[0].count+i,a,b);
    return 0; // EAX is zero after subtracting one for type 2; type 3 becomes 1, then AL cleared.
}
Record16* GetRecord(Context* s,uint32_t i,uint32_t t) {
    auto* fallback=Fallback(false); // initialization occurs even on successful lookup
    return i<s->groups[t].count ? &s->groups[t].records[i] : fallback;
}
InitData* Initialize(InitData* s) {
    s->begin=s->end=s->capacity=nullptr;
#if defined(_M_IX86)
    s->allocator=reinterpret_cast<void*>(0x00FFD1F4);
#else
    if(!dispatch) NotRewritten("A06 allocator vtable");
    s->allocator=dispatch->initAllocator;
#endif
    s->field14=s->field18=s->field20=nullptr; s->flag=0; return s;
}
uintptr_t Clear(Context* s) {
    uintptr_t result=reinterpret_cast<uintptr_t>(s->begin);
    if(s->pairs) {
        for(uint32_t i=0;i<s->childCount;++i) {
            auto* p=reinterpret_cast<void*>(s->groups[0].children[i]&~uintptr_t(1));
            if(p) Ref(p,1);
        }
        result=FreePairs(s->pairs); s->pairs=nullptr;
    }
    s->end=s->begin; return result;
}
Record16* GetTypedRecord(Context* s,uint32_t h) {
    auto* fallback=Fallback(true);
    uint32_t t=h>>28;
    return t<4 ? GetRecord(s,(h&0x0fffffffu)-1,t) : fallback;
}
uintptr_t Destroy(Context* s) {
    auto result=Clear(s);
    return s->begin ? FreeCore(s->begin) : result;
}
void* GetChild(Context* s,uint32_t h) {
    uint32_t t=h>>28,i=(h&0x0fffffffu)-1;
    if(t>=4 || i>=s->groups[t].count) return nullptr;
    uintptr_t p=s->groups[t].children[i];
    if(p&1) {
        uint32_t index=t==1?s->groups[0].count+i:i;
        uint8_t ok=dispatch && dispatch->materialize ? dispatch->materialize(s,index) : CallGameMethod<uint8_t,0x00726480>(s,index);
        if(!ok) return nullptr;
        p=s->groups[t].children[i];
    }
    return reinterpret_cast<void*>(p); // do not clear a tag left by the callee
}
uint8_t* Assign(Buffer* s,const uint8_t* first,const uint8_t* last,uint32_t) {
    size_t n=static_cast<size_t>(reinterpret_cast<uintptr_t>(last)-reinterpret_cast<uintptr_t>(first));
    size_t cap=reinterpret_cast<uintptr_t>(s->capacity)-reinterpret_cast<uintptr_t>(s->begin);
    size_t size=reinterpret_cast<uintptr_t>(s->end)-reinterpret_cast<uintptr_t>(s->begin);
    if(n>cap) {
        auto* p=n?static_cast<uint8_t*>(Allocate(n,s->allocator)):nullptr;
        std::memcpy(p,first,n);
        if(s->begin) FreeCore(s->begin);
        s->begin=p; s->capacity=p+n; s->end=p+n;
    } else if(n<=size) {
        std::memcpy(s->begin,first,n); s->end=s->begin+n;
    } else {
        std::memcpy(s->begin,first,size);
        std::memcpy(s->end,first+size,n-size); s->end+=n-size;
    }
    return s->end;
}
void* MutexAddress(void* s) { return static_cast<uint8_t*>(s)+0x40; }
void* QueryInterface(void* s,uint32_t id) { return id==0x03b5f02fu || id==0xee3f516eu ? s : nullptr; }
void* Singleton() {
    if(dispatch) return dispatch->singleton;
#if defined(_M_IX86)
    return *reinterpret_cast<void**>(0x011D3C84);
#else
    NotRewritten("A06 async singleton");
#endif
}
Record16* CopyKey(LoadPrefix* s,Record16* out) {
    // Scalar order matters for overlapping output ranges, as in the original.
    auto a=s->key.words[0]; out->words[0]=a;
    auto b=s->key.words[1]; out->words[1]=b;
    auto c=s->key.words[2],d=s->key.words[3]; out->words[2]=c; out->words[3]=d; return out;
}
Record16* CopyOther(LoadPrefix* s,Record16* out) {
    auto a=s->other.words[0]; out->words[0]=a;
    auto b=s->other.words[1]; out->words[1]=b;
    auto c=s->other.words[2],d=s->other.words[3]; out->words[2]=c; out->words[3]=d; return out;
}
}
