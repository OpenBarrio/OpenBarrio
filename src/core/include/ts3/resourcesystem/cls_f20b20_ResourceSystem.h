// The audio codec unit around TS3W.exe 0x00F20B20 (subsystem ResourceSystem): three game
// classes that share one object prefix and the same registration shape.
//
//   AudioEncoder_Pcm      ctor 0x00F20E60 (cdecl, table entry), vtable 0x0112CBC8
//                         slot 1 Init 0x00F20B20, slot 2 ReleaseBlock 0x00F20B40,
//                         slot 4 ReadRows 0x00F20D10 (u8/u16 samples out)
//   AudioDecoder_FlvImaAdpcm  ctor 0x00F21CC0 (cdecl, table entry), vtable 0x0112CE30
//                         slot 1 Init 0x00F21D50, slot 4 ReadRows 0x00F21DE0
//   CAELayer3Decoder      ctor 0x00F2A4B0 (thiscall, called by 0x00F2A920),
//                         vtable 0x0113855C
//
// Shared prefix: vtable, the ICoreAllocator at +0x04, the block source at +0x08 (the game
// object whose methods fetch blocks: 0x00402E50/0x00402F70, and whose Free 0x00403E00
// releases them), its two state dwords, then at +0x18 the RefCountedObject subobject the
// game's base ctor 0x004E17E0 builds. At +0x48 the currently fetched block ({size at +4,
// data at +8}) and at +0x4C the cursor inside it. The classes' objects are larger than the
// fields this unit touches; the sizes below are only the touched extent.
#pragma once

#include <cstdint>

#include "ts3/layout.h"

namespace ts3::resourcesystem {

// The object a fetched block is read from / released to. Only +4 and +8 are used here.
struct CodecBlock {
    uint32_t header;   // +0x00 untouched by this unit
    uint32_t size;     // +0x04 bytes of data
    const void* data;  // +0x08
};
TS3_X86_OFFSET(CodecBlock, size, 0x04);
TS3_X86_OFFSET(CodecBlock, data, 0x08);
TS3_X86_SIZE(CodecBlock, 0x0C);

// What ReadRows' caller passes (a pointer to): a destination sample buffer and the number
// of rows to fill / already filled. The original decrements `count` only by the rows it
// could NOT deliver (a full read leaves it untouched — sic).
struct AudioSampleBuffer {
    void* data;       // +0x00 the samples go here
    uint32_t unused4; // +0x04
    uint32_t unused8; // +0x08
    uint32_t count;   // +0x0C rows the caller asks for / reports
};
TS3_X86_OFFSET(AudioSampleBuffer, count, 0x0C);
TS3_X86_SIZE(AudioSampleBuffer, 0x10);

// AudioEncoder_Pcm (vtable 0x0112CBC8): interleaves PCM into the caller's sample buffer.
// ReadRows converts 8-bit unsigned samples to 16-bit ((v - 128) << 8) or copies 16-bit
// words, depending on `depth`; any other depth skips the copy but still advances.
struct AudioEncoderPcm {
    void* vtable;                     // +0x00 the game's 0x0112CBC8 on x86
    void* allocator;                  // +0x04 set by Init's first argument
    void* blockSource;                // +0x08 fetches/releases blocks (0x00402E50/0x00403E00)
    uint32_t sourceStateA;            // +0x0C
    uint32_t sourceStateB;            // +0x10
    uint32_t unused14;                // +0x14
    uint8_t baseObject[0x30];         // +0x18 RefCountedObject, built by 0x004E17E0
    CodecBlock* block;                // +0x48 the fetched block
    uint32_t cursor;                  // +0x4C bytes into it already converted
    uint32_t width;                   // +0x50 samples per row
    uint32_t depth;                   // +0x54 bits per sample (8 or 16)

