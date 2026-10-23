#pragma once
#include "ts3/layout.h"
namespace ts3::anim::b06 {
struct Link { Link* next; Link* previous; };
struct alignas(16) PoseNode {
    void** vtable; uint32_t reserved[2]; Link children; PoseNode* parent;
    unsigned char opaque[24]; float translation[4], rotation[4];
};
TS3_X86_OFFSET(PoseNode, children, 0x0c);
TS3_X86_OFFSET(PoseNode, translation, 0x30);
struct Block { Block* next; unsigned char* begin; unsigned char* end; unsigned char* limit; };
struct Reader { int32_t cursor; void* owner; Block* block; unsigned char* end; unsigned char* limit; uint8_t error; };
struct Writer { int32_t cursor; void* owner; void* reserved; Block* block; unsigned char* end; unsigned char* limit; uint8_t error; };
TS3_X86_OFFSET(Reader,end,0x0c);
TS3_X86_OFFSET(Writer,end,0x10);
struct State {
    unsigned char opaque[0x30]; void* callback; void* user;
    float first, second; unsigned char unknown[12]; uint32_t a,b;
    uint8_t enabled; unsigned char padding[3]; float last;
};
TS3_X86_OFFSET(State,first,0x38);
TS3_X86_OFFSET(State,enabled,0x54);
// TS3W.exe 0x0055D750: Return the local-pose node class identifier.
uint32_t Type();
// TS3W.exe 0x0055D760: Query either supported interface without adjusting this.
void* Query(void*,uint32_t);
// TS3W.exe 0x0055D840: Save rotation then the first three translation lanes.
uintptr_t SavePose(PoseNode*,Writer*);
// TS3W.exe 0x0055D8C0: Load rotation then three translation lanes, retaining W.
uintptr_t LoadPose(PoseNode*,Reader*);
// TS3W.exe 0x0055D950: Evaluate the first child then overwrite the result pose.
uintptr_t Evaluate(PoseNode*,void*,void*);
// TS3W.exe 0x0055D9C0: Subtract the original routing lookup from the total at +68.
float Remaining(void*,uint32_t,uint32_t);
// TS3W.exe 0x0055D9E0: Recognize the eight local-pose mode constants.
uint32_t IsLocal(uint32_t);
// TS3W.exe 0x0055DA30: Recognize the five routing mode constants.
uint32_t IsRouting(uint32_t);
// TS3W.exe 0x0055DAB0: Store callback and user pointers, returning callback.
void* SetCallback(State*,void*,void*);
// TS3W.exe 0x0055DAD0: Save six state fields using advance-before-access cursor semantics.
uintptr_t SaveState(State*,Writer*);
// TS3W.exe 0x0055DBD0: Load six fields; failed bool read uses the stream argument's low byte.
uintptr_t LoadState(State*,Reader*);
// TS3W.exe 0x0055DCB0: Find an owner binding using global 011E9138.
void* FindRoute(const void*);
// TS3W.exe 0x0055DCD0: Find an owner binding using global 011DC484.
void* FindPose(const void*);
// TS3W.exe 0x0055DCF0: Return the greater float's address; ties and unordered select first.
const float* Maximum(const float*,const float*);
// TS3W.exe 0x0055DD10: Copy two aligned vectors into the large pose workspace; return source.
const void* SetWorkspace(void*,const void*);
}
#if defined(_M_IX86)
namespace ts3::anim::b06 {
inline void* __fastcall HookQuery(void* s,void*,uint32_t id) { return Query(s,id); }
inline uintptr_t __fastcall HookSavePose(PoseNode* s,void*,Writer* w) { return SavePose(s,w); }
inline uintptr_t __fastcall HookLoadPose(PoseNode* s,void*,Reader* r) { return LoadPose(s,r); }
inline uintptr_t __fastcall HookEvaluate(PoseNode* s,void*,void* a,void* b) { return Evaluate(s,a,b); }
inline float __fastcall HookRemaining(void* s,void*,uint32_t a,uint32_t b) { return Remaining(s,a,b); }
inline void* __fastcall HookSetCallback(State* s,void*,void* a,void* b) { return SetCallback(s,a,b); }
inline uintptr_t __fastcall HookSaveState(State* s,void*,Writer* w) { return SaveState(s,w); }
inline uintptr_t __fastcall HookLoadState(State* s,void*,Reader* r) { return LoadState(s,r); }
inline const void* __fastcall HookSetWorkspace(void* s,void*,const void* v) { return SetWorkspace(s,v); }
}
#endif
