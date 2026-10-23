#pragma once
#include <cstdint>
#include <cstddef>
#include "ts3/math/float_values.h"
namespace ts3::anim::b25 {
using ts3::math::Float4Value;
struct Event {
    void* vtable;
    uint32_t owner, type, subtype;
    float time, tolerance, duration;
    uint32_t flags, unused, fallback;
};
struct ScalarEvent { Event base; float value; };
struct DataEvent { Event base; uint32_t data[7]; };
struct alignas(16) MatrixEvent { Event base; uint32_t keys[3]; alignas(16) float matrix[16]; };
struct alignas(16) Accumulator { Float4Value untouched, translation, rotation, other, weight; };
#if defined(_M_IX86)
static_assert(sizeof(Event)==0x28 && offsetof(MatrixEvent,matrix)==0x40);
#endif
// TS3W.exe 0x0057CF60: Normalize accumulated translation and quaternion; NaN weight takes reset path.
Accumulator* Normalize(Accumulator*);
// TS3W.exe 0x0057F170: Accept an empty event payload (only AL is defined).
uint8_t __fastcall Accept(Event*,void*,void*,uint8_t**,uint32_t*,uint32_t);
// TS3W.exe 0x0057F180: Read a nonnegative scalar; count is tested as bytes but decremented as words.
uint8_t __fastcall ReadScalar(ScalarEvent*,void*,void*,uint8_t**,uint32_t*,uint32_t);
// TS3W.exe 0x0057F1D0: Read one payload word, retaining the byte/word count mismatch.
uint8_t __fastcall ReadWord(DataEvent*,void*,void*,uint8_t**,uint32_t*,uint32_t);
// TS3W.exe 0x0057F220: Read six words but store only five, in EA's permutation.
uint8_t __fastcall ReadSix(DataEvent*,void*,void*,uint8_t**,uint32_t*,uint32_t);
// TS3W.exe 0x0057F280: Select the nonzero payload identifier, otherwise the fallback identifier.
uint32_t __fastcall Identifier(DataEvent*,void*);
// TS3W.exe 0x0057F290: Read two words with the original byte/word count mismatch.
uint8_t __fastcall ReadPair(DataEvent*,void*,void*,uint8_t**,uint32_t*,uint32_t);
// TS3W.exe 0x0057F2E0: Initialize the base event, preserving all bytes beyond its prefix.
Event* __fastcall Construct(Event*,void*,uint32_t);
// TS3W.exe 0x0057F330: Initialize a matrix event and leave its alignment gap untouched.
MatrixEvent* __fastcall ConstructMatrix(MatrixEvent*,void*,uint32_t);
// TS3W.exe 0x0057F980: Compare base fields with x87 tolerance; unordered differences compare equal, upper EAX survives.
uint32_t __fastcall Equal(Event*,void*,const Event*);
// TS3W.exe 0x0057F9E0: Read a matrix payload, ignoring its fourth word and retaining count mismatch.
uint8_t __fastcall ReadMatrix(MatrixEvent*,void*,void*,uint8_t**,uint32_t*,uint32_t);
// TS3W.exe 0x0057FB50: Compare matrix event fields with ordered SSE tolerance and preserved upper EAX.
uint32_t __fastcall EqualMatrix(MatrixEvent*,void*,const MatrixEvent*);
// TS3W.exe 0x0057FC20: Compare scalar events using the unordered-accepting x87 tolerance.
uint32_t __fastcall EqualScalar(ScalarEvent*,void*,const ScalarEvent*);
// TS3W.exe 0x0057FF10: Restore the base vtable and optionally free, returning this even after free.
Event* __fastcall Destroy(Event*,void*,uint32_t);
// TS3W.exe 0x0057FF70: Insert key 1 then key 0 into the supplied set and return the second insertion result.
void* __fastcall CollectKeys(MatrixEvent*,void*,void*);
}
