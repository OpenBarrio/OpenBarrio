#include "ts3/anim/m05_M80.h"
#include <algorithm>
namespace ts3::anim::m80 {
namespace {
template<class T> uintptr_t Address(T* p) { return reinterpret_cast<uintptr_t>(p); }
template<class T> int32_t Count(T* a,T* b) { return static_cast<int32_t>(static_cast<intptr_t>(Address(b)-Address(a))/static_cast<intptr_t>(sizeof(T))); }
void Empty(Text* p) { p->begin=p->end=p->storage; p->capacity=p->storage+96; p->storage[0]=0; }
void Clear(Text& p) { if(p.begin!=p.end) { *p.begin=0; p.end=p.begin; } }
uintptr_t Assign(Text* p,const Text* q) { return CallGameMethod<uintptr_t,0x0099C9C0>(p,q->begin,q->end); }
void Copy(Text** out,Text* first,Text* last,Text* dest) { CallGameFunction<void,0x00568BA0>(out,first,last,dest); }
void Copy(Record** out,Record* first,Record* last,Record* dest) { CallGameFunction<void,0x00569110>(out,first,last,dest); }
}
uintptr_t InsertText(Vector<Text>* v,Text* pos,const Text* source) {
    if(v->end!=v->capacity) {
        if(Address(source)>=Address(pos) && Address(source)<Address(v->end)) ++source;
        if(v->end) { Empty(v->end); Assign(v->end,v->end-1); }
        auto result=CallGameFunction<uintptr_t,0x00577D70>(pos,v->end-1,v->end);
        if(pos!=source) { Clear(*pos); result=Assign(pos,source); }
        ++v->end;
        return result;
    }
    auto count=Count(v->begin,v->end); uint32_t capacity=count?2*count:1;
    Text* allocation=capacity?CallGameFunction<Text*,0x004E3F90>(sizeof(Text)*capacity,v->descriptor,0):nullptr;
    Text* next;
    Copy(&next,v->begin,pos,allocation);
    Text* inserted=next;
    if(inserted) { Empty(inserted); Assign(inserted,source); }
    Copy(&next,pos,v->end,inserted+1);
    if(v->begin) CallGameFunction<void,0x004E3F70>(v->begin);
    v->begin=allocation; v->end=next; v->capacity=allocation+capacity;
    return Address(v->capacity);
}
uintptr_t InsertRecords(Vector<Record>* v,Record* pos,uint32_t n,const Record* source) {
    uint32_t available=Count(v->end,v->capacity);
    Record* next;
    if(n>available) {
        uint32_t count=Count(v->begin,v->end);
        uint32_t capacity=std::max(count?2*count:1,count+n);
        // EA's actual body never allocates here, nor frees the old range.
        Copy(&next,v->begin,pos,static_cast<Record*>(nullptr));
        CallGameFunction<void,0x00577E00>(next,n,source);
        Copy(&next,pos,v->end,reinterpret_cast<Record*>(Address(next)+sizeof(Record)*n));
        v->begin=nullptr; v->end=next; v->capacity=reinterpret_cast<Record*>(sizeof(Record)*capacity);
        return Address(next);
    }
    if(!n) return available;
    Record temp;
    CallGameMethod<void,0x00568C50>(&temp,source);
    Record* old=v->end; uint32_t tail=Count(pos,old);
    if(n>=tail) {
        for(uint32_t i=0;i<n-tail;++i) if(old+i) CallGameMethod<void,0x00568C50>(old+i,&temp);
        v->end+=n-tail;
        Copy(&next,pos,old,v->end); v->end+=tail;
        return CallGameFunction<uintptr_t,0x00577F50>(pos,old,&temp);
    }
    Copy(&next,old-n,old,old); v->end+=n;
    CallGameFunction<void,0x00577DC0>(pos,old-n,old);
    return CallGameFunction<uintptr_t,0x00577F50>(pos,pos+n,&temp);
}
uintptr_t InsertTexts(Vector<Text>* v,Text* pos,uint32_t n,const Text* source) {
    uint32_t available=Count(v->end,v->capacity);
    Text* next;
    if(n>available) {
        uint32_t count=Count(v->begin,v->end), capacity=std::max(count?2*count:1,count+n);
        Text* allocation=capacity?CallGameFunction<Text*,0x004E3F90>(sizeof(Text)*capacity,v->descriptor,0):nullptr;
        Copy(&next,v->begin,pos,allocation);
        CallGameFunction<void,0x00577CE0>(next,n,source);
        Copy(&next,pos,v->end,next+n);
        if(v->begin) CallGameFunction<void,0x004E3F70>(v->begin);
        v->begin=allocation; v->end=next; v->capacity=allocation+capacity;
        return Address(v->capacity);
    }
    if(!n) return available;
    Text temp; Empty(&temp); Assign(&temp,source);
    Text* old=v->end; uint32_t tail=Count(pos,old);
    if(n>=tail) {
        CallGameFunction<void,0x00577CE0>(old,n-tail,&temp);
        v->end+=n-tail; Copy(&next,pos,old,v->end); v->end+=tail;
        return CallGameFunction<uintptr_t,0x00577CA0>(pos,old,&temp);
    }
    Copy(&next,old-n,old,old); v->end+=n;
    CallGameFunction<void,0x00577D70>(pos,old-n,old);
    return CallGameFunction<uintptr_t,0x00577CA0>(pos,pos+n,&temp);
}
uintptr_t AppendText(Vector<Text>* v) {
    Text* old=v->end;
    if(Address(old)>=Address(v->capacity)) { Text temp; Empty(&temp); return InsertText(v,old,&temp); }
    ++v->end;
    if(old) Empty(old);
    return Address(old);
}
bool Bind(State* s,uint32_t group,uint32_t slot,const uint32_t* rig,void* key,void* target) {
    if(!s->enabled || !(s->flags&1) || !key || !target) return false;
    if(group>=static_cast<uint32_t>(Count(s->begin,s->end))) return false;
    auto& records=s->begin[group].records;
    if(slot>=static_cast<uint32_t>(Count(records.begin,records.end))) return false;
    auto& r=records.begin[slot];
    if(target!=r.target) { r.target=target; Clear(r.second); }
    if(key!=r.key) {
        r.key=key; Clear(r.first); r.index=-1;
        CallGameMethod<void,0x005775E0>(&r.data[0],rig);
        r.data[2]=rig?rig[2]:0; r.data[3]=rig?rig[3]:0;
    }
    uint32_t count=CallGameMethod<uint32_t,0x005682C0>(&s->names);
    uint32_t i=0; while(i<count && s->keys.begin[i]!=key) ++i;
    if(i==count) { AppendText(&s->names); CallGameMethod<void,0x005A4470>(&s->keys,&key); }
    return true;
}
uintptr_t ResizeRecords(Vector<Record>* v,uint32_t n) {
    uint32_t count=Count(v->begin,v->end);
    if(n<=count) {
        Record* dest=v->begin+n;
        CallGameFunction<void,0x00577D30>(v->end,v->end,dest);
        int32_t delta=-static_cast<int32_t>(sizeof(Record)*(count-n));
        v->end=reinterpret_cast<Record*>(Address(v->end)+delta);
        return static_cast<uint32_t>(delta);
    }
    Record temp;
    auto source=CallGameMethod<Record*,0x00577E30>(&temp);
    return InsertRecords(v,v->end,n-count,source);
}
}

#if defined(_M_IX86)
namespace ts3::anim::m80::hooks {
uintptr_t __fastcall InsertText(Vector<Text>* v, void*, Text* p, const Text* q) { return m80::InsertText(v,p,q); }
uintptr_t __fastcall InsertRecords(Vector<Record>* v, void*, Record* p, uint32_t n, const Record* q) { return m80::InsertRecords(v,p,n,q); }
uintptr_t __fastcall InsertTexts(Vector<Text>* v, void*, Text* p, uint32_t n, const Text* q) { return m80::InsertTexts(v,p,n,q); }
uintptr_t __fastcall AppendText(Vector<Text>* v, void*) { return m80::AppendText(v); }
bool __fastcall Bind(State* s, void*, uint32_t g, uint32_t i, const uint32_t* r, void* k, void* t) { return m80::Bind(s,g,i,r,k,t); }
uintptr_t __fastcall ResizeRecords(Vector<Record>* v, void*, uint32_t n) { return m80::ResizeRecords(v,n); }
}
#endif
