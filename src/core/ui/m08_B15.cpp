#include "ts3/ui/m08_B15.h"
namespace ts3::ui::b15 {
namespace {
void* SecondaryBase(void* p) { return static_cast<uint8_t*>(p)-sizeof(void*); }
void* CallbackBase(void* p) { return static_cast<uint8_t*>(p)-offsetof(ControllerPrefix,callback); }
}
void* __fastcall F515C40(void* self, void*,uint32_t id) { return CallGameMethod<void*,0x00513600>(SecondaryBase(self),id); }
const void* __fastcall F515C50(void*, void*) {
#if defined(_M_IX86)
 return reinterpret_cast<const void*>(0x00FBA46C);
#else
 static const uint32_t descriptor=0; return &descriptor;
#endif
}
uint32_t __fastcall F515C60(void*, void*) { return 0x4EC1B8D8; }
uint32_t __fastcall F515C70(void* self, void*) { return CallGameMethod<uint32_t,0x00513950>(SecondaryBase(self)); }
void* __fastcall F515EC0(void* self, void*,uint32_t id) {
 if(id==0x08239C2F) return self?static_cast<uint8_t*>(self)+offsetof(ControllerPrefix,callback):nullptr;
 return CallGameMethod<void*,0x00504970>(self,id);
}
uint32_t __fastcall F515EF0(void*, void*) { return 0x4D8; }
uintptr_t __fastcall F515F00(Callback* self, void*) {
 // ADD does not define EAX. Only paths loading mode or invoking a slot define it.
 uintptr_t result=0;
 if(--self->references==0 && self->target) {
  result=self->mode;
  if(self->mode==0 || self->mode==1) result=CallGameVirtual<uintptr_t,40>(self->target);
  if(self->target) return CallGameVirtual<uintptr_t,41>(self->target);
 }
 return result;
}
uint32_t __fastcall F515F40(void*, void*) { return 0x08239C4E; }
uint32_t __fastcall F515FC0(void* self, void*,Descriptor* out) {
 out->object=SecondaryBase(self);
#if defined(_M_IX86)
 out->table=reinterpret_cast<const void*>(0x00FBAB68);
#else
 static const uint32_t descriptor=0; out->table=&descriptor;
#endif
 out->count=1;
 auto result=CallGameVirtual<uint32_t,5>(self);
 out->type=result; return result;
}
uint32_t __fastcall F515FF0(void* self, void*) { return CallGameMethod<uint32_t,0x00531FB0>(SecondaryBase(self)); }
void* __fastcall F516010(void* self, void*,uint32_t id) { return CallGameMethod<void*,0x00504970>(SecondaryBase(self),id); }
void* __fastcall F516020(void* self, void*,uint32_t id) { return F515EC0(CallbackBase(self),nullptr,id); }
void* __fastcall F516030(void* self, void*,uint32_t flags) { return CallGameMethod<void*,0x005160B0>(CallbackBase(self),flags); }
}
