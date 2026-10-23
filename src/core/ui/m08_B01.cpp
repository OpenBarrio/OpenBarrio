#include "ts3/ui/m08_B01.h"
#include <algorithm>
#include <cstring>
#include <stdexcept>
namespace ts3::ui {
namespace {
void* Table(uint32_t address) {
#if defined(_M_IX86)
    return reinterpret_cast<void*>(uintptr_t(address));
#else
    (void)address;
    throw std::runtime_error("B01 requires injected native animation tables");
#endif
}
intptr_t Span(B01Animation* p) {
    // Unsigned subtraction followed by signed interpretation matches x86 SUB.
    return static_cast<intptr_t>(reinterpret_cast<uintptr_t>(p->end)-reinterpret_cast<uintptr_t>(p->begin));
}
B01Animation* Create(void* allocator,bool path) {
    const char* tag=nullptr;
#if defined(_M_IX86)
    tag=reinterpret_cast<const char*>(path?0x00FBA208:0x00FBA1E0);
#else
    tag=path?"UI/UTFWin/EA::UTFWin::AnimationPath2D":"UI/UTFWin/EA::UTFWin::AnimationCurve";
#endif
    auto* p=static_cast<B01Animation*>(B01Allocate(sizeof(B01Animation),4,tag,allocator));
    if(!p) return nullptr;
    p->table=Table(path?0x00FBA18C:0x00FBA15C);
    p->begin=nullptr;p->end=nullptr;p->capacity=nullptr;
    p->descriptorTable=Table(path?0x00FBA1CC:0x00FBA1BC);
    p->descriptorVersion=1;p->references=0;
    return p;
}
uint32_t Describe(B01Animation* p,B01Descriptor* out,uint32_t table) {
    out->table=Table(table);out->object=p;out->version=1;
    const uint32_t type=CallGameVirtual<uint32_t,5>(p);
    out->type=type;
    return type;
}
}
void* __cdecl B01Allocate(uint32_t size,uint32_t alignment,const char* tag,void* allocator) {
    const uint32_t rounded=(alignment+15u)&~15u;
    void* base;
    if(allocator) {
        base=CallGameVirtual<void*,1>(allocator,size+16u,tag,uint32_t(0),rounded,uint32_t(16));
    } else {
#if defined(_M_IX86)
        void* heap=*reinterpret_cast<void**>(0x011CB864);
        base=CallGameMethod<void*,0x004E6A20>(heap,size+16u,rounded,uint32_t(16),uint32_t(0));
#else
        throw std::runtime_error("B01 default game heap unavailable on native host");
#endif
    }
    if(!base) return nullptr;
    *static_cast<void**>(base)=allocator;
    return static_cast<unsigned char*>(base)+16;
}
// Adapted from verified B18QueryA; only the three ID immediates differ.
void* __fastcall B01QueryCurve(void* p,void*,uint32_t id) {
    return id==0x03852C99 || id==0xEE3F516E || id==0xEEC58382 ? p : nullptr;
}
uint32_t __fastcall B01CurveType(void*,void*) { return 0x03852D17; }
void* __fastcall B01QueryPath(void* p,void*,uint32_t id) {
    return id==0x03852CA0 || id==0xEE3F516E || id==0xEEC58382 ? p : nullptr;
}
// MemoryAddRef twin, reference offset changed from 0x0c to 0x18.
uint32_t __fastcall B01Retain(B01Animation* p,void*) { return ++p->references; }
uint32_t __fastcall B01Release(B01Animation* p,void*) {
    const uint32_t count=--p->references;
    if(count==0) CallGameVirtual<void*,2>(p,uint32_t(1));
    return count;
}
uint32_t __fastcall B01PathType(void*,void*) { return 0x03852D14; }
// Count twin uses begin/end +4/+8, with an arithmetic shift of three.
int32_t __fastcall B01CurveCount(B01Animation* p,void*) { return static_cast<int32_t>(Span(p)>>3); }
int32_t __fastcall B01CopyCurve(B01Animation* p,void*,void* out,uint32_t capacity) {
    if(out) std::memcpy(out,p->begin,size_t(std::min(uint32_t(B01CurveCount(p,nullptr)),capacity))*8u);
    return B01CurveCount(p,nullptr);
}
uint32_t __fastcall B01DescribeCurve(B01Animation* p,void*,B01Descriptor* out) { return Describe(p,out,0x00FBA0DC); }
int32_t __fastcall B01PathCount(B01Animation* p,void*) { return static_cast<int32_t>(Span(p)/12); }
int32_t __fastcall B01CopyPath(B01Animation* p,void*,void* out,uint32_t capacity) {
    if(out) std::memcpy(out,p->begin,size_t(std::min(uint32_t(B01PathCount(p,nullptr)),capacity))*12u);
    return B01PathCount(p,nullptr);
}
uint32_t __fastcall B01DescribePath(B01Animation* p,void*,B01Descriptor* out) { return Describe(p,out,0x00FBA0FC); }
B01Animation* __stdcall B01CreateCurve(void*,void* allocator) { return Create(allocator,false); }
B01Animation* __stdcall B01CreatePath(void*,void* allocator) { return Create(allocator,true); }
}
