#pragma once
#include "ts3/anim/m05_layouts.h"
namespace ts3::anim::m43 {
struct Clip {
    void* vtable;
    int32_t refs;
    unsigned char opaque08[0x5d0-8];
    void* resource;
};
struct RandomController {
    Controller base;
    void* interface28;
    void* transition;
    Clip* current;
    uint32_t argument;
    void** resourcesBegin;
    void** resourcesEnd;
    void** resourcesCapacity;
    void* descriptor44;
    unsigned char* entriesBegin;
    unsigned char* entriesEnd;
    unsigned char* entriesCapacity;
    void* descriptor54;
    float* cumulativeBegin;
    float* cumulativeEnd;
    float* cumulativeCapacity;
    void* descriptor64;
    float* probabilitiesBegin;
    float* probabilitiesEnd;
    float* probabilitiesCapacity;
    void* descriptor74;
    float blendTime;
    float repeatProbability;
    uint32_t selections;
    uint8_t disabled;
    unsigned char opaque85[3];
};
TS3_X86_OFFSET(Clip,resource,0x5d0);
TS3_X86_OFFSET(RandomController,current,0x30);
TS3_X86_OFFSET(RandomController,resourcesBegin,0x38);
TS3_X86_OFFSET(RandomController,entriesBegin,0x48);
TS3_X86_OFFSET(RandomController,cumulativeBegin,0x58);
TS3_X86_OFFSET(RandomController,probabilitiesBegin,0x68);
TS3_X86_OFFSET(RandomController,blendTime,0x78);
TS3_X86_OFFSET(RandomController,disabled,0x84);
// TS3W.exe 0x0056D050: Assign a range of tagged resource handles, retaining before releasing, or delegating to the manager.
void** CopyHandles(void** first, void** last, void** destination);
// TS3W.exe 0x0056D180: Replace the retained clip and attach it to the transition/delegate interfaces; returns AL only.
uint8_t SetCurrent(RandomController* self, Clip* clip);
// TS3W.exe 0x0056D230: Lower-bound search with unordered comparisons taking the left branch.
const float* LowerBound(const float* first, const float* last, const float* value);
// TS3W.exe 0x0056D310: Clone the current clip's resource and select the clone; allocation failure returns false.
uint8_t Repeat(RandomController* self);
// TS3W.exe 0x0056D3A0: Instantiate an indexed untagged resource, copying its repeat probability before selection.
uint8_t Select(RandomController* self, uint32_t index);
// TS3W.exe 0x0056D470: Choose by cumulative weight, fall back sequentially only after a valid selection fails, then repeat.
uint8_t Choose(RandomController* self);
#if defined(_M_IX86)
uint8_t __fastcall HookSetCurrent(RandomController*, void*, Clip*);
uint8_t __fastcall HookRepeat(RandomController*, void*);
uint8_t __fastcall HookSelect(RandomController*, void*, uint32_t);
uint8_t __fastcall HookChoose(RandomController*, void*);
#endif
}
