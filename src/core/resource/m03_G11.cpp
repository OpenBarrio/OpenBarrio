#include "ts3/resource/m03_G11.h"
#include <cstring>
#include <utility>
namespace ts3::resource::m03::g11_detail {
const Ops* dispatch=nullptr;
namespace {
void* Allocate(uint32_t bytes) {
    if(dispatch) return dispatch->allocate(bytes);
#if defined(_M_IX86)
    return CallGameMethod<void*,0x004E69D0>(*reinterpret_cast<void**>(0x011CB864),bytes,0u);
#else
    NotRewritten("m03_G11 allocator");
#endif
}
void Swap(uint32_t* p,uint32_t n) {
    for(uint32_t i=0;i<n;++i) { uint32_t v=p[i]; p[i]=(v>>24)|((v>>8)&0xff00)|((v<<8)&0xff0000)|(v<<24); }
}
}
uint8_t Parse(Parser* self,uint32_t* source,uint32_t length,uint32_t copy) {
    if(length<20) return 0;
    uint32_t* base=source;
    if(static_cast<uint8_t>(copy)) {
        self->end=self->begin;
        if(dispatch) dispatch->copy(self,reinterpret_cast<uint8_t*>(source),reinterpret_cast<uint8_t*>(source)+length,copy);
        else CallGameMethod<void,0x00726A60>(self,source,reinterpret_cast<uint8_t*>(source)+length,copy);
        base=reinterpret_cast<uint32_t*>(self->begin);
    }
    if(base[0]>=0x10000u && static_cast<uint16_t>(base[0])==0) {
        Swap(base,5);
        self->swapped=1;
    }
    uint32_t count=base[2]+base[4]+base[3];
    self->count=count;
    self->strings=base[4];
    uint32_t payload=8u*(base[4]+2u*count);
    if(length<payload+20u) return 0;
    if(self->swapped) Swap(base+5,payload>>2);
    count=base[2]+base[4]+base[3];
    // The original allocates 32-bit pointer slots; native builds grow them.
    uint32_t bytes=static_cast<uint32_t>(sizeof(StringRef)*base[4]+sizeof(void*)*count);
    void* allocation=Allocate(bytes);
    if(!allocation) return 0;
    self->storage=allocation;
    auto refs=static_cast<StringRef*>(allocation);
    uint32_t* strings=base+5+4u*count;
    for(uint32_t i=0;i<base[4];++i) {
        refs[i].data=reinterpret_cast<uint8_t*>(base)+strings[2*i];
        refs[i].size=strings[2*i+1];
    }
    if(self->swapped) {
        for(uint32_t i=0;i<count;++i) std::swap(base[5+4*i],base[6+4*i]);
    }
    auto resolved=reinterpret_cast<void**>(refs+base[4]);
    if(count) std::memset(resolved,0,sizeof(void*)*count);
    uint32_t* records=base+5;
    uint32_t sizes[4]={base[1],base[4]-base[1],base[2],base[3]};
    for(unsigned i=0;i<4;++i) {
        self->groups[i]={records,resolved,sizes[i]};
        records+=4u*sizes[i]; resolved+=sizes[i];
    }
    if(static_cast<uint8_t>(copy)) {
        uint32_t required=8u*(self->strings+2u*(sizes[0]+sizes[1]+sizes[2]+sizes[3]))+20u;
        uint32_t used=static_cast<uint32_t>(self->end-self->begin);
        if(required>used) {
            uint8_t zero=0;
            if(dispatch) dispatch->extend(self,self->end,required-used,&zero);
            else CallGameMethod<void,0x00B708E0>(self,self->end,required-used,&zero);
            return 1;
        }
        // EA calls memmove with length zero here, even when truncating.
        self->end=self->begin+required;
    }
    return 1;
}
void ClearCache(Cache* self) {
    if(self->database) {
        KeyList list{nullptr,nullptr,nullptr,"Scripting/CacheKeyList"};
        if(dispatch) dispatch->keys(self->database,&list,0);
        else CallGameVirtual<void,12>(self->database,&list,0u);
        for(auto cursor=list.end;cursor!=list.begin;list.end=cursor) {
            if(cursor[-2]) {
                if(dispatch) dispatch->remove(self->database,cursor-4);
                else CallGameVirtual<void,16>(self->database,cursor-4);
                cursor=list.end;
            }
            cursor-=4;
        }
        if(dispatch) dispatch->flush(self->database);
        else CallGameVirtual<void,9>(self->database);
        if(list.begin) {
            if(dispatch) dispatch->free(list.begin);
            else CallGameFunction<void,0x004E3F70>(list.begin);
        }
    }
}
}
namespace ts3::resource::m03::g11_detail {
uint8_t Parser::Parse(void* data,uint32_t length,uint32_t copy) { return g11_detail::Parse(this,static_cast<uint32_t*>(data),length,copy); }
void Cache::ClearCache() { g11_detail::ClearCache(this); }
}

namespace ts3::resource::m03 {
uint8_t g11::Parse(void* source,uint32_t length,uint32_t copy) {
    return g11_detail::Parse(reinterpret_cast<g11_detail::Parser*>(this),static_cast<uint32_t*>(source),length,copy);
}
void g11::ClearCache() { g11_detail::ClearCache(reinterpret_cast<g11_detail::Cache*>(this)); }
}
