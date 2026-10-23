#include "ts3/anim/m05_M52.h"
namespace ts3::anim {
uint8_t M52Load(M52Controller* self, M52Stream* stream) {
    uint32_t version=0xffffffff;
    auto token=CallGameFunction<uint32_t,0x004DD700>(stream->source,&version,stream->value);
    CallGameMethod<void,0x007D77C0>(stream,token);
    if (!stream->ok || version > 1) return 0;
    auto child=CallGameFunction<Controller*,0x00565C80>(self->base.hostSlot,stream);
    // This type has its retained pointer at +28, not the forwarding types' +2c.
    CallGameMethod<void,0x00CF3C70>(&self->observer,child);
    return stream->ok;
}
M52Controller* M52Destroy(M52Controller* self,uint32_t flags) {
#if defined(_M_IX86)
    self->base.vtable=reinterpret_cast<void*>(0x00fc2178);
    self->base.secondary8=reinterpret_cast<void*>(0x00fc2170);
    self->base.secondaryC=reinterpret_cast<void*>(0x00fc216c);
#endif
    auto child=static_cast<Controller*>(self->observer);
    if (child) {
        if (--child->refs == 0) {
            child->refs=1;
            CallGameVirtual<void*,0>(child,1u);
        }
    }
    CallGameMethod<void,0x00565B40>(&self->base);
    if (flags & 1) CallGameFunction<void,0x004E3F70>(self);
    return self;
}
float M52Time(M52Controller* self) { return CallGameVirtual<float,5>(self->child); }
float M52Start(M52Controller* self) { return CallGameVirtual<float,7>(self->child); }
float M52End(M52Controller* self) { return CallGameVirtual<float,8>(self->child); }
bool M52Active(M52Controller* self) {
    return self->child && static_cast<uint8_t>(CallGameVirtual<uint32_t,11>(self->child)) != 0;
}
}
#if defined(_M_IX86)
namespace ts3::anim::m52hooks {
uint8_t __fastcall Load(M52Controller* s,void*,M52Stream* p){return M52Load(s,p);}
M52Controller* __fastcall Destroy(M52Controller* s,void*,uint32_t f){return M52Destroy(s,f);}
float __fastcall Time(M52Controller* s,void*){return M52Time(s);}
float __fastcall Start(M52Controller* s,void*){return M52Start(s);}
float __fastcall End(M52Controller* s,void*){return M52End(s);}
bool __fastcall Active(M52Controller* s,void*){return M52Active(s);}
}
#endif
