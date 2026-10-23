#include "ts3/anim/m05_B02.h"
#include <cstring>
#include <emmintrin.h>
namespace ts3::anim::b02 {
uintptr_t RegisterFactory(uintptr_t key, uintptr_t value) {
    uintptr_t pair[2]={key,value}, result[3];
#if defined(_M_IX86)
    auto r=CallGameMethod<uintptr_t,0x008BC3D0>(reinterpret_cast<void*>(0x011CD120),result,pair,key & ~uintptr_t(255));
    return (r & ~uintptr_t(255)) | 1;
#else
    (void)pair; (void)result;
    NotRewritten("animation factory registry");
#endif
}
uintptr_t Evaluate(Registry* self, uintptr_t argument0, uintptr_t argument1) {
    return CallGameVirtual<uintptr_t,2>(self->current,argument0,argument1);
}
void* SetCurrent(Registry* self, void* node) {
    self->current=node;
    return node;
}
void* GetCurrent(Registry* self) { return self->current; }
Pool* InitPool(Pool* self) {
    PoolNode* scratch=nullptr;
    CallGameMethod<void,0x004E07A0>(&scratch,static_cast<void*>(nullptr),uint32_t(0x300),uint32_t(12),uint32_t(4),uint32_t(0));
    self->next=nullptr;
    self->previous=nullptr;
    self->free=nullptr;
    CallGameMethod<void,0x004E07A0>(&self->free,scratch,uint32_t(0x300),uint32_t(12),uint32_t(4),uint32_t(0));
    self->next=self;
    self->previous=self;
    CallGameMethod<void,0x004E07A0>(&self->free,self->storage,uint32_t(sizeof self->storage),uint32_t(sizeof(PoolNode)),uint32_t(4),uint32_t(0));
    return self;
}
Registry* InitRegistry(Registry* self) {
    CallGameMethod<void,0x004E8780>(self->heap,uint32_t(0),int32_t(-1),uint32_t(0),uint32_t(0),uint32_t(0));
    InitPool(&self->pool);
    self->current=nullptr;
#if defined(_M_IX86)
    CallGameMethod<void,0x004E85B0>(self->heap,self,uint32_t(0x4000),reinterpret_cast<void*>(0x005BFB90),reinterpret_cast<void*>(0x00C0FD60),uint32_t(0));
#else
    NotRewritten("animation registry descriptor");
#endif
    return self;
}
// Adapted from verified class-id/query and setter twins; only offsets and IDs differ.
uint32_t TypeC810(void*) { return 0x13ea6f90u; }
void* QueryC820(void* self, uint32_t id) {
    if (id==0x13ea6f90u) return self;
    if (id==0x73deb8c8u) return self;
    return nullptr;
}
void* SetPayload(void* self, const void* source) {
    const __m128i value=_mm_load_si128(static_cast<const __m128i*>(source));
    _mm_store_si128(reinterpret_cast<__m128i*>(static_cast<unsigned char*>(self)+16),value);
    return const_cast<void*>(source);
}
uint32_t TypeCD00(void*) { return 0xf40282a6u; }
void* QueryCD10(void* self, uint32_t id) {
    if (id==0x73deb8c8u) return self;
    if (id==0xf40282a6u) return self;
    return nullptr;
}
uint32_t TypeCEB0(void*) { return 0x33e02b1du; }
namespace {
// The stream cursor points at the LAST byte consumed relative to end, not the next byte.
// Failed refill leaves the destination unchanged. EAX remains the refill result on failure.
uintptr_t Read(Reader* self, void* dest, uint32_t count) {
    self->position=static_cast<int32_t>(uint32_t(self->position)+count);
    uintptr_t eax=0;
    if (self->position>=0) eax=CallGameMethod<uintptr_t,0x00593FF0>(self,count);
    if (self->position<0) {
        eax=reinterpret_cast<uintptr_t>(self->end);
        std::memcpy(dest,self->end+static_cast<ptrdiff_t>(self->position)-count+1,count);
    }
    return eax;
}
void ReadFloat(Reader* self, float* destination) {
    uint32_t bits=0;
    Read(self,&bits,4);
    if (self->position<0) {
#if defined(_M_IX86)
        // Original FLD/FSTP quiets signaling NaNs and updates x87 exception status.
        __asm { fld dword ptr bits }
        __asm { mov eax, destination }
        __asm { fstp dword ptr [eax] }
#else
        if ((bits & 0x7f800000u)==0x7f800000u && (bits & 0x007fffffu)) bits |= 0x00400000u;
        std::memcpy(destination,&bits,4);
#endif
    }
}
uintptr_t Acquire(const Key* key, void** resource) {
    CallGameFunction<uintptr_t,0x004AFD20>();
    *resource=nullptr;
    return CallGameFunction<uintptr_t,0x0054C010>(key,resource);
}
}
uintptr_t ReadKey(Reader* self, Key* key) {
    Read(self,&key->words[2],4);
    Read(self,&key->words[3],4);
    Read(self,&key->words[0],4);
    return Read(self,&key->words[1],4);
}
uintptr_t SetSecondary(Clip* self, const Key* key) {
    // Sequential copies intentionally preserve overlapping-input behavior.
    for (int i=0;i<4;++i) {
        static_cast<volatile uint32_t*>(self->secondary.words)[i]=static_cast<const volatile uint32_t*>(key->words)[i];
    }
    uintptr_t eax=self->secondary.words[3];
    if (self->secondary.words[0] || self->secondary.words[2] || eax || self->secondary.words[1]) {
        void* resource;
        eax=Acquire(key,&resource);
        void* previous=self->secondaryResource;
        self->secondaryResource=resource;
        if (previous) eax=CallGameVirtual<uintptr_t,1>(previous);
    }
    return (eax & ~uintptr_t(255)) | 1;
}
uintptr_t LoadClip(Clip* self, Reader* reader) {
    uint8_t flags=0;
    Read(reader,&flags,1);
    if (flags & 1) ReadKey(reader,&self->primary);
    if (flags & 2) ReadKey(reader,&self->secondary);
    self->flag0=(flags>>2)&1;
    self->flag1=(flags>>3)&1;
    ReadFloat(reader,&self->first);
    ReadFloat(reader,&self->second);
    uint8_t rows=0;
    Read(reader,&rows,1);
    for (unsigned row=0;row<=rows;++row) {
        uint8_t columns=0;
        Read(reader,&columns,1);
        for (unsigned col=0;col<=columns;++col) {
            // EA does not check either dimension against 5 x 11.
            auto* binding=reinterpret_cast<unsigned char*>(self)+0x28+(row*11+col)*16;
            Read(reader,binding,8);
            Read(reader,binding+8,4);
            Read(reader,binding+12,4);
        }
    }
    void* resource;
    Acquire(&self->primary,&resource);
    void* previous=self->primaryResource;
    self->primaryResource=resource;
    if (previous) CallGameVirtual<uintptr_t,1>(previous);
    return SetSecondary(self,&self->secondary);
}
}

