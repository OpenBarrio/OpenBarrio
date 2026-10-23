#include "ts3/anim/m05_B32.h"
#include <cstring>
#include <atomic>
namespace ts3::anim::b32 {
Entity* Child(Entity* s, int32_t i) {
    if (i < 0 || i >= s->count || i >= s->end-s->begin) return nullptr;
    return s->begin[i];
}
void Identifiers(Entity* s, uint64_t* out) {
    for (int32_t i=0;i<s->count;++i) out[i]=s->begin[i]?CallGameVirtual<uint64_t,4>(s->begin[i]):0;
}
Ref** Assign(Ref** s, Ref* p) {
    Ref* old=*s;
    if (old!=p) {
        if(p) ++p->refs;
        *s=p;
        if(old && --old->refs==0) {
            old->refs=1;
            CallGameVirtual<void,0>(&old->deleting,1u);
        }
    }
    return s;
}
void SetOwner(Owner* s, Ref* p) { Assign(&s->value,p); }
// The scorer ABI leaves ST0 extended. EA compares before popping/rounding it.
static bool CandidateAbove(void* scorer, uint64_t identifier, float score, float* rounded) {
#if defined(_M_IX86)
    auto target=(*reinterpret_cast<void***>(scorer))[41];
    uint32_t lo=static_cast<uint32_t>(identifier),hi=static_cast<uint32_t>(identifier>>32);
    uint8_t above;
    __asm {
        push hi
        push lo
        mov ecx,scorer
        call target
        mov edx,rounded
        fst dword ptr [edx]
        fld score
        fxch st(1)
        fcompp
        fnstsw ax
        sahf
        seta al
        mov above,al
    }
    return above!=0;
#else
    *rounded=CallGameVirtual<float,41>(scorer,identifier);
    return *rounded>score;
#endif
}
Entity* Best(Entity* s, void* scorer) {
    if(!scorer) return nullptr;
    Entity* best=s;
    float score=CallGameVirtual<float,41>(scorer,CallGameVirtual<uint64_t,4>(s));
    for(int32_t i=0;i<s->count;++i) {
        Entity* p=s->begin[i];
        if(!p) continue;
        float candidate;
        if(CandidateAbove(scorer,CallGameVirtual<uint64_t,4>(p),score,&candidate)) {
            best=s->begin[i]; score=candidate;
        }
    }
    auto* refs=reinterpret_cast<int32_t*>(static_cast<unsigned char*>(scorer)+sizeof(void*));
    if(std::atomic_ref<int32_t>(*refs).fetch_sub(1)==1) {
        std::atomic_ref<int32_t>(*refs).exchange(1);
        CallGameVirtual<void,0>(scorer,1u);
    }
    return best;
}
void SetChildren(Entity* s, Entity** p, int32_t n, uint8_t flag) {
    if(p && n>0) {
        s->flag=flag; s->count=n;
        CallGameMethod<void,0x00AEF830>(&s->begin,n);
        for(int32_t i=0;i<s->count;++i) s->begin[i]=p[i];
    } else { s->end=s->begin; s->count=0; s->flag=0; }
}
void Advance(void* it) {
#if defined(_M_IX86)
    while(!static_cast<uint8_t>(CallGameMethod<uint32_t,0x00471020>(it))) {
        if(!static_cast<uint8_t>(CallGameMethod<uint32_t,0x00471260>(it))) break;
        if(CallGameMethod<int32_t,0x00471040>(it)==2) {
            const uint16_t* text=CallGameMethod<const uint16_t*,0x00471060>(it);
            const uint16_t* token=reinterpret_cast<const uint16_t*>(0x01022FDC);
            while(*token==*text && *token) { ++token; ++text; }
            if(*token==*text) break;
        }
    }
#else
    (void)it; AbortNotRewritten("external iterator");
#endif
}
void Attach(Linked* s, Linked* p) {
    if(!p || p->link.previous || p->link.next) return;
    p->link.next=s->link.next;
    p->link.previous=&s->link;
    s->link.next=&p->link;
    p->link.next->previous=&p->link;
}
bool Ancestor(const Handle* a, const Handle* b) {
    if(!a || !b || !b->object) return false;
    void* parent=b->object;
    do {
#if defined(_M_IX86)
        void* obj=CallGameMethod<void*,0x00935770>(parent,*reinterpret_cast<uint32_t*>(0x011E9158));
        if(!obj) return false;
        parent=*reinterpret_cast<void**>(static_cast<unsigned char*>(obj)+0x30);
#else
        AbortNotRewritten("parent lookup"); return false;
#endif
        if(parent==a->object) return true;
    } while(parent);
    return false;
}
template<class N> void Rehash(Table<N>* s, uint32_t n) {
    N** fresh;
#if defined(_M_IX86)
    fresh=CallGameFunction<N**,0x004E3F90>(n*4+4,s->descriptor,0x400000,0,0x00F98980,0xe7);
#else
    fresh=new N*[size_t(n)+1];
#endif
    std::memset(fresh,0,n*sizeof(N*));
    fresh[n]=reinterpret_cast<N*>(~uintptr_t(0));
    for(uint32_t i=0;i<s->count;++i) {
        while(N* p=s->buckets[i]) {
            uint32_t hash=p->key;
            if constexpr (requires { p->other; }) hash^=p->other;
            uint32_t bucket=hash%n;
            s->buckets[i]=p->next;
            p->next=fresh[bucket]; fresh[bucket]=p;
        }
    }
    if(s->count>1) {
#if defined(_M_IX86)
        CallGameFunction<void,0x004E3F70>(s->buckets);
#else
        delete[] s->buckets;
#endif
    }
    s->buckets=fresh; s->count=n;
}
void RehashLarge(Table<NodeLarge>* s,uint32_t n) { Rehash(s,n); }
void RehashSmall(Table<NodeSmall>* s,uint32_t n) { Rehash(s,n); }
void RehashPair(Table<NodePair>* s,uint32_t n) { Rehash(s,n); }
uintptr_t Reverse(const uint16_t* first,const uint16_t* last,const uint16_t* needle,const uint16_t* end) {
    if(first==last || needle==end) return reinterpret_cast<uintptr_t>(first);
    ptrdiff_t n=end-needle;
    if(n==1) {
        for(auto p=last;p!=first;) { --p; if(*p==*needle) return reinterpret_cast<uintptr_t>(p); }
        return reinterpret_cast<uintptr_t>(last);
    }
    if(n<=last-first) {
        auto p=last-n;
        for(;;) {
            ptrdiff_t j=0;
            while(j<n && p[j]==needle[j]) ++j;
            if(j==n) return reinterpret_cast<uintptr_t>(p);
            if(p==first) break;
            --p;
        }
    }
    return reinterpret_cast<uintptr_t>(last);
}
uint32_t Find(Text* s,const uint16_t* needle,uint32_t pos,uint32_t n) {
    uint32_t length=static_cast<uint32_t>(s->end-s->begin);
    if(n>length) return ~0u;
    if(!n) return pos<length?pos:length;
    uint32_t limit=pos<length-n?pos:length-n;
    auto last=s->begin+limit+n;
    uintptr_t result=Reverse(s->begin,last,needle,needle+n);
    if(result==reinterpret_cast<uintptr_t>(last)) return ~0u;
    return static_cast<uint32_t>(static_cast<intptr_t>(result-reinterpret_cast<uintptr_t>(s->begin))>>1);
}
uint32_t FindZ(Text* s,const uint16_t* needle,uint32_t pos) {
    uint32_t n=0; while(needle[n]) ++n;
    return Find(s,needle,pos,n);
}
}

