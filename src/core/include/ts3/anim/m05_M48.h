#pragma once
#include "ts3/anim/m05_B08.h"
namespace ts3::anim::m48 {
struct ScaleController { Controller base; void* delegate; Controller* child; float scale, duration; };
struct EventController {
    Controller base; void* delegate; void* source; void* factory; void* target;
    float weight; uint32_t mode, flags; uint8_t option44, option45, option46;
};
TS3_X86_OFFSET(ScaleController,child,0x2c);
TS3_X86_OFFSET(ScaleController,duration,0x34);
TS3_X86_OFFSET(EventController,factory,0x30);
TS3_X86_OFFSET(EventController,option46,0x46);
// TS3W.exe 0x0056ED30: Average sampled event responses with sign-bit clamping to [-2.5,2.5].
float __fastcall Sample(EventController*,void*,uint32_t,uint32_t,float,float,uint32_t,float);
// TS3W.exe 0x0056EEA0: Build and configure a controller event, evaluating route direction and timing.
void* __fastcall Evaluate(EventController*,void*,void*);
// TS3W.exe 0x0056F700: Save the retained controller and two floats, stopping at each failed stream status.
uint8_t __fastcall Save(ScaleController*,void*,b08::StreamState*);
// TS3W.exe 0x0056F790: Construct a scale controller with scale one and no child.
ScaleController* __fastcall Construct(ScaleController*,void*,void*);
// TS3W.exe 0x0056F820: Adjust the secondary interface by twelve bytes and forward deleting destruction.
Controller* __fastcall DeleteSecondary(void*,void*,uint32_t);
// TS3W.exe 0x0056F830: Retain a child, calculate its scaled duration and attach a delegate; null child faults.
ScaleController* __fastcall ConstructChild(ScaleController*,void*,void*,Controller*,float);
}
