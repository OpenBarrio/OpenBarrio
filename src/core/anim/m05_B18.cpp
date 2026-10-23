#include "ts3/anim/m05_B18.h"
#include <cstring>
namespace ts3::anim {
// Constant leaves follow the verified FileType/MemoryType twins; only immediates differ.
uint32_t __fastcall B18SizeA(void*,void*) { return 0x3d0; }
uint32_t __fastcall B18SizeB(void*,void*) { return 0x24; }
uint32_t __fastcall B18SizeC(void*,void*) { return 0x2c; }
uint32_t __fastcall B18SizeD(void*,void*) { return 0x240; }
uint32_t __fastcall B18SizeE(void*,void*) { return 0x28; }
uint32_t __fastcall B18SizeF(void*,void*) { return 0x50; }
uint32_t __fastcall B18IdA(void*,void*) { return 0x5419e34e; }
uint32_t __fastcall B18IdB(void*,void*) { return 0x7419e37c; }
uintptr_t __stdcall B18Forward(void* object) {
    return CallGameVirtual<uintptr_t,5>(object,uint32_t(0));
}
// Adapted from QueryIface723DC0, using a pointer-wide result rather than its x86 mask.
void* __fastcall B18QueryA(void* self,void*,uint32_t id) {
    return id == 0x5419e34e || id == 0xf3f23a79 || id == 0x73deb8c8 ? self : nullptr;
}
void* __fastcall B18QueryB(void* self,void*,uint32_t id) {
    return id == 0x7419e37c || id == 0xf3f23a79 || id == 0x73deb8c8 ? self : nullptr;
}
uint32_t __fastcall B18EqualOne(void*,void*,uint32_t value) { return value == 1; }
uint32_t __fastcall B18GreaterOne(void*,void*,uint32_t value) { return value > 1; }
uintptr_t __fastcall B18Insert(B18Vector* self,void*,B18Pair* position,const B18Pair* value) {
    if (self->end != self->capacity) {
        // Alias adjustment happens before shifting, and the last element is copied first.
        if (reinterpret_cast<uintptr_t>(value) >= reinterpret_cast<uintptr_t>(position) &&
            reinterpret_cast<uintptr_t>(value) < reinterpret_cast<uintptr_t>(self->end))
            ++value;
        if (self->end) *self->end = self->end[-1];
        const auto bytes = reinterpret_cast<uintptr_t>(self->end) - sizeof(B18Pair) - reinterpret_cast<uintptr_t>(position);
        // EA moves to end - floor((end-1-position)/8), not an invented generic insert.
        std::memmove(position+1,position,bytes);
        const uint32_t low = value->low;
        position->low = low;
        position->high = value->high;
        ++self->end;
        return low;
    }
    const auto count = (reinterpret_cast<uintptr_t>(self->end)-reinterpret_cast<uintptr_t>(self->begin))/sizeof(B18Pair);
    const uint32_t newCount = count ? uint32_t(count)*2u : 1u;
    B18Pair* storage = nullptr;
    if (newCount) {
#if defined(_M_IX86)
        storage = CallGameFunction<B18Pair*,0x004E3F90>(newCount*uint32_t(sizeof(B18Pair)),self->descriptor,
            uint32_t(0),uint32_t(0),reinterpret_cast<const char*>(0x00F98980),uint32_t(0xe7));
#else
        AbortNotRewritten("B18 vector game allocator");
#endif
    }
    const auto prefix = reinterpret_cast<uintptr_t>(position)-reinterpret_cast<uintptr_t>(self->begin);
    std::memmove(storage,self->begin,prefix);
    auto* inserted = reinterpret_cast<B18Pair*>(reinterpret_cast<uintptr_t>(storage)+prefix);
    if (inserted) *inserted = *value;
    const auto suffix = reinterpret_cast<uintptr_t>(self->end)-reinterpret_cast<uintptr_t>(position);
    std::memmove(inserted+1,position,suffix);
    auto* newEnd = reinterpret_cast<B18Pair*>(reinterpret_cast<uintptr_t>(inserted+1)+suffix);
    if (self->begin) CallGameFunction<void,0x004E3F70>(self->begin);
    self->begin = storage;
    self->end = newEnd;
    self->capacity = reinterpret_cast<B18Pair*>(reinterpret_cast<uintptr_t>(storage)+size_t(newCount)*sizeof(B18Pair));
    return reinterpret_cast<uintptr_t>(self->capacity);
}
uintptr_t __fastcall B18Append(B18Manager* self,void*,uint32_t low,uint32_t high) {
    B18Vector& vector = self->pending;
    B18Pair* result = vector.end;
    if (reinterpret_cast<uintptr_t>(result) < reinterpret_cast<uintptr_t>(vector.capacity)) {
        vector.end = reinterpret_cast<B18Pair*>(reinterpret_cast<uintptr_t>(result)+sizeof(B18Pair));
        if (result) { result->low = low; result->high = high; }
        return reinterpret_cast<uintptr_t>(result);
    }
    const B18Pair value{low,high};
    return B18Insert(&vector,nullptr,result,&value);
}
}
