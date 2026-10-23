#include "ts3/anim/m05_B04.h"
namespace ts3::anim {
namespace {
bool Less(const B04Record& a, const B04Record& b) {
    if(a.parent==-1) return b.parent!=-1 || a.key<b.key;
    if(b.parent==-1) return false;
    if(a.parent==b.parent) return a.key<b.key;
    if(a.parent==b.id) return false;
    if(b.parent==a.id) return true;
    return a.parent>b.parent;
}
int32_t Residue(int32_t n) { return static_cast<int32_t>(uint32_t(n)*20u*0x66666667u); }
void Swap(B04Record& a,B04Record& b) { auto t=a; a=b; b=t; }
}
void B04Insertion(B04Record* first,B04Record* last) {
    if(first==last) return;
    for(auto i=first+1;i!=last;++i) {
        auto t=*i; auto hole=i;
        while(hole!=first && Less(t,hole[-1])) { *hole=hole[-1]; --hole; }
        *hole=t;
    }
}
void B04Unguarded(B04Record* first,B04Record* last) {
    for(auto i=first;i!=last;++i) {
        auto t=*i; auto hole=i;
        while(Less(t,hole[-1])) { *hole=hole[-1]; --hole; }
        *hole=t;
    }
}
B04Record* B04Push(B04Record* base,int32_t top,int32_t hole,const B04Record* value) {
    int32_t parent=(hole-1)>>1;
    while(hole>top && Less(base[parent],*value)) {
        base[hole]=base[parent]; hole=parent; parent=(parent-1)>>1;
    }
    base[hole]=*value;
    return base+hole;
}
B04Record* B04Adjust(B04Record* base,int32_t top,int32_t count,int32_t hole,const B04Record* value) {
    int32_t child=2*hole+2;
    while(child<count) {
        if(Less(base[child],base[child-1])) --child;
        base[hole]=base[child]; hole=child; child=2*child+2;
    }
    if(child==count) { base[hole]=base[child-1]; hole=child-1; }
    return B04Push(base,top,hole,value);
}
B04Record* B04Pop(B04Record* first,B04Record* last) {
    auto value=last[-1]; last[-1]=*first;
    return B04Adjust(first,0,int32_t(last-first)-1,0,&value);
}
uintptr_t B04MakeHeap(B04Record* first,B04Record* last) {
    int32_t n=int32_t(last-first);
    uintptr_t result=uint32_t(Residue(n));
    if(n>=2) {
        int32_t hole=((n-2)>>1)+1;
        do { --hole; auto value=first[hole]; result=reinterpret_cast<uintptr_t>(B04Adjust(first,hole,n,hole,&value)); } while(hole);
    }
    return result;
}
// Adapted from verified SortHeap80 (0x00C758D0); stride is 20, comparator is hierarchy-specific.
int32_t B04SortHeap(B04Record* first,B04Record* last) {
    int32_t n=int32_t(last-first);
    if(n<=1) return n;
    do { B04Pop(first,last); --last; --n; } while(n>1);
    return Residue(n);
}
B04Record* B04Partition(B04Record* first,B04Record* last,const B04Record* pivot) {
    const auto value=*pivot;
    for(;;) {
        while(Less(*first,value)) ++first;
        do { --last; } while(Less(value,*last));
        if(first>=last) return first;
        Swap(*first,*last); ++first;
    }
}
int32_t B04PartialSort(B04Record* first,B04Record* middle,B04Record* last) {
    B04MakeHeap(first,middle);
    for(auto i=middle;i<last;++i) if(Less(*i,*first)) {
        auto value=*i; *i=*first; B04Adjust(first,0,int32_t(middle-first),0,&value);
    }
    return B04SortHeap(first,middle);
}
int32_t B04Intro(B04Record* first,B04Record* last,int32_t depth) {
    int32_t n=int32_t(last-first);
    while(n>28 && depth>0) {
        auto mid=first+n/2; auto end=last-1; B04Record* pivot;
        if(Less(*first,*mid)) {
            if(Less(*mid,*end)) pivot=mid;
            else if(Less(*first,*end)) pivot=end;
            else pivot=first;
        } else {
            if(Less(*first,*end)) pivot=first;
            else if(Less(*mid,*end)) pivot=end;
            else pivot=mid;
        }
        auto cut=B04Partition(first,last,pivot);
        B04Intro(cut,last,--depth); last=cut; n=int32_t(last-first);
    }
    if(depth==0) return B04PartialSort(first,last,last);
    return n;
}
void B04Sort(B04Record* first,B04Record* last) {
    if(first==last) return;
    int32_t n=int32_t(last-first), bits=0;
    for(int32_t t=n;t;t>>=1) ++bits;
    B04Intro(first,last,2*bits-2);
    if(n<=28) B04Insertion(first,last);
    else { B04Insertion(first,first+28); B04Unguarded(first+28,last); }
}
B04Node** B04UpperBound(B04Node** out,B04Node* first,B04Node* last,const uint32_t* key) {
    int32_t n=0; for(auto p=first;p!=last;p=p->next) ++n;
    while(n>0) {
        int32_t half=n>>1; auto mid=first;
        for(int32_t i=0;i<half;++i) mid=mid->next;
        if(*key>mid->key) n=half;
        else { first=mid->next; n-=half+1; }
    }
    *out=first; return out;
}
uint32_t B04ClassId(void*) { return 0x93dffdc3u; }
void* B04Query(void* self,uint32_t id) {
    return id==1943976136u || id==0x93dffdc3u ? self : nullptr;
}
B04Node* B04Insert(B04List* self,uint32_t key,uint32_t value) {
    auto sentinel=reinterpret_cast<B04Node*>(&self->first); B04Node* position;
    B04UpperBound(&position,self->first,sentinel,&key);
    auto result=self->free;
    if(result) self->free=result->next;
    // Original tests result+8 (not result) before constructing the payload.
    if(reinterpret_cast<uintptr_t>(result)!=uintptr_t(-2*sizeof(void*))) { result->key=key; result->value=value; }
    result->next=position; result->previous=position->previous;
    position->previous->next=result; position->previous=result;
    return result;
}
}
namespace ts3::anim {
uint32_t __fastcall B04ClassHook(void* self,void*) { return B04ClassId(self); }
void* __fastcall B04QueryHook(void* self,void*,uint32_t id) { return B04Query(self,id); }
B04Node* __fastcall B04InsertHook(B04List* self,void*,uint32_t key,uint32_t value) { return B04Insert(self,key,value); }
}
