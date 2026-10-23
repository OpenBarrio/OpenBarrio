#include "ts3/anim/m05_B37.h"
#include <xmmintrin.h>
namespace ts3::anim::b37 {
namespace {
void* Secondary(void* self) {
 return self ? reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(self)+offsetof(Controller,secondaryVtable)) : nullptr;
}
void* Parent(void* self) {
 return reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(self)-offsetof(Controller,secondaryVtable));
}
}
void* __fastcall QueryInterface(void* self, void*, uint32_t id) {
 return id==0xf4da7c8du ? self : nullptr;
}
void* __fastcall QueryBase(void* self, void*, uint32_t id) {
 if(id==0x0754f112u || id==0x54cfb94fu) return self;
 return id==0xf4da7c8du ? Secondary(self) : nullptr;
}
// Adapted from verified Get5334C0: only the member offset changes.
float __fastcall GetAC(Controller* self, void*) { return self->valueAC; }
float __fastcall GetA8(Controller* self, void*) { return self->valueA8; }
// Constant-return leaves follow verified m01::FileType.
uint32_t __fastcall TypeTurn(void*, void*) { return 0x06aac53au; }
void* __fastcall QueryTurn(void* self, void*, uint32_t id) {
 if(id==0x06aac53au || id==0x0754f112u) return self;
 if(id==0xf4da7c8du) return Secondary(self);
 return QueryBase(self,nullptr,id);
}
// The verified QueueNotification thunk likewise adjusts this before forwarding.
uint32_t __fastcall TypeTurnInterface(void* self, void*) { return TypeTurn(Parent(self),nullptr); }
void* __fastcall QueryTurnInterface(void* self, void*, uint32_t id) { return QueryTurn(Parent(self),nullptr,id); }
uintptr_t __fastcall ZeroVector(void*, void*, float* output) {
 _mm_store_ps(output,_mm_setzero_ps());
 return reinterpret_cast<uintptr_t>(output)&~uintptr_t(0xff);
}
uint32_t __fastcall TypeVehicle(void*, void*) { return 0xf4dba07fu; }
void* __fastcall QueryVehicle(void* self, void*, uint32_t id) {
 if(id==0x0754f112u || id==0xf4dba07fu) return self;
 if(id==0xf4da7c8du) return Secondary(self);
 return QueryBase(self,nullptr,id);
}
void* __fastcall QueryVehicleInterface(void* self, void*, uint32_t id) { return QueryVehicle(Parent(self),nullptr,id); }
uint32_t __fastcall TypeVehicleInterface(void* self, void*) { return TypeVehicle(Parent(self),nullptr); }
uint32_t __fastcall TypeOtherVehicle(void*, void*) { return 0x7bbc9f2fu; }
void* __fastcall QueryOtherVehicle(void* self, void*, uint32_t id) {
 if(id==0x0754f112u || id==0x7bbc9f2fu) return self;
 if(id==0xf4da7c8du) return Secondary(self);
 return QueryBase(self,nullptr,id);
}
}
