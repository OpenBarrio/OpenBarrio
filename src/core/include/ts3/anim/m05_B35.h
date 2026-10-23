#pragma once
#include "ts3/layout.h"
namespace ts3::anim::b35 {
// Only the fields consumed by this batch are exposed; opaque bytes stay untouched.
struct Body {
    unsigned char opaque0[0xc4]; void* master;
    unsigned char opaqueC8[0x130-0xc8];
    float first,second,minimum; float* values; float* deltas;
    unsigned char opaque144[0x15c-0x144]; float scale;
    unsigned char opaque160[0x170-0x160]; int32_t goal,current; uint32_t token;
    unsigned char opaque17C[2]; uint8_t blockFirst,blockSecond;
    unsigned char opaque180[0x248-0x180]; uint32_t start,end;
    uint8_t flag250,flag251,useFirst,useSecond;
};
TS3_X86_OFFSET(Body,master,0xc4); TS3_X86_OFFSET(Body,values,0x13c);
TS3_X86_OFFSET(Body,current,0x174); TS3_X86_OFFSET(Body,start,0x248);
struct State { void** vtable; uint32_t opaque4; float x,y; uint32_t opaque10; uint8_t dirty; };
TS3_X86_OFFSET(State,x,8); TS3_X86_OFFSET(State,dirty,0x14);
// TS3W.exe 0x00848470: Return the base interface identifier (adapted FileType twin).
uint32_t __fastcall BaseType(void*,void*);
// TS3W.exe 0x00848480: Query the base interface identifier.
void* __fastcall BaseQuery(void*,void*,uint32_t);
// TS3W.exe 0x008484A0: Reset the speed limit to FLT_MAX without touching other fields.
void __fastcall Reset(State*,void*);
// TS3W.exe 0x008484B0: Query an optional global tuning service, returning positive zero if absent.
float Tuning(uint32_t);
// TS3W.exe 0x008484D0: Query a tuning mask, preserving x86 shift-count wrapping.
float TuningMask(uint32_t,uint32_t);
// TS3W.exe 0x00848500: Return the length of the virtual vector, summing Z squared plus Y squared before X squared.
float __fastcall Speed(void*,void*);
// TS3W.exe 0x00848560: Check signed progression or unequal range endpoints.
uint32_t __fastcall Pending(Body*,void*);
// TS3W.exe 0x008485F0: Notify the optional master of a nonzero token, then clear the token.
void __fastcall ClearToken(Body*,void*);
// TS3W.exe 0x00848620: Mark state dirty and invert sign encoding, retaining negative zero as sentinel.
void __fastcall Invert(State*,void*);
// TS3W.exe 0x00848700: Return the derived interface identifier (adapted FileType twin).
uint32_t __fastcall Type(void*,void*);
// TS3W.exe 0x00848710: Query either derived or base identifier (adapted query twin).
void* __fastcall Query(void*,void*,uint32_t);
// TS3W.exe 0x00848740: Restore base vtable, optionally free, and return this even after deletion (DestroyBase twin).
void* __fastcall Destroy(void*,void*,uint32_t);
// TS3W.exe 0x00848760: Query a controller interface and set its scalar and mode in that order.
void Configure(void*,float,uint32_t);
// TS3W.exe 0x008487A0: Interpolate the selected four-point tuning curve without clamping the first segment.
float Curve(float,uint32_t);
// TS3W.exe 0x00848890: Accumulate progression contributions in original order and apply the minimum.
float __fastcall Total(Body*,void*,uint8_t);
}
