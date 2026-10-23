#include "ts3/anim/m05_M123.h"
#include "ts3/locomotion/clu_58abf0_Locomotion.h"
#include "ts3/locomotion/clu_471020_Locomotion.h"
#include "ts3/misc/cls_408760_x.h"
#include "ts3/anim/m05_M106.h"
#include "ts3/anim/m05_B32.h"
#include "ts3/locomotion/clu_59d440_Locomotion.h"
#include "ts3/renderer/clu_4fed70_Renderer.h"
#include "ts3/swarm/clu_405a50_Swarm.h"
#include <cwchar>
namespace {
struct XmlCursor {
    ts3::misc::ObserverOwner owner;
    unsigned char opaqueTail[0x24];
};
struct ClipNames { char16_t* begin; char16_t* end; char16_t* capacity; const char* descriptor; };
struct Scratch {
    uint32_t opaque;
    void* buckets;
    uint32_t bucketCount,count;
    float lower,upper;
    uint32_t flags;
    const char* descriptor;
    void* begin; void* end; void* capacity;
    const char* weatherDescriptor;
};
TS3_X86_OFFSET(XmlCursor,opaqueTail,0x68);
TS3_X86_SIZE(XmlCursor,0x8c);
TS3_X86_OFFSET(Scratch,begin,0x20);
TS3_X86_SIZE(Scratch,0x30);
#if defined(_M_IX86)
using namespace ts3;
using Variant=anim::m106::Style;
struct Effect { swarm::ByteVector text; const char* descriptor; uint32_t value; };
template<class T,size_t N> struct InlineRange {
    T* begin; T* end; T* capacity; uint32_t untouched; T storage[N];
    InlineRange():begin(storage),end(storage),capacity(storage+N) {}
};
using Variants=InlineRange<Variant,32>;
using Effects=InlineRange<Effect,4>;
TS3_X86_SIZE(Effect,20);
uint32_t Token(XmlCursor& c) { return CallGameMethod<uint32_t,0x00471040>(&c); }
bool IsName(XmlCursor& c,const wchar_t* name) { return !std::wcscmp(name,CallGameMethod<const wchar_t*,0x00471060>(&c)); }
bool Done(XmlCursor& c) { return locomotion::HasLocomotionState(&c); }
bool Next(XmlCursor& c) { return CallGameMethod<uint8_t,0x00471260>(&c)!=0; }
const wchar_t* Attribute(XmlCursor& c,uintptr_t address) {
    return CallGameMethod<const wchar_t*,0x00471070>(&c,reinterpret_cast<const wchar_t*>(address));
}
void VariantElement(void* self,Scratch& scratch,XmlCursor& cursor,Variants& list,uint32_t id,uint32_t products) {
    Variant value;
    anim::m106::Construct(&value);
    CallGameMethod<void,0x008402A0>(self,&scratch,id,&cursor,&value,products);
    if(list.end>=list.capacity) CallGameMethod<void,0x0083F0B0>(&list,list.end,&value);
    else { auto* dst=list.end++;if(dst) CallGameMethod<void,0x00757690>(dst,&value); }
    CallGameMethod<void,0x00754060>(&value);
}
void StoreVariants(void* self,Variants& list,uint32_t id,void* stream) {
    auto* destination=CallGameMethod<Variants*,0x00844870>(static_cast<unsigned char*>(self)+0x4c,&id);
    if(destination!=&list) {
        CallGameMethod<void,0x0083DD40>(destination);
        CallGameMethod<void,0x0083F2A0>(destination,list.begin,list.end,stream);
    }
}
void StyleElement(void* self,Scratch& scratch,XmlCursor& cursor,uint32_t products,void* stream) {
    uint32_t flags=1;
    if(const auto* s=Attribute(cursor,0x010239F0))
        flags=renderer::Sub4FED70(reinterpret_cast<uint32_t>(s),0,10);
    if((flags & products)==0) {
        if(!Done(cursor)) do {
            if(!Next(cursor) || (Token(cursor)==2 && IsName(cursor,L"Style"))) break;
        } while(!Done(cursor));
        return;
    }
    uint32_t id=CallGameFunction<uint32_t,0x007CFDA0>(Attribute(cursor,0x00FC0C6C));
    Variants variants;Effects effects;
    while(!Done(cursor)) {
        if(!Next(cursor) || (Token(cursor)==2 && IsName(cursor,L"Style"))) break;
        if(Token(cursor)==1 && IsName(cursor,L"Variant")) VariantElement(self,scratch,cursor,variants,id,flags);
        if(Token(cursor)==1 && IsName(cursor,L"Effect")) {
            Effect value;
            value.text.begin=value.text.end=reinterpret_cast<uint8_t*>(0x011C7E54);
            value.text.capacity=reinterpret_cast<uint8_t*>(0x011C7E55);
            value.descriptor=reinterpret_cast<const char*>(0x01023064);
            CallGameMethod<void,0x0083CDF0>(self,&cursor,&value);
            if(effects.end>=effects.capacity)
                reinterpret_cast<swarm::StrVector*>(&effects)->Insert(reinterpret_cast<swarm::Obj20*>(effects.end),reinterpret_cast<swarm::Obj20*>(&value));
            else {
                auto* dst=effects.end++;
                if(dst) {CallGameMethod<void,0x00405AD0>(dst,&value);dst->value=value.value;}
            }
            if(static_cast<int32_t>(value.text.capacity-value.text.begin)>1 && value.text.begin)
                CallGameFunction<void,0x004E3F70>(value.text.begin);
        }
    }
    StoreVariants(self,variants,id,stream);
    void* effectDestination=CallGameMethod<void*,0x008446C0>(static_cast<unsigned char*>(self)+0x6c,&id,&effects);
    CallGameMethod<void,0x00841FF0>(effectDestination);
    CallGameMethod<void,0x00912B60>(&effects,effects.begin,effects.end);
    CallGameMethod<void,0x0083DF30>(&variants);
}
struct ProgressionRow {
    uint32_t* begin;uint32_t* end;uint32_t* capacity;const char* descriptor;
    uint32_t* inlineBegin;uint32_t storage[1];uint32_t id;
};
struct Progressions {
    ProgressionRow* begin;ProgressionRow* end;ProgressionRow* capacity;const char* descriptor;
    ProgressionRow* inlineBegin;ProgressionRow storage[6];
    Progressions():begin(storage),end(storage),capacity(storage+6),descriptor(reinterpret_cast<const char*>(0x010268E4)),inlineBegin(storage) {}
};
TS3_X86_SIZE(ProgressionRow,28);
void SkipFor(ProgressionRow& row,const wchar_t* text) {
    using Lower=uint16_t(__cdecl*)(uint16_t);
    auto lower=*reinterpret_cast<Lower const*>(0x00F95764);
    while(text && *text) {
        uint32_t mask=0;bool valid=true;
        switch(lower(*text)) {
        case 'p':mask=2;break;case 'c':mask=4;break;case 't':mask=8;break;
        case 'y':mask=16;break;case 'a':mask=32;break;case 'e':mask=64;break;
        default:valid=false;break;
        }
        switch(lower(text[1])) {
        case 'u':mask|=0x100;break;case 'd':mask|=0x400;break;case 'l':mask|=0x500;break;
        case 'c':mask|=0x300;break;case 'h':mask|=0x200;break;case 'b':mask|=0x600;break;
        case 'r':mask|=0x700;break;default:return;
        }
        if(!valid) return;
        if(row.end>=row.capacity) CallGameMethod<void,0x00B7B1E0>(&row,row.end,&mask);
        else {auto* dst=row.end++;if(dst) *dst=mask;}
        text+=2;
        if(*text==L',') do {++text;} while(*text==L' ');
    }
}
void ProgressionElements(void* self,XmlCursor& cursor) {
    while(!Done(cursor)) {
        if(!Next(cursor) || (Token(cursor)==2 && IsName(cursor,L"WalkstyleProgressions"))) break;
        if(Token(cursor)==1 && IsName(cursor,reinterpret_cast<const wchar_t*>(0x01026908))) {
            Progressions list;
            while(!Done(cursor)) {
                if(!Next(cursor) || (Token(cursor)==2 && IsName(cursor,reinterpret_cast<const wchar_t*>(0x01026908)))) break;
                if(Token(cursor)==1 && IsName(cursor,L"Style")) {
                    const uint32_t id=CallGameFunction<uint32_t,0x007CFDA0>(Attribute(cursor,0x00FC0C6C));
                    CallGameMethod<void,0x008447A0>(&list);
                    auto* row=list.end-1;row->id=id;
                    SkipFor(*row,Attribute(cursor,0x010268D4));
                }
            }
            CallGameMethod<void,0x00844EF0>(static_cast<unsigned char*>(self)+0xac,&list);
            CallGameMethod<void,0x00759410>(&list,list.begin,list.end);
            if(list.begin && list.begin!=list.inlineBegin) CallGameFunction<void,0x004E3F70>(list.begin);
        }
    }
}
struct Key { uint32_t lo,hi,type,group; };
struct WideString { wchar_t* begin;wchar_t* end;wchar_t* capacity;uint32_t opaque;wchar_t storage[256]; };
struct ClipDescription {
    uint16_t* begin;uint16_t* end;uint16_t* capacity;uint32_t opaque;
    uint16_t storage[64];Key key;float state[3];uint32_t moveSide,turnSide;uint8_t flag,enabled;
};
TS3_X86_OFFSET(ClipDescription,moveSide,0xac);
TS3_X86_OFFSET(ClipDescription,flag,0xb4);
uint8_t Validate(void* resource,ClipDescription* description) {
    // 0083BFE0 consumes ECX, but leaves its stack argument to the caller.
    uint32_t result;
    __asm {
        mov ecx,resource
        push description
        mov eax,0083bfe0h
        call eax
        add esp,4
        mov result,eax
    }
    return static_cast<uint8_t>(result);
}
uint32_t ProductGroup(uint32_t flags) { return (uint32_t(locomotion::LookupLocomotionMask(int32_t(flags)))-1u)<<27; }
void* NewBuilder(const wchar_t* classifiers) {
    unsigned kind=8;
    const uintptr_t names[]={0x01026a44,0x01026a20,0x00fcbfa4,0x01026a18,0x01026a10,0x01026a08,0x01026a00,0x010269f8};
    for(unsigned i=0;i<8;++i) if(!std::wcscmp(classifiers,reinterpret_cast<const wchar_t*>(names[i]))) {kind=i;break;}
    void* p=CallGameFunction<void*,0x004E3F90>(kind==8?0xc0:0x228,reinterpret_cast<const char*>(0x01026a24),0,0,0,0);
    if(!p) return nullptr;
    switch(kind) {
    case 0:return CallGameMethod<void*,0x0083F3B0>(p);case 1:return CallGameMethod<void*,0x0083F400>(p);
    case 2:return CallGameMethod<void*,0x0083F4E0>(p);case 3:return CallGameMethod<void*,0x0083F550>(p);
    case 4:return CallGameMethod<void*,0x0083F5B0>(p);case 5:return CallGameMethod<void*,0x0083F610>(p);
    case 6:return CallGameMethod<void*,0x0083F660>(p);case 7:return CallGameMethod<void*,0x0083F6A0>(p);
    default:return CallGameMethod<void*,0x0083DA80>(p);
    }
}
void MoveElement(void* self,void* provider,Scratch& scratch,ClipNames& missing,Key& clipKey,XmlCursor& cursor,uint32_t products) {
    uint32_t flags=1;
    if(auto* s=Attribute(cursor,0x010239F0)) flags=renderer::Sub4FED70(reinterpret_cast<uint32_t>(s),0,10);
    uint32_t group=ProductGroup(flags);
    if(!(flags & products)) {anim::b32::Advance(&cursor);return;}
    WideString builderName,classifiers;
    CallGameMethod<void,0x009B1FE0>(&builderName,Attribute(cursor,0x00FC0C6C));
    const wchar_t* rig=Attribute(cursor,0x01026A60);
    if(!rig || !*rig) {anim::b32::Advance(&cursor);return;}
    CallGameMethod<void,0x009B1FE0>(&classifiers,Attribute(cursor,0x01026A48));
    char rigName[256];
    CallGameFunction<void,0x004FDDD0>(rigName,rig,256,-1);
    const uint64_t hash=CallGameFunction<uint64_t,0x007CFE10>(rigName);
    Key rigKey{uint32_t(hash),uint32_t(hash>>32),0x8eaf13de,0};
    void* resource=nullptr;
    void** output=CallGameMethod<void**,0x006FDF20>(&resource);
    if(!CallGameVirtual<uint8_t,3>(provider,&rigKey,output,0,0,0,0)) rigKey.group=group;
    output=CallGameMethod<void**,0x006FDF20>(&resource);
    CallGameVirtual<void,3>(provider,&rigKey,output,0,0,0,0);
    if(!resource) {
        anim::b32::Advance(&cursor);
        if(resource) CallGameVirtual<void,1>(resource);
        return;
    }
    void* builder=nullptr;
    CallGameMethod<void,0x00B96150>(&builder,NewBuilder(classifiers.begin));
    bool accepted=false;
    while(!Done(cursor)) {
        if(!Next(cursor) || (Token(cursor)==2 && IsName(cursor,L"MoveController"))) break;
        if(Token(cursor)==1 && IsName(cursor,L"Clip")) {
            const wchar_t* name=Attribute(cursor,0x00FC0C6C);
            if(std::wcslen(name)<0x40) {
                uint32_t clipGroup=group;
                if(auto* s=Attribute(cursor,0x010239F0)) clipGroup=ProductGroup(renderer::Sub83A6B0(reinterpret_cast<uint32_t>(s)));
                const uint64_t clipHash=CallGameFunction<uint64_t,0x0057A470>(name,0);
                clipKey.lo=uint32_t(clipHash);clipKey.hi=uint32_t(clipHash>>32);clipKey.group=clipGroup;
                ClipDescription description;
                CallGameMethod<void,0x0083D9D0>(&description,&clipKey,name,0);
                CallGameMethod<void,0x0083F920>(self,&scratch,name,clipKey);
                if(Attribute(cursor,0x010269DC)) description.flag=1;
                if(const auto* s=Attribute(cursor,0x010269C8)) {
                    if(!std::wcscmp(reinterpret_cast<const wchar_t*>(0x010269BC),s)) description.moveSide=1;
                    else if(!std::wcscmp(L"right",s)) description.moveSide=2;
                }
                if(const auto* s=Attribute(cursor,0x0102699C)) {
                    if(!std::wcscmp(reinterpret_cast<const wchar_t*>(0x010269BC),s)) description.turnSide=1;
                    else if(!std::wcscmp(L"right",s)) description.turnSide=2;
                }
                uint8_t pivot=!accepted;
                if(Attribute(cursor,0x01026988)) pivot=1;
                if(const auto* s=Attribute(cursor,0x01026968)) description.enabled=std::wcscmp(reinterpret_cast<const wchar_t*>(0x00FCBF94),s)!=0;
                if(Validate(resource,&description)) {
                    if(CallGameVirtual<uint8_t,5>(builder,&description,pivot)) accepted=true;
                } else {
                    CallGameMethod<void,0x004F1F50>(&missing,name);
                    CallGameMethod<void,0x004F1F50>(&missing,reinterpret_cast<const wchar_t*>(0x01026960));
                }
            }
        }
    }
    CallGameMethod<void,0x0083E2F0>(self,builderName.begin,builder);
    CallGameVirtual<void,7>(builder,0);
    CallGameMethod<void,0x007460F0>(&builder);
    if(resource) CallGameVirtual<void,1>(resource);
}
struct WeatherEntry { ClipNames name;uint16_t mask[2];unsigned char opaque14[16];uint8_t enabled;unsigned char pad[3]; };
struct WeatherNode { unsigned char opaque[16];ClipNames name;uint16_t mask[2]; };
struct WeatherMap { void* opaque;WeatherNode** buckets;uint32_t count; };
TS3_X86_SIZE(WeatherEntry,40);
void ApplyWeather(void* self,Scratch& scratch) {
    CallGameFunction<void,0x0083D390>(0,self,reinterpret_cast<void*>(0x0083AB00),&scratch.begin);
    auto* map=reinterpret_cast<WeatherMap*>(static_cast<unsigned char*>(self)+0xb8);
    auto* end=static_cast<WeatherEntry*>(scratch.end);
    for(auto* p=static_cast<WeatherEntry*>(scratch.begin);p!=end;++p) {
        if(!p->enabled) continue;
        WeatherNode* found;
        CallGameMethod<void,0x0083B960>(map,&found,p);
        if(found!=map->buckets[map->count]) {found->mask[0]|=p->mask[0];found->mask[1]|=p->mask[1];}
        else {
            auto* inserted=CallGameMethod<ClipNames*,0x0083E0B0>(map,p);
            auto* masks=reinterpret_cast<uint16_t*>(inserted+1);
            masks[0]=p->mask[0];masks[1]=p->mask[1];
            auto* text=CallGameMethod<ClipNames*,0x0083DF60>(&scratch,p);
            if(text!=inserted) reinterpret_cast<swarm::ByteVector*>(inserted)->Assign(
                reinterpret_cast<const char*>(text->begin),reinterpret_cast<const char*>(text->end));
        }
    }
}
void DefaultsElement(void* self,Scratch& scratch,XmlCursor& cursor,void* stream) {
    Variants variants;
    while(!Done(cursor)) {
        if(!Next(cursor) || (Token(cursor)==2 && IsName(cursor,L"Defaults"))) break;
        if(Token(cursor)==1 && IsName(cursor,L"Variant")) VariantElement(self,scratch,cursor,variants,0,1);
    }
    StoreVariants(self,variants,0,stream);
    CallGameMethod<void,0x0083DF30>(&variants);
}
#endif
}

