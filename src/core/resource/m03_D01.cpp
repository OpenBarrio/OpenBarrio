#include "ts3/resource/m03_D01.h"
namespace ts3::resource::m03 {
using Leaves=d01::Leaves;
Leaves& d01::Dependencies() { static Leaves d; return d; }
namespace {
void* Service() {
 if(auto f=d01::Dependencies().service) return f();
 return CallGameFunction<void*,0x005A00A0>();
}
const void* Name(uintptr_t address) {
#if defined(_M_IX86)
 return reinterpret_cast<const void*>(address);
#else
 // Native fake dispatch identifies controls by these stable tokens, never dereferences them.
 return reinterpret_cast<const void*>(address);
#endif
}
struct Callback { void* vtable; void* function; };
void* CallbackFor(uintptr_t fn) {
 void* p;
 if(auto f=d01::Dependencies().allocate) p=f(sizeof(Callback));
 else p=CallGameFunction<void*,0x004E3F90>(sizeof(Callback),Name(0x00FCC3AC),0,0,nullptr,0);
 if(p) {
#if defined(_M_IX86)
  *static_cast<Callback*>(p)={reinterpret_cast<void*>(0x0101F31C),reinterpret_cast<void*>(fn)};
#else
  // No game code is executed in native builds; native service owns callback dispatch.
  *static_cast<Callback*>(p)={nullptr,reinterpret_cast<void*>(fn)};
#endif
 }
 return p;
}
uintptr_t Update(void* p) {
 if(auto f=d01::Dependencies().update) return f(p);
 return CallGameMethod<uintptr_t,0x00733AA0>(p);
}
}
uintptr_t d01::RegisterCache(void* self) {
 if(void* s=Service()) CallGameVirtual<uintptr_t,6>(s,Name(0x00FFDEA0),uint32_t{4000});
 if(void* s=Service()) { void* cb=CallbackFor(0x00733120); CallGameVirtual<uintptr_t,22>(s,Name(0x00FFDD78),self,cb); }
 if(void* s=Service()) CallGameVirtual<uintptr_t,13>(s,static_cast<uint8_t*>(self)+0x8C,Name(0x00FFDD38),uint32_t{0},uint32_t{500000000},uint32_t{1000000});
 if(void* s=Service()) CallGameVirtual<uintptr_t,13>(s,static_cast<uint8_t*>(self)+0x90,Name(0x00FFDD50),uint32_t{0},uint32_t{50000000},uint32_t{1000000});
 if(void* s=Service()) { void* cb=CallbackFor(0x00C0FD60); CallGameVirtual<uintptr_t,22>(s,Name(0x00FFDE80),self,cb); }
 if(void* s=Service()) { void* cb=CallbackFor(0x00C0FD60); CallGameVirtual<uintptr_t,22>(s,Name(0x00FFDE54),self,cb); }
 if(void* s=Service()) return CallGameVirtual<uintptr_t,6>(s,static_cast<const void*>(nullptr),uint32_t{0x7FFFFFFF});
 return 0;
}
uintptr_t d01::UpdateFirst(void* self,uintptr_t,uintptr_t) { return Update(static_cast<uint8_t*>(self)+0x30); }
uintptr_t d01::UpdateSecond(void* self,uintptr_t,uintptr_t) { return Update(static_cast<uint8_t*>(self)+0xB0); }
}
