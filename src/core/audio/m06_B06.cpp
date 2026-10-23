#include "ts3/audio/m06_B06.h"
#include "ts3/world/clu_4ec290_World.h"
#include "ts3/cas/clu_4a6710_CAS.h"
#include <cstring>
#include <cstdlib>
#include <atomic>
#include <string>
#include <initializer_list>
namespace ts3::audio {
namespace {
template<class R, int Slot, class... A> R V(void* p, A... a) { return CallGameVirtual<R,Slot>(p,a...); }
void* Allocate(size_t n, const char* label, uint32_t line=0) {
#if defined(_M_IX86)
    return CallGameFunction<void*,0x004E3F90>(uint32_t(n),label,0u,0u,line ? reinterpret_cast<const char*>(0x00F98980) : nullptr,line);
#else
    (void)label; (void)line; return std::malloc(n);
#endif
}
void CopyName(B06String& dest, const uint16_t* source) {
    dest={};
    cas::CopyUtf16(reinterpret_cast<uint16_t*>(&dest),source,250u);
}
void Free(void* p) {
#if defined(_M_IX86)
    CallGameFunction<void,0x004E3F70>(p);
#else
    std::free(p);
#endif
}
void* CommandSystem() {
#if defined(_M_IX86)
    return CallGameFunction<void*,0x008B0A70>();
#else
    AbortNotRewritten("B06 command-system singleton"); return nullptr;
#endif
}
void KeyCommand(void* p, uint32_t op, uint32_t low, uint32_t high) {
    V<void,17>(p,op); V<void,20>(p,0x39efd85fu,low,high); V<void,26>(p);
}
B06Node* Find(B06Map& m, uint32_t low, uint32_t high) {
    B06Node* out=nullptr; uint32_t key[2]={low,high};
    CallGameMethod<void,0x00D726F0>(&m,&out,key);
    return out;
}
void ClearMap(B06Map& m) {
    CallGameMethod<void,0x00700C60>(&m,m.buckets,m.bucketCount); m.size=0;
}
}
void B06Strings::Insert(void* positionRaw, const void* valueRaw) {
    auto position=static_cast<B06String*>(positionRaw);
    auto value=static_cast<const B06String*>(valueRaw);
    if(end!=capacity) {
        if(value>=position && value<end) ++value;
        if(end) *end=end[-1];
        for(auto p=end-1;p!=position;--p) *p=p[-1];
        *position=*value;
        ++end;
        return;
    }
    const auto count=begin ? end-begin : 0;
    const auto newCount=count ? count*2 : 1;
    auto memory=static_cast<B06String*>(Allocate(size_t(newCount)*sizeof(B06String),label,0xe7));
    auto out=memory;
    for(auto p=begin;p!=position;++p,++out) if(out) *out=*p;
    if(out) *out=*value;
    ++out;
    for(auto p=position;p!=end;++p,++out) if(out) *out=*p;
    if(begin) Free(begin);
    begin=memory; end=out; capacity=memory+newCount;
}
void B06Strings::Append(const void* rawValue) {
    auto value=static_cast<const B06String*>(rawValue);
    if(end<capacity) { auto dest=end++; if(dest) *dest=*value; }
    else Insert(end,value);
}
bool B06Ambience::Attach(void* target,uint32_t low,uint32_t high) {
    auto node=Find(map,low,high);
    if(node==map.buckets[map.bucketCount]) return false;
    auto layer=static_cast<uint8_t*>(node->layer);
    V<void,33>(target,layer ? layer+4 : nullptr);
    V<void,11>(layer);
    return true;
}
bool B06Ambience::Reload(uint32_t low,uint32_t high) {
    auto node=Find(map,low,high);
    if(node==map.buckets[map.bucketCount]) return false;
#if defined(_M_IX86)
    auto adjusted=*reinterpret_cast<uint8_t**>(0x011D8C40);
    void* system=adjusted ? adjusted-0x27bd0 : nullptr;
#else
    void* system=nullptr; AbortNotRewritten("B06 adjusted manager singleton");
#endif
    void* value=nullptr;
    const bool ok=V<uint8_t,7>(system,low,high,&value,0x1407ecu)!=0;
    if(!ok) { if(value) V<void,1>(value); return false; }
    V<void,10>(node->layer);
    KeyCommand(system,0xada39292u,low,high);
    V<void,8>(node->layer,value);
    KeyCommand(system,0xe69030b6u,low,high);
    if(value) V<void,1>(value);
    return true;
}
bool B06Ambience::AddLayer(void* system,const void* rawName) {
    auto name=static_cast<const char16_t*>(rawName);
    const uint64_t key=CallGameFunction<uint64_t,0x007CFE10>(name);
    const uint32_t low=uint32_t(key),high=uint32_t(key>>32);
    void* config=nullptr;
    if(!V<uint8_t,7>(system,low,high,&config,0x1407ecu)) { if(config) V<void,1>(config); return false; }
    uint32_t type=0;
    V<void,11>(config,0xb10f785du,&type);
    void* layer=nullptr;
    if(type==0x57572dac) {
        auto p=Allocate(0x168,
#if defined(_M_IX86)
            reinterpret_cast<const char*>(0x01027BE8)
#else
            "Audio/AmbienceLayerGlobal"
#endif
        );
        if(p) layer=CallGameMethod<void*,0x00861FB0>(p);
    } else if(type==0x0d10af00) {
        auto p=Allocate(0x188,
#if defined(_M_IX86)
            reinterpret_cast<const char*>(0x01027BC8)
#else
            "Audio/AmbienceLayerDistributed"
#endif
        );
        if(p) layer=CallGameMethod<void*,0x00861F20>(p);
    }
    if(!layer) { if(config) V<void,1>(config); return false; }
    if(!V<uint8_t,8>(layer,config)) {
        V<void,0>(static_cast<uint8_t*>(layer)+8,1u);
        if(config) V<void,1>(config);
        return false;
    }
    V<void,0>(layer); V<void,0>(layer);
    struct Entry { uint32_t low,high; void* layer; } entry{low,high,layer};
    uint8_t result[12]{};
    CallGameMethod<void,0x007009E0>(&map,result,&entry,0u);
    if(entry.layer) V<void,1>(entry.layer);
    V<void,1>(layer);
    KeyCommand(system,0xe69030b6u,low,high);
    if(config) V<void,1>(config);
    return true;
}
bool B06Ambience::Initialize(void* system) {
    if(resource) { auto old=resource; resource=nullptr; V<void,1>(old); }
    const auto ok=V<uint8_t,7>(system,0x385d469eu,0x2fba673fu,&resource,0x1407ecu);
    if(!ok) return false;
#if defined(_M_IX86)
    auto service=*reinterpret_cast<void**>(0x011ECBC4);
    if(service) {
        B06Strings strings{nullptr,nullptr,nullptr,reinterpret_cast<const char*>(0x01027C60)};
        B06String temporary{};
        for(auto flag: {0x200u,0x8000u}) {
            if(CallGameMethod<uint8_t,0x00523860>(reinterpret_cast<void*>(0x011CDDE0),flag)) {
                auto text=reinterpret_cast<const char16_t*>(flag==0x200 ? 0x01027C54 : 0x01027C40);
                CopyName(temporary,reinterpret_cast<const uint16_t*>(text)); strings.Append(&temporary);
            }
        }
        B06String path{}, world;
        CopyName(path,reinterpret_cast<const uint16_t*>(0x01027C30));
        ts3::world::WorldPath worldPath;
        worldPath.Clear();
        auto scheduler=static_cast<ts3::world::WorldScheduler*>(service)->GetScheduler();
        worldPath.Assign(static_cast<const uint16_t*>(scheduler));
        CallGameMethod<void,0x004ED670>(reinterpret_cast<uint8_t*>(&worldPath)+8,&world,250u);
        auto expected=reinterpret_cast<const char16_t*>(0x01027C14);
        if(std::char_traits<char16_t>::compare(world.text,expected,std::char_traits<char16_t>::length(expected)+1)==0) {
            auto space=std::char_traits<char16_t>::find(world.text,std::char_traits<char16_t>::length(world.text),u' ');
            if(space) { *const_cast<char16_t*>(space)=0; CallGameFunction<void,0x004FF470>(&world,space+1,250u); }
        }
        CallGameFunction<void,0x004FF470>(&path,&world,250u);
        auto value=CallGameMethod<uint8_t*,0x00862610>(this,&strings,&path);
        if(!value) {
            CopyName(temporary,reinterpret_cast<const uint16_t*>(0x01027C04));
            value=CallGameMethod<uint8_t*,0x00862610>(this,&strings,&temporary);
        }
        if(value) {
            auto flags=*reinterpret_cast<uint16_t*>(value+8)&0x30;
            auto type=*reinterpret_cast<uint16_t*>(value+10);
            auto count=flags ? *reinterpret_cast<int32_t*>(value+24) : int32_t(type!=0);
            auto data=flags ? *reinterpret_cast<uint8_t**>(value+16) : value+16;
            for(int32_t i=0;i<count;++i) AddLayer(system,*reinterpret_cast<const char16_t**>(data+16*i));
        }
        if(strings.begin) Free(strings.begin);
    }
#else
    AbortNotRewritten("B06 world scheduler/path and variant configuration interfaces");
#endif
    loaded=1;
    return true;
}
bool B06Ambience::Clear(void* target) {
    auto bucket=map.buckets;
    while(!*bucket) ++bucket;
    auto node=*bucket;
    const auto sentinel=map.buckets[map.bucketCount];
    while(node!=sentinel) {
        V<void,11>(node->layer);
        V<void,33>(target,static_cast<uint8_t*>(node->layer)+4);
        node=node->next;
        if(!node) { do { node=*++bucket; } while(!node); }
    }
    ClearMap(map);
    return true;
}
void B06Ambience::Dispose() {
    if(nodes.capacity>=0) CallGameMethod<void,0x00B9C560>(&nodes,uint32_t(nodes.capacity)&0x7fffffffu);
    CallGameMethod<void,0x007CCE20>(&nodes);
    ClearMap(map);
    if(map.bucketCount>1) Free(map.buckets);
    if(resource) V<void,1>(resource);
}
B06Ambience* B06Ambience::Construct() {
    std::memset(state,0,sizeof(state)); resource=nullptr;
    map.load=1; map.growth=2.0f;
#if defined(_M_IX86)
    map.label=reinterpret_cast<const char*>(0x01027C8C);
    map.buckets=reinterpret_cast<B06Node**>(0x0114C900);
    nodes.label=reinterpret_cast<const char*>(0x01027C78);
#else
    static B06Node* empty[2]{};
    map.label="Audio/AmbienceLayerMap"; map.buckets=empty; nodes.label="Audio/AmbienceNodes";
#endif
    map.bucketCount=1; map.size=0; map.allocator=0;
    nodes.begin=nullptr; nodes.end=nullptr; nodes.capacity=0x154;
    std::memset(inlineNodes,0,0xd4);
    const int32_t minusOne=-1; std::memcpy(inlineNodes+0xd4,&minusOne,4);
    std::memset(inlineNodes+0xd8,0,4);
    extra=nullptr; loaded=0;
    return this;
}
uint32_t B06SetFloat(uint32_t key,float value) {
    auto system=CommandSystem();
    if(!system) return 0;
    V<void,17>(system,0x03cdd1a9u);
    V<void,19>(system,0x034753a7u,key);
    V<void,18>(system,0x034753aau,value);
    return V<uint32_t,26>(system);
}
bool B06SettingEvent::Handle(uint32_t id,void* data) {
    if(id!=0x157e620f) return false;
    auto raw=static_cast<uint8_t*>(data);
    auto first=*reinterpret_cast<uint8_t**>(raw+8);
    auto last=*reinterpret_cast<uint8_t**>(raw+8+sizeof(void*));
    bool zero=false;
    if(first!=last) {
        const auto type=*reinterpret_cast<uint16_t*>(first+10);
        zero=type==1 ? first[16]==0 : *reinterpret_cast<uint32_t*>(first+16)==0;
    }
    B06SetFloat(0x899eb157u,zero ? 1.0f : 0.0f);
    auto flag=reinterpret_cast<std::atomic<uint32_t>*>(raw+0xd0+2*sizeof(void*));
    flag->exchange(0,std::memory_order_seq_cst);
    return true;
}
uint32_t B06SoundFloat(uint32_t sound,uint32_t key,float value,uint32_t flags) {
    auto system=CommandSystem();
    if(!system) return 0;
    V<void,17>(system,0x03475376u);
    V<void,19>(system,0x03475385u,sound);
    V<void,19>(system,0x034753a7u,key);
    V<void,18>(system,0x034753aau,value);
    V<void,19>(system,0x034753adu,flags);
    return V<uint32_t,26>(system);
}
}
