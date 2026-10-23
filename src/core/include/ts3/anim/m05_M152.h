#pragma once
#include "ts3/layout.h"
#include <xmmintrin.h>
namespace ts3::anim::m152 {
struct Entity {
 void** vtable; void* unknown4; uint32_t idLow,idHigh;
 unsigned char opaque10[28]; uint32_t flags; Entity* parent;
};
struct Lookup { void** vtable; void* renderer; };
struct Component {
 void** vtable; Entity* entity; unsigned char opaque8[24];
 void* rig; void* unknown24; void* lookAt;
};
struct Pose { unsigned char opaque0[32]; void* rig; };
struct Rig { unsigned char opaque0[40]; unsigned char skeleton[1]; };
struct alignas(16) Matrix { __m128 row[4]; };
TS3_X86_OFFSET(Entity,flags,0x2c);
TS3_X86_OFFSET(Entity,parent,0x30);
TS3_X86_OFFSET(Component,rig,0x20);
TS3_X86_OFFSET(Component,lookAt,0x28);
// TS3W.exe 0x0093E9B0: resolve an object and query its animation interface.
void* Query(Lookup*,uint32_t,uint32_t);
// TS3W.exe 0x0093EAF0: recursively determine visibility; null input still dereferences null+0x30.
uint8_t Visible(Entity*);
// TS3W.exe 0x0093ED00: draw flagged skeleton joints and their axis arrowheads.
void DrawSkeleton(void*);
// TS3W.exe 0x0093F1E0: draw three axes; only the origin is transformed by the optional matrix.
uintptr_t DrawAxes(void*,const Matrix*,const Matrix*,float);
// TS3W.exe 0x0093F3D0: project and shadow text; viewport origin is intentionally ignored.
uintptr_t ProjectText(const int32_t*,const __m128*,void*,const char*,void*,uint32_t,const uint32_t*);
// TS3W.exe 0x0093F510: update rig registration and lazily construct a look-at controller; tests AL only.
uintptr_t UpdateLookAt(Component*);
#if defined(_M_IX86)
namespace hooks {
void* __fastcall Query(Lookup*,void*,uint32_t,uint32_t);
uintptr_t __fastcall UpdateLookAt(Component*,void*);
// Custom register-ABI adapters, not C-callable argument lists.
void DrawAxes();
void ProjectText();
}
#endif
}
