#pragma once
#include "ts3/layout.h"
namespace ts3::render::m07::m10 {
struct Material { uint8_t opaque[0x74]; int32_t kind; uint8_t gap[4]; uint16_t order; uint8_t tail[0x43]; uint8_t flags1,flags2; };
struct Model { uint8_t opaque[0x40]; Material** materials; };
struct Record { uint8_t transform[48]; uint32_t flags; uint16_t order,offset; uint8_t tail[8]; };
struct RecordVector { Record* begin; Record* end; Record* capacity; const char* tag; };
struct ResourceState {
    void* light; void** begin; void** end; void** capacity; const char* tag; uint32_t flags;
    uint8_t pad20c[4]; uint8_t resolver[0x78];
};
struct Instance { uint8_t pad[0x2c]; void* data2c; void* data30; uint8_t pad34[12]; float constants[4]; };
struct Packet {
    uint32_t unused,flags; uint8_t pad08[11]; uint8_t flag13,flag14,flag15; uint8_t pad16[2];
    void* data18; void* collector1c; void* special; void* reflection; uint32_t tick,budget; void* lighting;
};
struct Descriptor {
    float color[4],inverse; void* matrices; uint32_t fade,index,one; void* lightmap;
    uint32_t pad28; void* data18; uint32_t state;
    uint8_t noLight,different,lightState,option,translucent,lightingState,modelState,pad3b;
    float fadeValue; void* data30; void* data2c; uint8_t pad48[8]; float constants[4];
};
TS3_X86_OFFSET(Descriptor,constants,0x50);
TS3_X86_SIZE(Descriptor,0x60);
TS3_X86_OFFSET(Packet,lighting,0x30);
struct SceneModel {
    void** vtable; uint8_t pad04[0x3c]; uint32_t flags,renderFlags;
    uint8_t pad48[8]; uint8_t bounds[32]; uint8_t pad70[0x120]; Model* secondary; uint8_t pad194[8];
    uint32_t count; Record* records; uint8_t pad1a4[0x2c];
    float color[4]; uint8_t pad1e0[8]; float opacity; uint32_t state1ec,state1f0;
    ResourceState resource; RecordVector vector;
    Instance* instance; uint8_t options,options2,options3,pad29f;
    uint32_t resourceState,state2a4,tick,state2ac; uint16_t order; uint8_t pad2b2[6]; uint32_t fade; uint32_t bone;
};
TS3_X86_OFFSET(SceneModel,flags,0x40);
TS3_X86_OFFSET(SceneModel,count,0x19c);
TS3_X86_OFFSET(SceneModel,color,0x1d0);
TS3_X86_OFFSET(SceneModel,resource,0x1f4);
TS3_X86_OFFSET(SceneModel,vector,0x288);
TS3_X86_OFFSET(SceneModel,options,0x29c);
TS3_X86_OFFSET(SceneModel,fade,0x2b8);
TS3_X86_OFFSET(Material,flags1,0xc1);
// TS3W.exe 0x006F7B40: rebuild material instance flags and the terminal bounds record; suppression clearing persists across materials.
uintptr_t __fastcall Fixup(SceneModel*,void*);
// TS3W.exe 0x006F6250: submit per-material render descriptors or special transforms, preserving stale EAX on the disabled path.
uintptr_t __fastcall Submit(SceneModel*,void*,uint32_t,Packet*);
}
