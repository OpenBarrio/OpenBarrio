#pragma once
#include "ts3/render/m07_interfaces.h"
namespace ts3::render::m07 {
// Additional initialization-only fields inside the scaffold's opaque host ranges.
struct DeviceInitM01 {
    uint8_t opaque00[8];void* d3d9;void* device;uint32_t thread;
    uint8_t opaque14[0x10];uint8_t windowed,vsync,multisample,pad27;
    void* window;uint32_t width,height,refresh,format,adapter,selectedAdapter,creations;
    uint8_t opaque48[0xC];uint32_t word54,word58,quadVertices;
    uint8_t opaque60[0x28];void* quadIndex;uint32_t word8C;void* backBuffer;void* depthBuffer;
    uint8_t opaque98[0x3D];uint8_t querySupport;uint8_t opaqueD6[0xA];
    DevicePrivate* privateData;uint8_t opaqueE4[8];void* callback;void* userdata;
    void* stateStack;uint32_t stackIndex,stackCapacity;
    uint8_t opaque100[0xE50];uint8_t fakeDevice;uint8_t padF51[3];uint32_t capsResult;
};
TS3_X86_OFFSET(DeviceInitM01,privateData,0xE0);
TS3_X86_OFFSET(DeviceInitM01,quadIndex,0x88);
TS3_X86_OFFSET(DeviceInitM01,fakeDevice,0xF50);
// TS3W.exe 0x00611080: Allocate and initialize host, device and immediate resources, retaining failure-path state.
bool InitDeviceM01(DeviceInitM01* self,uint8_t fake);
// TS3W.exe 0x0060D290: Upload stream mip/face data, preserving original-depth swizzle iteration and ignored upload failures.
bool UploadTextureM01(void* texture,void* stream,const void* options,uint8_t swizzle);
// TS3W.exe 0x00611C30: Atomically release a fake shader and dispatch the deleting destructor on zero.
uint32_t __stdcall ShaderReleaseM01(FakeShader* self);
// TS3W.exe 0x00611C60: Atomically retain a fake shader, returning the wrapping count.
uint32_t __stdcall ShaderAddRefM01(FakeShader* self);
// TS3W.exe 0x00611EA0: Initialize pixel bytecode and retain its device, with initial reference count zero.
FakeShader* InitPixelM01(FakeShader* self,void* device,const void* bytes,uint32_t size);
// TS3W.exe 0x00611EF0: Initialize vertex bytecode and retain its device, with initial reference count zero.
FakeShader* InitVertexM01(FakeShader* self,void* device,const void* bytes,uint32_t size);
}
namespace ts3::render::m07 {
// x86 ECX/unused-EDX adapters for original thiscall entry points.
bool __fastcall InitDeviceEntryM01(DeviceInitM01*,void*,uint8_t);
FakeShader* __fastcall InitPixelEntryM01(FakeShader*,void*,void*,const void*,uint32_t);
FakeShader* __fastcall InitVertexEntryM01(FakeShader*,void*,void*,const void*,uint32_t);
}
