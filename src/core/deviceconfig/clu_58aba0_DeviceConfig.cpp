#include "ts3/deviceconfig/clu_58aba0_DeviceConfig.h"
#include <cstdlib>
#include <cwchar>
#include <initializer_list>
#include <cstring>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
namespace ts3::deviceconfig {
namespace {
template<class T> T Global(unsigned address) {
#if defined(_M_IX86)
    return *reinterpret_cast<T*>(static_cast<uintptr_t>(address));
#else
    (void)address; AbortNotRewritten("DeviceConfig global"); return {};
#endif
}
const void* Section() { return Global<const void*>(0x114FAB0); }
}
// 0x0058ABA0: only AL of the lookup result determines success.
int32_t ReadInteger(const void* section,const char16_t* key,int32_t fallback) {
    auto config=CallGameFunction<void*,0x58B880>(); const char16_t* text;
    if (!config || !CallGameMethod<uint8_t,0x58C380>(config,section,key,&text)) return fallback;
    // Windows' wtol contract is a 32-bit long; char16_t keeps game text UTF-16.
    const int32_t value=static_cast<int32_t>(std::wcstol(reinterpret_cast<const wchar_t*>(text),nullptr,10));
    return CallGameFunction<int32_t,0x58AA80>(section,key,value);
}
// 0x0058E3F0
DeviceState* GetDevice() { return Global<DeviceState*>(0x11CDD10); }
// 0x0058E1F0
uintptr_t NotifyDevice(void* self) {
    auto sink=CallGameMethod<void*,0x44C470>(self);
    return sink ? CallGameVirtual<uintptr_t,7>(sink,int32_t(0xDD840F2E),0,0,0) : 0;
}
// 0x00692F90: command header + polymorphic payload, aligned as the original stack.
uint8_t DispatchMemory(uint32_t first,uint32_t second) {
    struct Command { uint32_t id; void* payload; uint32_t a,b; uint32_t words[16]; };
    TS3_X86_OFFSET(Command,words,16);
    alignas(16) Command cmd;
    cmd.id=0x59EF8B87; cmd.a=cmd.b=0;
#if defined(_M_IX86)
    cmd.payload=reinterpret_cast<void*>(0x115892C);
#else
    AbortNotRewritten("memory command payload"); cmd.payload=nullptr;
#endif
    cmd.words[0]=first; cmd.words[1]=second;
    auto dispatcher=CallGameFunction<void*,0x5934C0>();
    const uint8_t result=dispatcher!=nullptr;
    if (dispatcher) CallGameMethod<void,0x593E60>(dispatcher,&cmd);
    if (cmd.payload) CallGameVirtual<void,0>(cmd.payload,cmd.words);
    return result;
}
// 0x006E1F90: EA uses twice SMALL, not LARGE, in the final four registrations.
int32_t ShaderSettings::RegisterSizes(void* target) {
    if (!small) small=ReadInteger(Section(),u"RttSizeSmall",256);
    if (!large) large=ReadInteger(Section(),u"RttSizeLarge",512);
    auto put=[&](uint32_t a,uint32_t b,uint32_t size) { return CallGameMethod<int32_t,0x6E1EA0>(this,target,a,b,size); };
    put(0x21E9CD2,0x21E9CD3,small); put(0x21E9CD4,0x21E9CD5,small);
    for (uint32_t i=0;i<4;++i) put(0x3C58092+i,0x3C58096+i,small);
    int32_t result=static_cast<int32_t>(large);
    if (small!=large) {
        put(0x21E9CD6,0x21E9CD7,large); put(0x21E9CD8,0x21E9CD9,large);
        for (uint32_t i=0;i<4;++i) result=put(0x4765A41+i,0x4765A45+i,small*2u);
    }
    return result;
}
// 0x006E31E0
int32_t ShaderSettings::Refresh() {
    const auto level=ReadInteger(Section(),u"FSAALevel",0);
    auto renderer=CallGameFunction<uint8_t*,0x60E2C0>();
    auto current=[&]() { int32_t v; std::memcpy(&v,renderer+56,4); return v; };
    if (level!=current()) {
        auto actual=CallGameMethod<int32_t,0x60E5F0>(renderer,level);
        if (actual!=current()) {
            CallGameMethod<void,0x60E650>(renderer,actual,1);
            CallGameMethod<void,0x6115C0>(renderer);
        }
    }
    CallGameMethod<void,0x6E19D0>(this,0);
    return CallGameMethod<int32_t,0x6E2760>(this,0);
}
// 0x006E34B0: interface this is base+12; return contract is AL only.
uint8_t ShaderListener::OnEvent(int32_t event,const void* payload) {
    if (static_cast<uint32_t>(event)==0xFC6AA1FEu) {
        if (payload) {
            uint32_t pid; std::memcpy(&pid,static_cast<const uint8_t*>(payload)+8,4);
            if (pid!=GetCurrentProcessId()) CallGameFunction<void,0xC0FD60>(-1);
        }
        reinterpret_cast<ShaderSettings*>(reinterpret_cast<uint8_t*>(this)-12)->Refresh();
    }
    return 1;
}
// 0x007274D0
uint8_t ResourceSettings::Initialize() {
    for (int i : {0,3,1,2}) {
        // All pending fields are four-byte counters, not embedded pointers.
        InterlockedExchange(reinterpret_cast<volatile LONG*>(&pending[i]),0);
    }
    first=second=0; limit=ReadInteger(u"ResourceSystem",u"MemoryUsageLimit",20971520);
    service=CallGameVirtual<void*,5>(CallGameFunction<void*,0x599160>(),2,1,5);
    return service!=nullptr;
}
// 0x00740D80
uint8_t GetResolutionList(uint32_t* a,uint32_t* b,uint32_t* c,uint8_t* flag) {
    auto d=GetDevice(); if (!d) return 0;
    *a=d->resolution[0]; *b=d->resolution[1]; *c=d->resolution[2];
    *flag=d->resolutionFlag; return d->resolutionFlag;
}
// 0x00740DC0
int32_t GetCurrentResolution() { auto d=GetDevice(); return d?d->currentResolution:0; }
// 0x00740DE0
int32_t GetErrorCode() { auto d=GetDevice(); return d?d->error:-1; }
// 0x00740E00
uint8_t GetErrorLine() { auto d=GetDevice(); return d?d->errorLine:0; }
// 0x00740E20
uint8_t IsCardRecognized() {
    auto d=GetDevice(); if (!d) return 0;
    if (d->process!=GetCurrentProcessId()) CallGameFunction<void,0xC0FD60>(-1);
    return d->recognized;
}
// 0x00740E60
int32_t IsCardSupported() { return ReadInteger(Section(),u"ReportOldGraphicsDriver",0)!=0; }
// 0x00740E80
uint8_t IsAASupported() { auto d=GetDevice(); return d?d->aa:0; }
// 0x00740EA0
int32_t IsDriverOld() { return ReadInteger(Section(),u"DisallowEdgeSmoothing",0)==0; }
// 0x00740EC0
uint8_t IsMac() { auto d=GetDevice(); return d?d->mac:0; }
// 0x00740ED0: out-of-range tail goes into placement-new (returns second argument).
int32_t SetOption(uint32_t key,int32_t fallback) {
    auto strings=CallGameFunction<void*,0x7692B0>();
    auto name=CallGameVirtual<const char16_t*,22>(strings,key);
#if defined(_M_IX86)
    const auto section=reinterpret_cast<const void*>(0xFCC908);
#else
    AbortNotRewritten("DeviceConfig option section");
    const void* section=nullptr;
#endif
    return name?ReadInteger(section,name,fallback):fallback;
}
// 0x00740F70
uintptr_t SetProperty() {
    auto d=GetDevice(); if (!d) return 0;
    CallGameMethod<void,0x58E7D0>(d); return NotifyDevice(d);
}
// 0x00740F90: actual tail target differs from misleading overlapping decompile.
uintptr_t ApplyOptions() {
    auto d=GetDevice(); return d?CallGameMethod<uintptr_t,0x58E450>(&d->optionsVtable):0;
}
// 0x00741130
int32_t IsNewConfiguration() { auto d=GetDevice(); return d?static_cast<int32_t>(std::atol(d->configuration)):0; }
// 0x00741150
int32_t GetProcessorLevel() { auto d=GetDevice(); return d?static_cast<int32_t>(std::atol(d->processor)):0; }
namespace {
template<unsigned Address> uint8_t Lookup(uint32_t name,void* out) {
    auto strings=CallGameFunction<void*,0x7692B0>();
    auto text=CallGameVirtual<const char*,22>(strings,name); if (!text) return 0;
    auto d=GetDevice();
    struct Text { char16_t* begin; char16_t* end; char16_t* capacity; uint32_t extra; } textBuffer;
    TS3_X86_SIZE(Text,16);
    auto converted=CallGameFunction<Text*,0x4F38C0>(&textBuffer,text,-1,0);
    auto result=CallGameMethod<uint8_t,Address>(&d->optionsVtable,converted->begin,out);
    // Signed byte difference, not element count, controls the original free.
    auto span=static_cast<intptr_t>(reinterpret_cast<uintptr_t>(textBuffer.capacity)-reinterpret_cast<uintptr_t>(textBuffer.begin));
    if (span>1 && textBuffer.begin) CallGameFunction<void,0x4E3F70>(textBuffer.begin);
    return result;
}
}
// 0x00741230
uint8_t GetVideoCardLevel(uint32_t name,void* out) { return Lookup<0x58ED70>(name,out); }
// 0x007412B0
uint8_t GetOption(uint32_t name,void* out) { return Lookup<0x58EDE0>(name,out); }
// 0x00741330
uint8_t GetOptionDefault(uint32_t name,void* out) { return Lookup<0x58EC20>(name,out); }
// 0x007F15C0: scalar copy order intentionally follows EA for aliasing callers.
uint8_t ResolutionSettings::CopyResolution() {
    auto d=GetDevice(); if (!d) return 0;
    for (int i=0;i<8;++i) resolution[i]=d->resolution[i];
    flag=d->resolutionFlag; return flag;
}
// 0x009164F0: retain unsigned-byte quality comparison and the sentinel traversal.
int32_t SwarmSettings::RefreshQuality() {
    const auto old=quality;
    int32_t result=ReadInteger(Section(),u"SwarmQuality",3); quality=static_cast<uint8_t>(result);
    if (old!=quality) {
        for (int group=0;group<96;++group) {
            auto bucket=buckets; while (!*bucket) ++bucket;
            auto node=*bucket; const auto sentinel=buckets[bucketCount];
            while (node!=sentinel) {
                static_cast<SwarmGroup*>(node->value)->Update(group);
                node=node->next;
                while (!node) node=*++bucket;
            }
            result=group+1;
        }
    }
    return result;
}
// 0x0091BA60
int32_t SwarmGroup::Update(int32_t group) {
    for (auto it=begin;it!=end;++it) {
        auto item=static_cast<uint8_t*>(items[*it]);
        if (group<0 || group==static_cast<int8_t>(item[59])) {
            CallGameMethod<void,0x91A570>(this,item); CallGameMethod<void,0x91B990>(this,item);
        }
    }
    return CallGameMethod<int32_t,0x91A760>(this,1);
}
// 0x009638B0
#if defined(_M_IX86)
uint8_t __stdcall SetMemoryLimits(uint8_t enabled) {
#else
uint8_t SetMemoryLimits(uint8_t enabled) {
#endif
    uint32_t second=0x7FFFF000;
    if (enabled) {
        auto mode=ReadInteger(Section(),u"MapViewShadowMode",1);
        if (mode==1) second=0x7BFFF000;
        else if (mode==2) second=0;
    }
    return DispatchMemory(0x7FFFF000,second);
}
// 0x00C60C00: disassembly stores the returned float, not the decompiler's literal 1.
void LotSettings::RefreshLots() {
    maxLots=ReadInteger(Section(),u"MaxActiveLots",4);
    uint32_t platform=ReadInteger(Section(),u"PlatformMaxActiveLots",4);
    if (maxLots>platform) maxLots=platform;
    cameraThreshold=CallGameFunction<float,0x58ABF0>(Section(),u"CameraSpeedThreshold",1.0f);
    if (service) CallGameMethod<void,0xC0B4C0>(service);
}
}
