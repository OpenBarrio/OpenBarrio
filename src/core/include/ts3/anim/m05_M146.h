#pragma once
#include "ts3/layout.h"
#include "ts3/math/float_values.h"
namespace ts3::anim::m146 {
struct Child { void** vtable; uint32_t refs; };
struct Segment { unsigned char* begin; unsigned char* end; };
struct alignas(16) Controller {
    void* mainTable; void* secondaryTable;
    unsigned char opaque08[0x10];
    void* interfaceTable;
    unsigned char opaque1c[0x88];
    void* host;
    uint32_t opaquea8;
    float angle;
    uint32_t opaqueb0;
    float first,second;
    unsigned char opaquebc[8];
    uint32_t state;
    void* handle;
    uint32_t opaquecc;
    Child* child;
    int32_t joints[7];
    unsigned char opaquef0[16];
    ts3::math::Float4Value position;
    unsigned char opaque110[16];
    uint32_t destination,index;
    Segment* segment;
    uint32_t reverse;
};
TS3_X86_OFFSET(Controller,host,0xa4);
TS3_X86_OFFSET(Controller,child,0xd0);
TS3_X86_OFFSET(Controller,joints,0xd4);
TS3_X86_OFFSET(Controller,position,0x100);
TS3_X86_OFFSET(Controller,segment,0x128);
// TS3W.exe 0x00856B60: adjust the secondary interface and return the controller type id.
uint32_t Type(void* secondary);
// TS3W.exe 0x00856B70: adjust the secondary interface and call the verified primary query-interface method.
void* Query(void* secondary,uint32_t id);
// TS3W.exe 0x00856B80: prepare the rotate controller and bind seven optional rig joints; preserve allocation-failure faults and release ordering.
uintptr_t Initialize(Controller*);
// TS3W.exe 0x00856F70: restore tables, tear down, release the child and optionally free, returning this even after free.
Controller* Destroy(Controller*,uint32_t flags);
// TS3W.exe 0x00856FF0: select a byte-sized route index and store its time, leaving the segment untouched without a manager. EAX is unspecified on the null-manager path.
void Set(Controller*,uint8_t index,float time);
// TS3W.exe 0x00857030: choose the shortest cyclic route direction and copy an endpoint; count-zero unsigned underflow is retained. EAX is unspecified without a manager.
void Select(Controller*,uint32_t start,uint32_t end);
}
#if defined(_M_IX86)
namespace ts3::anim::m146::hooks {
uint32_t __fastcall Type(void*,void*);
void* __fastcall Query(void*,void*,uint32_t);
uintptr_t __fastcall Initialize(Controller*,void*);
Controller* __fastcall Destroy(Controller*,void*,uint32_t);
void __fastcall Set(Controller*,void*,uint8_t,float);
void __fastcall Select(Controller*,void*,uint32_t,uint32_t);
}
#endif
