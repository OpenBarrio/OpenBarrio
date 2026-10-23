#include "ts3/anim/m05_B06.h"
#include <cstring>
namespace ts3::anim::b06 {
uint32_t Type() { return 0x06b264ea; }
void* Query(void* self,uint32_t id) { return id==Type() || id==0x73deb8c8 ? self : nullptr; }
static uint32_t Word(const void* p) { uint32_t v; std::memcpy(&v,p,4); return v; }
static void Advance(int32_t& c,uint32_t n) { c=static_cast<int32_t>(static_cast<uint32_t>(c)+n); }
uintptr_t SavePose(PoseNode* p,Writer* s) {
    uintptr_t result=0;
    const void* fields[]={p->rotation,p->translation}; const int sizes[]={16,12};
    for(int i=0;i<2;++i) {
        if(s->cursor>=-sizes[i]) {
            result=CallGameMethod<uint32_t,0x00594140>(s,0);
            if(!(result&255)) { s->error=1; continue; }
        }
        auto* dest=s->end+s->cursor+1;
        for(int lane=0;lane<sizes[i];lane+=4) {
            uint32_t v=Word(static_cast<const unsigned char*>(fields[i])+lane);
            std::memcpy(dest+lane,&v,4);
        }
        Advance(s->cursor,sizes[i]);
        result=reinterpret_cast<uintptr_t>(dest);
    }
    return result;
}
uintptr_t LoadPose(PoseNode* p,Reader* s) {
    uintptr_t result=0;
    void* fields[]={p->rotation,p->translation}; const int sizes[]={16,12};
    for(int i=0;i<2;++i) {
        if(s->cursor>=-sizes[i]) {
            result=CallGameMethod<uint32_t,0x00593ff0>(s,0);
            if(!(result&255)) { s->error=1; continue; }
        }
        auto* src=s->end+s->cursor+1;
        for(int lane=0;lane<sizes[i];lane+=4) {
            uint32_t v=Word(src+lane);
            std::memcpy(static_cast<unsigned char*>(fields[i])+lane,&v,4);
            result=v;
        }
        Advance(s->cursor,sizes[i]);
    }
    return result;
}
uintptr_t Evaluate(PoseNode* p,void* arg,void* output) {
    auto* list=CallGameMethod<Link*,0x004da9d0>(p);
    uintptr_t result=reinterpret_cast<uintptr_t>(list);
    if(list->previous!=list) {
        auto* child=list->next ? reinterpret_cast<PoseNode*>(reinterpret_cast<unsigned char*>(list->next)-sizeof(void*)) : nullptr;
        result=CallGameVirtual<uintptr_t,2>(child,arg,output);
    }
    std::memcpy(static_cast<unsigned char*>(output)+16,p->translation,16);
    std::memcpy(static_cast<unsigned char*>(output)+32,p->rotation,16);
    return result;
}
float Remaining(void* p,uint32_t a,uint32_t b) {
    // The dependency returns float in ST0; subtraction stays in x87 until return on x86.
    return *reinterpret_cast<float*>(static_cast<unsigned char*>(p)+0x68)-CallGameMethod<float,0x0085cc20>(p,a,b);
}
uint32_t IsLocal(uint32_t v) { return v==0x200 || v==0x300 || v==0x400 || v==0x500 || v==0x600 || v==0x700 || v==0x800 || v==0x4400; }
uint32_t IsRouting(uint32_t v) { return v==0x100 || v==0x10000 || v==0x900 || v==0xa00 || v==0xb00; }
void* SetCallback(State* p,void* callback,void* user) { p->callback=callback; p->user=user; return callback; }
uintptr_t SaveState(State* p,Writer* s) {
    const void* fields[]={&p->first,&p->second,&p->enabled,&p->a,&p->b,&p->last};
    const int sizes[]={4,4,1,4,4,4}; uintptr_t eax=0;
    for(int i=0;i<6;++i) {
        Advance(s->cursor,sizes[i]);
        if(s->cursor>=0) eax=CallGameMethod<uint32_t,0x00594140>(s,sizes[i]);
        if(s->cursor<0) {
            auto* dest=s->end+s->cursor+1-sizes[i];
            std::memcpy(dest,fields[i],sizes[i]);
            if(i==0 || i==3) eax=reinterpret_cast<uintptr_t>(s->end);
            if(i==1 || i==4) eax=static_cast<uint32_t>(s->cursor);
        }
    }
    return eax;
}
uintptr_t LoadState(State* p,Reader* s) {
    void* fields[]={&p->first,&p->second,&p->enabled,&p->a,&p->b,&p->last};
    const int sizes[]={4,4,1,4,4,4}; uintptr_t eax=0;
    for(int i=0;i<6;++i) {
        Advance(s->cursor,sizes[i]);
        if(s->cursor>=0) eax=CallGameMethod<uint32_t,0x00593ff0>(s,sizes[i]);
        if(i==2) {
            uint8_t v=s->cursor<0 ? *(s->end+s->cursor) : static_cast<uint8_t>(reinterpret_cast<uintptr_t>(s));
            eax=(eax&~uintptr_t(255)) | (v!=0); p->enabled=v!=0;
        } else if(s->cursor<0) {
            auto* src=s->end+s->cursor+1-sizes[i];
            // FLD/FSTP quiets signalling NaNs on float fields, unlike the integer fields.
            if(i==0 || i==1 || i==5) {
                uint32_t v=Word(src);
                if((v&0x7f800000)==0x7f800000 && (v&0x7fffff)) v|=0x400000;
                std::memcpy(fields[i],&v,4);
                if(i==0) eax=reinterpret_cast<uintptr_t>(s->end);
                if(i==1) eax=static_cast<uint32_t>(s->cursor);
            } else { eax=Word(src); std::memcpy(fields[i],src,4); }
        }
    }
    return eax;
}
static void* Find(const void* p,bool pose) {
    if(!p) return nullptr;
#if defined(_M_IX86)
    auto mask=*reinterpret_cast<const uint32_t*>(pose?0x011dc484:0x011e9138);
    return CallGameMethod<void*,0x00935770>(p,mask);
#else
    (void)pose; AbortNotRewritten("m05_B06 binding mask"); return nullptr;
#endif
}
void* FindRoute(const void* p) { return Find(p,false); }
void* FindPose(const void* p) { return Find(p,true); }
const float* Maximum(const float* a,const float* b) { return *b>*a ? b : a; }
const void* SetWorkspace(void* p,const void* src) {
    auto* dest=static_cast<unsigned char*>(p)+0x19b10;
    std::memcpy(dest,src,16);
    std::memcpy(dest+16,static_cast<const unsigned char*>(src)+16,16);
    return src;
}
}