namespace ts3::anim {
uint32_t m123::LoadStyleDefinitions(void* self) {
#if defined(_M_IX86)
    // The inventory extent omits the epilogue: 008465EB writes AL, not EAX.
    ts3::locomotion::ConfigDoubleLookup lookup;
    const double tuning=lookup.ConfigDouble(reinterpret_cast<const wchar_t*>(0x01026AE0),
        reinterpret_cast<const wchar_t*>(0x01026AF0),*reinterpret_cast<const float*>(0x00F9D504));
    *reinterpret_cast<float*>(0x01166200)=static_cast<float>(tuning);
    void* provider=CallGameFunction<void*,0x004AFD20>();
    uintptr_t request[4]{};
    CallGameVirtual<void,32>(provider,request,reinterpret_cast<const wchar_t*>(0x01026AB4),0x0333406cu,0);
    void* source=CallGameVirtual<void*,17>(provider,request);
    if(source==nullptr) return 1;
    void* document=nullptr;
    const uint32_t result=CallGameVirtual<uint32_t,13>(source,request,&document,1,6,1,0);
    if(static_cast<uint8_t>(result)==0) return (result & 0xffffff00u) | 1u;
    void* stream=CallGameVirtual<void*,6>(document);
    if(stream) CallGameVirtual<void,1>(stream);
    XmlCursor cursor;
    CallGameMethod<void,0x004719C0>(&cursor,stream,0,0);
    const uint32_t products=CallGameMethod<uint32_t,0x009F0DE0>(reinterpret_cast<void*>(0x011CDDE0));
    ClipNames names{reinterpret_cast<char16_t*>(0x011C7E54),reinterpret_cast<char16_t*>(0x011C7E54),
        reinterpret_cast<char16_t*>(0x011C7E56),reinterpret_cast<const char*>(0x01026A94)};
    Scratch scratch; // +00 is deliberately not initialized in EA's frame.
    scratch.buckets=reinterpret_cast<void*>(0x0114C900);scratch.bucketCount=1;scratch.count=0;
    scratch.lower=*reinterpret_cast<const float*>(0x0107A538);
    scratch.upper=*reinterpret_cast<const float*>(0x00F98A58);
    scratch.flags=0;scratch.descriptor=reinterpret_cast<const char*>(0x01023328);
    scratch.begin=scratch.end=scratch.capacity=nullptr;
    scratch.weatherDescriptor=reinterpret_cast<const char*>(0x01023304);
    const bool weather=CallGameMethod<uint8_t,0x00523860>(reinterpret_cast<void*>(0x011CDDE0),0x8000)!=0;
    Key clipKey{0,0,0x6b20c4f3,0};
    while(!ts3::locomotion::HasLocomotionState(&cursor)) {
        if(!CallGameMethod<uint8_t,0x00471260>(&cursor)) break;
        if(CallGameMethod<uint32_t,0x00471040>(&cursor)==1) {
            if(CallGameMethod<uint32_t,0x00471040>(&cursor)==1 &&
                !std::wcscmp(L"Style",CallGameMethod<const wchar_t*,0x00471060>(&cursor)))
                { StyleElement(self,scratch,cursor,products,stream);continue; }
            if(CallGameMethod<uint32_t,0x00471040>(&cursor)==1 &&
                !std::wcscmp(L"MoveController",CallGameMethod<const wchar_t*,0x00471060>(&cursor)))
                { MoveElement(self,provider,scratch,names,clipKey,cursor,products);continue; }
            if(CallGameMethod<uint32_t,0x00471040>(&cursor)==1 &&
                !std::wcscmp(L"Defaults",CallGameMethod<const wchar_t*,0x00471060>(&cursor)))
                { DefaultsElement(self,scratch,cursor,stream);continue; }
            if(CallGameMethod<uint32_t,0x00471040>(&cursor)==1 &&
                !std::wcscmp(L"WalkstyleProgressions",CallGameMethod<const wchar_t*,0x00471060>(&cursor)))
                { ProgressionElements(self,cursor);continue; }
        }
    }
    CallGameVirtual<void,8>(document);
    CallGameVirtual<void,2>(document);
    if(weather) ApplyWeather(self,scratch);
    CallGameMethod<void,0x0083EE50>(&scratch);
    const auto span=reinterpret_cast<uintptr_t>(names.capacity)-reinterpret_cast<uintptr_t>(names.begin);
    if(static_cast<int32_t>(span & ~uintptr_t(1))>2 && names.begin) CallGameFunction<void,0x004E3F70>(names.begin);
    uintptr_t last=cursor.owner.Destruct();
    if(stream) last=CallGameVirtual<uintptr_t,2>(stream);
    return static_cast<uint32_t>((last & ~uintptr_t(0xff)) | 1);
#else
    (void)self;
    NotRewritten("m05_M123 original configuration service");
#endif
}
}
