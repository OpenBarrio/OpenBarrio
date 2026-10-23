#include "ts3/anim/m05_M114.h"
#include <atomic>
namespace ts3::anim::m114 {
namespace {
uintptr_t U(const void* p) { return reinterpret_cast<uintptr_t>(p); }
unsigned char* P(uintptr_t p) { return reinterpret_cast<unsigned char*>(p); }
int32_t Count(unsigned char* a,unsigned char* b) { return static_cast<int32_t>(U(b)-U(a))/124; }
uintptr_t Destroy(unsigned char* first,unsigned char* last,uintptr_t eax) {
    while(U(first)<U(last)) { eax=CallGameMethod<uintptr_t,0x00754060>(first); first=P(U(first)+124); }
    return eax;
}
uintptr_t Copy(unsigned char** out,unsigned char* first,unsigned char* last,unsigned char* dest,void* alloc) {
    return CallGameFunction<uintptr_t,0x00758a90>(out,first,last,dest,alloc);
}
void Table(Builder* s,uint32_t address) {
#if defined(_M_IX86)
    s->vtable=reinterpret_cast<void*>(address);
#else
    // Identity tokens only: original virtual methods are unavailable natively.
    static uintptr_t tables[7]{};
    unsigned i=address==0x0107396c?0:address==0x010231e8?1:address==0x01023274?2:address==0x01023350?3:address==0x0102338c?4:address==0x010233c8?5:6;
    s->vtable=&tables[i];
#endif
}
Builder* Construct(Builder* s,uint32_t base,uint32_t derived) {
    Table(s,0x0107396c);
    // x86 XCHG is atomic, even though the old value is discarded.
    reinterpret_cast<std::atomic<int32_t>*>(&s->refs)->exchange(0);
    Table(s,base);
    CallGameMethod<void,0x0083e190>(s->list);
    s->weight=0.0f;
    Table(s,derived);
    return s;
}
}
uintptr_t __fastcall Insert(Range* s,void*,unsigned char* pos,unsigned char* source) {
    uintptr_t eax;
    if(s->end!=s->capacity) {
        auto* adjusted=source;
        if(U(source)>=U(pos) && U(source)<U(s->end)) adjusted=P(U(source)+124);
        if(s->end) CallGameMethod<void,0x00757690>(s->end,P(U(s->end)-124));
        CallGameFunction<uintptr_t,0x00758ad0>(pos,P(U(s->end)-124),s->end);
        eax=CallGameMethod<uintptr_t,0x007578b0>(pos,adjusted);
        s->end=P(U(s->end)+124);
        return eax;
    }
    const int32_t old=Count(s->begin,s->end);
    const uint32_t capacity=old?static_cast<uint32_t>(old)*2:1;
    unsigned char* cursor;
    eax=Copy(&cursor,s->begin,pos,nullptr,pos);
    if(cursor) eax=CallGameMethod<uintptr_t,0x00757690>(cursor,source);
    eax=Copy(&cursor,pos,s->end,P(U(cursor)+124),cursor);
    eax=Destroy(s->begin,s->end,eax);
    s->begin=nullptr; s->end=cursor; s->capacity=P(capacity*124u);
    return eax;
}
uintptr_t __fastcall Assign(Range* s,void*,unsigned char* first,unsigned char* last,void*) {
    const uint32_t n=static_cast<uint32_t>(Count(first,last));
    const uint32_t capacity=static_cast<uint32_t>(Count(s->begin,s->capacity));
    uintptr_t eax;
    if(n>capacity) {
        unsigned char* cursor;
        eax=Copy(&cursor,first,last,nullptr,last);
        eax=Destroy(s->begin,s->end,eax);
        s->begin=nullptr; s->end=s->capacity=P(n*124u);
        return eax;
    }
    const uint32_t size=static_cast<uint32_t>(Count(s->begin,s->end));
    if(n<=size) {
        auto* end=CallGameFunction<unsigned char*,0x0083d920>(first,last,s->begin);
        eax=Destroy(end,s->end,U(end));
        s->end=end;
        return eax;
    }
    auto* middle=P(U(first)+size*124u);
    CallGameFunction<uintptr_t,0x0083d920>(first,middle,s->begin);
    unsigned char* cursor;
    eax=Copy(&cursor,middle,last,s->end,last);
    s->end=cursor;
    return eax;
}
Builder* __fastcall ConstructA(Builder* s,void*) { return Construct(s,0x010231e8,0x01023350); }
Builder* __fastcall ConstructB(Builder* s,void*) { return Construct(s,0x010231e8,0x0102338c); }
Builder* __fastcall ConstructC(Builder* s,void*) { return Construct(s,0x010231e8,0x010233c8); }
Builder* __fastcall ConstructD(Builder* s,void*) { return Construct(s,0x01023274,0x01023404); }
}
