#pragma once
#include "ts3/layout.h"
namespace ts3::anim::m117 {
struct WideRange { wchar_t* begin; wchar_t* end; wchar_t* capacity; void* descriptor; };
struct Definition {
    void* controllers[20];
    uint32_t tag, age, gender, species, direction;
    uint8_t mirrorStop, disablePose, disableTurn, skipOne, skipTwo, stopOverride;
    uint8_t untouched[2];
    WideRange loopSound;
};
TS3_X86_OFFSET(Definition,tag,0x50);
TS3_X86_OFFSET(Definition,mirrorStop,0x64);
TS3_X86_OFFSET(Definition,loopSound,0x6c);
// TS3W.exe 0x008402A0: Parse locomotion properties and replace retained controller slots; unknown age/species preserve old values, boolean values test presence only.
bool __fastcall Build(void* self, void*, void* builder, uint32_t tag, void* properties, Definition* output, uint32_t productFlags);
}
