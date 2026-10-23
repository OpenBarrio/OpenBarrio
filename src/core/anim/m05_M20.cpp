#include "ts3/anim/m05_M20.h"
namespace ts3::anim::m20 {
namespace {
void** Table() {
#if defined(_M_IX86)
    return reinterpret_cast<void**>(0x00fc17e8);
#else
    static void* table[11]{};
    return table;
#endif
}
}
Evaluator* Construct(Evaluator* self,void* host) {
    b01::Construct(&self->base,host);
    for(auto& word:self->values) word=0;
    for(auto& flag:self->flags) flag=0;
    self->base.vtable=Table();
    return self;
}
// Adapted from m03_B04 DestroyFactory: evaluator table/base reset and general free.
Evaluator* Destroy(Evaluator* self,uint32_t flags) {
    self->base.vtable=Table();
    b01::Reset(&self->base);
    if(flags&1u) CallGameFunction<void,0x004e3f70>(self);
    return self;
}
}
#if defined(_M_IX86)
namespace ts3::anim::m20::hooks {
Evaluator* __fastcall Construct(Evaluator* p,void*,void* host) { return m20::Construct(p,host); }
Evaluator* __fastcall Destroy(Evaluator* p,void*,uint32_t flags) { return m20::Destroy(p,flags); }
}
#endif
