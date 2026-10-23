#include "ts3/audio/m06_B34.h"
#include <cstring>
namespace ts3::audio {
namespace {
void* System() {
#if defined(_M_IX86)
    return *reinterpret_cast<void**>(0x011ffc28);
#else
    return nullptr;
#endif
}
void Lock() { if(auto p=System()) CallGameMethod<void,0x00ed7170>(p); }
void Unlock() { if(auto p=System()) CallGameMethod<void,0x00ed7190>(p); }
void* Commands() { return CallGameFunction<void*,0x008b0a70>(); }
void Set(void* p,uint32_t key,void* data) { CallGameMethod<void,0x00ed5a20>(p,key,data); }
void Release(void* p) { CallGameMethod<void,0x00ec17a0>(p); }
}
float B34Emitter::ReadPosition() {
    if (!player) return 0.0f;
    Lock();
    double value;
    CallGameMethod<void,0x00ed5a00>(player,1u,&value);
    volatile float rounded=static_cast<float>(value);
    Unlock();
    return rounded;
}
uint32_t B34Emitter::SetParameter(uint32_t k,void* value,uint32_t) {
    uint32_t result=k;
    if (k==0x2fe09c83) {
        auto resource=CallGameVirtual<void*,69>(Commands(),value);
        CallGameMethod<void,0x008c1e40>(voices,resource);
        result=CallGameMethod<uint32_t,0x008c1d40>(voices);
    } else if (k==0x6e3d22e0) {
        auto properties=reinterpret_cast<uint8_t*>(this)+0x10;
        result=CallGameMethod<uint32_t,0x008aafe0>(properties,value,0u);
    }
    return (result & 0xffffff00u)|1u;
}
void* B34Emitter::GetVoices() { return voices; }
bool B34Emitter::SetVoices(void* value) {
    if (!value) return false;
    auto old=voices;
    if (value!=old) {
        CallGameMethod<void,0x005044c0>(value);
        voices=value;
        if(old) Release(old);
    }
    return true;
}
void B34Emitter::SetSource(void* value) {
    auto old=source;
    if (value!=old) {
        if(value) CallGameVirtual<void,0>(value);
        source=value;
        if(old) CallGameVirtual<void,1>(old);
    }
}
void B34Emitter::Tick(uint32_t,float delta) {
    // This entry is installed on the secondary interface, one pointer past primary.
    auto p=reinterpret_cast<B34Emitter*>(reinterpret_cast<uint8_t*>(this)-sizeof(void*));
    if (p->flags&0x80) { CallGameVirtual<void,31>(p); return; }
    auto descriptor=*reinterpret_cast<uint8_t**>(reinterpret_cast<uint8_t*>(p->voices)+2*sizeof(void*));
    bool stopped=descriptor[0x47]==2;
    if ((p->streaming || (p->flags&0x800)) && !stopped) {
        Lock();
        struct Query { float gain; float state; } q;
        // EA reads +4DC, not the submitted packet's gain at +4D8.
        q.gain=p->queryGain;
        Set(p->player,2,&q);
        stopped=q.state!=0.0f;
        Unlock();
    }
    if (stopped) {
        if (p->streaming && p->enabled && !p->dirty && (p->flags&(8|0x200))) return;
        CallGameVirtual<void,85>(Commands(),0x03a0fd6eu,p->handle,0u,0u,0u);
        CallGameVirtual<void,6>(p);
        return;
    }
    if (!p->enabled || p->dirty) CallGameVirtual<void,30>(p,delta,0u);
}
void B34Emitter::Update(uint32_t,uint32_t force) {
    if (!(flags&0x20)) return;
    Lock();
    if (dirty) {
        dirty=0;
        if(fader) CallGameMethod<void,0x00edaf00>(fader,0u,enabled?1.0f:0.0f);
    }
    const bool all=static_cast<uint8_t>(force)!=0;
    if ((flags&4) && (all || (flags&0x40))) CallGameVirtual<void,28>(this,spatial);
    if(changed&(0x100|1)) {
        changed&=~0x100u;
        float value=scale*volume;
        CallGameMethod<void,0x008c1040>(voices,0x49eb68dbu,value);
    }
    if(all || (changed&0x81)) {
        effectiveVolume=(flags&0x100)?attenuation*volume:volume;
        CallGameVirtual<void,27>(this,effectiveVolume);
    }
    if(all || (changed&0xc)) CallGameVirtual<void,26>(this,mode>=2?distance*pitch:pitch);
    if(all || (changed&8)) CallGameVirtual<void,29>(this,distance);
    Unlock();
}
bool B34Emitter::SubmitPacket() {
    if(!player) return false;
    Lock();
    Set(player,5,&packet);
    Unlock();
    return true;
}
uint32_t B34Emitter::VoiceScalar() { return CallGameMethod<uint32_t,0x008c1370>(voices,0x536e5031u); }
bool B34Emitter::Reset() {
    Lock();
    if(auto s=System()) timestamp=*reinterpret_cast<uint32_t*>(static_cast<uint8_t*>(s)+0xe8);
    if(auto old=voices) { voices=nullptr; Release(old); }
    flags&=~0x10u;
    player=node410=node414=node418=fader=nullptr;
    Unlock();
    return true;
}
bool B34Emitter::ActivateFirst() {
    auto nodes=*reinterpret_cast<void***>(static_cast<uint8_t*>(voices)+3*sizeof(void*));
    Set(*nodes,1,nullptr);
    return true;
}
void B34Emitter::SetMode(int32_t value) {
    if(mode==value) return;
    uint32_t first=0,second=0;
    CallGameVirtual<void,110>(Commands(),0u,&first,&second);
    if(value==1) CallGameVirtual<void,19>(source,0xdf59188du,&second,&first);
    else if(value==2) CallGameVirtual<void,19>(source,0xb5760a90u,&second,&first);
    CallGameMethod<void,0x008c1220>(voices,first,second);
    if(mode==1) CallGameMethod<void,0x008c1d40>(voices);
    CallGameMethod<void,0x008c0f50>(voices);
    mode=value;
}
uintptr_t B34FormatName(const uint32_t* key,int32_t size,char* buffer) {
    struct Name { const wchar_t* begin; const wchar_t* end; const wchar_t* capacity; const char* label; } name;
#if defined(_M_IX86)
    name.begin=name.end=reinterpret_cast<const wchar_t*>(0x011c7e54);
    name.capacity=reinterpret_cast<const wchar_t*>(0x011c7e56);
    name.label=reinterpret_cast<const char*>(0x010296a8);
#else
    static const wchar_t empty=0;
    name.begin=name.end=&empty;name.capacity=&empty+1;name.label="Audio/String";
#endif
    auto manager=CallGameFunction<void*,0x004afd20>();
    CallGameVirtual<void,33>(manager,key,&name);
    bool bang=false;
    for(auto p=name.begin;p!=name.end;++p) if(*p==L'!') {bang=true;break;}
    int n;
    // Game snprintf is variadic cdecl: preserve its size-1 limit and conditional terminator.
    if(bang) n=CallGameFunction<int,0x004fbf60>(buffer,size-1,"streamable:%ls",name.begin);
    else n=CallGameFunction<int,0x004fbf60>(buffer,size-1,"streamable:0x%08x!%ls",key[3],name.begin);
    if(n>=size) buffer[size-1]=0;
    auto result=reinterpret_cast<uintptr_t>(name.begin);
    auto bytes=(reinterpret_cast<uintptr_t>(name.capacity)-result)&~uintptr_t(1);
    if(static_cast<intptr_t>(bytes)>2 && name.begin) result=CallGameFunction<uintptr_t,0x004e3f70>(name.begin);
    return result;
}
bool B34Emitter::PreparePacket() {
    if(streaming) {
        uint32_t resource[4]={key[0],key[1],0x01eef63a,key[3]};
        B34FormatName(resource,128,name);
        packet.gain=0.0f;
    } else packet.gain=(flags&0x800)?0.0f:1.0f;
    packet.context=context;
    packet.name=streaming?name:nullptr;
    packet.position=position;
    packet.word=word3b0;
    packet.zero=0.0;
    packet.source=metadata?*reinterpret_cast<uint32_t*>(static_cast<uint8_t*>(metadata)+0x18):0;
    packet.rate=static_cast<double>(rate);
    if(!(flags&0x800)) { Lock();Set(player,5,&packet);Unlock(); }
    return true;
}
}
