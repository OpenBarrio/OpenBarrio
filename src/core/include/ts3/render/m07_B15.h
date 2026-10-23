#pragma once
#include "ts3/render/m07_layouts.h"
#include "ts3/render/m07_abi.h"
namespace ts3::render::m07::b15 {
struct TextureRecord {
    uint32_t tag; void* resource; uint8_t gap08[0x14]; float radius;
    uint8_t gap20[0x20]; float position[3]; uint8_t gap4c[2]; uint8_t enabled; uint8_t tail;
};
struct alignas(16) MatrixRecord {
    float matrix[16]; float color[4]; uint32_t gap50; float scale;
    uint8_t enabled; uint8_t gap59[3]; float value; uint64_t key; void* handle; uint32_t tail;
};
struct Owner {
    uint8_t prefix[0xd8]; ShaderParamRecord* params; ShaderParamRecord* paramsEnd;
    uint8_t middle[0x3110-0xe0]; TextureRecord* textures; TextureRecord* texturesEnd;
    uint8_t gap3118[0x1c]; MatrixRecord* matrices; MatrixRecord* matricesEnd;
    uint8_t gap313c[0x778]; void* manager;
};
struct Cursor {
    uint8_t prefix[0x60]; int32_t limit; uint8_t gap64[0x3008];
    int32_t index; uint32_t kind; uint8_t* data;
};
struct Transform { uint16_t flags,revision; float position[3],scale,basis[9]; };
struct Dependencies {
    Thiscall<void*,void> textureManager=nullptr;
    Thiscall<void,void,void*,const void*> animate=nullptr;
    Thiscall<void*,Transform,const Transform*> copy=nullptr;
    Thiscall<void,Transform,float> normalize=nullptr;
    Thiscall<void,void> refresh=nullptr;
    float normalization=1.5707950592041016f;
};
// Optional native/relocated dependency injection; null entries use the x86 bridge.
extern Dependencies dependencies;
// TS3W.exe 0x0071B620: Set and expand parameter bounds; x87 copy order is retained.
void __fastcall Bounds(Owner*,void*,int32_t,const float*,float);
// TS3W.exe 0x0071B6F0: Store parameter enable byte when index is valid.
void __fastcall ParamEnabled(Owner*,void*,int32_t,uint8_t);
// TS3W.exe 0x0071B740: Store parameter word when index is valid.
void __fastcall ParamWord(Owner*,void*,int32_t,uint32_t);
// TS3W.exe 0x0071B790: Store parameter position and radius.
void __fastcall ParamPosition(Owner*,void*,int32_t,const float*,float);
// TS3W.exe 0x0071B800: Store texture enable byte.
void __fastcall TextureEnabled(Owner*,void*,int32_t,uint8_t);
// TS3W.exe 0x0071B850: Store texture position and radius.
void __fastcall TexturePosition(Owner*,void*,int32_t,const float*,float);
// TS3W.exe 0x0071B8B0: Submit animation parameters for a nonnull texture.
void __fastcall Animate(Owner*,void*,int32_t,uint8_t,uint8_t,float);
// TS3W.exe 0x0071B930: Store matrix enable byte.
void __fastcall MatrixEnabled(Owner*,void*,int32_t,uint8_t);
// TS3W.exe 0x0071B980: Normalize transform and build scaled matrix.
void __fastcall MatrixTransform(Owner*,void*,int32_t,const Transform*);
// TS3W.exe 0x0071BAF0: Store color with raw SSE float bits.
void __fastcall MatrixColor(Owner*,void*,int32_t,const float*,float);
// TS3W.exe 0x0071BB70: Store value, ignoring the middle stack argument.
void __fastcall MatrixValue(Owner*,void*,int32_t,const void*,float);
// TS3W.exe 0x0071BBC0: Release old binding then acquire the new resource key.
void __fastcall Bind(Owner*,void*,int32_t,uint64_t);
// TS3W.exe 0x0071BC70: Return a chunk; kinds 0/17 count groups of four, including negative requests.
int32_t __fastcall Next(Cursor*,void*,int32_t,void**,uint32_t*);
#if defined(_M_IX86)
static_assert(sizeof(TextureRecord)==0x50 && sizeof(MatrixRecord)==0x70);
static_assert(offsetof(Owner,textures)==0x3110 && offsetof(Owner,manager)==0x38b4);
static_assert(offsetof(Cursor,index)==0x306c);
#endif
}
