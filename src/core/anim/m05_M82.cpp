#include "ts3/anim/m05_M82.h"
#include <algorithm>
#include <cstring>
#include <string>
#include <cstdio>
namespace ts3::anim::m82 {
namespace {
size_t Count(const SlotVector* s) { return (reinterpret_cast<uintptr_t>(s->end)-reinterpret_cast<uintptr_t>(s->begin))/sizeof(Slot); }
void Copy(Slot** out,Slot* first,Slot* last,Slot* dest) {
    CallGameFunction<void*,0x00569990>(out,first,last,dest,0u);
}
void Fill(Slot* dest,uint32_t n,const Slot* src) { CallGameFunction<void,0x00578930>(dest,n,src,0u); }
void Init(Slot& s) { s.channels.begin=s.channels.end=s.channels.storage; s.channels.capacity=s.channels.storage+10; }
SlotVector* Slots(ClipIkInfo* s) { return reinterpret_cast<SlotVector*>(&s->slotBegin); }
void Assign(InlineString* s,const char* b,const char* e) { CallGameMethod<void*,0x00577c20>(s,b,e); }
uint32_t Hash(const char* s) { return CallGameFunction<uint32_t,0x007cfe70>(s); }
void Read32(Reader* r,void* p) {
    auto ok=CallGameFunction<uint8_t,0x004dd700>(r->stream,p,r->endian);
    CallGameMethod<void,0x007d77c0>(r,ok);
}
void Read64(Reader* r,void* p) {
    auto ok=CallGameFunction<uint8_t,0x004dd660>(r->stream,p,r->endian);
    CallGameMethod<void,0x007d77c0>(r,ok);
}
}
uintptr_t Insert(SlotVector* s,Slot* pos,uint32_t n,const Slot* value) {
    uint32_t spare=uint32_t((reinterpret_cast<uintptr_t>(s->capacity)-reinterpret_cast<uintptr_t>(s->end))/sizeof(Slot));
    if(n>spare) {
        uint32_t old=uint32_t(Count(s));
        uint32_t capacity=std::max(old?2*old:1u,old+n);
#if defined(_M_IX86)
        constexpr uintptr_t source=0x00f98980;
#else
        constexpr uintptr_t source=0;
#endif
        Slot* fresh=capacity?CallGameFunction<Slot*,0x004e3f90>(uint32_t(sizeof(Slot)*capacity),s->descriptor,0u,0u,source,0xe7u):nullptr;
        Slot* end;
        Copy(&end,s->begin,pos,fresh);
        Fill(end,n,value);
        Copy(&end,pos,s->end,end+n);
        if(s->begin) CallGameFunction<void,0x004e3f70>(s->begin);
        s->begin=fresh; s->end=end; s->capacity=fresh+capacity;
        return reinterpret_cast<uintptr_t>(s->capacity);
    }
    if(!n) return spare;
    // The temporary is a deep copy: value can alias an element being moved.
    Slot temp; temp.index=value->index; Init(temp);
    CallGameMethod<void,0x00569840>(&temp.channels,value->channels.begin,value->channels.end,0u);
    Slot* oldEnd=s->end; uint32_t tail=uint32_t(oldEnd-pos); Slot* ignored;
    if(n<tail) {
        Slot* cut=oldEnd-n;
        Copy(&ignored,cut,oldEnd,oldEnd); s->end+=n;
        CallGameFunction<void,0x00578990>(pos,cut,oldEnd);
        return CallGameFunction<uintptr_t,0x00578c10>(pos,pos+n,&temp);
    }
    Fill(oldEnd,n-tail,&temp); s->end+=n-tail;
    Copy(&ignored,pos,oldEnd,s->end); s->end+=tail;
    auto result=CallGameFunction<uintptr_t,0x00578c10>(pos,oldEnd,&temp);
    return tail?result:reinterpret_cast<uintptr_t>(oldEnd);
}
uintptr_t Resize(SlotVector* s,uint32_t n) {
    uint32_t count=uint32_t(Count(s));
    if(n>count) { Slot temp; Init(temp); return Insert(s,s->end,n-count,&temp); }
    Slot* end=s->begin+n;
    CallGameFunction<void,0x00569920>(s->end,s->end,end);
    intptr_t delta=-intptr_t((count-n)*sizeof(Slot));
    s->end=reinterpret_cast<Slot*>(reinterpret_cast<uintptr_t>(s->end)+delta);
    return uintptr_t(delta);
}
uint8_t Populate(ClipIkInfo* s,const Source* source,uint32_t minimum) {
    if(!source) return 1;
    if(source->slots) {
        auto* input=source->slots; auto* slots=Slots(s);
        uint32_t n=std::max(minimum,input->count);
        if(n>Count(slots)) { slots->end=slots->begin; Resize(slots,n); }
        CallGameMethod<void,0x00577f80>(s);
        InlineString names[60];
        for(auto& name:names) { name.begin=name.end=name.storage; name.capacity=name.storage+96; name.storage[0]=0; }
        uint32_t unique=0;
        for(uint32_t i=0;i<input->count;++i) {
            auto* src=input->slots[i]; auto& slot=slots->begin[i]; slot.index=i;
            CallGameMethod<void,0x005787c0>(&slot.channels,src->count);
            for(uint32_t j=0;j<src->count;++j) {
                auto* in=src->channels[j]; auto& out=slot.channels.begin[j];
                Assign(&out.name,in->name,in->name+std::strlen(in->name));
                Assign(&out.bone,in->bone,in->bone+std::strlen(in->bone));
                out.nameHash=Hash(in->name); out.boneHash=Hash(in->bone);
                // +F4 is an opaque scalar in the shared layout, not the serialized +F0.
                std::memcpy(out.opaqueE0+0x14,&in->value,4);
                uint32_t k=0; while(k<unique && std::strcmp(in->name,names[k].begin)) ++k;
                if(k==unique) { Assign(&names[unique],in->name,in->name+std::strlen(in->name)); ++unique; }
            }
        }
        if(unique) {
            CallGameMethod<void,0x00578870>(&s->namespaceBegin,unique);
            CallGameMethod<void,0x00aef830>(&s->hashBegin,unique);
            auto* hashes=static_cast<uint32_t*>(s->hashBegin);
            for(uint32_t i=0;i<unique;++i) {
                Assign(static_cast<InlineString*>(s->namespaceBegin)+i,names[i].begin,names[i].end);
                hashes[i]=Hash(names[i].begin);
            }
            for(auto* slot=slots->begin;slot!=slots->end;++slot)
                for(auto* c=slot->channels.begin;c!=slot->channels.end;++c)
                    for(uint32_t i=0;i<unique;++i) if(c->nameHash==hashes[i]) { c->namespaceIndex=i; break; }
        }
    }
    Assign(reinterpret_cast<InlineString*>(&s->begin),source->name,source->name+std::strlen(source->name));
    s->valueD0=Hash(static_cast<char*>(s->begin));
    if(source->rotation) std::memcpy(&s->rotation,source->rotation,16);
    std::memset(s->opaquec0+8,0,4);
    return 1;
}
uint8_t Read(ClipIkInfo* s,Reader* r,void*) {
    std::memset(s->opaquec0,0,8);
    uint32_t count=0,present=0;
    Read32(r,&s->valueCC); Read32(r,&count); Read32(r,&present); Read64(r,s->opaquec0);
    s->valueCC &= 0xfffffff3u;
    auto* slots=Slots(s); if(count!=Count(slots)) Resize(slots,count);
    if(present) for(auto* slot=slots->begin;slot!=slots->end;++slot) {
        uint32_t n=0; Read32(r,&n); CallGameMethod<void,0x005787c0>(&slot->channels,n);
        for(uint32_t i=0;i<n;++i) {
            auto& c=slot->channels.begin[i];
            Read32(r,c.opaqueE0+0x10); Read32(r,&c.nameHash); Read32(r,&c.boneHash); Read64(r,c.opaqueE0+8);
        }
    }
    return r->good;
}
uint64_t ClipId(const String* input,uint64_t* unqualified) {
    std::string full(input->begin,input->end);
    auto split=full.find('_');
    std::string prefix=split==std::string::npos?"":full.substr(0,split);
    std::string suffix=split==std::string::npos?full:full.substr(split+1);
    auto marker=suffix.rfind("_0x");
    if(marker!=std::string::npos) {
        if(unqualified) {
            auto pos=full.rfind("_0x"); std::string stripped=full.substr(0,pos);
            String view{stripped.data(),stripped.data()+stripped.size(),stripped.data()+stripped.size()+1,"Animation/ClipObject"};
            return ClipId(&view,unqualified);
        }
        auto bang=full.find('!');
        std::string hex=bang==std::string::npos?suffix.substr(marker+1):full.substr(bang+1);
        uint64_t explicitId=0;
        if(std::sscanf(hex.c_str(),"%llx",&explicitId) && explicitId) return explicitId;
    }
    auto view=[](std::string& str) { return String{str.data(),str.data()+str.size(),str.data()+str.size()+1,"Animation/ClipObject"}; };
    String pref=view(prefix);
    // This dependency has its own verified rewrite. Its output is game-allocated.
    char empty=0;
    String normalized{&empty,&empty,&empty+1,"Animation/ClipObject"};
    auto ok=swarm::NormalizeString579D80(reinterpret_cast<const swarm::ByteVector*>(&pref),reinterpret_cast<swarm::ByteVector*>(&normalized));
    if(ok==1) full.assign(normalized.begin,normalized.end),full+=suffix;
    if(normalized.begin && normalized.capacity-normalized.begin>1) CallGameFunction<void,0x004e3f70>(normalized.begin);
    uint16_t ns=CallGameFunction<uint16_t,0x00579bb0>(&pref);
    return CallGameFunction<uint64_t,0x005798f0>(full.c_str(),uint64_t(ns)<<48,unqualified);
}
uint64_t ClipIdCString(const char* text,uint64_t* unqualified) {
    if(!text) return 0;
    String str{const_cast<char*>(text),const_cast<char*>(text)+std::strlen(text),nullptr,"Animation/ClipObject"};
    return ClipId(&str,unqualified);
}
}
