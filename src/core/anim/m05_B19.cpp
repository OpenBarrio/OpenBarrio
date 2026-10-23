#include "ts3/anim/m05_B19.h"
#include <cstring>
namespace ts3::anim::b19 {
namespace {
void Release(Ref* p) { if(p && --p->refs==0) { p->refs=1; CallGameVirtual<void,0>(p,uint32_t(1)); } }
// EA uses FLD/FSTP for scalar copies, not MOVSS: signaling NaNs become quiet.
void CopyScalar(void* destination,const void* source) {
#if defined(_M_IX86)
    __asm {
        mov eax, source
        fld dword ptr [eax]
        mov eax, destination
        fstp dword ptr [eax]
    }
#else
    uint32_t bits;
    std::memcpy(&bits,source,4);
    if((bits&0x7f800000u)==0x7f800000u && (bits&0x007fffffu)) bits|=0x00400000u;
    std::memcpy(destination,&bits,4);
#endif
}
void CopyRecord(Record* d,const Record* s) {
    std::memcpy(d->bytes,s->bytes,48);
    CopyScalar(d->bytes+48,s->bytes+48);
}
void Tail(Group* d,const Group* s) {
    std::memcpy(d->vectors,s->vectors,32);
    CopyScalar(&d->weight,&s->weight);
    CopyScalar(&d->contribution,&s->contribution);
    d->flags=s->flags;
}
template<class T> T* Advance(T* p,ptrdiff_t n) { return reinterpret_cast<T*>(reinterpret_cast<uintptr_t>(p)+n*sizeof(T)); }
}
uint8_t ConsumeDirty(DirtyState* s) { uint8_t r=uint8_t(s->flags&1); s->flags&=~1u; if(s->auxiliary) { r|=s->auxiliary->dirty; s->auxiliary->dirty=0; } return r; }
Coherent* InitCoherent(Coherent* s) {
    auto b=s->bytes; std::memset(b,0,0x24); float one=1,minus=-1;
    std::memcpy(b+0x30,&one,4); std::memset(b+0x34,0,4); std::memcpy(b+0x38,&minus,4);
    std::memset(b+0x3c,0,8); std::memset(b+0x50,0,0x28); return s;
}
Coherent* GetCoherent(CoherentOwner* o,bool create) {
    auto p=reinterpret_cast<Coherent*>(o->tagged&~uintptr_t(3));
    if(create && !p) {
#if defined(_M_IX86)
        p=CallGameFunction<Coherent*,0x004E3F90>(uint32_t(0x80),uintptr_t(0xfc2d08),0,0,0,0);
#else
        AbortNotRewritten("coherent allocator");
#endif
        if(p) InitCoherent(p);
        o->tagged=(o->tagged&3)|reinterpret_cast<uintptr_t>(p);
    } return p;
}
Ref* Find(Cache* c,uint32_t stamp) {
    for(auto& e:c->entries) { if(int64_t(stamp)-int64_t(e.stamp)>2) { auto p=e.value; e.value=nullptr; e.stamp=0; Release(p); } if(e.stamp==stamp) return e.value; } return nullptr;
}
void Put(Cache* c,Ref* p,uint32_t stamp) {
    Entry* e=&c->entries[0]; uint32_t oldest=~0u;
    for(auto& v:c->entries) if(v.stamp<=oldest) { oldest=v.stamp; e=&v; }
    e->stamp=stamp; if(e->value!=p) { if(p) ++p->refs; auto old=e->value; e->value=p; Release(old); }
}
void Clear(Cache* c) { for(auto& e:c->entries) { auto p=e.value; e.value=nullptr; e.stamp=0; Release(p); } }
Record** ConstructRecords(Record** out,const Record* first,const Record* last,Record* dest,uint32_t) {
    *out=dest; for(;first!=last;++first,dest=Advance(dest,1)) if(dest) CopyRecord(dest,first); *out=dest; return out;
}
Record* CopyRecords(const Record* first,const Record* last,Record* dest) { for(;first!=last;++first,++dest) CopyRecord(dest,first); return dest; }
void AssignRange(Range* r,const Record* first,const Record* last,uint32_t arg) {
    auto n=(reinterpret_cast<uintptr_t>(last)-reinterpret_cast<uintptr_t>(first))/sizeof(Record);
    auto cap=(reinterpret_cast<uintptr_t>(r->capacity)-reinterpret_cast<uintptr_t>(r->begin))/sizeof(Record);
    auto len=(reinterpret_cast<uintptr_t>(r->end)-reinterpret_cast<uintptr_t>(r->begin))/sizeof(Record);
    if(n>cap) { Record* end; ConstructRecords(&end,first,last,nullptr,arg); r->begin=nullptr; r->end=r->capacity=reinterpret_cast<Record*>(n*sizeof(Record)); }
    else if(n<=len) r->end=CopyRecords(first,last,r->begin);
    else { auto middle=first+len; CopyRecords(first,middle,r->begin); ConstructRecords(&r->end,middle,last,r->end,arg); }
}
Group* AssignGroup(Group* d,const Group* s) { if(d!=s) { d->range.end=d->range.begin; AssignRange(&d->range,s->range.begin,s->range.end,0); } Tail(d,s); return d; }
Group* CopyBackward(const Group* first,const Group* last,Group* dest) { while(last!=first) { --last; --dest; AssignGroup(dest,last); } return dest; }
Group* ConstructGroup(Group* d,const Group* s) { d->range={d->storage,d->storage,d->storage+11}; AssignRange(&d->range,s->range.begin,s->range.end,0); Tail(d,s); return d; }
Group** ConstructGroups(Group** out,const Group* first,const Group* last,Group* dest,uint32_t) { *out=dest; for(;first!=last;++first) { if(*out) ConstructGroup(*out,first); *out=Advance(*out,1); } return out; }
void Insert(Groups* v,Group* pos,const Group* source) {
    if(v->end!=v->capacity) {
        auto src=source; if(source>=pos && source<v->end) src=source+1;
        if(v->end) ConstructGroup(v->end,v->end-1);
        CopyBackward(pos,v->end-1,v->end); AssignGroup(pos,src); ++v->end;
    } else {
        auto n=(reinterpret_cast<uintptr_t>(v->end)-reinterpret_cast<uintptr_t>(v->begin))/sizeof(Group); auto capacity=n?2*n:1;
        Group* middle; ConstructGroups(&middle,v->begin,pos,nullptr,0);
        if(middle) ConstructGroup(middle,source);
        Group* finish; ConstructGroups(&finish,pos,v->end,Advance(middle,1),0);
        v->begin=nullptr; v->end=finish; v->capacity=reinterpret_cast<Group*>(capacity*sizeof(Group));
    }
}
void Append(Groups* v,const Group* s) {
    if(!(s->contribution>0) || !(s->weight>0)) return;
    if(v->end<v->capacity) { auto p=v->end; v->end=Advance(p,1); if(p) ConstructGroup(p,s); } else Insert(v,v->end,s);
    v->total=v->total+s->contribution;
}
}
