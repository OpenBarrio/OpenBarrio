#pragma once
#include "ts3/render/m07_abi.h"
#include <xmmintrin.h>
namespace ts3::render::m07 {
template<class T> struct M14Vector { T* begin; T* end; T* capacity; const char* tag; };
struct M14Item {
    void* active; uint8_t type,variant; uint16_t reserved06; void* mesh;
    uint8_t reserved0c[12]; void* key18; uint32_t reserved1c;
    void* key20; void* key24; void* material;
    float low[3],high[3]; uint8_t enabled,reserved45[19];
    float key58; uint16_t override5c,reserved5e; float bias;
    uint32_t reserved64,key68,key6c; uint8_t special,flag,reserved72[6];
};
struct M14Scenery {
    void* active; void* object; uint8_t reserved08[8]; void* material;
    uint8_t reserved14[20]; float low[3],high[3]; uint8_t reserved40[12];
    uint16_t override4c; uint8_t enabled,type;
};
struct M14Mesh { uint8_t reserved00[80]; void* object; uint32_t reserved54; uint8_t enabled,reserved59[23]; };
struct M14Node { M14Node* next; M14Node* previous; uint32_t index; };
struct M14Pool { M14Node* free; void* blocks; const char* tag; uint32_t growth; };
struct M14List { M14Node* next; M14Node* previous; M14Pool* pool; };
struct alignas(16) M14Record { __m128 low,high,sphere; uint32_t flags; uint8_t tail[12]; };
struct M14Renderer {
    uint8_t reserved00[0xd8]; M14Vector<M14Item> items;
    uint8_t reservede8[0x3110-0xe8]; M14Vector<M14Scenery> scenery;
    uint8_t reserved3120[0x14]; M14Vector<M14Mesh> meshes;
    uint8_t reserved3144[0x38cc-0x3144]; M14Vector<M14Record> records;
    M14Pool pool; M14Vector<M14List> lists; uint32_t reserved38fc;
    __m128 matrix[4],cameraMatrix[4]; uint32_t swarmListCount;
};
struct M14Output {
    uint8_t reserved00[0x40]; __m128 matrix[4];
    uint8_t reserved80[0x15c-0x80]; uint32_t count; M14Record* records;
};
TS3_X86_SIZE(M14Item,120); TS3_X86_SIZE(M14Scenery,80); TS3_X86_SIZE(M14Mesh,112);
TS3_X86_OFFSET(M14Renderer,records,0x38cc); TS3_X86_OFFSET(M14Renderer,matrix,0x3900);
TS3_X86_OFFSET(M14Renderer,swarmListCount,0x3980); TS3_X86_OFFSET(M14Output,count,0x15c);
// TS3W.exe 0x007212E0: builds Swarm draw groups and output bounds; keeps first-item grouping and AL material flags.
int BuildSwarmDrawLists(M14Renderer* self,M14Output* output,void* transform);
}
