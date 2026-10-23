#include "ts3/audio/m06_B31.h"
#include "ts3/layout.h"
#include <cstddef>
namespace ts3::audio {
static_assert(sizeof(void*) != 4 || offsetof(B31Delegates, second)==0x10);
void* B31WriteMatrix(void* self, uint32_t handle, const uint32_t* matrix)
{
    auto append = [self](const uint32_t* word) { CallGameMethod<void,0x0089FEB0>(self,word,4u); };
    append(&handle);
    uint32_t word=0x24; append(&word);
    word=7; append(&word);
    for (unsigned i=0;i<9;++i) append(matrix+i);
#if defined(_M_IX86)
    auto bytes=static_cast<unsigned char*>(self);
    auto out=*reinterpret_cast<uint32_t**>(bytes+0x3c);
    if(out) *out=++*reinterpret_cast<uint32_t*>(bytes+0x48);
    return out;
#else
    AbortNotRewritten("B31 queue native layout"); return nullptr;
#endif
}
B31Delegates* B31Construct(B31Delegates* self)
{
#if defined(_M_IX86)
    self->primaryTable=reinterpret_cast<void*>(0x01031460);
    self->secondaryTable=reinterpret_cast<void*>(0x0103145C);
#else
    self->primaryTable=nullptr; self->secondaryTable=nullptr;
#endif
    self->refs=0; self->first=nullptr; self->second=nullptr;
    return self;
}
void* B31DestroySecondary(void* secondary, uint32_t flags)
{
    // Same -one-vptr adjustor as verified ResourceBaseDestroySecondary.
    auto self=static_cast<unsigned char*>(secondary)-sizeof(void*);
    return CallGameMethod<void*,0x008A0A30>(self,flags);
}
void B31SetSecond(B31Delegates* self, void* value)
{
    void* old=self->second;
    if(value!=old) {
        if(value) CallGameVirtual<void,0>(value);
        self->second=value;
        if(old) CallGameVirtual<void,1>(old);
    }
}
uintptr_t B31FirstOnly(B31Delegates* self, uint32_t a, uint32_t b)
{
    if(self->first) return CallGameVirtual<uintptr_t,4>(self->first,a,b);
    if(self->second) return CallGameVirtual<uintptr_t,4>(self->second,a,b);
    return 0;
}
void B31Broadcast(B31Delegates* self, uint32_t a)
{
    if(self->first) CallGameVirtual<void,5>(self->first,a);
    if(self->second) CallGameVirtual<void,5>(self->second,a);
}
uint32_t B31Query04C0(B31Delegates* self, uint32_t a, uint32_t b)
{
    if(self->first && static_cast<uint8_t>(CallGameVirtual<uint32_t,2>(self->first,a,b))) return 1;
    if(self->second && static_cast<uint8_t>(CallGameVirtual<uint32_t,2>(self->second,a,b))) return 1;
    return 0;
}
uint32_t B31Query0570(B31Delegates* self, uint32_t a)
{
    if(self->first && static_cast<uint8_t>(CallGameVirtual<uint32_t,6>(self->first,a))) return 1;
    if(self->second && static_cast<uint8_t>(CallGameVirtual<uint32_t,6>(self->second,a))) return 1;
    return 0;
}
uint32_t B31Query05B0(B31Delegates* self, uint32_t a, uint32_t b)
{
    if(self->first && static_cast<uint8_t>(CallGameVirtual<uint32_t,8>(self->first,a,b))) return 1;
    if(self->second && static_cast<uint8_t>(CallGameVirtual<uint32_t,8>(self->second,a,b))) return 1;
    return 0;
}
uint32_t B31Query0600(B31Delegates* self, uint32_t a, uint32_t b)
{
    if(self->first && static_cast<uint8_t>(CallGameVirtual<uint32_t,7>(self->first,a,b))) return 1;
    if(self->second && static_cast<uint8_t>(CallGameVirtual<uint32_t,7>(self->second,a,b))) return 1;
    return 0;
}
uint32_t B31Query0650(B31Delegates* self, uint32_t a, uint32_t b)
{
    if(self->first && static_cast<uint8_t>(CallGameVirtual<uint32_t,9>(self->first,a,b))) return 1;
    if(self->second && static_cast<uint8_t>(CallGameVirtual<uint32_t,9>(self->second,a,b))) return 1;
    return 0;
}
uint32_t B31Query06A0(B31Delegates* self, uint32_t a, uint32_t b)
{
    if(self->first && static_cast<uint8_t>(CallGameVirtual<uint32_t,10>(self->first,a,b))) return 1;
    if(self->second && static_cast<uint8_t>(CallGameVirtual<uint32_t,10>(self->second,a,b))) return 1;
    return 0;
}
uint32_t B31Query06F0(B31Delegates* self, uint32_t a, uint32_t b)
{
    if(self->first && static_cast<uint8_t>(CallGameVirtual<uint32_t,11>(self->first,a,b))) return 1;
    if(self->second && static_cast<uint8_t>(CallGameVirtual<uint32_t,11>(self->second,a,b))) return 1;
    return 0;
}
uint32_t B31Query0740(B31Delegates* self, uint32_t a, uint32_t b)
{
    if(self->first && static_cast<uint8_t>(CallGameVirtual<uint32_t,12>(self->first,a,b))) return 1;
    if(self->second && static_cast<uint8_t>(CallGameVirtual<uint32_t,12>(self->second,a,b))) return 1;
    return 0;
}
uint32_t B31Query0790(B31Delegates* self, uint32_t a, uint32_t b)
{
    if(self->first && static_cast<uint8_t>(CallGameVirtual<uint32_t,13>(self->first,a,b))) return 1;
    if(self->second && static_cast<uint8_t>(CallGameVirtual<uint32_t,13>(self->second,a,b))) return 1;
    return 0;
}
}

