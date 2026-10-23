#include "ts3/anim/m05_M42.h"
namespace ts3::anim::m42_impl {
void* DeleteTransform(void* self,uint32_t flags) {
    CallGameMethod<void,0x0056CC30>(self);
    if (flags & 1) CallGameFunction<void,0x004E3F70>(self);
    return self;
}
void Enable(RandomController* self) { self->enabled = 1; }
uintptr_t Query(RandomController* self,uint32_t argument) {
    return self->child ? CallGameVirtual<uintptr_t,3>(self->child,argument) : 0;
}
float Time(RandomController* self) {
    if (self->enabled) return self->child ? CallGameVirtual<float,7>(self->child) : 0.0f;
#if defined(_M_IX86)
    return *reinterpret_cast<const float*>(0x0114F594);
#else
    AbortNotRewritten("disabled controller time global"); return 0;
#endif
}
void SetRetained(RandomController* self,void* value) {
    void* old=self->retained;
    if (value != old) {
        if (value) CallGameVirtual<void,0>(value);
        self->retained=value;
        if (old) CallGameVirtual<void,1>(old);
    }
    if (self->clip) CallGameMethod<void,0x00568330>(self->clip,self->retained);
}
namespace {
bool Update(Cursor* c,uint32_t result) { CallGameMethod<void,0x007D77C0>(c,result); return c->good != 0; }
}
bool Save(RandomController* self,Cursor* c) {
    if (!Update(c,CallGameFunction<uint32_t,0x004DDBD0>(c->stream,1u,c->mode))) return false;
    if (!Update(c,CallGameFunction<uint32_t,0x004DDA60>(c->stream,static_cast<uint32_t>(self->enabled)))) return false;
    const auto count=static_cast<uint16_t>(self->entriesEnd-self->entries);
    if (!Update(c,CallGameFunction<uint32_t,0x004DDAD0>(c->stream,static_cast<uint32_t>(count),c->mode))) return false;
    for (auto* p=self->entries;p!=self->entriesEnd;++p) {
        if (!Update(c,CallGameFunction<uint32_t,0x004DDD20>(c->stream,p->lo,p->hi,c->mode))) return false;
        if (!Update(c,CallGameFunction<uint32_t,0x004DDBD0>(c->stream,p->a,c->mode))) return false;
        if (!Update(c,CallGameFunction<uint32_t,0x004DDBD0>(c->stream,p->b,c->mode))) return false;
    }
    for (auto* p=self->weights;p!=self->weightsEnd;++p)
        if (!Update(c,CallGameFunction<uint32_t,0x004DDFF0>(c->stream,*p,c->mode))) return false;
    for (auto* p=self->times;p!=self->timesEnd;++p)
        if (!Update(c,CallGameFunction<uint32_t,0x004DDFF0>(c->stream,*p,c->mode))) return false;
    return c->good != 0;
}
}
