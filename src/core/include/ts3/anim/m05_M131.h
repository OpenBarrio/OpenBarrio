#pragma once
#include "ts3/layout.h"
namespace ts3::anim::m131 {
struct Query {
    void* source;
    float limit;
    float zero[3];
    unsigned char flags[2];
    unsigned char padding[2];
    void* retained;
};
struct QueryResult { void* controller; unsigned char flag; };
struct RouteEntry { unsigned char opaque[24]; void* route; };
struct Body {
    unsigned char prefix[40];
    unsigned char scratch[84];
    void* routeArgs[4];
    unsigned char middle[252];
    RouteEntry* begin;
    RouteEntry* end;
};
TS3_X86_OFFSET(Body,routeArgs,0x7c);
TS3_X86_OFFSET(Body,begin,0x188);
TS3_X86_OFFSET(Body,end,0x18c);
TS3_X86_SIZE(Query,28);
// TS3W.exe 0x0084B430: query a controller distance, releasing both temporary references in order.
double Distance(void* source, void* provider, void* argument, unsigned char force);
// TS3W.exe 0x0084BC00: sample the first route or use the embedded scratch state when the range is empty.
uintptr_t Sample(Body* self, void* output);
struct SlaveState { unsigned char prefix[101]; unsigned char enabled; unsigned char tail[22]; };
struct ResetBody {
    unsigned char prefix[20]; void* host;
    unsigned char p18[117]; unsigned char enabled;
    unsigned char p8e[34]; SlaveState* slaves; void** controllers;
    unsigned char pb8[12]; void* manager; int count;
    unsigned char pcc[69]; unsigned char active;
    unsigned char p112[18]; void* graph;
    unsigned char p128[80]; uintptr_t token;
    unsigned char p17c[5]; unsigned char latch;
    unsigned char p182[3]; unsigned char forced;
};
TS3_X86_OFFSET(ResetBody,enabled,0x8d);
TS3_X86_OFFSET(ResetBody,slaves,0xb0);
TS3_X86_OFFSET(ResetBody,manager,0xc4);
TS3_X86_OFFSET(ResetBody,active,0x111);
TS3_X86_OFFSET(ResetBody,graph,0x124);
TS3_X86_OFFSET(ResetBody,token,0x178);
TS3_X86_OFFSET(ResetBody,latch,0x181);
TS3_X86_OFFSET(ResetBody,forced,0x185);
// TS3W.exe 0x0084B0C0: reset body and slave controllers, retaining the host resource across callbacks.
uintptr_t Reset(ResetBody* self, unsigned char force);
struct BuildSlave { unsigned char prefix[84]; void* a; unsigned char pad[4]; void* b; unsigned char tail[28]; };
struct ControllerLink { ControllerLink* next; ControllerLink* previous; void* controller; int mode; };
struct BuildBody {
    unsigned char prefix[20]; void* host; void* owner;
    ControllerLink* next; ControllerLink* previous; const char* descriptor;
    unsigned char p28[84]; void* routeArgs[4];
    unsigned char p8c[36]; BuildSlave* slaves; void** controllers;
    unsigned char pb8[12]; void* manager; int count; unsigned char shared;
    unsigned char pcd[87]; void* graph;
    unsigned char p128[85]; unsigned char started;
    unsigned char p17e[6]; unsigned char conformFlag;
};
TS3_X86_SIZE(BuildSlave,124);
TS3_X86_OFFSET(BuildBody,next,0x1c);
TS3_X86_OFFSET(BuildBody,shared,0xcc);
TS3_X86_OFFSET(BuildBody,graph,0x124);
TS3_X86_OFFSET(BuildBody,started,0x17d);
TS3_X86_OFFSET(BuildBody,conformFlag,0x184);
// TS3W.exe 0x0084B590: attach primary/slave controllers with optional route-conforming wrappers and retained list entries.
uintptr_t Attach(BuildBody* self, void* controller, void** slaves, int count, int mode, int track, unsigned char conform, float time);
#if defined(_M_IX86)
uintptr_t __fastcall AttachHook(BuildBody* self, void*, void* controller, void** slaves, int count, int mode, int track, unsigned char conform, float time);
uintptr_t __fastcall ResetHook(ResetBody* self, void*, unsigned char force);
uintptr_t __fastcall SampleHook(Body* self, void*, void* output);
#endif
}
