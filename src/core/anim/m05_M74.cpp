#include "ts3/anim/m05_M74.h"
#include <cstring>
#include <cctype>
namespace ts3::anim::m74 {
namespace {
Joint* Next(const Joint* j) {
    Joint* p;
    std::memcpy(&p,j->opaque00,sizeof(p));
    return p;
}
void SetNext(Joint* j, Joint* p) { std::memcpy(j->opaque00,&p,sizeof(p)); }
void* GameData(uintptr_t address) {
#if defined(_M_IX86)
    return reinterpret_cast<void*>(address);
#else
    // Native layouts retain full pointers; these tags are not game addresses.
    static char tags[4];
    switch(address) {
    case 0x00fc2e24: return &tags[0];
    case 0x00fc2e08: return &tags[1];
    case 0x00fc2df0: return &tags[2];
    default: return &tags[3];
    }
#endif
}
void Clear(Skeleton* s) {
    std::memset(s->buckets,0,sizeof(s->buckets));
    s->count=0;
}
void FreeName(char* p, char* capacity) {
    // EA tests the signed byte-distance, not length or capacity!=begin.
    if (reinterpret_cast<intptr_t>(capacity)-reinterpret_cast<intptr_t>(p)>1 && p)
        CallGameFunction<void,0x004e3f70>(p);
}
}
uint32_t __cdecl HashName(const char* name) {
    if (!name) return 0;
    unsigned char ignored=0;
    return CallGameMethod<uint32_t,0x00575430>(&ignored,name);
}
int32_t __cdecl ClassifyName(const char* name, int32_t* side) {
    const char* p=name;
    int32_t n=static_cast<int32_t>(std::strlen(name));
    if (n>3 && p[0]=='b' && p[1]=='_' && p[2]=='_') { p+=3; n-=3; }
    while (n>1) {
        if (n>3 && (p[0]=='F'||p[0]=='R') && (p[1]=='L'||p[1]=='R') && p[2]>='0' && p[2]<='9') {
            *side=p[1]=='L'?0:1;
            return static_cast<int32_t>(p+1-name);
        }
        if (p[0]=='l'||p[0]=='L') {
            if (p[1]=='_' || (n>=5 && p[1]=='e' && p[2]=='f' && p[3]=='t' &&
                (p[4]=='_' || std::isupper(static_cast<signed char>(p[4]))))) {
                *side=0;
                return static_cast<int32_t>(p-name);
            }
        } else if (p[0]=='r'||p[0]=='R') {
            if (p[1]=='_' || (n>=6 && p[1]=='i' && p[2]=='g' && p[3]=='h' && p[4]=='t' &&
                (p[5]=='_' || std::isupper(static_cast<signed char>(p[5]))))) {
                *side=1;
                return static_cast<int32_t>(p-name);
            }
        }
        // Only underscore-delimited starts are reconsidered.
        while (n-- > 1) { if (*p++=='_') break; }
    }
    *side=2;
    return 0;
}
int32_t __fastcall FindJoint(Skeleton* self, void*, const char* name) {
    uint32_t hash=HashName(name);
    Joint* p=self->buckets[hash%13];
    while (p && p->flags!=hash) p=Next(p);
    if (!p) p=self->sentinel;
    return p==self->sentinel ? -1 : p->parent;
}
void __fastcall Rebuild(Skeleton* self, void*) {
    Clear(self);
    const auto count=(reinterpret_cast<intptr_t>(self->end)-reinterpret_cast<intptr_t>(self->begin))/sizeof(Joint);
    for (intptr_t i=0;i<count;++i) {
        Joint* j=self->begin+i;
        j->parent=static_cast<int32_t>(i);
        j->flags=HashName(j->nameBegin);
        Joint*& bucket=self->buckets[j->flags%13];
        Joint* p=bucket;
        while (p && p->flags!=j->flags) p=Next(p);
        if (!p) {
            SetNext(j,bucket);
            bucket=j;
            ++self->count;
        }
    }
}
Skeleton* __fastcall Construct(Skeleton* self, void*) {
    self->vtable=GameData(0x00fc2e24);
    self->begin=nullptr;
    self->end=nullptr;
    self->capacity=nullptr;
    self->descriptor=GameData(0x00fc2e08);
    Clear(self);
    self->sentinel=reinterpret_cast<Joint*>(~uintptr_t(0));
    self->name.descriptor=GameData(0x00fc2df0);
    self->name.begin=static_cast<char*>(GameData(0x011c7e54));
    self->name.end=self->name.begin;
    self->name.capacity=self->name.begin+1;
    return self;
}
void __fastcall Destruct(Skeleton* self, void*) {
    self->vtable=GameData(0x00fc2e24);
    Clear(self);
    FreeName(self->name.begin,self->name.capacity);
    for (Joint* p=self->begin;p<self->end;++p) FreeName(p->nameBegin,p->nameCapacity);
    if (self->begin) CallGameFunction<void,0x004e3f70>(self->begin);
}
}
