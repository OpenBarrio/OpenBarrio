#include "ts3/audio/m06_B28.h"
#include <cstring>
namespace ts3::audio::b28 {
namespace {
uint32_t Word(const uint8_t* p) { uint32_t v; std::memcpy(&v,p,4); return v; }
void Install(Allocator* a, uintptr_t table) { a->vtable=reinterpret_cast<void*>(table); }
void Free(void* p) { CallGameFunction<void,0x004E3F70>(p); }
}
void* Allocate(Allocator* a, uint32_t bytes, const char*, uint32_t flags) {
    return CallGameMethod<void*,0x004E69D0>(a->heap,bytes,flags);
}
void* AllocateAligned(Allocator* a, uint32_t bytes, const char*, uint32_t flags, uint32_t alignment, uint32_t offset) {
    return CallGameMethod<void*,0x004E6A20>(a->heap,bytes,alignment,offset,flags);
}
Allocator* Construct(Allocator* a) {
    Install(a,0x0103138C);
    a->heap=nullptr;
    return a;
}
uint8_t Initialize(Allocator* a, uint32_t bytes) {
#if defined(_M_IX86)
    auto label=reinterpret_cast<const char*>(uintptr_t(0x010313A4));
    void* h=CallGameFunction<void*,0x004E3F90>(uint32_t{0x518},label,uint32_t{0},uint32_t{0},uint32_t{0},uint32_t{0});
    if (h) h=CallGameMethod<void*,0x004E5AA0>(h,uint32_t{0},uint32_t{0},uint32_t{1},uint32_t{0},uint32_t{0},uint32_t{0});
    a->heap=h;
    if (!h) return 0;
    void* global=*reinterpret_cast<void**>(uintptr_t(0x011CB864));
    void* buffer=CallGameMethod<void*,0x004E69D0>(global,bytes,uint32_t{1});
    if (!buffer) return 0;
    *reinterpret_cast<void**>(static_cast<uint8_t*>(h)+0x510)=reinterpret_cast<void*>(uintptr_t(0x01027EA4));
    // Each callback can replace the owner's heap; EA reloads it before every call.
    CallGameMethod<void,0x004E4510>(a->heap,reinterpret_cast<void*>(uintptr_t(0x0089F490)),uint32_t{0});
    CallGameMethod<void,0x004E48F0>(a->heap,uint32_t{3},uint32_t{0});
    CallGameMethod<void,0x004E48F0>(a->heap,uint32_t{1000},uint32_t{1});
    return CallGameMethod<uint8_t,0x004E5710>(a->heap,buffer,bytes,uint32_t{1},uint32_t{0},reinterpret_cast<void*>(uintptr_t(0x0089F430)),uint32_t{0});
#else
    (void)a; (void)bytes; AbortNotRewritten("B28 audio heap initialization"); return 0;
#endif
}
void Shutdown(Allocator* a) {
    if (a->heap) {
        CallGameMethod<void,0x004E5CB0>(a->heap);
        void* h=a->heap; // Drain may change the owner field.
        if (h) { CallGameMethod<void,0x004E69C0>(h); Free(h); }
        a->heap=nullptr;
    }
}
void Destruct(Allocator* a) {
    Install(a,0x0103138C);
    Shutdown(a);
    Install(a,0x00F97FD4);
}
Allocator* Delete(Allocator* a, uint32_t flags) {
    Destruct(a);
    if (flags & 1) Free(a);
    return a;
}
uint8_t* LoadBlock(BlockView* v, uint8_t* p) {
    if (!p) return nullptr;
    v->block=p;
    v->tag=Word(p);
    v->bytes=Word(p+4);
    v->count=Word(p+8);
    v->data=p+12;
    v->end=v->data+v->bytes;
    return v->end;
}
uint8_t* Publish(Queue* q) {
    auto p=q->writing;
    if (p) {
        q->cursor=p;
        q->writing=nullptr;
        q->length=nullptr;
        q->start=nullptr;
    }
    return p;
}
uint32_t Empty(const Queue* q) { return q->total == 0; }
uint32_t Available(const Queue* q) { return q->total-q->consumed; }
BlockView* Finish(Queue* q) {
    q->view={};
    if (q->writing) {
        *q->length=static_cast<uint32_t>(reinterpret_cast<uintptr_t>(q->cursor)-reinterpret_cast<uintptr_t>(q->start)-4);
        LoadBlock(&q->view,q->writing);
    }
    return &q->view;
}
uint8_t* Next(BlockView* v, uint32_t* offset, uint32_t* tag, uint32_t* extra, uint32_t* size) {
    if (*offset >= v->bytes) return nullptr;
    uint8_t* p=v->data+*offset;
    uint32_t t=Word(p), n=Word(p+4), e=Word(p+8);
    if (tag) *tag=t;
    if (extra) *extra=e;
    if (size) *size=n;
    *offset += ((n+3)&~uint32_t{3})+12;
    return p+12;
}
uint8_t* Find(BlockView* v, uint32_t tag, uint32_t* extra, uint32_t* size) {
    auto p=v->data;
    for (uint32_t i=0; i<v->count; ++i) {
        uint32_t t=Word(p), n=Word(p+4), e=Word(p+8);
        p+=12;
        if (t==tag) {
            if (extra) *extra=e;
            if (size) *size=n;
            return p;
        }
        p+=(n+3)&~uint32_t{3};
    }
    return nullptr;
}
uint8_t* Rewind(Queue* q) {
    q->consumed=0;
    q->readable=q->begin!=q->end?q->begin:nullptr;
    return q->readable;
}
#if defined(_M_IX86)
void* __fastcall HookAllocate(Allocator* a0, void*, uint32_t a1, const char* a2, uint32_t a3) { return Allocate(a0, a1, a2, a3); }
void* __fastcall HookAllocateAligned(Allocator* a0, void*, uint32_t a1, const char* a2, uint32_t a3, uint32_t a4, uint32_t a5) { return AllocateAligned(a0, a1, a2, a3, a4, a5); }
Allocator* __fastcall HookConstruct(Allocator* a0, void*) { return Construct(a0); }
uint8_t __fastcall HookInitialize(Allocator* a0, void*, uint32_t a1) { return Initialize(a0, a1); }
void __fastcall HookShutdown(Allocator* a0, void*) { Shutdown(a0); }
void __fastcall HookDestruct(Allocator* a0, void*) { Destruct(a0); }
Allocator* __fastcall HookDelete(Allocator* a0, void*, uint32_t a1) { return Delete(a0, a1); }
uint8_t* __fastcall HookLoadBlock(BlockView* a0, void*, uint8_t* a1) { return LoadBlock(a0, a1); }
uint8_t* __fastcall HookPublish(Queue* a0, void*) { return Publish(a0); }
uint32_t __fastcall HookEmpty(const Queue* a0, void*) { return Empty(a0); }
uint32_t __fastcall HookAvailable(const Queue* a0, void*) { return Available(a0); }
BlockView* __fastcall HookFinish(Queue* a0, void*) { return Finish(a0); }
uint8_t* __fastcall HookNext(BlockView* a0, void*, uint32_t* a1, uint32_t* a2, uint32_t* a3, uint32_t* a4) { return Next(a0, a1, a2, a3, a4); }
uint8_t* __fastcall HookFind(BlockView* a0, void*, uint32_t a1, uint32_t* a2, uint32_t* a3) { return Find(a0, a1, a2, a3); }
uint8_t* __fastcall HookRewind(Queue* a0, void*) { return Rewind(a0); }
#endif
}