    // Virtual slot 1 (0x00F20B20, ret 0x14): the constructor's Init call. Arguments 2 and
    // 5 are ignored; 1 becomes the allocator, 3 the width, 4 the depth.
    void Init(void* allocatorArg, void* unusedB, uint32_t widthArg, uint32_t depthArg, void* unusedE);
    // Virtual slot 2 (0x00F20B40): release the fetched block through the block source
    // (game 0x00403E00) and forget it. Nothing to do without a block.
    void ReleaseBlock();
    // Virtual slot 4 (0x00F20D10, ret 4): convert the block's next rows into the caller's
    // sample buffer, fetching more blocks as the current one is used up. Fetching and
    // freeing go through game code (0x00F20BA0, 0x00403E00); the copies are done here.
    // Returns the sample buffer (eax = *out); with a null argument the original leaves
    // eax = out itself, which this returns as-is.
    AudioSampleBuffer* ReadRows(AudioSampleBuffer** out);
};
TS3_X86_OFFSET(AudioEncoderPcm, allocator, 0x04);
TS3_X86_OFFSET(AudioEncoderPcm, blockSource, 0x08);
TS3_X86_OFFSET(AudioEncoderPcm, block, 0x48);
TS3_X86_OFFSET(AudioEncoderPcm, cursor, 0x4C);
TS3_X86_OFFSET(AudioEncoderPcm, width, 0x50);
TS3_X86_OFFSET(AudioEncoderPcm, depth, 0x54);
TS3_X86_SIZE(AudioEncoderPcm, 0x58);

// AudioDecoder_FlvImaAdpcm (vtable 0x0112CE30; its buffers are allocated under the
// "AudioDecoder_MsImaAdpcm …" names): decodes IMA ADPCM blocks into 16-bit samples. The
// game's decoder 0x00F21A10 and the state it keeps (allocated by Init, 0x1004 bytes) stay
// original; ReadRows here drives them and the 1017-row sample-buffer handoff.
struct AudioDecoderFlvImaAdpcm {
    void* vtable;             // +0x00 the game's 0x0112CE30 on x86
    void* allocator;          // +0x04 Init's first argument (its Alloc builds the state)
    void* blockSource;        // +0x08
    uint32_t sourceStateA;    // +0x0C
    uint32_t sourceStateB;    // +0x10
    uint32_t unused14;        // +0x14
    uint8_t baseObject[0x30]; // +0x18 RefCountedObject, built by 0x004E17E0
    CodecBlock* block;        // +0x48 the fetched block
    uint32_t cursor;          // +0x4C compressed bytes into it already decoded
    uint32_t width;           // +0x50 channels
    void* state;              // +0x54 the MsImaAdpcm state (0x1004 bytes from the allocator)
    uint32_t rowsPerBlock;    // +0x58 always 1017
    void* sampleBuffer;       // +0x5C 2034 * width bytes, one decoded block
    uint32_t pendingRows;     // +0x60 rows left in the sample buffer
    void* pendingPtr;         // +0x64 where in it the pending rows start

