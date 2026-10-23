#include "ts3/anim/m05_M43.h"
#include "ts3/layout.h"
namespace ts3::anim::m43 {
namespace {
void* Manager() {
#if defined(_M_IX86)
    return *reinterpret_cast<void**>(0x011d3c84);
#else
    ts3::AbortNotRewritten("resource handle manager"); return nullptr;
#endif
}
void* Descriptor() {
#if defined(_M_IX86)
    return reinterpret_cast<void*>(0x00d4b150);
#else
    ts3::AbortNotRewritten("resource handle descriptor"); return nullptr;
#endif
}
void* Callback() {
#if defined(_M_IX86)
    return reinterpret_cast<void*>(0x0056cd70);
#else
    ts3::AbortNotRewritten("resource handle callback"); return nullptr;
#endif
}
bool Tagged(const void* p) { return (reinterpret_cast<uintptr_t>(p) & 3) != 0; }
void Release(Clip* clip) {
    if (clip && --clip->refs == 0) {
        clip->refs = 1;
        ts3::CallGameVirtual<void,0>(clip,1u);
    }
}
Clip* Make(RandomController* self) {
#if defined(_M_IX86)
    auto storage=ts3::CallGameFunction<void*,0x004e3f90>(0x600u,reinterpret_cast<void*>(0x00fc1ec4),0,0,0,0);
#else
    auto storage=ts3::CallGameFunction<void*,0x004e3f90>(0x600u,static_cast<void*>(nullptr),0,0,0,0);
#endif
    if (!storage) return nullptr;
    return ts3::CallGameMethod<Clip*,0x00568820>(storage,self->base.hostSlot,0.0f);
}
double Draw() {
#if defined(_M_IX86)
    return ts3::CallGameMethod<double,0x004fa870>(reinterpret_cast<void*>(0x011cd7e4));
#else
    ts3::AbortNotRewritten("animation random generator"); return 0;
#endif
}
float ScaledDraw(float total) {
#if defined(_M_IX86)
    float result;
    // Keep the x87 product extended until the single store, as in 0056D4DF.
    __asm {
        mov ecx, 011cd7e4h
        mov eax, 004fa870h
        call eax
        fmul total
        fstp result
    }
    return result;
#else
    return static_cast<float>(Draw()*total);
#endif
}
}
void** CopyHandles(void** first, void** last, void** destination) {
    while (first != last) {
        if (auto manager=Manager()) {
            ts3::CallGameVirtual<void,14>(manager,destination,first,Callback(),Descriptor(),1,0,0);
        } else {
            void* source=Tagged(*first) ? nullptr : *first;
            if (source) ts3::CallGameVirtual<void,0>(source);
            if (auto manager=Manager()) ts3::CallGameVirtual<void,15>(manager,destination,Descriptor());
            if (*destination && !Tagged(*destination)) {
                ts3::CallGameVirtual<void,1>(*destination);
                *destination=nullptr;
            }
            *destination=source;
        }
        ++first; ++destination;
    }
    return destination;
}
uint8_t SetCurrent(RandomController* self, Clip* clip) {
    if (self->current == clip) return 0;
    auto old=self->current;
    if (clip) ++clip->refs;
    self->current=clip;
    Release(old);
    if (!self->current || !self->transition) return 0;
    ts3::CallGameMethod<void,0x00566cb0>(self->transition,self->current,self->blendTime,self->blendTime,1,0,0,0);
    ++self->selections;
    void* delegates=self->current ? &reinterpret_cast<Controller*>(self->current)->secondaryC : nullptr;
    ts3::CallGameMethod<void,0x00566170>(delegates,&self->interface28,-1);
    ts3::CallGameMethod<void,0x00568330>(self->current,self->argument);
    return 1;
}
const float* LowerBound(const float* first, const float* last, const float* value) {
    auto count=last-first;
    while (count>0) {
        auto half=count/2;
        if (*value > first[half]) { first+=half+1; count-=half+1; }
        else count=half;
    }
    return first;
}
uint8_t Repeat(RandomController* self) {
    if (!self->current) return 0;
    auto clip=Make(self);
    if (!clip) return 0;
    ++clip->refs;
    ts3::CallGameMethod<void,0x00569c50>(clip,self->current->resource);
    auto result=SetCurrent(self,clip);
    Release(clip);
    return result;
}
uint8_t Select(RandomController* self, uint32_t index) {
    if (index >= static_cast<uint32_t>(self->resourcesEnd-self->resourcesBegin)) return 0;
    if (!self->resourcesBegin[index] || Tagged(self->resourcesBegin[index])) return 0;
    auto clip=Make(self);
    if (!clip) return 0;
    ++clip->refs;
    auto resource=self->resourcesBegin[index]; // Reload: construction may mutate the handles.
    ts3::CallGameMethod<void,0x00569c50>(clip,Tagged(resource) ? nullptr : resource);
    self->repeatProbability=self->probabilitiesBegin[index];
    auto result=SetCurrent(self,clip);
    Release(clip);
    return result;
}
uint8_t Choose(RandomController* self) {
    if (self->disabled) return 0;
    if (self->current && self->repeatProbability > 0.0f && self->repeatProbability > Draw()) return Repeat(self);
    auto count=(self->entriesEnd-self->entriesBegin)/16;
    if (!count) return 0;
    float value=ScaledDraw(self->cumulativeEnd[-1]);
    auto found=LowerBound(self->cumulativeBegin,self->cumulativeEnd,&value);
    if (found == self->cumulativeEnd) return 0;
    auto index=static_cast<uint32_t>(found-self->cumulativeBegin);
    if (index >= static_cast<uint32_t>(self->resourcesEnd-self->resourcesBegin)) return 0;
    if (Select(self,index)) return 1;
    for (uint32_t i=0;i<static_cast<uint32_t>(count);++i) if (Select(self,i)) return 1;
    return Repeat(self);
}
#if defined(_M_IX86)
uint8_t __fastcall HookSetCurrent(RandomController* p, void*, Clip* c) { return SetCurrent(p,c); }
uint8_t __fastcall HookRepeat(RandomController* p, void*) { return Repeat(p); }
uint8_t __fastcall HookSelect(RandomController* p, void*, uint32_t i) { return Select(p,i); }
uint8_t __fastcall HookChoose(RandomController* p, void*) { return Choose(p); }
#endif
}
