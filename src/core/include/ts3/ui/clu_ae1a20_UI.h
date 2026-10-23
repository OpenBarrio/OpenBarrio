#pragma once
#include "ts3/layout.h"

namespace ts3::ui {
struct UiSegmentWalk {
    unsigned char reserved00[0x68];
    int32_t firstIndex;
    unsigned char reserved6c[4];
    int32_t count;
    unsigned char reserved74[0x204];
    void** position;
    void** origin;
    unsigned char reserved280[4];
    void*** blocks;

    // TS3W.exe 0x00AE1A20: visit segmented-list entries with (0, firstIndex+i).
    // Nonempty EAX is the final block pointer; empty EAX is unspecified (normalized to null).
    void** VisitAE1A20();
};
TS3_X86_OFFSET(UiSegmentWalk, firstIndex, 0x68);
TS3_X86_OFFSET(UiSegmentWalk, count, 0x70);
TS3_X86_OFFSET(UiSegmentWalk, position, 0x278);
TS3_X86_OFFSET(UiSegmentWalk, origin, 0x27c);
TS3_X86_OFFSET(UiSegmentWalk, blocks, 0x284);
TS3_X86_SIZE(UiSegmentWalk, 0x288);
}