namespace ts3::anim::b32 {
Entity* __fastcall HookChild(Entity* s, void*, int32_t i) { return Child(s,i); }
void __fastcall HookIdentifiers(Entity* s, void*, uint64_t* out) { return Identifiers(s,out); }
Ref** __fastcall HookAssign(Ref** s, void*, Ref* p) { return Assign(s,p); }
void __fastcall HookSetOwner(Owner* s, void*, Ref* p) { return SetOwner(s,p); }
Entity* __fastcall HookBest(Entity* s, void*, void* p) { return Best(s,p); }
void __fastcall HookSetChildren(Entity* s, void*, Entity** p, int32_t n, uint8_t f) { return SetChildren(s,p,n,f); }
void __fastcall HookAdvance(void*, void*, void* p) { return Advance(p); }
void __fastcall HookAttach(Linked* s, void*, Linked* p) { return Attach(s,p); }
bool __cdecl HookAncestor(const Handle* a, const Handle* b) { return Ancestor(a,b); }
void __fastcall HookRehashLarge(Table<NodeLarge>* s, void*, uint32_t n) { return RehashLarge(s,n); }
void __fastcall HookRehashSmall(Table<NodeSmall>* s, void*, uint32_t n) { return RehashSmall(s,n); }
void __fastcall HookRehashPair(Table<NodePair>* s, void*, uint32_t n) { return RehashPair(s,n); }
uintptr_t __cdecl HookReverse(const uint16_t* a, const uint16_t* b, const uint16_t* c, const uint16_t* d) { return Reverse(a,b,c,d); }
uint32_t __fastcall HookFind(Text* s, void*, const uint16_t* p, uint32_t pos, uint32_t n) { return Find(s,p,pos,n); }
uint32_t __fastcall HookFindZ(Text* s, void*, const uint16_t* p, uint32_t pos) { return FindZ(s,p,pos); }
}
