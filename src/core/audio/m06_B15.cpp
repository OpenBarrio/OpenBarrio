#include "ts3/audio/m06_B15.h"
#include <intrin.h>
namespace ts3::audio {
namespace {
void* RWSystem() {
#if defined(_M_IX86)
    return *reinterpret_cast<void**>(0x011FFC28);
#else
    AbortNotRewritten("B15 RW singleton"); return nullptr;
#endif
}
void SetTables(B15Player* self) {
#if defined(_M_IX86)
    self->primary=reinterpret_cast<void*>(0x010293F8);
    self->secondary=reinterpret_cast<void*>(0x010293E8);
    self->tertiary=reinterpret_cast<void*>(0x010293E4);
#else
    AbortNotRewritten("B15 original interface tables");
#endif
}
}
bool B15SelectMode(B15Player* self) {
    self->mode=2;
    return true;
}
float B15Position(B15Player* self) {
    if (!self->voice) return 0.0f;
    if (auto* system=RWSystem()) CallGameMethod<void,0x00ED7170>(system);
    double position;
    CallGameMethod<void,0x00ED5A00>(self->voice,0,&position);
    const float rounded=static_cast<float>(position);
    if (auto* system=RWSystem()) CallGameMethod<void,0x00ED7190>(system);
    return rounded;
}
uint32_t B15Property(B15Player* self) {
    return CallGameMethod<uint32_t,0x008C1370>(self->system,0x50506C30u);
}
uintptr_t B15Release(B15Packet** slot) {
    B15Packet* packet=*slot;
    if (!packet) return 0;
    const auto address=reinterpret_cast<uintptr_t>(&packet->refs);
    if (_InterlockedDecrement(&packet->refs)!=0) return address;
    _InterlockedExchange(&packet->refs,1);
#if defined(_M_IX86)
    using Delete=uintptr_t(__thiscall*)(void*,uint32_t);
#else
    using Delete=uintptr_t(*)(void*,uint32_t);
#endif
    return reinterpret_cast<Delete>(packet->secondary[0])(&packet->secondary,1);
}
void B15Clear(B15Node* sentinel) {
    auto* node=sentinel->next;
    while(node!=sentinel) {
        auto* next=node->next;
        B15Release(&node->packet);
        CallGameFunction<void,0x004E3F70>(node);
        node=next;
    }
}
B15Player* B15DestroySecondary(void* self,uint32_t flags) {
    // Adapted from ResourceBaseDestroySecondary (0x004A3A90); this twin adjusts by 8, not 4.
    return B15Destroy(reinterpret_cast<B15Player*>(static_cast<uint8_t*>(self)-2*sizeof(void*)),flags);
}
B15Player* B15Construct(B15Player* self) {
    CallGameMethod<void,0x008A2820>(self);
    SetTables(self);
    auto* sentinel=&self->packets;
#if defined(_M_IX86)
    sentinel->packet=reinterpret_cast<B15Packet*>(0x010294AC);
#endif
    sentinel->next=sentinel;
    sentinel->previous=sentinel;
    self->started=0;
    self->stopped=0;
    self->notified=0;
    return self;
}
B15Player* B15Destroy(B15Player* self,uint32_t flags) {
    SetTables(self);
    B15Clear(&self->packets);
    CallGameMethod<void,0x008A2560>(self);
    if(flags&1) CallGameFunction<void,0x004E3F70>(self);
    return self;
}
B15Cache* B15InitCache(B15Cache* self) {
    self->low=0;
    self->high=0;
    self->result=0;
    return self;
}

uint32_t B15PlaylistTick(B15Playlist* self,uint32_t unused,float elapsed) {
    (void)unused;
    uint32_t result=self->flags;
    if(result&0x400) {
        if(result&4) {
            self->flags&=~4u;
            void* commands=CallGameFunction<void*,0x008B0A70>();
            return CallGameVirtual<uint32_t,33>(commands,reinterpret_cast<uintptr_t>(self)==4?nullptr:self);
        }
        return result;
    }
    if(result&0x2000) {
        CallGameFunction<void,0x00F02F10>();
        if(result&0x1000) CallGameMethod<void,0x00F01D10>(self->player);
        else CallGameMethod<void,0x00F01D90>(self->player);
        result=CallGameFunction<uint32_t,0x00F02F20>();
        self->flags&=~0x2000u;
    }
    if(!(self->flags&0x1000)) {
        CallGameFunction<void,0x00F02F10>();
        uint32_t state;
        CallGameMethod<void,0x00F02020>(self->player,&state);
        CallGameFunction<void,0x00F02F20>();
        auto* primary=reinterpret_cast<uint8_t*>(self)-sizeof(void*);
        if(state==3) CallGameMethod<void,0x0086AB60>(primary);
        return CallGameMethod<uint32_t,0x00869D20>(primary,elapsed,0);
    }
    return result;
}
namespace {
void SetVoice(void* voice,uint32_t property,void* value) {
    CallGameMethod<void,0x00ED5A20>(voice,property,value);
}
void StopVoice(B15Player* self) {
    if(self->started && self->voice) {
        if(auto* rw=RWSystem()) CallGameMethod<void,0x00ED7170>(rw);
        SetVoice(self->voice,1,nullptr);
        if(auto* rw=RWSystem()) CallGameMethod<void,0x00ED7190>(rw);
    }
}
void* Payload(B15Packet* packet) { return const_cast<long*>(&packet->refs)+1; }
struct PacketQuery { void* payload; float done; };
void PopPacket(B15Player* self) {
    auto* node=self->packets.next;
    node->previous->next=node->next;
    node->next->previous=node->previous;
    B15Release(&node->packet);
    CallGameFunction<void,0x004E3F70>(node);
}
void NotifyStopped(B15Player* self) {
    void* bus=CallGameFunction<void*,0x0044C470>();
    auto* message=CallGameFunction<uint8_t*,0x004E3F90>(12,"Video/AudioStoppedMessage",0,0,0,0);
    if(message) {
#if defined(_M_IX86)
        *reinterpret_cast<void**>(message)=reinterpret_cast<void*>(0x0100DF80);
#else
        AbortNotRewritten("B15 message vtable");
#endif
        _InterlockedExchange(reinterpret_cast<volatile long*>(message+sizeof(void*)),0);
        CallGameVirtual<void,1>(message);
    }
    // EA dereferences a failed allocation here, intentionally not guarded.
    *reinterpret_cast<uint32_t*>(message+sizeof(void*)+sizeof(long))=self->handle;
    auto* video=CallGameFunction<uint8_t*,0x009D7F30>();
    CallGameVirtual<void,7>(bus,0x1EAB13F0u,message,0,video?video+4*sizeof(void*):nullptr);
    self->notified=2;
    CallGameVirtual<void,2>(message);
}
}
bool B15Event(B15Player* self,uint32_t event,uint32_t arg,uintptr_t token) {
    switch(event) {
    case 0x201D0413: self->stopped=0;self->notified=0;return true;
    case 0xF183EA41: StopVoice(self);self->started=0;[[fallthrough]];
    case 0x0775F209: self->stopped=1;if(self->notified!=2) self->notified=1;return true;
    case 0xB28D0AD1: {
        auto* video=CallGameFunction<void*,0x009D7F30>();
        if(!video) return true;
        auto* packet=CallGameMethod<B15Packet*,0x009D8500>(video,token);
        if(!packet) { B15Release(&packet);return true; }
        _InterlockedIncrement(&packet->refs);
        if(!self->started && !self->stopped) {
            self->started=1;
            uint32_t rate=CallGameMethod<uint32_t,0x009D76C0>(packet);
            int32_t channels=CallGameMethod<int32_t,0x009D76B0>(packet);
            self->mode=channels;
            float format[]={0,static_cast<float>(rate),static_cast<float>(channels)};
            if(auto* rw=RWSystem()) CallGameMethod<void,0x00ED7170>(rw);
            SetVoice(self->voice,0,format);
            if(auto* rw=RWSystem()) CallGameMethod<void,0x00ED7190>(rw);
        }
        if(!self->stopped) {
            CallGameMethod<void,0x009D76D0>(packet,1);
            auto* node=CallGameFunction<B15Node*,0x004E3F90>(sizeof(B15Node),self->packets.packet,0,0,reinterpret_cast<const void*>(0x00F98980),0xE7);
            node->packet=packet;_InterlockedIncrement(&packet->refs);
            node->next=&self->packets;node->previous=self->packets.previous;
            self->packets.previous->next=node;self->packets.previous=node;
            void* payload=Payload(packet);
            if(auto* rw=RWSystem()) CallGameMethod<void,0x00ED7170>(rw);
            SetVoice(self->voice,2,&payload);
#if defined(_M_IX86)
            auto* rw=static_cast<uint8_t*>(RWSystem());
            CallGameMethod<void,0x006CABF0>(packet,*reinterpret_cast<uint32_t*>(rw+0xE8));
#else
            AbortNotRewritten("B15 RW clock");
#endif
            // If the clock callback clears the singleton, EA skips unlock (but still releases).
            if(auto* system=RWSystem()) CallGameMethod<void,0x00ED7190>(system);
        } else CallGameMethod<void,0x009D76D0>(packet,2);
        B15Release(&packet);return true;
    }
    default: return CallGameMethod<bool,0x008A17C0>(self,event,arg,token);
    }
}
bool B15Stop(B15Player* self) {
    self->stopped=1;
    StopVoice(self);
    if(self->notified==2) return true;
    void* saved=RWSystem();
    CallGameMethod<void,0x00ED7190>(saved);
    while(self->packets.next!=&self->packets) {
        auto* packet=self->packets.next->packet;
        auto* rw=RWSystem();
        uint32_t sequence=CallGameMethod<uint32_t,0x00580420>(packet);
        PacketQuery query{packet?Payload(packet):nullptr,0};
        if(auto* system=RWSystem()) CallGameMethod<void,0x00ED7170>(system);
        bool done=false;
        if(CallGameMethod<bool,0x00ED71B0>(rw,sequence)) {
            SetVoice(self->voice,3,&query);
            done=query.done==1.0f; // unordered is not equal (ucomiss/lahf parity path).
        }
        if(auto* system=RWSystem()) CallGameMethod<void,0x00ED7190>(system);
        if(done) {CallGameMethod<void,0x009D76D0>(packet,2);PopPacket(self);}
        else {uint32_t delay=10;CallGameFunction<void,0x004E1320>(&delay);}
    }
    CallGameMethod<void,0x00ED7170>(saved);
    NotifyStopped(self);
    return true;
}
uint32_t B15Tick(void* secondary,float first,float second) {
    auto* self=reinterpret_cast<B15Player*>(static_cast<uint8_t*>(secondary)-sizeof(void*));
    while(self->packets.next!=&self->packets) {
        auto* packet=self->packets.next->packet;
        if(!packet) break;
        if(self->voice) {
            auto* rw=RWSystem();
            uint32_t sequence=CallGameMethod<uint32_t,0x00580420>(packet);
            if(!CallGameMethod<bool,0x00ED71B0>(rw,sequence)) break;
            PacketQuery query{Payload(packet),0};
            if(auto* system=RWSystem()) CallGameMethod<void,0x00ED7170>(system);
            SetVoice(self->voice,3,&query);
            if(auto* system=RWSystem()) CallGameMethod<void,0x00ED7190>(system);
            if(query.done==0.0f) break; // NaN is unequal and retires, just as EA does.
        }
        CallGameMethod<void,0x009D76D0>(packet,2);PopPacket(self);
    }
    if(self->packets.next==&self->packets && self->notified==1) NotifyStopped(self);
    return CallGameMethod<uint32_t,0x008A18B0>(secondary,first,second);
}
uint8_t B15Query(B15Cache* self,const B15KeySource* source) {
    bool enabled=false;
#if defined(_M_IX86)
    enabled=CallGameMethod<bool,0x00523860>(reinterpret_cast<void*>(0x011CDDE0),0x8000);
#else
    AbortNotRewritten("B15 query service");
#endif
    if(!enabled || !source) return false;
    if(source->low==self->low && source->high==self->high) return self->result;
    self->low=source->low;self->high=source->high;self->result=0;
#if defined(_M_IX86)
    auto* actor=CallGameFunction<void*,0x00935770>(*reinterpret_cast<void**>(0x011E9158));
    if(!actor) return false;
    alignas(16) float position[4];
    CallGameMethod<void,0x00959000>(actor,position);
    auto* world=*reinterpret_cast<uint8_t**>(0x011ECBC4);
    if(!world) return false;
    auto* spatial=*reinterpret_cast<void**>(world+0x74);
    if(!spatial) return false;
    uint16_t mode=CallGameMethod<uint16_t,0x00D699A0>(spatial);
    if(mode!=1 && mode!=2) return false;
    uint8_t detail=0;
    // NaN is accepted: x87 fcompi's unordered flags do not take JAE.
    if(CallGameMethod<float,0x00D6E170>(spatial,position,&detail)<=0.0f) return false;
    self->result=1;return true;
#else
    return false;
#endif
}
}
