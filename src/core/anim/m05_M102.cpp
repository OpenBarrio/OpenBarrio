#include "ts3/anim/m05_M102.h"
#include "ts3/layout.h"
#include <cstring>
namespace ts3::anim::m102 {
static uint16_t Word(const unsigned char* p) { uint16_t v; std::memcpy(&v,p,2); return v; }
template<unsigned Stride, unsigned Decoder>
static uintptr_t Sample(uint32_t key, float* first, uint16_t* firstFlags, uint32_t limit,
                        float* second, uint16_t* secondFlags, const void* table,
                        const unsigned char* records, uint32_t count) {
    // EA's lower_bound, followed by at most one predecessor correction.
    uint32_t begin=0, remaining=count;
    while (remaining) {
        const uint32_t half=remaining/2, mid=begin+half;
        if (Word(records+mid*Stride)<key) { begin=mid+1; remaining-=half+1; }
        else remaining=half;
    }
    if (begin>=count) --begin;
    if (begin && Word(records+begin*Stride)>key) --begin;
    const auto* selected=records+begin*Stride;
    CallGameFunction<uintptr_t,Decoder>(selected,first,firstFlags,table);
    const auto* next=selected+Stride;
    if (next!=records+count*Stride && Word(next)<=limit && next!=selected && !(next[2]&0x80))
        return CallGameFunction<uintptr_t,Decoder>(next,second,secondFlags,table);
    // Sequential scalar copies intentionally retain aliasing and x87 float loads.
    for (unsigned lane=0;lane<4;++lane) second[lane]=first[lane];
    *secondFlags=*firstFlags;
    return reinterpret_cast<uintptr_t>(secondFlags);
}
uintptr_t Sample6(uint32_t key, float* first, uint16_t* firstFlags, uint32_t limit, float* second, uint16_t* secondFlags, const void* table, const unsigned char* records, uint32_t count) {
    return Sample<6,0x00584480>(key,first,firstFlags,limit,second,secondFlags,table,records,count);
}
uintptr_t Sample8(uint32_t key, float* first, uint16_t* firstFlags, uint32_t limit, float* second, uint16_t* secondFlags, const void* table, const unsigned char* records, uint32_t count) {
    return Sample<8,0x00584550>(key,first,firstFlags,limit,second,secondFlags,table,records,count);
}
uintptr_t Sample10(uint32_t key, float* first, uint16_t* firstFlags, uint32_t limit, float* second, uint16_t* secondFlags, const void* table, const unsigned char* records, uint32_t count) {
    return Sample<10,0x005846A0>(key,first,firstFlags,limit,second,secondFlags,table,records,count);
}
uintptr_t Sample12(uint32_t key, float* first, uint16_t* firstFlags, uint32_t limit, float* second, uint16_t* secondFlags, const void* table, const unsigned char* records, uint32_t count) {
    return Sample<12,0x00584800>(key,first,firstFlags,limit,second,secondFlags,table,records,count);
}
uintptr_t Sample6B(uint32_t key, float* first, uint16_t* firstFlags, uint32_t limit, float* second, uint16_t* secondFlags, const void* table, const unsigned char* records, uint32_t count) {
    return Sample<6,0x00584910>(key,first,firstFlags,limit,second,secondFlags,table,records,count);
}
uintptr_t Sample6C(uint32_t key, float* first, uint16_t* firstFlags, uint32_t limit, float* second, uint16_t* secondFlags, const void* table, const unsigned char* records, uint32_t count) {
    return Sample<6,0x005849F0>(key,first,firstFlags,limit,second,secondFlags,table,records,count);
}
}
