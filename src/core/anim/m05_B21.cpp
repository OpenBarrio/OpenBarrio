#include "ts3/anim/m05_B21.h"
#include "ts3/swarm/clu_405a50_Swarm.h"
#include "ts3/scratch/clu_4e3f70_Scratch.h"
#include <cstring>
namespace ts3::anim::b21 {
namespace {
uintptr_t Address(const void* p) { return reinterpret_cast<uintptr_t>(p); }
intptr_t Span(const void* a,const void* b) { return static_cast<intptr_t>(Address(a)-Address(b)); }
Joint* Next(Joint* p) { Joint* n; std::memcpy(&n,p->opaque00,sizeof n); return n; }
void Link(Joint* p,Joint* n) { std::memcpy(p->opaque00,&n,sizeof n); }
char emptyName[2]{};
template<unsigned A> void* GameData() {
#if defined(_M_IX86)
    return reinterpret_cast<void*>(uintptr_t(A));
#else
    if constexpr(A==0x011c7e54) return emptyName;
    if constexpr(A==0x011c7e55) return emptyName+1;
    static unsigned char token[2]{}; return token;
#endif
}
void Free(void* p) { scratch::Free4E3F70(p); }
Joint* CopyConstruct(Joint* first,Joint* last,Joint* dest) {
    Joint* out=dest;
    CallGameFunction<void*,0x005758d0>(&out,first,last,dest,uint32_t(0));
    return out;
}
void FillConstruct(Joint* dest,uint32_t n,const Joint* src) {
    CallGameFunction<void,0x005759f0>(dest,n,src,uint32_t(0));
}
void FillAssign(Joint* first,Joint* last,const Joint* src) { CallGameFunction<void,0x00575f20>(first,last,src); }
}
int32_t Count(const SkeletonView* p) { return static_cast<int32_t>(Span(p->end,p->begin)/intptr_t(sizeof(Joint))); }
char* JointName(const SkeletonView* p,int32_t n) { return p->begin[n].nameBegin; }
uint32_t JointHash(const SkeletonView* p,int32_t n) { return p->begin[n].flags; }
int32_t ChainIndex(const SkeletonView* p,int32_t n) { return p->begin[n].index58; }
int32_t MappedIndex(const SkeletonView* p,int32_t n) { return n<0 || p->begin[n].index54<0 ? -1 : p->begin[n].index54; }
uint32_t IsAncestor(const SkeletonView* p,int32_t n,int32_t target) {
    if(n>=0 && target>=0) {
        while(n!=target) { n=p->begin[n].index58; if(n<0) return static_cast<uint32_t>(n)&~255u; }
        return (static_cast<uint32_t>(n)&~255u)|1;
    }
    return static_cast<uint32_t>(n)&~255u;
}
Joint* ConstructJoint(Joint* p) {
    p->nameBegin=p->nameEnd=static_cast<char*>(GameData<0x011c7e54>());
    p->nameCapacity=static_cast<char*>(GameData<0x011c7e55>());
    p->descriptor=GameData<0x00fc2d98>();
    p->parent=p->index54=p->index58=-1; p->flags=0; p->value60=0x20;
    p->translation={{0,0,0,0}}; p->rotation={{0,0,0,1}}; p->scale={{1,1,1,1}};
    return p;
}
Iterator* Find(Hash* p,Iterator* out,const uint32_t* key) {
    Joint** bucket=&p->buckets[*key%13];
    for(Joint* n=*bucket;n;n=Next(n)) if(n->flags==*key) { out->node=n; out->bucket=bucket; return out; }
    out->node=p->sentinel; out->bucket=&p->sentinel; return out;
}
InsertResult* InsertHash(Hash* p,InsertResult* out,Joint* node,uint32_t) {
    Joint** bucket=&p->buckets[node->flags%13];
    for(Joint* n=*bucket;n;n=Next(n)) if(n->flags==node->flags) {
        out->node=n; out->bucket=bucket; out->inserted=0; return out;
    }
    Link(node,*bucket); *bucket=node; ++p->count;
    out->node=node; out->bucket=bucket; out->inserted=1; return out;
}
int32_t FindIndex(SkeletonView* p,uint32_t key) { Iterator out; Find(&p->hash,&out,&key); return out.node==p->hash.sentinel ? -1 : out.node->parent; }
Name* MirrorName(Name* out,const char* source,uint32_t offset,int32_t right) {
    out->begin=out->end=static_cast<char*>(GameData<0x011c7e54>());
    out->capacity=static_cast<char*>(GameData<0x011c7e55>()); out->descriptor=GameData<0x00fc2dd8>();
    auto append=[&](const char* a,const char* b) { CallGameMethod<void,0x00405790>(out,a,b); };
    auto text=[&](const char* s) { CallGameMethod<void,0x00405960>(out,s); };
    append(source,source+offset);
    const char* s=source+offset; uint32_t skip;
    if(s[1]=='_') {
        if(s[0]==(right?'r':'l')) { const char* replacement=right?"l_":"r_"; append(replacement,replacement+2); }
        else if(s[0]==(right?'R':'L')) text(right?"L_":"R_");
        skip=2;
    } else if(static_cast<signed char>(s[1])>='0' && static_cast<signed char>(s[1])<='9') {
        text(right?"L":"R"); skip=1;
    } else {
        if(s[0]==(right?'r':'l')) text(right?"left":"right");
        else if(s[0]==(right?'R':'L')) text(right?"Left":"Right");
        skip=right?5:4;
    }
    s+=skip; append(s,s+std::strlen(s)); return out;
}
uintptr_t __stdcall FreeNames(Joint* first,const Joint* last) {
    uintptr_t result=Address(last); // Empty-range EAX is caller-seeded, as in the verified twin.
    for(auto* p=first;Address(p)<Address(last);++p) {
        result=Address(p->nameBegin);
        if(Span(p->nameCapacity,p->nameBegin)>1 && p->nameBegin) {
            result=Address(scratch::Free4E3F70(p->nameBegin));
        }
    }
    return result;
}
Joint* Erase(Vector* p,Joint* first,Joint* last) {
#if defined(_M_IX86)
    // The verified twin uses an explicitly x86 raw-byte joint layout.
    auto* tail=reinterpret_cast<Joint*>(swarm::Copy575D50(reinterpret_cast<const swarm::Obj575CE0*>(last),reinterpret_cast<const swarm::Obj575CE0*>(p->end),reinterpret_cast<swarm::Obj575CE0*>(first)));
#else
    // Widen the link and name pointers rather than invoking that fixed-stride view.
    Joint* tail=first;
    for(auto* src=last;src!=p->end;++src,++tail) {
        Link(tail,Next(src));
        tail->translation=src->translation; tail->rotation=src->rotation; tail->scale=src->scale;
        if(tail!=src) reinterpret_cast<swarm::ByteVector*>(&tail->nameBegin)->Assign(src->nameBegin,src->nameEnd);
        tail->parent=src->parent; tail->index54=src->index54; tail->index58=src->index58;
        tail->flags=src->flags; tail->value60=src->value60;
    }
#endif
    FreeNames(tail,p->end);
    p->end-=Span(last,first)/sizeof(Joint); return first;
}
void Insert(Vector* p,Joint* position,uint32_t n,const Joint* value) {
    const uint32_t spare=static_cast<uint32_t>(Span(p->capacity,p->end)/sizeof(Joint));
    if(n<=spare) {
        if(!n) return;
        alignas(16) Joint temporary;
        CallGameMethod<Joint*,0x00575870>(&temporary,value);
        auto* oldEnd=p->end;
        auto after=static_cast<uint32_t>(Span(oldEnd,position)/sizeof(Joint));
        if(n<after) {
            CopyConstruct(oldEnd-n,oldEnd,oldEnd); p->end+=n;
            CallGameFunction<Joint*,0x00575d90>(position,oldEnd-n,oldEnd);
            FillAssign(position,position+n,&temporary);
        } else {
            FillConstruct(oldEnd,n-after,&temporary); p->end+=n-after;
            CopyConstruct(position,oldEnd,p->end); p->end+=after;
            FillAssign(position,oldEnd,&temporary);
        }
        FreeNames(&temporary,&temporary+1);
    } else {
        uint32_t size=static_cast<uint32_t>(Span(p->end,p->begin)/intptr_t(sizeof(Joint)));
        uint32_t capacity=size*2; if(!capacity) capacity=1;
        if(capacity<=size+n) capacity=size+n;
        Joint* storage=nullptr;
        if(capacity) storage=CallGameFunction<Joint*,0x004e3fb0>(uint32_t(capacity*sizeof(Joint)),uint32_t(16),uint32_t(0),p->descriptor,uint32_t(0),uint32_t(0),GameData<0x00f98980>(),uint32_t(0xfb));
        Joint* end=CopyConstruct(p->begin,position,storage);
        FillConstruct(end,n,value); end+=n;
        end=CopyConstruct(position,p->end,end);
        FreeNames(p->begin,p->end); if(p->begin) Free(p->begin);
        p->begin=storage; p->end=end; p->capacity=storage+capacity;
    }
}
void Resize(Vector* p,uint32_t n) {
    uint32_t size=static_cast<uint32_t>(Span(p->end,p->begin)/intptr_t(sizeof(Joint)));
    if(n>size) { alignas(16) Joint value; ConstructJoint(&value); Insert(p,p->end,n-size,&value); FreeNames(&value,&value+1); }
    else Erase(p,p->begin+n,p->end);
}
}
