#pragma once
#include "ts3/anim/m05_layouts.h"
namespace ts3::anim {
struct alignas(16) M36Record { Float4Value value; uint32_t key; float start, end; unsigned char padding[4]; };
struct M36Range { M36Record *begin, *end, *capacity; void* descriptor; };
struct M36Controller { Controller base; void* secondary; M36Range rotation, timed, other; };
TS3_X86_SIZE(M36Record,0x20);
TS3_X86_OFFSET(M36Controller,rotation,0x2c);
TS3_X86_OFFSET(M36Controller,timed,0x3c);
TS3_X86_OFFSET(M36Controller,other,0x4c);
// TS3W.exe 0x0056B160: append a keyed quaternion, returning the old end or growth-helper EAX.
void* M36Append(M36Controller*,uint32_t,const Float4Value*);
// TS3W.exe 0x0056B1B0: append an identity quaternion without touching record padding.
void* M36Identity(M36Controller*,uint32_t);
// TS3W.exe 0x0056B230: append default timed data then an identity quaternion, returning the latter EAX.
void* M36DefaultTimed(M36Controller*,uint32_t);
// TS3W.exe 0x0056B2E0: append timed data before its identity quaternion, preserving write order.
void* M36Timed(M36Controller*,uint32_t,float,float,const Float4Value*);
// TS3W.exe 0x0056B370: append a keyed quaternion to the third range.
void* M36AppendOther(M36Controller*,uint32_t,const Float4Value*);
// TS3W.exe 0x0056B3C0: append an identity quaternion to the third range.
void* M36IdentityOther(M36Controller*,uint32_t);
struct M36Hooks {
 void* Append(uint32_t k,void* v) { return M36Append(reinterpret_cast<M36Controller*>(this),k,static_cast<Float4Value*>(v)); }
 void* Identity(uint32_t k) { return M36Identity(reinterpret_cast<M36Controller*>(this),k); }
 void* DefaultTimed(uint32_t k) { return M36DefaultTimed(reinterpret_cast<M36Controller*>(this),k); }
 void* Timed(uint32_t k,float a,float b,void* v) { return M36Timed(reinterpret_cast<M36Controller*>(this),k,a,b,static_cast<Float4Value*>(v)); }
 void* AppendOther(uint32_t k,void* v) { return M36AppendOther(reinterpret_cast<M36Controller*>(this),k,static_cast<Float4Value*>(v)); }
 void* IdentityOther(uint32_t k) { return M36IdentityOther(reinterpret_cast<M36Controller*>(this),k); }
};
}
