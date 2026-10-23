#pragma once
#include "ts3/layout.h"
#include <cstdint>
namespace ts3::audio::b107 {
struct Segment {
    uint8_t opaque[12];
    float start, rate;
    uint32_t length;
    int32_t end;
    uint32_t cursor, frames, total;
    uint8_t flags[2], type, channels;
    uint32_t tail;
};
struct Slot {
    uint8_t opaque[8];
    const uint8_t* data;
    uint32_t loop, end, total;
    uint8_t gap18[0x20];
    void* resource;
    uint32_t word3c, word40, word44;
    uint8_t codec, mode, gap4a[2], format, gap4d[3];
};
struct Player {
    uint8_t opaque[0x58];
    Slot* slots;
    uint8_t gap5c[0x144];
    float start, rate;
    uint32_t cursor, length;
    uint8_t gap1b0[12];
    float outputRate;
    uint16_t limit, fadeOffset, segmentOffset;
    uint8_t gap1c6[3], index, count, active, fade;
};
struct Block {
    uint32_t opaque;
    float* samples;
    uint32_t word8;
    uint16_t offset, stride;
};
struct Config { uint8_t gap[12]; float rate; };
struct Context {
    uint8_t opaque[0x30000];
    double time;
    uint32_t word30008;
    Block *input, *output;
    uint32_t word30014;
    Config* config;
    uint32_t word3001c, count;
    float rate, scale;
    uint8_t channels;
};
struct Mix;
struct Link { Link *next, *previous; void* token; Mix* owner; uint8_t active; };
struct Mix {
    uint8_t opaque[0x21], channels, pad22[2];
    float* buffer;
    Link* head;
    uint8_t pad2c[8];
    float gains[22];
    uint16_t pending;
    uint8_t pad8e, dirty;
};
struct Stretch {
    void* table;
    void* allocator;
    uint8_t gap8[0x28];
    void* owned;
    uint8_t gap34[0x18];
    int32_t size;
};
// TS3W.exe 0x00EF6D30: Return the SndPlayer1 named descriptor (not a vtable).
void* __cdecl PlayerDescriptor();
// TS3W.exe 0x00EF6D40: Decode one big-endian packed player segment; return incidental last reader EAX.
uint32_t __fastcall Decode(Player*, void*, uint32_t index, const uint8_t* bits);
// TS3W.exe 0x00EF6E60: Attach resource metadata to a segment, keeping the partially cleared invalid path.
uint32_t __fastcall Attach(Player*, void*, uint32_t index, const uint8_t* data, int32_t bytes);
// TS3W.exe 0x00EF6F10: Convert a timestamp to a frame offset, with an AL-only success return.
uint8_t __stdcall Offset(Context*, double time, uint32_t* frames);
// TS3W.exe 0x00EF6FA0: Emit a subtractive fade and swap processing blocks, even for zero output frames.
uint32_t __fastcall Fade(Player*, void*, Context*);
// TS3W.exe 0x00EF7120: Store the low word of the fourth callback argument as the player frame limit.
uint32_t __cdecl SetLimit(Player*, void*, void*, uint32_t limit);
// TS3W.exe 0x00EF7140: Advance the byte-sized segment index, wrapping only on equality.
uintptr_t __fastcall Advance(Player*, void*);
// TS3W.exe 0x00EF71C0: Initialize SndPlayer1 parameter metadata and return its defaults.
void* __cdecl InitPlayerDescriptor();
// TS3W.exe 0x00EF71F0: Return the SubMix named descriptor.
void* __cdecl MixDescriptor();
// TS3W.exe 0x00EF7240: Unlink an attached input and optionally accumulate its final gains.
uintptr_t __fastcall Unlink(Link*, void*, const float* gains);
// TS3W.exe 0x00EF72D0: Publish/clear a 256-frame submix and apply dirty gain tails through the original kernel.
uint32_t __cdecl ProcessMix(Mix*, Context*);
// TS3W.exe 0x00EF73E0: Initialize SubMix parameter metadata and return its defaults.
void* __cdecl InitMixDescriptor();
// TS3W.exe 0x00EF7410: Return the TimeStretch named descriptor.
void* __cdecl StretchDescriptor();
// TS3W.exe 0x00EF7420: Free the owned stretch buffer through its allocator before clearing the field.
uintptr_t __fastcall FreeStretch(Stretch*, void*);
// TS3W.exe 0x00EF7440: Crossfade shifted blocks using a float-stored ramp and return the absolute shift.
uint32_t __fastcall Crossfade(Stretch*, void*, const float* a, const float* b, float* out, int32_t shift);
}
