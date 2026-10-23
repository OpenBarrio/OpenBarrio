#include "ts3/render/m07_B29.h"
#include <cstring>
namespace ts3::render::m07::b29 {
namespace {
void* Image(uint32_t a) {
#if defined(_M_IX86)
    return reinterpret_cast<void*>(uintptr_t(a));
#else
    (void)a; AbortNotRewritten("m07_B29 image datum"); return nullptr;
#endif
}
struct Iterator { void* node; void** bucket; };
Iterator Find(Owner* s,uint32_t lo,uint32_t hi) {
    uint32_t key[2]={lo,hi}; Iterator out;
    CallGameMethod<void,0x006FDFB0>(s->buckets,&out,key); return out;
}
bool Invalid(Owner* s,void* p) { return p==s->sentinel || !p || uintptr_t(p)==8; }
void* Query(void* p) { return CallGameVirtual<void*,3>(static_cast<uint8_t*>(p)-2*sizeof(void*),uint32_t(0x017F1D88)); }
void* Resolve(Owner* s,uint32_t lo,uint32_t hi,bool twice,uintptr_t* early=nullptr) {
    auto p=Find(s,lo,hi).node;
    if(early) *early=reinterpret_cast<uintptr_t>(p);
    if(p==s->sentinel || !p) return nullptr;
    if(early) *early=reinterpret_cast<uintptr_t>(p)-8;
    if(uintptr_t(p)==8) return nullptr;
    if(twice) {
        p=Find(s,lo,hi).node;
        if(early) *early=reinterpret_cast<uintptr_t>(p);
        if(Invalid(s,p)) return nullptr;
    }
    void* model=Query(p); if(early) *early=reinterpret_cast<uintptr_t>(model); return model;
}
template<unsigned Address,class... A> void* Make(uint32_t size,uint32_t align,uint32_t tag,A... args) {
    void* p=CallGameFunction<void*,0x004E3FB0>(size,align,0,Image(tag),0,0,0,0);
    if(p) { p=CallGameMethod<void*,Address>(p,args...); if(p) CallGameVirtual<void,0>(p); }
    return p;
}
void Release(void* p) { if(p) CallGameVirtual<void,1>(p); }
void Attach(void* model,void* p,uint32_t key) { CallGameVirtual<void,32>(model,p,Image(key)); }
uint8_t Remove(void* model,uint32_t key) { return CallGameVirtual<uint8_t,33>(model,Image(key)); }
void Dirty(void* model) { CallGameMethod<void,0x006DCED0>(static_cast<uint8_t*>(model)+0x210); }
uintptr_t Refresh(void* model) { return CallGameMethod<uintptr_t,0x006FAC70>(model); }
void* Existing(void* model,uint32_t id) { return CallGameMethod<void*,0x006DC110>(static_cast<uint8_t*>(model)+0x210,id); }
uint8_t Finish(void* model,void* p,uint32_t key) { Attach(model,p,key); Dirty(model); Release(p); return 1; }
}
void* __fastcall Delete(void* self,void*,uint32_t flags) {
    // Exact clone of the verified scalar-deleting destructor family.
    CallGameMethod<void,0x006FD930>(self);
    if(flags&1) CallGameFunction<void,0x004E3F70>(self);
    return self;
}
String* __fastcall Debug(DebugObject* self,void*,String* out) {
    auto* empty=static_cast<char*>(Image(0x011C7E54));
    *out={empty,empty,empty+1,static_cast<const char*>(Image(0x00FF9DE8))};
    uint32_t key[4]={};
    if(CallGameVirtual<uint8_t,14>(self,key)) {
        String name{empty,empty,empty+1,static_cast<const char*>(Image(0x00FF9DCC))};
        String tmp;
        auto* source=CallGameFunction<String*,0x007D9380>(&tmp,key);
        if(source!=&name) CallGameMethod<void,0x00405A50>(&name,source->begin,source->end);
        if(reinterpret_cast<intptr_t>(tmp.capacity)-reinterpret_cast<intptr_t>(tmp.begin)>1 && tmp.begin)
            CallGameFunction<void,0x004E3F70>(tmp.begin);
        CallGameFunction<void,0x00590900>(out,"Object 0x%016I64X [%s]",self->id,name.begin);
        if(reinterpret_cast<intptr_t>(name.capacity)-reinterpret_cast<intptr_t>(name.begin)>1 && name.begin)
            CallGameFunction<void,0x004E3F70>(name.begin);
    } else CallGameFunction<void,0x00590900>(out,"Object 0x%016I64X",self->id);
    if(self->state && *reinterpret_cast<uint32_t*>(static_cast<uint8_t*>(self->state)+44)) {
        CallGameFunction<void,0x00567460>(out," at [%4.2f,%4.2f,%4.2f](%4.2f)",
            double(self->position[0]),double(self->position[1]),double(self->position[2]),double(self->position[3]));
    }
    return out;
}
uint8_t __fastcall Cutout(Owner* s,void*,uint32_t lo,uint32_t hi,void* data,uint32_t value) {
    void* m=Resolve(s,lo,hi,false); if(!m) return 0;
    void* p=nullptr;
    if(*static_cast<uint32_t*>(data)) p=Make<0x006FE360>(48,16,0x00FF9F7C,data,value);
    return Finish(m,p,0x00FF08DC);
}
uint8_t __fastcall Video(Owner* s,void*,uint32_t lo,uint32_t hi,void* data) {
    void* m=Resolve(s,lo,hi,false); if(!m) return 0;
    return Finish(m,Make<0x007254F0>(12,4,0x00FF9F94,data),0x00FF6F7C);
}
uint8_t __fastcall Painting(Owner* s,void*,uint32_t lo,uint32_t hi,void* data,uint32_t value) {
    void* m=Resolve(s,lo,hi,false); if(!m) return 0;
    return Finish(m,Make<0x006DD760>(32,8,0x00FF9FAC,data,value),0x00FF6F7C);
}
uint8_t __fastcall Ghost(Owner* s,void*,uint32_t lo,uint32_t hi,void* data,uint32_t value) {
    void* m=Resolve(s,lo,hi,true); if(!m) return 0;
    void* p=Existing(m,0x05B702FA);
    if(p) { CallGameMethod<void,0x00694900>(p,data); CallGameMethod<void,0x00694920>(p,value); }
    else { p=Make<0x006962B0>(24,4,0x00FF9FC8,data,value); Attach(m,p,0x00FF28F8); Release(p); }
    Dirty(m); Refresh(m); return 1;
}
uint8_t __fastcall Sculpture(Owner* s,void*,uint32_t lo,uint32_t hi,uint32_t value,void* data) {
    void* m=Resolve(s,lo,hi,true); if(!m) return 0;
    void* p=Existing(m,0x6A0494A1);
    if(p) {
        CallGameMethod<void,0x00739A60>(p,value);
        *reinterpret_cast<void**>(static_cast<uint8_t*>(p)+2*sizeof(void*))=data;
    } else {
        p=Make<0x0070B900>(56,8,0x00FF9FE0,data);
        // EA calls this setter even on allocation failure (null receiver).
        CallGameMethod<void,0x00739A60>(p,value);
        Attach(m,p,0x00FF9EE0); Release(p);
    }
    Dirty(m); Refresh(m); return 1;
}
uintptr_t __fastcall Burninate(Owner* s,void*,uint32_t lo,uint32_t hi) {
    uintptr_t result; void* m=Resolve(s,lo,hi,true,&result); if(!m) return result;
    if(!Existing(m,0x7AE09109)) { auto* p=Make<0x0068DCF0>(8,4,0x00FF9FFC); Attach(m,p,0x00FF0858); Release(p); }
    Dirty(m); return Refresh(m);
}
uintptr_t __fastcall Scratch(Owner* s,void*,uint32_t lo,uint32_t hi,uint8_t enabled) {
    uintptr_t result; void* m=Resolve(s,lo,hi,true,&result); if(!m) return result;
    if(enabled) {
        if(!Existing(m,0x0BD673A6)) { auto* p=Make<0x00706860>(8,4,0x00FFA018); Attach(m,p,0x00FF9EE4); Release(p); }
    } else Remove(m,0x00FF9EE4);
    Dirty(m); return Refresh(m);
}
uint8_t __fastcall PoolWindow(Owner* s,void*,uint32_t lo,uint32_t hi,uint8_t enabled) {
    void* m=Resolve(s,lo,hi,false); if(!m) return 0;
    auto* p=Make<0x006DE2A0>(8,4,0x00FFA118);
    if(enabled) { Attach(m,p,0x00FF70BC); Dirty(m); }
    else if(Remove(m,0x00FF70BC)) Dirty(m);
    Release(p); return 1;
}
uint8_t __fastcall DistantTerrain(Owner* s,void*,uint32_t lo,uint32_t hi) {
    void* m=Resolve(s,lo,hi,false); if(!m) return 0;
    return Finish(m,Make<0x0068E600>(8,4,0x00FFA134),0x00FF09E8);
}
uintptr_t __fastcall FrostedGlass(Owner* s,void*,uint8_t enabled) {
    void** bucket=s->buckets; void* node=*bucket;
    uintptr_t result=reinterpret_cast<uintptr_t>(bucket);
    while(!node) node=*++bucket;
    while(node!=s->sentinel) {
        if(node && uintptr_t(node)!=8) {
            void* m=Query(node); result=reinterpret_cast<uintptr_t>(m);
            if(m) {
                result=CallGameMethod<uint32_t,0x006C93D0>(m);
                if(result&0x20000) {
                    result=CallGameMethod<uintptr_t,0x006DD530>(static_cast<uint8_t*>(m)+0x210,0x0D646E03);
                    if(enabled) {
                        auto* p=Make<0x00693BD0>(12,4,0x00FFA058,0,1);
                        result=CallGameVirtual<uintptr_t,32>(m,p,Image(0x00FF27B4));
                        if(p) result=CallGameVirtual<uintptr_t,1>(p);
                    }
                }
            }
        }
        node=*static_cast<void**>(node);
        while(!node) node=*++bucket;
    }
    return result;
}
uint8_t __fastcall TexComp(Owner* s,void*,uint32_t lo,uint32_t hi,void* first,void* second) {
    void* m=Resolve(s,lo,hi,false); if(!m) return 0;
    if(static_cast<uint32_t*>(first)[2] || static_cast<uint32_t*>(second)[2])
        return Finish(m,Make<0x006FE300>(48,8,0x00FFA154,first,second),0x00FF9ECC);
    uint32_t key[4]={}; CallGameVirtual<void,14>(m,key); Remove(m,0x00FF9ECC);
    CallGameMethod<void,0x006F4860>(m,key,0,uint32_t(s->flag),0);
    return 1;
}
}
