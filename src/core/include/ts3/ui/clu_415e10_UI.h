// clu_415e10_UI: the GIF frame decoder behind the game's .gif textures. Fifteen entry
// points around the two loaders 0x00416E20 (parse a stream into a frame-count wrapper)
// and 0x00416EA0/0x004170A0 (decode one frame into a 0x2A70 texture object), plus two
// texture builders 0x00417720 ("GNP") and 0x00418AB0 ("AGT.p*"), the game-heap free
// 0x004461A0
//
// The original mixes calling conventions: 0x00415E10/0x00416200 take their object in
// ESI, 0x00416A30/0x00416840 are plain __cdecl, and the entry points are __stdcall
// (except the two cleanup helpers, which are __cdecl). The odd register conventions are
// internal to this file; the exported entry points keep the original stack ABIs so the
// harness can swap them in.
#pragma once

#include <cstdint>

#include "ts3/io/stream.h"
#include "ts3/layout.h"
#include "ts3/ui/clu_414370_UI.h"

namespace ts3::ui {

// The 0x4C-byte GIF object 0x00416410 builds. The first fields match GifImage in
// clu_414370_UI.h; this unit additionally uses +0x14../+0x44 for the current frame
// (image descriptor, packed flags, palettes and the frame counter).
struct GifFile {
    int32_t width;             // +0x00 logical screen width
    int32_t height;            // +0x04
    int32_t colorDepth;        // +0x08 ((packed >> 4) & 7) + 1
    int32_t bitsPerEntry;      // +0x0C (packed & 7) + 1
    int32_t backgroundIndex;   // +0x10
    int32_t frameLeft;         // +0x14 image descriptor left
    int32_t frameTop;          // +0x18 top
    int32_t frameWidth;        // +0x1C width
    int32_t frameHeight;       // +0x20 height
    int32_t interlaceFlag;     // +0x24 packed & 0x40
    int32_t localBits;         // +0x28 (packed & 7) + 1
    int32_t transparentIndex;  // +0x2C 0xFFFFFFFF when no graphic control extension
    int32_t delay;             // +0x30 graphic control delay (u16)
    int32_t disposal;          // +0x34 graphic control packed & 0x1C
    uint8_t pad38[4];          // +0x38 never written by this unit
    uint8_t* globalPalette;    // +0x3C
    uint8_t* localPalette;     // +0x40
    int32_t frameCount;        // +0x44 set only by 0x00416A30 with a null destination
    GifContext* context;       // +0x48 the decoder context
};
TS3_X86_OFFSET(GifFile, frameLeft, 0x14);
TS3_X86_OFFSET(GifFile, frameWidth, 0x1C);
TS3_X86_OFFSET(GifFile, frameHeight, 0x20);
TS3_X86_OFFSET(GifFile, interlaceFlag, 0x24);
TS3_X86_OFFSET(GifFile, transparentIndex, 0x2C);
TS3_X86_OFFSET(GifFile, delay, 0x30);
TS3_X86_OFFSET(GifFile, disposal, 0x34);
TS3_X86_OFFSET(GifFile, globalPalette, 0x3C);
TS3_X86_OFFSET(GifFile, localPalette, 0x40);
TS3_X86_OFFSET(GifFile, frameCount, 0x44);
TS3_X86_OFFSET(GifFile, context, 0x48);
TS3_X86_SIZE(GifFile, 0x4C);

// The decoder context GifContext describes only its head; this unit writes one more
// field (+0x34) that clu_414370_UI leaves as padding.
struct GifContextFile {
    uint8_t pad00[0x34];
    int32_t decodedSize;       // +0x34 frameWidth * frameHeight (0x00416200)
    io::IStream* stream;       // +0x38
    int32_t field3C;           // +0x3C
    uint8_t tail[0x6170 - 0x40];
};
TS3_X86_OFFSET(GifContextFile, decodedSize, 0x34);
TS3_X86_OFFSET(GifContextFile, stream, 0x38);
TS3_X86_SIZE(GifContextFile, 0x6170);

// ---------------------------------------------------------------------------
// Entry points (address order)
// ---------------------------------------------------------------------------

// 0x00415E10 (object in ESI, plain ret): frees the local palette (+0x40), the global
// palette (+0x3C), the decoder context (+0x48) and the object itself through the game
// heap. Returns 1 when the object was non-null, 0 otherwise.
int32_t FreeGifFile415E10(GifFile* file);

// 0x00416200 (object in ESI, plain ret): reads the image descriptor (left/top/width/
// height, packed byte) and, when the packed byte's sign bit is set, the local colour
// table into +0x40. On success stores frameWidth*frameHeight in the context (+0x34)
// and calls the LZW setup 0x00415E50. Returns 1, or 0 when a read fails.
int32_t ReadImageDescriptor416200(GifFile* file);

// 0x00416840 (__cdecl): decodes one frame's LZW data row by row and blits the opaque
// pixels into the BGRA destination (stride bytes per row). The interlace pass advance
// is the original's. Returns 1 on success, 0 when the decode fails; the caller pops
// the three arguments.
int32_t __cdecl BlitGifFrame416840(GifFile* file, uint8_t* dest, int32_t stride);

// 0x00416A30 (__cdecl, stream first): the GIF parser. Reads block headers from the
// stream until a frame is decoded (or the requested frame `maxFrame` is reached),
// blitting into `dest` (stride bytes) when it is non-null. With a null destination it
// stops after the frame and stores frame+1 in the object's +0x44. Returns the object,
// or null (after freeing it) on any stream error.
GifFile* __cdecl DecodeGif416A30(io::IStream* stream, int32_t maxFrame, uint8_t* dest, int32_t stride);

// 0x00416E20 (ENTRY, __stdcall ret 0x10): allocates the 0x18-byte "FIG." wrapper,
// stores it in *out, parses the stream (0x00416A30 with a null destination) and copies
// the parse's frame count into wrapper+8. Returns the frame count, or 0 - freeing the
// parse object. The wrapper's +0x0C dword is -1.
int32_t __stdcall CreateGifWrapper416E20(GifFile** out, io::IStream* stream, uint32_t unusedA,
                                         uint32_t unusedB);

// 0x00416EA0 (ENTRY, __stdcall ret 8): parses frame `frameIndex` of source->stream
// (source+0x10) and builds the "FIG.p*" 0x2A70 texture object from the frame's
// palette, dimensions and delay. Returns the texture, or null.
TextureObject* __stdcall ExtractGifFrame416EA0(TextureSource* source, uint32_t frameIndex);

// 0x004170A0 (ENTRY, __stdcall ret 0x10): parses with 0x00416A30 (source->stream,
// frameInfo->+0x0C as the frame limit, dest, stride) and frees the parse object.
// Returns 1 when the parse succeeded, 0 otherwise.
int32_t __stdcall DecodeGifToBuffer4170A0(TextureSource* source, const void* frameInfo,
                                          uint8_t* dest, int32_t stride);

// 0x00417170 (__cdecl, no arguments): once (byte 0x011B02A2) pushes the static
// 0x01146140 cleanup node onto the game's cleanup list (0x011B02AC). Its return value
// is only defined on the first call (the list registration's result).
void __cdecl RegisterGifCleanup417170();

// 0x00417720 (ENTRY, __stdcall ret 8): builds the "GNP" 0x2A70 texture object from
// source->+0x14 (a two-pointer texture record): format byte +0x18, palette kind +0x19,
// dimensions at +0x00/+0x04, palette at +0x10 (RGB) with count at +0x14. Returns the
// texture (or null when a palette count is 0 or the allocation fails).
TextureObject* __stdcall BuildGnpTexture417720(void* source, uint32_t unused);

// 0x00418AB0 (ENTRY, __stdcall ret 8): seeks source->stream to 0, reads the 18-byte
// "AGT" header and (unless the format's bits exceed 15) the palette, and builds the
// "AGT.p*" 0x2A70 texture object. Returns the texture, or null when the header read,
// the palette read or the allocation fails.
TextureObject* __stdcall BuildAgtTexture418AB0(TextureSource* source, uint32_t unused);

// 0x00434260 (__cdecl): the texture height at +0x70 when a1 and a2 are non-null,
// a2+8 has the sign bit and a2+0x78 == 1; 0 otherwise.
int32_t __cdecl TextureHeight434260(void* a1, void* a2);

// 0x00434290 (__cdecl): reports a2+0x38 in *out1 and a2+0x30 in *out2 (when the
// pointers are non-null) and a2+0x30 as the result, when a2+0x30 > 0; 0 otherwise.
int32_t __cdecl TextureInfo434290(void* a1, void* a2, int32_t* out1, int32_t* out2);

// 0x004342D0 (__cdecl): reads the format flags out of a2 (byte +8 has bit 4): the
// +0x4C dword, the +0x16 u16 and the +0x50 pointer, according to a2+0x19 == 3.
// Returns the accumulated flag bits.
int32_t __cdecl TextureFlags4342D0(void* a1, void* a2, int32_t* out1, int32_t* out2,
                                   int32_t* out3);

// 0x004461A0 (__stdcall ret 4): frees through the game heap (0x004E3F70) and reports
// success in AL. The original ignores its argument, including null.
uint8_t __stdcall Free4461A0(void* block);

// 0x00446270 (__cdecl): pushes `node` onto the game's cleanup list (its next pointer
// is written at +0) and returns it.
void* __cdecl PushCleanup446270(void* node);

// --- internal helpers -----------------------------------------------------------------
// 0x00416A30's LZW helpers live in clu_415e50_x and are called through x86-only
// trampolines (their ABIs use EAX/ESI/EDI for `this`).

}  // namespace ts3::ui