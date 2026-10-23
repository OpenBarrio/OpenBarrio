#pragma once
#include "ts3/layout.h"
#include "ts3/routemanager/cls_857220_RouteManager.h"
namespace ts3::anim::b38 {
struct TurnState {
    unsigned char prefix[0xa4];
    void* host;
    unsigned char middle[0x28];
    routemanager::RouteManagerChild* child;
};
TS3_X86_OFFSET(TurnState,host,0xa4);
TS3_X86_OFFSET(TurnState,child,0xd0);
// Numeric-only observed controller state; opaque bytes are preserved.
struct alignas(16) State { unsigned char bytes[0x130]; };
// TS3W.exe 0x00856F10: Clear and release the retained child before notifying the host; re-arm refs before deletion.
void __fastcall Stop(TurnState*,void*);
// TS3W.exe 0x00856FE0: Clear the route manager's field at +128.
void __fastcall Clear(routemanager::RouteManagerObject*,void*);
// TS3W.exe 0x00857170: Accumulate the original shuffled SSE rotation/translation expression into the output.
float* __fastcall Accumulate(void*,void*,float*,const float*);
// TS3W.exe 0x00857270: Return the route controller type identifier.
uint32_t __fastcall RouteType(void*,void*);
// TS3W.exe 0x008572B0: Adjust the secondary interface by -18 and return its route type.
uint32_t __fastcall RouteSubType(void*,void*);
// TS3W.exe 0x008572C0: Adjust the secondary interface by -4 and invoke the verified destructor.
routemanager::RouteManagerObject* __fastcall DestroySub(void*,void*,uint32_t);
// TS3W.exe 0x00858490: Return the vehicle controller type identifier.
uint32_t __fastcall VehicleType(void*,void*);
// TS3W.exe 0x008584A0: Return this for either supported interface identifier, otherwise null.
void* __fastcall Query(void*,void*,uint32_t);
// TS3W.exe 0x008584D0: Read the float at +90.
float __fastcall Weight(State*,void*);
// TS3W.exe 0x008584F0: Copy the aligned vector at +80 and return the output pointer.
float* __fastcall Vector(State*,void*,float*);
// TS3W.exe 0x008585D0: Reset vehicle interpolation parameters and zero its vector.
void __fastcall ResetVehicle(State*,void*);
// TS3W.exe 0x00859190: Return the turn controller type identifier.
uint32_t __fastcall TurnType(void*,void*);
// TS3W.exe 0x008591A0: Initialize five turn parameters without touching surrounding state.
void __fastcall ResetTurn(State*,void*);
// TS3W.exe 0x00859220: Reset interpolation parameters and zero two scalar accumulators.
void __fastcall ResetMotion(State*,void*);
// TS3W.exe 0x00859260: Store the scalar at +A8, preserving its bits.
void __fastcall SetMotion(State*,void*,float);
}