namespace ts3::audio {
uint32_t B31Adapter::B31WriteMatrix(uint32_t a0, void* a1) { return static_cast<uint32_t>(reinterpret_cast<uintptr_t>(ts3::audio::B31WriteMatrix(static_cast<void*>(this),a0,static_cast<const uint32_t*>(a1)))); }
uint32_t B31Adapter::B31Construct() { return static_cast<uint32_t>(reinterpret_cast<uintptr_t>(ts3::audio::B31Construct(reinterpret_cast<B31Delegates*>(this)))); }
void B31Adapter::B31SetSecond(void* a0) { ts3::audio::B31SetSecond(reinterpret_cast<B31Delegates*>(this),a0); }
uint32_t B31Adapter::B31FirstOnly(uint32_t a0, uint32_t a1) { return static_cast<uint32_t>(ts3::audio::B31FirstOnly(reinterpret_cast<B31Delegates*>(this),a0,a1)); }
void B31Adapter::B31Broadcast(uint32_t a0) { ts3::audio::B31Broadcast(reinterpret_cast<B31Delegates*>(this),a0); }
uint32_t B31Adapter::B31Query04C0(uint32_t a0, uint32_t a1) { return static_cast<uint32_t>(ts3::audio::B31Query04C0(reinterpret_cast<B31Delegates*>(this),a0,a1)); }
uint32_t B31Adapter::B31Query0570(uint32_t a0) { return static_cast<uint32_t>(ts3::audio::B31Query0570(reinterpret_cast<B31Delegates*>(this),a0)); }
uint32_t B31Adapter::B31Query05B0(uint32_t a0, uint32_t a1) { return static_cast<uint32_t>(ts3::audio::B31Query05B0(reinterpret_cast<B31Delegates*>(this),a0,a1)); }
uint32_t B31Adapter::B31Query0600(uint32_t a0, uint32_t a1) { return static_cast<uint32_t>(ts3::audio::B31Query0600(reinterpret_cast<B31Delegates*>(this),a0,a1)); }
uint32_t B31Adapter::B31Query0650(uint32_t a0, uint32_t a1) { return static_cast<uint32_t>(ts3::audio::B31Query0650(reinterpret_cast<B31Delegates*>(this),a0,a1)); }
uint32_t B31Adapter::B31Query06A0(uint32_t a0, uint32_t a1) { return static_cast<uint32_t>(ts3::audio::B31Query06A0(reinterpret_cast<B31Delegates*>(this),a0,a1)); }
uint32_t B31Adapter::B31Query06F0(uint32_t a0, uint32_t a1) { return static_cast<uint32_t>(ts3::audio::B31Query06F0(reinterpret_cast<B31Delegates*>(this),a0,a1)); }
uint32_t B31Adapter::B31Query0740(uint32_t a0, uint32_t a1) { return static_cast<uint32_t>(ts3::audio::B31Query0740(reinterpret_cast<B31Delegates*>(this),a0,a1)); }
uint32_t B31Adapter::B31Query0790(uint32_t a0, uint32_t a1) { return static_cast<uint32_t>(ts3::audio::B31Query0790(reinterpret_cast<B31Delegates*>(this),a0,a1)); }
}
