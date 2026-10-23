#pragma once

#include <cstdint>
#include "ts3/layout.h"

namespace ts3::videorecording {

// Video-recording buffer queue: vtable 0x01051EDC (slot 0 = the scalar deleting
// destructor at 0x009DA640, slot 1 = 0x009DA9E0), on top of a base whose vtable is
// 0x00FEFEC0. The recording manager at 0x009E12E0 allocates 0x0F8 bytes for it
// (operator-new call at 0x009E155A, name string at 0x01052984) and constructs it with
// the call at 0x009E1568.
//
// The fields the two functions of this unit touch, and nothing more is established
// about them here: +0x00 vtable; +0x04 a word cleared by an atomic exchange; +0x08 and
// +0x10..+0x27 scalar state (three bytes at +0x0C, a float at +0x1C); +0x2C..+0x34
// bump pointers describing the 0x80-byte inline pool at +0x3C; a self-pointing list
// sentinel at +0xBC with a name pointer at +0xC4 ("VideoRecording/CompletedAudioBuffers",
// 0x01051EEC); and the ResourceSystem lock (0x28 bytes, initialized by 0x004E17E0,
// destroyed by 0x004E1670) at +0xC8.
//
// +0x0F, +0x28, +0x38, the pool interior and +0xF0..+0xF4 are NOT written by the
// constructor; the exehost test poisons them and requires the poison to survive.
struct VideoRecording {
    void* const* vtable;    // +0x00 0x00FEFEC0 briefly, then 0x01051EDC
    uint32_t lockWord;      // +0x04
    uint32_t field08;       // +0x08
    uint8_t flag0c;         // +0x0C
    uint8_t flag0d;         // +0x0D
    uint8_t flag0e;         // +0x0E
    uint8_t pad0f;          // +0x0F never written
    uint32_t field10;       // +0x10
    uint32_t field14;       // +0x14
    uint32_t field18;       // +0x18
    float field1c;          // +0x1C
    uint32_t field20;       // +0x20
    uint32_t field24;       // +0x24
    uint32_t pad28;         // +0x28 never written
    uint8_t* blockBegin;    // +0x2C blocks
    uint8_t* blockEnd;      // +0x30 blocks
    uint8_t* blockLimit;    // +0x34 blocks + 0x80
    uint32_t pad38;         // +0x38 never written
    uint8_t blocks[0x80];   // +0x3C inline pool, never initialized here
    void* listNext;         // +0xBC list sentinel, points at itself
    void* listPrev;         // +0xC0 list sentinel, points at itself
    const char* listName;   // +0xC4 "VideoRecording/CompletedAudioBuffers"
    uint8_t lock[0x28];     // +0xC8 owned by 0x004E17E0 / 0x004E1670
    uint32_t padF0;         // +0xF0 never written
    uint32_t padF4;         // +0xF4 never written

    // Original 0x009DA5D0 (__thiscall, this in ecx, plain ret): lays out the object and
    // returns this in eax.
    VideoRecording* Construct();

    // Original 0x009DA640 (__thiscall, one 4-byte stack argument, ret 4; vtable slot 0):
    // runs the shared destructor body 0x009DA580, frees this through operator delete
    // 0x004E3F70 when the flag argument's bit 0 is set, and still returns this.
    VideoRecording* DeletingDtor(uint32_t flags);
};
TS3_X86_SIZE(VideoRecording, 0xF8);
TS3_X86_OFFSET(VideoRecording, vtable, 0x00);
TS3_X86_OFFSET(VideoRecording, lockWord, 0x04);
TS3_X86_OFFSET(VideoRecording, flag0c, 0x0C);
TS3_X86_OFFSET(VideoRecording, field1c, 0x1C);
TS3_X86_OFFSET(VideoRecording, blockBegin, 0x2C);
TS3_X86_OFFSET(VideoRecording, blockLimit, 0x34);
TS3_X86_OFFSET(VideoRecording, blocks, 0x3C);
TS3_X86_OFFSET(VideoRecording, listNext, 0xBC);
TS3_X86_OFFSET(VideoRecording, listName, 0xC4);
TS3_X86_OFFSET(VideoRecording, lock, 0xC8);

}  // namespace ts3::videorecording
