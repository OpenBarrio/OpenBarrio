#include "ts3/audio/m06_B40.h"
namespace ts3::audio::b40 {
uint8_t Dirty(const Scene* s) { return s->dirty; }
uint8_t SetDirty(Scene* s,uint32_t v) { s->dirty=static_cast<uint8_t>(v); return s->dirty; }
void* SetCallback(Scene* s,void* p) { s->callback=p; return p; }
void Enable(Scene* s,uint32_t enable) {
    if (static_cast<uint8_t>(enable)) {
        auto* system=CallGameFunction<void*,0x008B0A70>();
        CallGameVirtual<void,116>(system,s);
        s->registered=1;
    } else if (s->registered) {
        auto* system=CallGameFunction<void*,0x008B0A70>();
        CallGameVirtual<void,117>(system,s);
        s->registered=0;
    }
}
uint32_t SetKey(Scene* s,uint32_t lo,uint32_t hi) { s->keyLow=lo; s->keyHigh=hi; return lo; }
void* Owner(const Scene* s) { return s->owner; }
void* Query(Scene* s,uint32_t id) {
    if (id==0xee3f516e || id==0x4bc808e5) return s;
    if (id==0x4e73a43a && s) return &s->secondary;
    return nullptr;
}
void* Allocate(uint32_t size) {
    if (size!=0x70) return CallGameFunction<void*,0x005C32D0>(size);
#if defined(_M_IX86)
    auto& head=*reinterpret_cast<void**>(0x011D9FE0);
    while (!head) {
        if (!CallGameMethod<uint8_t,0x004E42F0>(reinterpret_cast<void*>(0x011D9FD0),0,0)) return nullptr;
    }
    void* p=head;
    head=*static_cast<void**>(p);
    return p;
#else
    NotRewritten("m06_B40 audio pool");
#endif
}
int32_t Count(const void* secondary) {
    auto* s=reinterpret_cast<const Scene*>(static_cast<const uint8_t*>(secondary)-offsetof(Scene,secondary));
    const intptr_t bytes=static_cast<intptr_t>(reinterpret_cast<uintptr_t>(s->children.end)-reinterpret_cast<uintptr_t>(s->children.begin));
    return static_cast<int32_t>(bytes >> (sizeof(void*)==4 ? 2 : 3));
}
float Aggregate(Scene* s) {
    const auto n=static_cast<intptr_t>(reinterpret_cast<uintptr_t>(s->children.end)-reinterpret_cast<uintptr_t>(s->children.begin)) >> (sizeof(void*)==4 ? 2 : 3);
    if (n<=0) return 0.0f;
    auto get=[&](ptrdiff_t i) { return CallGameVirtual<float,8>(s->children.begin[i]); };
    if (n==1) return get(0);
    const uint64_t key=uint64_t(s->keyHigh)<<32|s->keyLow;
    if (key==0xc0de9a9b8ec0e479ull) return get(0);
    if (key==0xd06d7f7ec6478465ull) return get(n-1);
    if (key==0x84a8c7bf3b0e9b86ull) {
        float value=0;
        for (ptrdiff_t i=0;i<n;++i) value=static_cast<float>(get(i)+value);
        return value/static_cast<float>(n);
    }
    if (key!=0x93a25c7d3025861aull && key!=0xd8b3cd186b8cadc3ull && key!=0xd8b3c5186b8ca06dull) return 0.0f;
    float value=get(0);
    for (ptrdiff_t i=1;i<n;++i) {
        float next=get(i);
        if (key==0x93a25c7d3025861aull) value=static_cast<float>(next*value);
        else if (key==0xd8b3cd186b8cadc3ull) { if (value>next) value=next; }
        else { if (next>value) value=next; }
    }
    return value;
}
uint32_t AddRef(Scene* s) { return ++s->refs; }
uintptr_t Clear(Scene* s) {
    CallGameVirtual<void,12>(s,uint32_t{0});
    void** end=s->children.end;
    for (void** p=s->children.begin;p!=end;++p) {
        void* child=*p;
        void* owner=CallGameVirtual<void*,13>(child);
        if (owner==CallGameVirtual<void*,13>(s)) CallGameVirtual<void,5>(child);
    }
    end=s->children.end;
    for (void** p=s->children.begin;p<end;++p) if (*p) CallGameVirtual<void,1>(*p);
    s->children.end=s->children.begin;
    return (reinterpret_cast<uintptr_t>(s->children.begin)&~uintptr_t(255))|1;
}
void Update(Scene* s,float dt,uint32_t tick) {
    if (s->tick && s->tick==tick) return;
    void** end=s->children.end;
    s->tick=tick;
    for (void** p=s->children.begin;p!=end;++p) {
        void* child=*p;
        CallGameVirtual<void,11>(child,dt,tick);
        if (CallGameVirtual<uint8_t,9>(child)) CallGameVirtual<void,10>(s,uint32_t{1});
    }
    if (CallGameVirtual<uint8_t,9>(s)) s->value=Aggregate(s);
}
uintptr_t DestroyChildren(Children* v) {
    void** end=v->end;
    for (void** p=v->begin;p<end;++p) if (*p) CallGameVirtual<void,1>(*p);
    uintptr_t result=reinterpret_cast<uintptr_t>(v->begin);
    if (v->begin && v->begin!=v->storage) result=CallGameFunction<uintptr_t,0x004E3F70>(v->begin);
    return result;
}
void* DeleteSecondary(void* secondary,uint32_t flags) {
    auto* s=reinterpret_cast<Scene*>(static_cast<uint8_t*>(secondary)-offsetof(Scene,secondary));
    return CallGameMethod<void*,0x008A6A50>(s,flags);
}
}
