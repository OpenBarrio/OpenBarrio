#include "ts3/resource/m03_C01.h"
#include <atomic>
#include <bit>

namespace ts3::resource::m03::c01 {
namespace {
void Free(void* p) { CallGameFunction<void,0x004E3F70>(p); }
void Clear(KeyDeque* p) { CallGameMethod<void,0x00800890>(p); }
uintptr_t Insert(KeyDeque* p,Iterator it,uint32_t n,const Value* v) { return CallGameMethod<uintptr_t,0x00730950>(p,it,n,v); }
void* Parser() { return CallGameFunction<void*,0x0058B880>(); }
void ParseOriginal(void* parser,void* source,ParseContext* ctx) {
#if defined(_M_IX86)
    CallGameMethod<void,0x0058C4F0>(parser,source,reinterpret_cast<void*>(0x00731000),ctx);
#else
    (void)parser; (void)source; (void)ctx; NotRewritten("C01 config parser callback");
#endif
}
void Finish(KeyList* q) { CallGameFunction<void,0x004AFCD0>(q); }
void* Loader() {
#if defined(_M_IX86)
    return *reinterpret_cast<void**>(0x011D3C84);
#else
    NotRewritten("C01 loader singleton");
#endif
}
void Cancel(void* loader,void** slot) {
#if defined(_M_IX86)
    CallGameVirtual<void,15>(loader,slot,reinterpret_cast<void*>(0x00D4B150));
#else
    (void)loader; (void)slot; NotRewritten("C01 cancel");
#endif
}
void Release(void* p) { CallGameVirtual<void,1>(p); }
void Append(a12::PointerDeque* q,void* p) { CallGameMethod<void,0x00C48450>(q,&p); }
void* Factory(uint32_t type) {
    auto manager=CallGameFunction<void*,0x004AFD20>();
    return CallGameVirtual<void*,11>(manager,type,int32_t(-1));
}
bool Load(void* loader,const Value* key,void** slot,Preloader* owner,unsigned index) {
#if defined(_M_IX86)
    return CallGameVirtual<bool,13>(loader,key,0x02269ed1u,slot,owner,index,reinterpret_cast<void*>(0x00D4B150),int32_t(-1),1u);
#else
    (void)loader; (void)key; (void)slot; (void)owner; (void)index; NotRewritten("C01 async load");
#endif
}
void Next(Iterator& it) {
    if(++it.cur==it.end) { ++it.node; it.begin=*it.node; it.end=it.begin+16; it.cur=it.begin; }
}
void Prev(Iterator& it) {
    if(it.cur==it.begin) { --it.node; it.begin=*it.node; it.end=it.begin+16; it.cur=it.end; }
    --it.cur;
}
int32_t Distance(const Iterator& a,const Iterator& b) {
    return int32_t(16*(b.node-a.node-1)+(b.cur-b.begin)+(a.end-a.cur));
}
void Copy(Value* dst,const Value* src) { for(unsigned w=0;w<4;++w) dst->words[w]=src->words[w]; }
bool Raw(void* p) { return p && !(reinterpret_cast<uintptr_t>(p)&3); }
void CancelRelease(void** slot,const Boundaries& b) {
    if(auto loader=b.loader()) b.cancel(loader,slot);
    if(Raw(*slot)) { b.release(*slot); *slot=nullptr; }
}
}
const Boundaries& OriginalBoundaries() {
    static const Boundaries b{Free,Clear,Insert,Parser,ParseOriginal,Finish,Loader,Cancel,Release,Append,Factory,Load}; return b;
}
Iterator* Assign(const Iterator* dst,Iterator* out,const Iterator* first,const Iterator* last,void*) {
    // The verified AE6510 wrapper supplies the splat, not its intrusive-pointer callee.
    return a12::CopyRangeWrapper(out,*first,*last,*dst);
}
Iterator* CopyBackward(Iterator* out,Iterator first,Iterator last,Iterator dst) {
    // AE1750 wrapper shape, with 730400 -> 7301F0 -> 72FF50 value-copy substitution.
    while(last.cur!=first.cur) { Prev(last); Prev(dst); Copy(dst.cur,last.cur); }
    *out=dst; return out;
}
uintptr_t ShiftBackward(const Iterator* dst,const Iterator* first,const Iterator* last,void*) {
    Iterator scratch; CopyBackward(&scratch,*first,*last,*dst);
    return reinterpret_cast<uintptr_t>(&scratch);
}
bool Parse(KeyList* q,void* source,const char16_t* prefix,const Boundaries& b) {
    void* parser=b.parser();
    if(!parser) return false;
    uint32_t length=0; while(prefix[length]) ++length;
    ParseContext ctx{q,prefix,length,1};
    b.parse(parser,source,&ctx);
    // EA checks the vector's first two words, NOT the deque's first/last cursors.
    if(q->begin==q->end) return false;
    b.finish(q); return true;
}
Iterator* Erase(KeyDeque* q,Iterator* out,Iterator first,Iterator last,const Boundaries& b) {
    if(first.cur==q->first.cur && last.cur==q->last.cur) {
        b.clear(q); *out=q->last; return out;
    }
    const int32_t before=Distance(q->first,first),removed=Distance(first,last);
    const uint32_t remaining=uint32_t(Distance(q->first,q->last))-uint32_t(removed);
    if(before<int32_t(remaining>>1)) {
        Iterator newFirst; a12::Offset(&q->first,&newFirst,removed);
        Value** oldNode=q->first.node;
        ShiftBackward(&last,&q->first,&first,out);
        while(q->first.cur!=newFirst.cur) Next(q->first);
        for(auto p=oldNode;p<newFirst.node;++p) if(*p) b.free(*p);
    } else {
        Iterator newLast; a12::Offset(&q->last,&newLast,-removed);
        Iterator scratch; Assign(&first,&scratch,&last,&q->last,out);
        Iterator walk=newLast;
        while(walk.cur!=q->last.cur) Next(walk);
        for(auto p=newLast.node+1;p<q->last.node+1;++p) if(*p) b.free(*p);
        q->last=newLast;
    }
    return a12::Offset(&q->first,out,before);
}
uintptr_t Resize(KeyDeque* q,uint32_t size,const Value* value,const Boundaries& b) {
    const uint32_t old=uint32_t(Distance(q->first,q->last));
    if(size>old) return b.insert(q,q->last,size-old,value);
    else { Iterator from,out; a12::Offset(&q->first,&from,int32_t(size)); Erase(q,&out,from,q->last,b); return reinterpret_cast<uintptr_t>(&out); }
}
void Tick(Preloader* p,const Boundaries& b) {
    if(p->requested) {
        std::atomic_ref<int32_t>(p->requested).exchange(0);
        for(unsigned i=0;i<8;++i) {
            auto slot=&p->slots[i];
            if(Raw(*slot)) {
                // +0C is the low byte of reserved[1], not a C++ boolean word.
                if((p->reserved[1]&0xff)!=0) {
                    void* object=*slot; *slot=nullptr;
                    auto& q=p->pending;
                    auto next=q.last.cur+1;
                    if(next==q.last.end) b.append(&q,object);
                    else { auto dst=q.last.cur; q.last.cur=next; if(dst) *dst=object; }
                } else CancelRelease(slot,b);
                p->reserved[0]|=1u<<i;
            } else if((reinterpret_cast<uintptr_t>(*slot)>>1)&1) {
                CancelRelease(slot,b); p->reserved[0]|=1u<<i;
            }
        }
    }
    auto& q=p->auxiliary;
    if(!p->reserved[0] || q.first.cur==q.last.cur) return;
    Value key=*reinterpret_cast<Value*>(q.first.cur);
    auto next=reinterpret_cast<Value*>(q.first.cur)+1;
    if(reinterpret_cast<void**>(next)!=q.first.end) q.first.cur=reinterpret_cast<void**>(next);
    else {
        if(q.first.begin) b.free(q.first.begin);
        ++q.first.node; q.first.begin=*q.first.node;
        q.first.end=q.first.begin+64; q.first.cur=q.first.begin;
    }
    if(!b.factory(key.words[2])) return; // Consumed even if factory lookup fails.
    unsigned index=std::countr_zero(p->reserved[0]);
    uint32_t mask=1u<<index; p->reserved[0]-=mask;
    auto loader=b.loader();
    if(!loader || !b.load(loader,&key,&p->slots[index],p,index)) p->reserved[0]+=mask;
}
}
