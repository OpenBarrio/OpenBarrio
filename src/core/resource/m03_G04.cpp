#include "ts3/resource/m03_G04.h"
#include <cstring>
#include <cwchar>
namespace ts3::resource::g04_detail {
namespace {
// EA UTF16 string helpers access begin/end/inline at +0/+8/+10 on x86.
struct Text { void* begin; uintptr_t unused; void* end; uintptr_t unused2; void* local; uint8_t storage[32]; };
struct Keys { void* begin; void* end; void* capacity; const char* label; };
uint32_t Destroy(Text& text) {
    const auto bytes=(reinterpret_cast<uintptr_t>(text.end)-reinterpret_cast<uintptr_t>(text.begin)) & ~uintptr_t(1);
    if (static_cast<intptr_t>(bytes)>2 && text.begin && text.begin!=text.local)
        return CallGameFunction<uint32_t,0x004E3F70>(text.begin);
    return 0;
}
const char16_t* Literal(uintptr_t address) {
#if defined(_M_IX86)
    return reinterpret_cast<const char16_t*>(address);
#else
    (void)address; NotRewritten("m03_G04 locale constant table");
#endif
}
}
bool Refresh(Locale* self,const Key* key) {
    auto* manager=CallGameFunction<void*,0x004AFD20>();
    if (!manager) return false;
    void* record=nullptr;
    const auto found=CallGameVirtual<uint32_t,3>(manager,key,&record,0,0,0,0);
    if (!static_cast<uint8_t>(found)) {
        auto* policy=CallGameFunction<uint8_t*,0x00984550>();
        auto* directory=CallGameFunction<void*,0x0072EC00>();
        bool result=false;
        if (directory && policy && !policy[0xD5]) {
            Key output{}; uintptr_t path[6]{};
            CallGameMethod<void,0x00500A30>(path);
            CallGameMethod<void,0x009857F0>(policy,key,path,&output);
            result=!static_cast<uint8_t>(CallGameMethod<uint32_t,0x0072E7F0>(directory,path));
        }
        if (record) CallGameVirtual<uint32_t,1>(record);
        return result;
    }
    auto* sentinel=reinterpret_cast<Node*>(&self->first);
    auto* node=self->first;
    while (node!=sentinel) {
        if (node->key.word[0]==key->word[0] && node->key.word[2]==key->word[2] &&
            node->key.word[3]==key->word[3] && node->key.word[1]==key->word[1]) break;
        node=node->next;
    }
    if (node==sentinel) {
        const auto tag=key->word[1]&0xFF000000u;
        const bool primary=self->primaryLow==0 && tag==self->primaryTag;
        const bool secondary=self->secondaryLow==0 && tag==self->secondaryTag;
        if (!primary && !secondary) {
            if (record) CallGameVirtual<uint32_t,1>(record);
            return true;
        }
        struct Value { Key key; void* resource; } value{*key,nullptr};
        Node* inserted=nullptr;
        auto** result=CallGameMethod<Node**,0x0059B120>(sentinel,&inserted,primary?self->first:sentinel,&value);
        node=*result;
        if (node==sentinel) {
            if (record) CallGameVirtual<uint32_t,1>(record);
            return false;
        }
    }
    if (node->resource) CallGameVirtual<uint32_t,1>(node->resource);
    node->resource=record?CallGameVirtual<void*,3>(record,0xA058E66Au):nullptr;
    if (node->resource) CallGameVirtual<uint32_t,0>(node->resource);
    if (record) CallGameVirtual<uint32_t,1>(record);
    return true;
}
uint32_t Start(Locale* self) {
    CallGameFunction<void,0x0044C390>();
    uint32_t result=CallGameFunction<uint32_t,0x0044C340>();
    if (!static_cast<uint8_t>(result)) return result;
    CallGameFunction<void,0x00449C30>(Literal(0xFA1638),Literal(0xFA1968));
    CallGameFunction<void,0x00449C30>(Literal(0xFA1638),Literal(0xFA1868));
    auto* manager=CallGameFunction<void*,0x004AFD20>();
    if (!manager) return 0;
    Keys keys{nullptr,nullptr,nullptr,"String/LocalizedStringService/Keys"};
    if (CallGameVirtual<uint32_t,8>(manager,&keys,&self->map,0)) {
#if defined(_M_IX86)
        auto** table=reinterpret_cast<const char16_t**>(0x01152D30);
        for (unsigned i=0;i<69;i+=3) {
            auto id=CallGameMethod<uint64_t,0x0059ABA0>(self,table[i]);
            if (static_cast<uint8_t>(CallGameFunction<uint32_t,0x0059ADF0>(&keys,0,static_cast<uint32_t>(id)<<24)))
                CallGameFunction<void,0x00449A50>(table[i+1],table[i+2]);
        }
#else
        NotRewritten("m03_G04 locale alias table");
#endif
    }
    self->regionId=CallGameFunction<uint32_t,0x007D0370>();
    if (self->regionId==0xFFFFFFFFu) self->regionId=1;
    uint8_t data[20]{};
    auto* region=CallGameFunction<const char16_t*,0x007D0430>(data,16);
    CallGameFunction<void,0x004FF380>(self->region,region?region:Literal(0xFD9DD8),3);
    auto* requested=CallGameFunction<const char16_t*,0x0058A930>(Literal(0xFD9DD4),0);
    if (!requested) requested=CallGameFunction<const char16_t*,0x007D06A0>(data,16);
    if (!requested) requested=CallGameFunction<const char16_t*,0x007D0600>(data,16);
    if (!requested) requested=Literal(0xF9A458);
    Text text{};
    CallGameMethod<void,0x00446BF0>(&text,requested);
    CallGameMethod<void,0x00455930>(&text);
    if (!static_cast<uint8_t>(CallGameFunction<uint32_t,0x00448970>(&text))) {
        const uintptr_t alternatives[]={0xF9A458,0xFD9A60,0xFD99E8,0xFD9974};
        for (auto address:alternatives) {
            Text candidate{};
            auto* name=Literal(address);
            CallGameMethod<void,0x00446BF0>(&candidate,name);
            auto valid=CallGameFunction<uint32_t,0x00448970>(&candidate);
            if (address==0xFD9974) CallGameMethod<void,0x00447420>(&candidate);
            else Destroy(candidate);
            if (static_cast<uint8_t>(valid)) { CallGameMethod<void,0x0058BC30>(&text,name); break; }
        }
    }
    CallGameFunction<void,0x0044B9B0>(&text,0,0);
    auto primary=CallGameMethod<uint64_t,0x0059ABA0>(self,text.begin)<<56;
    self->primaryLow=static_cast<uint32_t>(primary); self->primaryTag=static_cast<uint32_t>(primary>>32);
    auto secondary=CallGameMethod<uint64_t,0x0059AC00>(self,text.begin)<<56;
    self->secondaryLow=static_cast<uint32_t>(secondary); self->secondaryTag=static_cast<uint32_t>(secondary>>32);
    CallGameFunction<void,0x004FF380>(self->name,text.begin,6);
    if (!CallGameVirtual<uint32_t,11>(manager,0x220557DAu,-1)) CallGameFunction<void,0x0059F650>();
#if defined(_M_IX86)
    auto callback=reinterpret_cast<void*>(0x0059BFC0);
#else
    void* callback=nullptr; // Native dispatch for this original callback is not implemented.
#endif
    CallGameVirtual<void,9>(manager,1,callback,self,&self->map,0);
    auto ok=static_cast<uint8_t>(CallGameMethod<uint32_t,0x0059BDE0>(self));
    if (ok) {
        CallGameMethod<void,0x0059AC80>(self);
        const auto* zh=Literal(0xFD9A60); const auto* zht=Literal(0xFD9BF8);
        const bool chinese= !_wcsicmp(reinterpret_cast<const wchar_t*>(self->name),reinterpret_cast<const wchar_t*>(zh)) ||
            !_wcsicmp(reinterpret_cast<const wchar_t*>(self->name),reinterpret_cast<const wchar_t*>(zht));
        CallGameFunction<void,0x005986B0>(static_cast<uint32_t>(chinese));
    }
    Destroy(text);
    result=keys.begin?CallGameFunction<uint32_t,0x004E3F70>(keys.begin):0;
    return (result&0xFFFFFF00u)|(ok?1u:0u);
}
}
namespace ts3::resource {
bool g04::Refresh(const void* key) { return g04_detail::Refresh(this,static_cast<const g04_detail::Key*>(key)); }
uint32_t g04::Start() { return g04_detail::Start(this); }
}
