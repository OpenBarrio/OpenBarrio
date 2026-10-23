#include "ts3/anim/m05_M112.h"
namespace ts3::anim::m112 {
uintptr_t BuildClip(Builder* self,const ClipArgs* args,Output* out) {
    auto* manager=CallGameFunction<void*,0x004AFD20>();
    uintptr_t result=reinterpret_cast<uintptr_t>(manager);
    if (!manager) return result & ~uintptr_t(255);
    void* resource=nullptr;
    result=CallGameVirtual<uintptr_t,3>(manager,&self->key,&resource,0,0,0,0);
    if (!(result&255)) {
        if (resource) result=CallGameVirtual<uintptr_t,1>(resource);
        return result & ~uintptr_t(255);
    }
    auto* memory=CallGameFunction<void*,0x004E3F90>(0x600,"Animation/ClipController",0,0,0,0);
    auto* controller=memory?CallGameMethod<unsigned char*,0x00568820>(memory,args->host,args->time):nullptr;
    uint8_t mirror=self->mirrored;
    if (args->invert) mirror=mirror==0;
    out->mirrored=mirror;
    CallGameMethod<void,0x00569C50>(controller,resource);
    CallGameMethod<void,0x00569330>(controller,mirror);
    controller[0x5fd]=args->flag; // EA faults after allocation failure, rather than returning false.
    if (args->extra) CallGameMethod<void,0x00569360>(controller,args->extra);
    result=CallGameMethod<uintptr_t,0x00CF3C70>(out,controller);
    if (resource) result=CallGameVirtual<uintptr_t,1>(resource);
    return (result & ~uintptr_t(255)) | 1;
}
uintptr_t Clear(Range* range,uintptr_t incoming) {
    auto* current=range->begin;
    auto* end=range->end;
    if (reinterpret_cast<uintptr_t>(current)<reinterpret_cast<uintptr_t>(end)) {
        do {
            CallGameMethod<uintptr_t,0x00754060>(current);
            current+=0x7c;
        } while (reinterpret_cast<uintptr_t>(current)<reinterpret_cast<uintptr_t>(end));
        incoming=reinterpret_cast<uintptr_t>(range->begin);
    }
    range->end=range->begin;
    return incoming;
}
uintptr_t Destroy(Range* range,uintptr_t incoming) {
    auto* current=range->begin;
    auto* end=range->end;
    while (reinterpret_cast<uintptr_t>(current)<reinterpret_cast<uintptr_t>(end)) {
        incoming=CallGameMethod<uintptr_t,0x00754060>(current);
        current+=0x7c;
    }
    return incoming;
}
uintptr_t BuildOther(void* self,void* arg,void* unused) {
    (void)unused;
    auto result=CallGameMethod<uintptr_t,0x0083DBD0>(static_cast<unsigned char*>(self)+8,arg);
    return (result & ~uintptr_t(255)) | 1;
}
namespace {
struct String { char* begin; char* end; char* capacity; const char* descriptor; };
struct Value { Key key; String name; };
struct ExtendedValue { Key key; String name; uint16_t first,second; };
struct ExtendedString { String name; uint16_t first,second; };
// Iterators are four machine words in the original map helpers.
struct Iterator { unsigned char* node; uintptr_t rest[3]; };
template<bool Extended> void* Get(Map* map,const Key* key) {
    Iterator it;
    if constexpr (Extended) CallGameMethod<void,0x0083B960>(map,&it,key);
    else CallGameMethod<void,0x005E5E20>(map,&it,key);
    if (it.node!=map->buckets[map->mask]) return it.node+0x10;
#if defined(_M_IX86)
    auto* empty=reinterpret_cast<char*>(0x011C7E54);
    auto* capacity=reinterpret_cast<char*>(0x011C7E55);
#else
    static char emptyBuffer[1]{};
    auto* empty=emptyBuffer; auto* capacity=emptyBuffer+1;
#endif
    String initial{empty,empty,capacity,"Locomotion/AnimationClipName"};
    // The extended variant appends two zero shorts to both the source and copied value.
    ExtendedString extended{initial,0,0};
    ExtendedValue value; value.key=*key;
    CallGameMethod<void,0x00405AD0>(&value.name,Extended?static_cast<void*>(&extended):static_cast<void*>(&initial));
    value.first=0; value.second=0;
    // EA clears only the low byte of the key argument slot, not the entire hint word.
    uintptr_t hint=reinterpret_cast<uintptr_t>(key)&~uintptr_t(255);
    if constexpr (Extended) CallGameMethod<void,0x0083D6F0>(map,&it,&value,hint);
    else CallGameMethod<void,0x0083D3E0>(map,&it,&value,hint);
    auto* result=it.node+0x10;
    const auto span=static_cast<intptr_t>(reinterpret_cast<uintptr_t>(value.name.capacity)-reinterpret_cast<uintptr_t>(value.name.begin));
    if (span>1 && value.name.begin) CallGameFunction<void,0x004E3F70>(value.name.begin);
    return result;
}
}
void* GetName(Map* m,const Key* k) { return Get<false>(m,k); }
void* GetExtendedName(Map* m,const Key* k) { return Get<true>(m,k); }
uintptr_t __fastcall BuildOtherEntry(void* self,void*,void* arg,void* other) { return BuildOther(self,arg,other); }
void* __fastcall GetNameEntry(Map* self,void*,const Key* key) { return GetName(self,key); }
void* __fastcall GetExtendedNameEntry(Map* self,void*,const Key* key) { return GetExtendedName(self,key); }
#if defined(_M_IX86)
__declspec(naked) uintptr_t __fastcall ClearEntry(Range*,void*) {
    __asm { push eax
            push ecx
            call Clear
            add esp,8
            ret }
}
__declspec(naked) uintptr_t __fastcall DestroyEntry(Range*,void*) {
    __asm { push eax
            push ecx
            call Destroy
            add esp,8
            ret }
}
#else
uintptr_t __fastcall ClearEntry(Range* r,void*) { return Clear(r,0); }
uintptr_t __fastcall DestroyEntry(Range* r,void*) { return Destroy(r,0); }
#endif
}
