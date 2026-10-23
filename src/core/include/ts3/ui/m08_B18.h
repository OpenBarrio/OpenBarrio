#pragma once
#include "ts3/layout.h"
#include <cstdint>
namespace ts3::ui::b18 {
// Sparse native view; opaque regions do not claim a complete Window layout.
struct Button {
    void* primary; void* secondary;
    uint8_t opaque08[0x28]; void* manager;
    uint8_t opaque34[0x44]; uint32_t state;
    uint8_t opaque7C[0x1D4]; void* button;
    uint8_t opaque254[8]; uint32_t options;
    uint32_t value260,value264,value268,value26C,value270;
    uintptr_t selected;
    uint8_t opaque278[0x20]; float rectangle[4],pair[2];
    uintptr_t images[8];
};
TS3_X86_OFFSET(Button,manager,0x30);
TS3_X86_OFFSET(Button,state,0x78);
TS3_X86_OFFSET(Button,button,0x250);
TS3_X86_OFFSET(Button,options,0x25C);
TS3_X86_OFFSET(Button,value264,0x264);
TS3_X86_OFFSET(Button,value26C,0x26C);
TS3_X86_OFFSET(Button,selected,0x274);
TS3_X86_OFFSET(Button,rectangle,0x298);
TS3_X86_OFFSET(Button,pair,0x2A8);
TS3_X86_OFFSET(Button,images,0x2B0);
// TS3W.exe 0x00517B60: Query the button interface, otherwise delegate to the original window query.
void* __fastcall Query(Button*,void*,uint32_t);
// TS3W.exe 0x00517B90: Read the window state through the button interface.
uint32_t __fastcall GetState(void*,void*);
// TS3W.exe 0x00517BA0: Modify state bits through slot 9; only the low byte of set is tested.
uintptr_t __fastcall ModifyState(void*,void*,uint32_t,uint32_t);
// TS3W.exe 0x00517BD0: Modify option bits through slot 12; only the low byte of set is tested.
uintptr_t __fastcall ModifyOptions(void*,void*,uint32_t,uint32_t);
// TS3W.exe 0x00517C00: Store value264 through the button interface and refresh the secondary interface.
uintptr_t __fastcall Set264(void*,void*,uint32_t);
// TS3W.exe 0x00517C20: Store value26C and refresh the secondary interface.
uintptr_t __fastcall Set26C(Button*,void*,uint32_t);
// TS3W.exe 0x00517C40: Set an image and refresh; invalid indices return the index unchanged.
uintptr_t __fastcall SetImage(void*,void*,uint32_t,uintptr_t);
// TS3W.exe 0x00517C70: Return an indexed image, or zero outside [0,8).
uintptr_t __fastcall GetImage(void*,void*,uint32_t);
// TS3W.exe 0x00517C90: Select the first available state image in EA priority order and refresh.
uintptr_t __fastcall SelectImage(Button*,void*);
// TS3W.exe 0x00517D70: Sequentially copy four floats, returning the third output pointer; preserve aliasing and sNaN quieting.
float* __fastcall GetRectangle(void*,void*,float*,float*,float*,float*);
// TS3W.exe 0x00517DA0: Sequentially copy two floats, returning the first output pointer; preserve aliasing and sNaN quieting.
float* __fastcall GetPair(void*,void*,float*,float*);
// TS3W.exe 0x00517DC0: Handle a press and optionally notify; result is AL only.
uint8_t __fastcall Press(Button*,void*,int32_t,uint32_t,uint32_t);
// TS3W.exe 0x00517E20: Handle a release and optionally notify; result is AL only.
uint8_t __fastcall Release(Button*,void*,int32_t,uint32_t,uint32_t);
// TS3W.exe 0x00517E80: Acquire manager capture before conditionally pressing; result is AL only.
uint8_t __fastcall CapturePress(Button*,void*,uint32_t,uint32_t,uint32_t,uint32_t);
// TS3W.exe 0x00517EF0: Release matching manager capture before conditionally releasing; result is AL only.
uint8_t __fastcall CaptureRelease(Button*,void*,uint32_t,uint32_t,uint32_t,uint32_t);
}
