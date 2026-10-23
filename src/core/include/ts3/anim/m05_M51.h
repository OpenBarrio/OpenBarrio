#pragma once
#include "ts3/anim/m05_B08.h"
namespace ts3::anim::m51 {
struct ControllerState { Controller base; Controller* retained; };
TS3_X86_OFFSET(ControllerState,retained,0x28);
// TS3W.exe 0x0056FDB0: Write version one then serialize the retained controller, returning the stream status byte.
uint8_t Save(ControllerState*, b08::StreamState*);
// TS3W.exe 0x0056FE00: Construct an empty retained-controller wrapper without clearing opaque bytes.
ControllerState* Construct(ControllerState*,void*);
// TS3W.exe 0x0056FE30: Return the wrapper interface type ID.
uint32_t Type(void*);
// TS3W.exe 0x0056FE40: Query the secondary interface and return the enclosing wrapper.
ControllerState* Query(void*,uint32_t);
// TS3W.exe 0x0056FE60: Adjust the delegate interface and forward deleting destruction, including its return value.
ControllerState* DeleteInterface(void*,uint32_t);
// TS3W.exe 0x0056FE70: Construct a wrapper and non-atomically retain the supplied controller.
ControllerState* ConstructRetained(ControllerState*,void*,Controller*);
}

#if defined(_M_IX86)
namespace ts3::anim::m51::hooks {
inline uint8_t __fastcall Save(ControllerState* c,void*,b08::StreamState* s) { return m51::Save(c,s); }
inline ControllerState* __fastcall Construct(ControllerState* c,void*,void* h) { return m51::Construct(c,h); }
inline uint32_t __fastcall Type(void* c,void*) { return m51::Type(c); }
inline ControllerState* __fastcall Query(void* c,void*,uint32_t id) { return m51::Query(c,id); }
inline ControllerState* __fastcall DeleteInterface(void* c,void*,uint32_t f) { return m51::DeleteInterface(c,f); }
inline ControllerState* __fastcall ConstructRetained(ControllerState* c,void*,void* h,Controller* r) { return m51::ConstructRetained(c,h,r); }
}
#endif
