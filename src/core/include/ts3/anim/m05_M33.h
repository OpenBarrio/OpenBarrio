#pragma once
#include "ts3/anim/m05_layouts.h"
namespace ts3::anim {
struct M33Controller {
    Controller base;
    void* observer;
    Controller* first;
    Controller* second;
    float blend, duration;
};
TS3_X86_OFFSET(M33Controller,first,0x2c);
TS3_X86_OFFSET(M33Controller,duration,0x38);
// TS3W.exe 0x0056A320: Query either supported secondary-interface identifier.
void* M33Query(void* secondary, uint32_t id);
// TS3W.exe 0x0056A340: Adjust secondary this by twelve bytes and invoke deleting destructor.
void* M33Destroy(void* secondary, uint32_t flags);
// TS3W.exe 0x0056A350: Construct the two-controller interpolating controller and attach its observer.
M33Controller* M33Construct(M33Controller*, void* host, Controller*, Controller*, float);
// TS3W.exe 0x0056A400: Scale time for each child, retaining this across callbacks and restoring refs before deletion.
uintptr_t M33SetTime(M33Controller*, float);
// TS3W.exe 0x0056A480: Build a weighted evaluator pair; both child evaluations occur even on failure.
void* M33Evaluate(M33Controller*, void* context);
// TS3W.exe 0x0056A520: Deserialize children and weights, then attach the second child's observer.
bool M33Load(M33Controller*, void* stream);
}
#if defined(_M_IX86)
namespace ts3::anim::m33hooks {
void* __fastcall Query(void*,void*,uint32_t);
void* __fastcall Destroy(void*,void*,uint32_t);
M33Controller* __fastcall Construct(M33Controller*,void*,void*,Controller*,Controller*,float);
uintptr_t __fastcall SetTime(M33Controller*,void*,float);
void* __fastcall Evaluate(M33Controller*,void*,void*);
bool __fastcall Load(M33Controller*,void*,void*);
}
#endif