#if defined(_M_IX86)
namespace ts3::anim::b02 {
uintptr_t __fastcall HookEvaluate(Registry* self, void*, uintptr_t a, uintptr_t b) { return Evaluate(self,a,b); }
void* __fastcall HookSetCurrent(Registry* self, void*, void* node) { return SetCurrent(self,node); }
void* __fastcall HookGetCurrent(Registry* self, void*) { return GetCurrent(self); }
Pool* __fastcall HookInitPool(Pool* self, void*) { return InitPool(self); }
Registry* __fastcall HookInitRegistry(Registry* self, void*) { return InitRegistry(self); }
uint32_t __fastcall HookTypeC810(void* self, void*) { return TypeC810(self); }
void* __fastcall HookQueryC820(void* self, void*, uint32_t id) { return QueryC820(self,id); }
void* __fastcall HookSetPayload(void* self, void*, const void* source) { return SetPayload(self,source); }
uint32_t __fastcall HookTypeCD00(void* self, void*) { return TypeCD00(self); }
void* __fastcall HookQueryCD10(void* self, void*, uint32_t id) { return QueryCD10(self,id); }
uint32_t __fastcall HookTypeCEB0(void* self, void*) { return TypeCEB0(self); }
uintptr_t __fastcall HookReadKey(Reader* self, void*, Key* key) { return ReadKey(self,key); }
uintptr_t __fastcall HookSetSecondary(Clip* self, void*, const Key* key) { return SetSecondary(self,key); }
uintptr_t __fastcall HookLoadClip(Clip* self, void*, Reader* reader) { return LoadClip(self,reader); }
}
#endif
