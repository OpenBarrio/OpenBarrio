#include "ts3/audio/m06_B07.h"
#include "ts3/renderer/clu_89f940_Renderer.h"
#include "ts3/deviceconfig/clu_58aba0_DeviceConfig.h"
#include <cstring>
#include <initializer_list>
namespace ts3::audio::b07 {
namespace {
void* Secondary(Manager* self) { return self ? &self->secondary : nullptr; }
void Unregister(void* service, Manager* self, uint32_t id) {
    CallGameVirtual<uintptr_t,11>(service,Secondary(self),id,0xffffd8f1u);
}
uint32_t Load(void* p,size_t off) { uint32_t v; std::memcpy(&v,static_cast<uint8_t*>(p)+off,4); return v; }
uint32_t Limit(Manager* s,uint32_t category) { return s->counts[6+s->tier+3*category]; }
}
uint32_t Category(uint32_t hash) {
    switch(hash) {
    case 0x05174939: return 1;
    case 0x6f394b77: return 2;
    case 0xedf036d6: return 3;
    case 0x687a9528: return 4;
    case 0x5c770db7: return 5;
    case 0x20681cd4: return 6;
    default: return 0;
    }
}
uintptr_t Shutdown(Manager* self) {
    void* service=CallGameFunction<void*,0x0044c470>();
    if(service) for(uint32_t id:{0x01ee1006u,0x01ee1007u,0x01ee1008u,0x01ee1010u,0x038e4e29u}) Unregister(service,self,id);
    service=CallGameFunction<void*,0x007692c0>();
    if(service) for(uint32_t id:{0xfc6aa1feu,0xf0707d65u}) Unregister(service,self,id);
    if(self->service) Unregister(self->service,self,0x03a127dc);
    return CallGameMethod<uintptr_t,0x00868e00>(self);
}
bool Start(Manager* self) {
    void* service=CallGameFunction<void*,0x0044c470>();
    CallGameVirtual<uintptr_t,9>(service,Secondary(self),0x0487c406u,0u,0u);
    if(self->ambience) CallGameMethod<uintptr_t,0x008632d0>(self->ambience,self);
    void* device=ts3::deviceconfig::GetDevice();
    if(device) {
#if defined(_M_IX86)
        using Thread=uint32_t(__stdcall*)();
        uint32_t thread=Load(device,0x2ec);
        if(thread!=(*reinterpret_cast<Thread*>(0x00f95170))()) CallGameFunction<void,0x00c0fd60>(-1);
#else
        AbortNotRewritten("audio device thread synchronization");
#endif
    }
    return true;
}
uintptr_t Stop(Manager* self) {
    void* service=CallGameFunction<void*,0x0044c470>();
    uintptr_t result=CallGameVirtual<uintptr_t,11>(service,Secondary(self),0x0487c406u,0xffffd8f1u);
    if(self->ambience) return CallGameMethod<uintptr_t,0x00862c00>(self->ambience);
    return result;
}
uintptr_t RegisterNodes(Manager* self) {
    void* node=CallGameMethod<void*,0x00eda660>(self->context);
    if(node) {
        CallGameMethod<uintptr_t,0x00ed6590>(node);
#define B07_NODE(addr) CallGameMethod<uintptr_t,0x00ed5a60>(node,CallGameFunction<void*,addr>())
        B07_NODE(0x00eef480); B07_NODE(0x00eef580); B07_NODE(0x00ef0090);
        B07_NODE(0x00ef4ce0); B07_NODE(0x00ef7410); B07_NODE(0x00ef1210);
        B07_NODE(0x008a58c0); B07_NODE(0x008a5530); B07_NODE(0x00eefdf0);
#undef B07_NODE
    }
    node=CallGameMethod<void*,0x00eda610>(self->context);
    if(!node) return 0;
    CallGameMethod<uintptr_t,0x00ed6620>(node);
    return CallGameMethod<uintptr_t,0x00ed16c0>(node,CallGameFunction<void*,0x00eeac40>());
}
uintptr_t SelectName(Manager* self,const uint16_t* name) {
    if(!name) return 0;
    uint16_t text[16];
#if defined(_M_IX86)
    CallGameFunction<uintptr_t,0x004fd3e0>(text,reinterpret_cast<const uint16_t*>(0x010284d4),name);
#else
    AbortNotRewritten("audio UTF16 name format string");
#endif
    uint32_t hash=CallGameFunction<uint32_t,0x004fa640>(text,0x811c9dc5u,0u);
    return CallGameVirtual<uintptr_t,120>(self,hash);
}
bool SetMode(Manager* self,uint32_t mode) {
    const bool selected=(mode==300);
    self->mode=selected;
    if(self->listener) static_cast<uint8_t*>(self->listener)[0xa61]=1;
    return selected;
}
uint32_t ResetCounts(Manager* self) {
    CallGameMethod<uintptr_t,0x008b6650>(self);
    for(unsigned i=0;i<7;++i) self->counts[i]=0;
    return 0;
}
uint32_t Policy(Manager* self,Request* r) {
    if(r->blocked) return 5;
    uint32_t category=Category(r->category);
    r->count=self->counts[category]; r->limit=Limit(self,category);
    if(r->current<r->maximum && r->count<r->limit) return 5;
    if(r->active) return 3;
    return r->kind==0xee8347b0u ? 2 : 4;
}
bool Reserve(Manager* self,void* sound,int32_t delta) {
    if(CallGameMethod<uint32_t,0x008abae0>(sound)) return true;
    uint32_t category=Category(CallGameMethod<uint32_t,0x008ab310>(sound));
    uint64_t key=CallGameVirtual<uint64_t,19>(sound,delta);
    uint32_t remainder=CallGameVirtual<uint32_t,100>(self,key)-uint32_t(delta);
    if(delta>=0) {
        uint32_t maximum=CallGameMethod<uint32_t,0x008ab300>(sound);
        if(remainder>=maximum || self->counts[category]>=Limit(self,category)) return false;
    }
    if(delta) self->counts[category]+=uint32_t(delta);
    return true;
}
uintptr_t Lookup(Manager* self) {
    void* group=CallGameVirtual<void*,108>(self,0xf1cc1687u);
    if(!group) return 0;
    return CallGameMethod<uintptr_t,0x008c1370>(static_cast<uint8_t*>(group)+0x10,0x44616330u);
}
bool CreateSound(Manager*,void** output) {
    if(!output) return false;
#if defined(_M_IX86)
    void* sound=CallGameFunction<void*,0x004e3f90>(0x18d8u,reinterpret_cast<const char*>(0x010284ec),0u,0u,0u,0u);
#else
    void* sound=nullptr;
    AbortNotRewritten("audio Sound allocator label");
#endif
    if(sound) sound=CallGameMethod<void*,0x00871100>(sound);
    *output=sound;
    CallGameVirtual<uintptr_t,0>(sound); // EA also does this on allocation failure.
    return true;
}
void Message(Manager* self,void* message,uint32_t entryScratch) {
    // 00B9C560 is the empty stdcall notification helper (ret 4), no side effects.
    uint32_t type=CallGameMethod<uint32_t,0x0089f6c0>(message);
    if(type==0xa935b29eu) {
        auto* reader=static_cast<ts3::renderer::RendererRecordReader*>(message);
        uint32_t index=0,value=entryScratch;
        reader->ReadScalar(0x0490d2eb,&index);
        CallGameMethod<bool,0x0089f9f0>(message,0x3c91e635u,&self->values[index]);
        reader->ReadScalar(0xd23d1bd7,&self->words[index]);
        reader->ReadScalar(0xc63adaf5,&value);
        self->flags[index]=value;
    } else if(type==0xada39292u || type==0xe69030b6u) {
        uint64_t key=0;
        CallGameMethod<bool,0x0089f980>(message,0x39efd85fu,&key);
        if(self->ambience) {
            if(type==0xada39292u) CallGameMethod<uintptr_t,0x00862e00>(self->ambience,self,key);
            else CallGameMethod<uintptr_t,0x00862d30>(self->ambience,self,key);
        }
    } else if(type==0xea7fbba5u && self->ambience) CallGameMethod<uintptr_t,0x00863580>(self->ambience,self);
}
#if defined(_M_IX86)
// EA's local at entry ESP-8 is never initialized in the budget-message branch.
// Capture it before the C++ frame/counter wrapper can reuse that stack memory.
__declspec(naked) void __fastcall HookMessage(Manager*,void*,void*) {
    __asm {
        mov eax, [esp-8]
        push eax
        push dword ptr [esp+8]
        push ecx
        call Message
        add esp, 12
        ret 4
    }
}
#endif
// Adapted from the verified exact-tail-thunk twins: bridge only the distinct target/ABI.
bool BaseOperation(Manager* self,void* message) { return CallGameMethod<bool,0x008bf6d0>(self,message); }
}