    // Virtual slot 1 (0x00F21D50, ret 0x14): arguments 2, 4 and 5 are ignored; 1 becomes
    // the allocator, 3 the width. Allocates the state and the sample buffer through it
    // (slot-2 Alloc, flags 1); a null allocator leaves the rest of the object alone.
    void Init(void* allocatorArg, void* unusedB, uint32_t widthArg, void* unusedD, void* unusedE);
    // Virtual slot 4 (0x00F21DE0, ret 4): decode the next rows into the caller's buffer,
    // draining the sample buffer's leftovers first. Blocks of more than 1017 rows decode
    // straight into the destination, shorter tails through the sample buffer (the rest
    // stays pending). The decoder 0x00F21A10 and the fetch (0x00F21340) stay game code.
    // Returns the sample buffer (eax = *out); with a null argument the original leaves
    // eax undefined, and this returns out instead.
    AudioSampleBuffer* ReadRows(AudioSampleBuffer** out);
};
TS3_X86_OFFSET(AudioDecoderFlvImaAdpcm, allocator, 0x04);
TS3_X86_OFFSET(AudioDecoderFlvImaAdpcm, blockSource, 0x08);
TS3_X86_OFFSET(AudioDecoderFlvImaAdpcm, block, 0x48);
TS3_X86_OFFSET(AudioDecoderFlvImaAdpcm, cursor, 0x4C);
TS3_X86_OFFSET(AudioDecoderFlvImaAdpcm, width, 0x50);
TS3_X86_OFFSET(AudioDecoderFlvImaAdpcm, state, 0x54);
TS3_X86_OFFSET(AudioDecoderFlvImaAdpcm, rowsPerBlock, 0x58);
TS3_X86_OFFSET(AudioDecoderFlvImaAdpcm, sampleBuffer, 0x5C);
TS3_X86_OFFSET(AudioDecoderFlvImaAdpcm, pendingRows, 0x60);
TS3_X86_OFFSET(AudioDecoderFlvImaAdpcm, pendingPtr, 0x64);
TS3_X86_SIZE(AudioDecoderFlvImaAdpcm, 0x68);

// CAELayer3Decoder (vtable 0x0113855C, "Snd::CAELayer3Decoder buffer"): only its
// constructor is in this unit; its methods live elsewhere. The constructor stores the
// block source, zeroes the shared prefix fields and the decode fields it owns, and leaves
// everything else (the +0x78 region in particular) as it found it.
struct CAELayer3Decoder {
    void* vtable;             // +0x00 the game's 0x011385C on x86
    void* unused04;           // +0x04 zeroed
    void* blockSource;        // +0x08 the constructor's argument
    uint32_t sourceStateA;    // +0x0C zeroed
    uint32_t sourceStateB;    // +0x10 zeroed
    uint32_t unused14;        // +0x14 zeroed
    uint8_t baseObject[0x30]; // +0x18 RefCountedObject, built by 0x004E17E0
    CodecBlock* block;        // +0x48 zeroed
    uint32_t cursor;          // +0x4C zeroed
    uint32_t field50;         // +0x50 zeroed
    void* state;              // +0x54 zeroed
    uint32_t field58;         // +0x58 zeroed
    void* sampleBuffer;       // +0x5C zeroed
    uint32_t pendingRows;     // +0x60 zeroed
    void* pendingPtr;         // +0x64 zeroed
    uint32_t field68;         // +0x68 zeroed
    uint32_t field6C;         // +0x6C zeroed
    uint32_t field70;         // +0x70 zeroed
    uint32_t field74;         // +0x74 zeroed
    uint8_t untouched78[0x18]; // +0x78..+0x8F the constructor leaves these alone
    uint32_t field90;         // +0x90 zeroed

    // 0x00F2A4B0, ret 4 (thiscall): the constructor body; returns `this`.
    CAELayer3Decoder* Construct(void* blockSourceArg);
};
TS3_X86_OFFSET(CAELayer3Decoder, unused04, 0x04);
TS3_X86_OFFSET(CAELayer3Decoder, blockSource, 0x08);
TS3_X86_OFFSET(CAELayer3Decoder, field90, 0x90);
TS3_X86_SIZE(CAELayer3Decoder, 0x94);

// The five dwords the registration tables pass to the two cdecl constructors, which
// forward them to the virtual Init (as five stack arguments; AudioEncoder_Pcm's Init
// receives a 0 in place of the fifth).
struct AudioCodecDesc {
    void* blockSource;  // +0x00 Init argument 1 (the allocator / block source)
    uint32_t field04;   // +0x04 Init argument 2 (ignored)
    uint32_t field08;   // +0x08 Init argument 3 (AudioEncoder_Pcm: width)
    uint32_t field0C;   // +0x0C Init argument 4 (AudioEncoder_Pcm: depth)
    uint32_t field10;   // +0x10 Init argument 5 (AudioDecoder_FlvImaAdpcm only)
};
TS3_X86_SIZE(AudioCodecDesc, 0x14);

// TS3W.exe 0x00F20E60 (AudioEncoder_Pcm's registration-table entry; __cdecl, the object
// as an explicit first argument — the table's callers pop both dwords): set the game's
// vtable, run the base constructor on the +0x18 subobject, clear the prefix fields (not
// +0x54 — the depth stays whatever the memory held), then call virtual slot 1 with the
// description's first four dwords and a 0. Returns 1; a null object crashes on the
// virtual dispatch, as in the original.
char ConstructAudioEncoderPcm(void* memory, const AudioCodecDesc* desc);

// TS3W.exe 0x00F21CC0 (AudioDecoder_FlvImaAdpcm's table entry, same shape): as above with
// this class's vtable, zeroing +0x54 too, and all five description dwords for Init.
char ConstructAudioDecoderFlvImaAdpcm(void* memory, const AudioCodecDesc* desc);

}  // namespace ts3::resourcesystem
