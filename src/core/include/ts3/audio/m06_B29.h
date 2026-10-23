#pragma once
#include "ts3/layout.h"
namespace ts3::audio {
struct B29Records {
    uint8_t* begin;
    uint8_t* end;
    uint8_t* records;
    uint32_t tag, bytes, count;
    // TS3W.exe 0x0089F8E0: report whether a tag exists and optionally return its type and byte size.
    bool Contains(uint32_t tag, uint32_t* type, uint32_t* size);
    // TS3W.exe 0x0089F900: copy a type-zero float, including x87 signaling-NaN quieting.
    bool ReadFloat(uint32_t tag, float* out);
    // TS3W.exe 0x0089F980: copy a type-two two-word key without checking payload length.
    bool ReadKey(uint32_t tag, uint32_t* out);
    // TS3W.exe 0x0089F9C0: return the payload of a type-three string record.
    void* ReadString(uint32_t tag);
    // TS3W.exe 0x0089F9F0: copy a type-four word; byte size is not validated.
    bool ReadWord(uint32_t tag, uint32_t* out);
    // TS3W.exe 0x0089FA30: return a type-five blob and optionally its byte size.
    void* ReadBlob(uint32_t tag, uint32_t* size);
    // TS3W.exe 0x0089FA70: copy three float words, quieting only the first signaling NaN.
    bool ReadVector(uint32_t tag, float* out);
    // TS3W.exe 0x0089FAC0: copy nine float words, quieting only the first signaling NaN.
    bool ReadMatrix(uint32_t tag, float* out);
    // TS3W.exe 0x0089FB50: overwrite a type-two key in place.
    bool WriteKey(uint32_t tag, uint32_t low, uint32_t high);
    // TS3W.exe 0x0089FBD0: tombstone the first matching record and return the traversal cursor, including on misses.
    uint8_t* Erase(uint32_t tag);
};
struct B29Queue {
    uint8_t *begin, *end, *capacity;
    const char* label;
    B29Records view;
    void* allocator;
    uint8_t *write, *read, *pending, *sizeSlot, *payload;
    uint32_t written, state, pendingCount, consumed, marker, tail;
    // TS3W.exe 0x0089F850: advance the read view; even consuming the final command returns true.
    bool Next();
    // TS3W.exe 0x0089F8A0: reset queue iteration state, preserving allocator, state, marker and tail.
    uint8_t* Reset();
    // TS3W.exe 0x0089FC20: initialize an empty Audio/CommandQueue and return this.
    B29Queue* Construct(void* allocator);
    // TS3W.exe 0x0089FC70: finish a pending command or reset an idle queue, returning the old size slot or initial cursor.
    uint8_t* Finish();
    // TS3W.exe 0x0089FCD0: partially swap queue storage and pending state; normalize only this queue's write cursor.
    B29Queue* Swap(B29Queue* other);
};
TS3_X86_SIZE(B29Records,0x18);
TS3_X86_OFFSET(B29Queue,view,0x10);
TS3_X86_OFFSET(B29Queue,allocator,0x28);
TS3_X86_OFFSET(B29Queue,written,0x40);
TS3_X86_SIZE(B29Queue,0x58);
}

namespace ts3::audio::b29hooks {
// TS3W.exe 0x0089F850: ABI adapter for B29Queue::Next.
bool __fastcall F0089F850(B29Queue* self, void*);
// TS3W.exe 0x0089F8A0: ABI adapter for B29Queue::Reset.
uint8_t* __fastcall F0089F8A0(B29Queue* self, void*);
// TS3W.exe 0x0089F8E0: ABI adapter for B29Records::Contains.
bool __fastcall F0089F8E0(B29Records* self, void*, uint32_t tag, uint32_t* type, uint32_t* size);
// TS3W.exe 0x0089F900: ABI adapter for B29Records::ReadFloat.
bool __fastcall F0089F900(B29Records* self, void*, uint32_t tag, float* out);
// TS3W.exe 0x0089F980: ABI adapter for B29Records::ReadKey.
bool __fastcall F0089F980(B29Records* self, void*, uint32_t tag, uint32_t* out);
// TS3W.exe 0x0089F9C0: ABI adapter for B29Records::ReadString.
void* __fastcall F0089F9C0(B29Records* self, void*, uint32_t tag);
// TS3W.exe 0x0089F9F0: ABI adapter for B29Records::ReadWord.
bool __fastcall F0089F9F0(B29Records* self, void*, uint32_t tag, uint32_t* out);
// TS3W.exe 0x0089FA30: ABI adapter for B29Records::ReadBlob.
void* __fastcall F0089FA30(B29Records* self, void*, uint32_t tag, uint32_t* size);
// TS3W.exe 0x0089FA70: ABI adapter for B29Records::ReadVector.
bool __fastcall F0089FA70(B29Records* self, void*, uint32_t tag, float* out);
// TS3W.exe 0x0089FAC0: ABI adapter for B29Records::ReadMatrix.
bool __fastcall F0089FAC0(B29Records* self, void*, uint32_t tag, float* out);
// TS3W.exe 0x0089FB50: ABI adapter for B29Records::WriteKey.
bool __fastcall F0089FB50(B29Records* self, void*, uint32_t tag, uint32_t low, uint32_t high);
// TS3W.exe 0x0089FBD0: ABI adapter for B29Records::Erase.
uint8_t* __fastcall F0089FBD0(B29Records* self, void*, uint32_t tag);
// TS3W.exe 0x0089FC20: ABI adapter for B29Queue::Construct.
B29Queue* __fastcall F0089FC20(B29Queue* self, void*, void* allocator);
// TS3W.exe 0x0089FC70: ABI adapter for B29Queue::Finish.
uint8_t* __fastcall F0089FC70(B29Queue* self, void*);
// TS3W.exe 0x0089FCD0: ABI adapter for B29Queue::Swap.
B29Queue* __fastcall F0089FCD0(B29Queue* self, void*, B29Queue* other);
}
