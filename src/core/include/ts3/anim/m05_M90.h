#pragma once
#include "ts3/layout.h"
namespace ts3::anim::m90 {
struct Event {
 void* vtable;
 uint32_t owner,type,subtype;
 float time,tolerance,duration;
 uint32_t flags,payloadBytes;
 uint8_t* payload;
};
struct ExtendedEvent { Event base; uint32_t words[6]; };
TS3_X86_SIZE(Event,0x28);
TS3_X86_OFFSET(Event,payload,0x24);
TS3_X86_SIZE(ExtendedEvent,0x40);
// TS3W.exe 0x0057F4C0: Construct an effect event, leaving its uninitialized tail untouched.
Event* __fastcall ConstructEffect(Event*,void*,uint32_t);
// TS3W.exe 0x0057F510: Construct a stop-effect event with flag 2 and zero tolerance.
Event* __fastcall ConstructStopEffect(Event*,void*,uint32_t);
// TS3W.exe 0x0057F560: Construct a visibility event with scalar value one.
Event* __fastcall ConstructVisibility(Event*,void*,uint32_t);
// TS3W.exe 0x0057F5C0: Construct a create-prop event, preserving the word at original offset 0x2C.
Event* __fastcall ConstructCreateProp(Event*,void*,uint32_t);
// TS3W.exe 0x0057F620: Construct a destroy-prop event with flag 2.
Event* __fastcall ConstructDestroyProp(Event*,void*,uint32_t);
// TS3W.exe 0x0057F670: Allocate and parse an event; retain mixed byte/word counts and version flag conversion.
Event* ParseEvent(uint32_t,uint32_t,uint8_t**,uint32_t*,uint32_t);
}
