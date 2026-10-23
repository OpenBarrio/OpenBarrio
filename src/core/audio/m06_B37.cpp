#include "ts3/audio/m06_B37.h"
#include <algorithm>
#include <cstring>
#include <cmath>
#include <xmmintrin.h>
namespace ts3::audio::b37 {
Queue* ConstructQueue(Queue* s,const char* label) {
    s->refs=0; s->secondary=reinterpret_cast<void*>(uintptr_t(0x0107396c));
    s->state=0; s->label=label;
    s->primary=reinterpret_cast<void*>(uintptr_t(0x01031680));
    s->secondary=reinterpret_cast<void*>(uintptr_t(0x0103167c));
    ts3::CallGameMethod<void,0x008BB3F0>(s->containers,&label,&label);
    ts3::CallGameMethod<void,0x008A3C80>(s->containers+0xff0,&label,&label);
    ts3::CallGameMethod<void,0x008A3C80>(s->containers+0x1428,&label,&label);
    return s;
}
Queue* DeleteQueue(Queue* s,uint32_t flags) {
    // Starting point: verified WorkQueue::DeletingDestruct; only destructor target changes.
    ts3::CallGameMethod<void,0x008A3B70>(s);
    if(flags & 1) ts3::CallGameFunction<void,0x004E3F70>(s);
    return s;
}
#if defined(_M_IX86)
template<class R,class... A> R Import(uintptr_t slot,A... args) {
    return (*reinterpret_cast<R(__stdcall**)(A...)>(slot))(args...);
}
template<class R,class... A> R Com(void* p,unsigned slot,A... args) {
    return reinterpret_cast<R(__stdcall*)(void*,A...)>((*static_cast<void***>(p))[slot])(p,args...);
}
#endif
uint32_t ProbeEndpoint() {
#if defined(_M_IX86)
    uint32_t version[37]{}; version[0]=0x94;
    if(!Import<int>(0x00F9518C,version) || version[1]<6) return 0;
    if(Import<int32_t>(0x00F95AAC,uintptr_t(0))<0) return 0;
    uint32_t result=0; void *enumerator=nullptr,*device=nullptr,*property=nullptr,*value=nullptr;
    if(Import<int32_t>(0x00F95AA8,reinterpret_cast<void*>(0x01031704),uintptr_t(0),uint32_t(23),reinterpret_cast<void*>(0x01031714),&enumerator)>=0) {
        if(Com<int32_t>(enumerator,4,uint32_t(0),uint32_t(1),&device)>=0) {
            if(Com<int32_t>(device,3,reinterpret_cast<void*>(0x010316F4),uint32_t(23),uintptr_t(0),&property)>=0) {
                if(Com<int32_t>(property,8,&value)>=0) {
                    result=static_cast<uint32_t*>(value)[1];
                    Import<void>(0x00F95AA4,value);
                }
                Com<uint32_t>(property,2);
            }
            Com<uint32_t>(device,2);
        }
        Com<uint32_t>(enumerator,2);
    }
    Import<void>(0x00F95AB8);
    return result;
#else
    ts3::AbortNotRewritten("Windows audio endpoint COM probe"); return 0;
#endif
}
uintptr_t Step(Selector* s,uint8_t forward,uintptr_t residue) {
    s->mode=forward ? 2 : 3;
    if(s->entries.end-s->entries.begin<=1) return (residue & ~uintptr_t(255)) | forward;
    if(forward) { if(++s->cursor==s->entries.end) s->cursor=s->entries.begin; }
    else { if(s->cursor==s->entries.begin) s->cursor=s->entries.end; --s->cursor; }
    s->current=*s->cursor;
    return reinterpret_cast<uintptr_t>(&s->current);
}
#if defined(_M_IX86)
__declspec(naked) uintptr_t __fastcall HookStep(Selector*,void*,uint8_t) {
    __asm {
        push eax
        movzx eax,byte ptr [esp+8]
        push eax
        push ecx
        call Step
        add esp,12
        ret 4
    }
}
#else
uintptr_t __fastcall HookStep(Selector* s,void*,uint8_t f) {return Step(s,f);}
#endif
Record* Get(Selector* s,Record* out) {
    s->mode=1;
    if(s->entries.begin==s->entries.end) {
        out->word[0]=out->word[1]=out->word[2]=0;
    } else {
        if(!s->cursor || s->cursor==s->entries.end) s->cursor=s->entries.begin;
        s->current=*s->cursor; *out=*s->cursor;
    }
    return out;
}
uint32_t ResetResidue(Selector* s,uint32_t residue) {
    s->remaining=0; s->mode=0; s->entries.end=s->entries.begin;
    s->current={{0,0,0,residue}}; s->cursor=s->entries.begin;
    return (residue & 0xffffff00u)|1;
}
#if defined(_M_IX86)
__declspec(naked) uint32_t __fastcall HookReset(Selector*,void*) {
    __asm {
        push dword ptr [esp-4]
        push ecx
        call ResetResidue
        add esp,8
        ret
    }
}
#else
uint32_t __fastcall HookReset(Selector* s,void*) {return ResetResidue(s,0);}
#endif
uintptr_t ShuffleResidue(Record* first,Record* last,uint32_t(__cdecl**random)(uint32_t),uintptr_t residue) {
    for(auto p=first+1;p<last;++p) {
        auto q=first+(*random)(uint32_t(p-first+1));
        Record saved=*p; *p=*q; *q=saved;
        residue=reinterpret_cast<uintptr_t>(q);
    }
    return residue;
}
#if defined(_M_IX86)
__declspec(naked) uintptr_t Shuffle(Record*,Record*,uint32_t(__cdecl**)(uint32_t)) {
    __asm {
        push eax
        push dword ptr [esp+16]
        push dword ptr [esp+16]
        push dword ptr [esp+16]
        call ShuffleResidue
        add esp,16
        ret
    }
}
#else
uintptr_t Shuffle(Record* first,Record* last,uint32_t(__cdecl**random)(uint32_t)) {return ShuffleResidue(first,last,random,0);}
#endif
uintptr_t Advance(Selector* s,uint8_t force) {
    uint32_t old=s->remaining--;
    if(old==0 || force) {
        auto random=+[](uint32_t n)->uint32_t {return ts3::CallGameMethod<uint32_t,0x004FA840>(reinterpret_cast<void*>(uintptr_t(0x011D9FAC)),n);};
        Shuffle(s->entries.begin,s->entries.end,&random);
        s->cursor=s->entries.begin;
        auto count=s->entries.end-s->entries.begin;
        s->remaining=count==1 ? 0x7fffffff : uint32_t(std::max<ptrdiff_t>(count-1,0));
        if(count>1 && std::memcmp(s->cursor,&s->current,12)==0) {Step(s,1);return s->current.word[3];}
        // EA reads begin even for an empty vector (the fixture supplies readable inline storage).
        s->current=*s->cursor;
        return reinterpret_cast<uintptr_t>(s->entries.begin);
    }
    if(s->entries.end-s->entries.begin<=1) return old;
    if(s->mode==0 || s->mode==1) return Step(s,1);
    return s->mode;
}
uintptr_t Insert(Vector* s,Record* pos,const Record* value) {
    if(s->end!=s->capacity) {
        if(value>=pos && value<s->end) ++value;
        *s->end=s->end[-1];
        // EA's copy-backward helper terminates on equality, not ordering: a non-full
        // insertion at end is invalid and walks backwards without a bound.
        auto from=s->end-1, to=s->end;
        while(from!=pos) *--to=*--from;
        *pos=*value; ++s->end;
        return value->word[3];
    }
    auto count=s->end-s->begin;
    auto capacity=count ? count*2 : 1;
    auto data=ts3::CallGameFunction<Record*,0x004E3F90>(uint32_t(capacity*sizeof(Record)),s->label,0u,0u,reinterpret_cast<const char*>(uintptr_t(0x00F98980)),231u);
    auto prefix=pos-s->begin;
    std::memcpy(data,s->begin,size_t(prefix)*sizeof(Record));
    data[prefix]=*value;
    std::memcpy(data+prefix+1,pos,size_t(s->end-pos)*sizeof(Record));
    uintptr_t result=reinterpret_cast<uintptr_t>(s->begin);
    if(s->begin && s->begin!=s->inlineBuffer)
        result=ts3::CallGameFunction<uintptr_t,0x004E3F70>(s->begin);
    s->begin=data; s->end=data+count+1; s->capacity=data+capacity;
    return result;
}
uintptr_t Append(Selector* s,Record value) {
    auto p=s->entries.end;
    if(p<s->entries.capacity) {s->entries.end=p+1; if(p) *p=value; return reinterpret_cast<uintptr_t>(p);}
    return Insert(&s->entries,p,&value);
}
uintptr_t FilterDescriptor() {return 0x01169804;}
uint32_t Process(Crusher* node,Context* ctx) {
    int bits=_mm_cvtt_ss2si(_mm_set_ss(node->bits));
    uint32_t shift=16u-uint32_t(bits);
    if(shift==0) return 1;
    uint16_t mask=uint16_t(~((1u<<(shift&31))-1u));
    float mix=node->mix, dry=_mm_cvtss_f32(_mm_sub_ss(_mm_set_ss(1),_mm_set_ss(mix)));
    for(unsigned channel=0;channel<node->channels;++channel) {
        auto in=ctx->input->data+channel*ctx->input->stride;
        auto out=ctx->output->data+channel*ctx->output->stride;
        for(int i=0;i<ctx->frames;++i) {
            float x=in[i];
            if(mix==0) {out[i]=x;continue;}
            auto q=_mm_cvtt_ss2si(_mm_mul_ss(_mm_set_ss(x),_mm_set_ss(32767.0f)));
            float wet=float(int16_t(uint16_t(q)&mask));
            auto y=_mm_mul_ss(_mm_set_ss(wet),_mm_set_ss(0.000030518509447574615f));
            if(mix!=1) y=_mm_add_ss(_mm_mul_ss(y,_mm_set_ss(mix)),_mm_mul_ss(_mm_set_ss(x),_mm_set_ss(dry)));
            out[i]=_mm_cvtss_f32(y);
        }
    }
    std::swap(ctx->input,ctx->output);
    return 1;
}
uintptr_t InitCrusher(Crusher* s) {
    if(s) s->table=reinterpret_cast<void*>(uintptr_t(0x0111DB94));
    s->bits=16; s->parameters=&s->bits; s->mix=0;
    return (reinterpret_cast<uintptr_t>(s)&~uintptr_t(255))|1;
}
uint32_t CrusherSize() {return 0x190;}
uintptr_t CrusherDescriptor() {return 0x01169BFC;}
}
