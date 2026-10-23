#pragma once
#include "ts3/layout.h"
#include <xmmintrin.h>
namespace ts3::anim::m147 {
struct Path { unsigned char* begin; unsigned char* end; unsigned char* capacity; };
struct Brain {
    void** vtable; void* secondary; unsigned char opaque08[12]; void* host; void** poseTable;
    unsigned char opaque1c[4]; __m128 velocity; __m128 facing;
    unsigned char opaque40[0x44]; unsigned char startEvent[16]; unsigned char endEvent[16];
    unsigned char opaquea4[8]; float speed; unsigned char opaqueb0[8]; float distance;
    unsigned char opaquebc[12]; void* handle; uint32_t flags; void* child; float time;
    unsigned char opaqued8[0x28]; __m128 position; __m128 tangent; int stopIndex; int currentIndex;
    Path* path; int reverse;
};
TS3_X86_OFFSET(Brain,host,0x14); TS3_X86_OFFSET(Brain,velocity,0x20);
TS3_X86_OFFSET(Brain,startEvent,0x84); TS3_X86_OFFSET(Brain,speed,0xac);
TS3_X86_OFFSET(Brain,handle,0xc8); TS3_X86_OFFSET(Brain,flags,0xcc);
TS3_X86_OFFSET(Brain,time,0xd4); TS3_X86_OFFSET(Brain,position,0x100);
TS3_X86_OFFSET(Brain,path,0x128);
struct Vehicle {
    void** vtable; void* secondary; unsigned char opaque08[0x18];
    void* firstTable; unsigned char first[0x1c]; void* secondTable; unsigned char second[0x20];
    void* thirdTable;
};
TS3_X86_OFFSET(Vehicle,firstTable,0x20); TS3_X86_OFFSET(Vehicle,secondTable,0x40); TS3_X86_OFFSET(Vehicle,thirdTable,0x64);
// TS3W.exe 0x00857280: Query the turn brain's interfaces, falling back to its base.
void* Query(Brain*,uint32_t);
// TS3W.exe 0x008572D0: Adjust the pose interface back to the brain and query it.
void* QueryPose(void*,uint32_t);
// TS3W.exe 0x008572E0: Advance the route spline and publish the interpolated pose; preserves low-byte flags and indices.
uintptr_t Update(Brain*,float);
// TS3W.exe 0x00857C20: Steer toward a target with the original rsqrt refinement and signed-bit angle clamp.
uintptr_t Steer(Brain*,const __m128*,const __m128*,uint32_t);
// TS3W.exe 0x00858430: Detach three vehicle interfaces, destroy the two owned states, then destroy the base.
uintptr_t Destroy(Vehicle*);
// TS3W.exe 0x008584E0: Adjust the secondary interface and perform flag-controlled deleting destruction.
void* DeleteSecondary(void*,uint32_t);
#if defined(_M_IX86)
void* __fastcall HookQuery(Brain*,void*,uint32_t);
void* __fastcall HookQueryPose(void*,void*,uint32_t);
uintptr_t __fastcall HookUpdate(Brain*,void*,float);
uintptr_t __fastcall HookSteer(Brain*,void*,const __m128*,const __m128*,uint32_t);
uintptr_t __fastcall HookDestroy(Vehicle*,void*);
void* __fastcall HookDeleteSecondary(void*,void*,uint32_t);
#endif
}
