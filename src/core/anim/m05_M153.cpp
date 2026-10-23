#include "ts3/anim/m05_M153.h"
#include <cstring>
#include <initializer_list>
namespace ts3::anim::m153 {
namespace {
template<class T> T& Field(void* p, size_t n) { return *reinterpret_cast<T*>(static_cast<unsigned char*>(p)+n); }
#if defined(_M_IX86)
template<class T> T Global(uintptr_t p) { return *reinterpret_cast<T*>(p); }
void* Manager() { return Global<void*>(0x011cd800); }
void* Query(void* host,uint32_t key) { return CallGameMethod<void*,0x00935770>(host,key); }
void Notify(Component* self,uint32_t key,bool changed) {
    struct Message { uint32_t key; void* handler; uint32_t lo,hi; uint32_t payload[16]; } m;
    m.key=key; m.handler=nullptr; m.lo=Field<uint32_t>(self->host,8); m.hi=Field<uint32_t>(self->host,12);
    if(changed) m.payload[0]=1;
    CallGameFunction<void,0x00692f70>(&m);
    if(changed) CallGameMethod<void,0x0093e340>(self);
    if(m.handler) CallGameVirtual<void,0>(m.handler,m.payload);
}
#endif
}
void* CopyPoseState(void* self,const void* source) {
    auto d=static_cast<unsigned char*>(self); auto s=static_cast<const unsigned char*>(source);
    // Scalar fields pass through x87 (quieting signaling NaNs); SIMD blocks and +74 are raw copies.
    for(auto range : {0,16,20,24,28,32,48,52,56,60,64,80,96,112,116}) {
        size_t size=(range==0||range==80||range==96)?16:4;
        if(size==4 && range!=116) {
#if defined(_M_IX86)
            const void* from=s+range; void* to=d+range;
            __asm { mov ecx, from
                    mov edx, to
                    fld dword ptr [ecx]
                    fstp dword ptr [edx] }
#else
            uint32_t bits; std::memcpy(&bits,s+range,4);
            if((bits&0x7f800000)==0x7f800000 && (bits&0x007fffff)) bits|=0x00400000;
            std::memcpy(d+range,&bits,4);
#endif
        } else {
            unsigned char temp[16]; std::memcpy(temp,s+range,size); std::memcpy(d+range,temp,size);
        }
    }
    return self;
}
uintptr_t Activate(Component* self) {
    uint32_t f=self->flags;
    if(f&0x800) return f;
#if defined(_M_IX86)
    if(!(f&0x100)) {
        if(auto manager=Manager()) {
            if(static_cast<int32_t>(f)<0) CallGameMethod<void,0x005718e0>(manager,self);
            else CallGameMethod<void,0x005718c0>(manager,self);
        }
        self->flags|=0x100;
        CallGameMethod<void,0x0093f580>(self);
    }
    return CallGameMethod<uintptr_t,0x009357e0>(self->host,Global<uint32_t>(0x011dc47c));
#else
    NotRewritten("M153 activation dependencies");
#endif
}
void Deactivate(Component* self) {
#if defined(_M_IX86)
    if(auto manager=Manager()) CallGameMethod<void,0x00571900>(manager,self);
    self->flags&=~0x100u;
    if(!(self->flags&0x800)) {
        if(!self->host||!Query(self->host,Global<uint32_t>(0x011e90fc))) {
            auto manager=Manager();
            if(manager&&Field<uint8_t>(manager,56)) Notify(self,0x7727ad2e,false);
        }
        if(self->host) if(auto p=Query(self->host,Global<uint32_t>(0x011e9158))) CallGameMethod<void,0x00959170>(p,0);
        CallGameMethod<void,0x009357c0>(self->host,Global<uint32_t>(0x011dc47c));
    }
    if(auto state=self->state) {
        if(auto group=Field<void*>(state,176)) {
            CallGameMethod<void,0x00582820>(group);
            group=Field<void*>(state,176); // callbacks may replace/remove the group
            if(group) CallGameVirtual<void,5>(group,1);
            Field<void*>(state,176)=nullptr;
        }
        CallGameFunction<void,0x004e3f70>(state);
        self->state=nullptr;
    }
#else
    NotRewritten("M153 deactivation dependencies");
#endif
}
bool SerializePose(Component* self,void* stream) {
#if defined(_M_IX86)
    bool wasDirty=(self->flags&0x40)!=0;
    self->flags&=~0x40u;
    bool enabled=(Global<uint32_t>(0x011e90fc)&Field<uint32_t>(self->host,44))!=0;
    void* service=CallGameFunction<void*,0x005eb7c0>();
    bool dirty=self->dirty!=0;
    if(self->flags&0x10000) {
        self->flags&=~0x10000u;
        Notify(self,0xecff5d63,true); dirty=true;
    }
    self->flags|=6;
    if(static_cast<int32_t>(self->flags)>=0&&!CallGameFunction<uint8_t,0x0093eaf0>(self->host)&&!dirty) return false;
    alignas(16) unsigned char graph[17216];
    CallGameMethod<void,0x0054c720>(graph);
    uint32_t flags=0;
    void* root=nullptr;
    auto setRoot=[&](void* p) { CallGameMethod<void,0x0054c5c0>(graph,p); };
    auto getRoot=[&]() { return CallGameMethod<void*,0x0054c5d0>(graph); };
    auto make=[&](uint32_t id) {
        auto factory=CallGameMethod<void*,0x0054c770>(graph,id);
        return CallGameVirtual<void*,1>(factory,id);
    };
    auto attach=[](void* p,void* child) { CallGameVirtual<void,3>(p,child); };
    auto identity=[&](void* p) { CallGameMethod<void,0x007c0660>(p,Field<uint32_t>(self->host,8),Field<uint32_t>(self->host,12)); };
    if(self->state) if(auto group=Field<void*>(self->state,176)) {
        if(CallGameMethod<uint32_t,0x005833a0>(group)>0) {
            root=CallGameMethod<void*,0x005833c0>(group,graph,self->host);
            if(root) { flags=1; setRoot(root); }
            if(enabled) {
                flags|=8;
                if(static_cast<int32_t>(self->flags)>=0&&!CallGameMethod<uint8_t,0x00582870>(group)) flags|=2;
            }
        }
    }
    if(self->rig&&CallGameMethod<int32_t,0x00575e30>(static_cast<unsigned char*>(self->rig)+40,Global<void*>(0x0114f550))!=-1) {
        auto p=make(196530341);
        uint16_t count=service?CallGameMethod<uint16_t,0x005ec9e0>(service,static_cast<unsigned char*>(self->rig)+8):0;
        identity(p); Field<uint32_t>(p,52)=count;
        if(root) attach(p,root);
        root=p;
    }
    if(root) {
        if(self->flags&0x1000) {
            auto p=make(106916802); setRoot(p);
            auto q=make(0xf40282a6); attach(p,q); attach(q,root);
        } else if((self->flags&0x400)||enabled) {
            void* tail=nullptr; setRoot(nullptr);
            auto append=[&](void* p) { if(!getRoot()) setRoot(p); if(tail) attach(tail,p); tail=p; };
            auto weighted=[&](uint32_t id) {
                auto p=make(id);
                Field<uint32_t>(p,36)=self->state?Field<uint32_t>(self->state,188):0x3f800000;
                Field<uint8_t>(p,41)=(self->flags&0x80)!=0;
                Field<uint8_t>(p,40)=(self->flags&0x200)!=0;
                append(p);
            };
            if(Field<uint8_t>(Manager(),53)&&(!(self->flags&0x80000)||(!(self->flags&0x200000)&&!(self->flags&0x100000)))) weighted(1947853692);
            if(Field<uint8_t>(Manager(),50)) {
                auto p=make(0x87eb0331);
                if(!(self->flags&0x2000)) {
                    struct Iterator { void* array; uint32_t index; } it;
                    CallGameMethod<void,0x00580220>(&it,self->look);
                    if(CallGameMethod<uint8_t,0x00580250>(&it)) {
                        flags&=~2u;
                        auto target=Field<void*>(it.array,8+it.index*4);
                        CallGameMethod<void,0x00581090>(target,p);
                    }
                }
                append(p);
            }
            if(self->host) if(auto entry=Query(self->host,Global<uint32_t>(0x011e90fc))) {
                auto payload=Field<void*>(entry,48);
                if(!(reinterpret_cast<uintptr_t>(payload)&3)&&payload) {
                    uint32_t mode=Field<uint32_t>(payload,56)&0xcf00;
                    uint32_t tag=Field<uint32_t>(payload,56)&0x7f;
                    // EA deliberately dereferences the second query without a null check.
                    auto other=Query(self->host,Global<uint32_t>(0x011e9158));
                    auto otherPayload=Field<void*>(other,48);
                    uint32_t otherMode=0;
                    if(otherPayload) if(auto e=CallGameFunction<void*,0x00582690>(otherPayload)) {
                        auto p=Field<void*>(e,48);
                        if(!(reinterpret_cast<uintptr_t>(p)&3)&&p) otherMode=Field<uint32_t>(p,56)&0xcf00;
                    }
                    auto special=[&]() { return CallGameFunction<uint8_t,0x0055da30>(mode)!=0; };
                    auto configure=[&](void* p,bool extra) {
                        Field<uint32_t>(p,48)=tag; Field<uint32_t>(p,52)=mode; identity(p);
                        Field<uint32_t>(p,56)=otherPayload?Field<uint32_t>(otherPayload,8):0;
                        Field<uint32_t>(p,60)=otherPayload?Field<uint32_t>(otherPayload,12):0;
                        if(extra) for(unsigned k=0;k<4;++k) Field<uint8_t>(p,64+k)=(self->flags&(0x80000u<<k))!=0;
                        append(p);
                    };
                    uint32_t effective=mode?mode:0x100;
                    if(!(self->flags&0x40000)&&((!otherMode&&effective!=0x100)||(effective==0x100&&otherMode==0x200)||((self->flags&0x80000)&&special())))
                        configure(CallGameMethod<void*,0x0093f6c0>(graph),true);
                    if(!special()&&mode!=0x800) configure(CallGameMethod<void*,0x0093f6e0>(graph),false);
                }
            }
            if(Field<uint8_t>(Manager(),52)) weighted(1410982734);
            auto p=make(0xf40282a6); append(p); attach(p,root);
        }
    }
    if(!(flags&1)) { CallGameMethod<void,0x0054c6e0>(graph); return false; }
    if(wasDirty) flags|=4;
    auto& position=Field<int32_t>(stream,0);
    uint32_t start=Field<uint32_t>(stream,20)+static_cast<uint32_t>(position)+1;
    position=static_cast<int32_t>(static_cast<uint32_t>(position)+4);
    if(position>=0) CallGameMethod<void,0x00594140>(stream,4);
    position=static_cast<int32_t>(static_cast<uint32_t>(position)+4);
    if(position>=0) CallGameMethod<void,0x00594140>(stream,4);
    if(position<0) std::memcpy(static_cast<unsigned char*>(Field<void*>(stream,16))+position-3,&flags,4);
    CallGameMethod<void,0x0054c580>(graph,stream);
    CallGameMethod<void,0x0093de70>(stream,start);
    if(position>=0) { position=0; Field<uint8_t>(stream,24)=1; }
    bool result=Field<uint8_t>(stream,24)==0;
    CallGameMethod<void,0x0054c6e0>(graph);
    return result;
#else
    (void)self; (void)stream; NotRewritten("M153 evaluator graph dependencies");
#endif
}
}
