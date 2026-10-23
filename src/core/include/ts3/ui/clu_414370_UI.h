// Cluster unit clu_414370_UI (subsystem UI): the texture header readers. Three
// entry points parse an image header out of an IStream and build a 0x2A70-byte
// texture description object (0x00414530 BMP, 0x004154D0 DDS/CT3S, 0x00415370 a
// 24-byte DDS mip-count probe), one builds the GIF loader pair (0x00416410 +
// 0x00416100), and 0x004461C0 is the shared stream-read helper (IStream vtable
// slot 12 with a negative-count guard) that 25 call sites across the engine use.
// None of the three header readers touch pixel data - only headers and palettes.
// ABIs match the originals exactly so the harness can swap them in: 0x00414530 and
// 0x004154D0 are __stdcall ret 8 (source object with the stream at +0x10, mip
// level), 0x00415370 is __stdcall ret 0x10 (out pointer, stream, two unused), and
// 0x00416410 takes its stream in EAX (harness hook goes through the x86-only naked
// shim GifOpenFromStreamEax). Dependencies not rewritten yet (allocator 0x00446180,
// free 0x004461A0, seek 0x00446220 - all in other units) are called through the
// bridge (ts3::CallGameStdcall); on x64 builds those abort, as there is no game.
#pragma once

#include <cstdint>

#include "ts3/io/stream.h"
#include "ts3/layout.h"

namespace ts3::ui {

// 0x00414530 / 0x004154D0's input: an object whose field at +0x10 is the IStream to
// read the header from (the original only ever loads [a1+0x10]).
struct TextureSource {
    uint8_t prefix[0x10];   // never read by this unit
    io::IStream* stream;    // +0x10
};
TS3_X86_OFFSET(TextureSource, stream, 0x10);

// The 0x2A70-byte texture description 0x00414530 and 0x004154D0 build (memset to
// zero first, so every field not listed below reads as 0). Both readers write the
// same layout; magic is "PMB." for BMP and "CT3S" for DDS, version is always 345.
struct TextureObject {
    uint32_t magic;         // +0x00 0x2E424D50 "PMB." / 0x53335443 "CT3S"
    uint32_t totalSize;     // +0x04 0x2A70
    uint32_t version;       // +0x08 345
    int32_t mode;           // +0x0C BMP: 0; DDS: the mip argument
    int32_t width;          // +0x10
    int32_t height;         // +0x14 (abs of the header's signed height for BMP)
    int32_t bytesPerPixel;  // +0x18 32 (true colour) or 8 (paletted)
    int32_t depth;          // +0x1C bits per channel / source bpp
    uint32_t reserved020;   // +0x20
    int32_t paletteCount;   // +0x24 entries at +0x28 (max 256)
    uint8_t palette[0x400]; // +0x28..+0x427 BGRA entries (the 3-byte BMP palette
                            //  gets 0xFF alpha forced in)
    int32_t formatKind;     // +0x428 BMP: header kind 0..3; DDS: 0x100 / 42 / 0x40000
    int32_t compressed;     // +0x42C BMP: compression 1 or 2; DDS: 3 for FOURCC
    int32_t dpiKind;        // +0x430 100 (BMP) / 90 (DDS)
    uint32_t blockBytes;    // +0x434 DDS: 8 * ceil4(width)/4 * ceil4(height)/4
    int32_t alphaBits;      // +0x438
    int32_t redBits;        // +0x43C
    int32_t greenBits;      // +0x440
    int32_t blueBits;       // +0x444
    uint32_t reserved448;   // +0x448
    int32_t pitchX;         // +0x44C DDS (computed only when the +0x6C extra dword says so)
    uint32_t reserved450;   // +0x450
    int32_t pitchY;         // +0x454
    uint32_t reserved458;   // +0x458
    uint8_t mipmapFlag;     // +0x45C always written 0
    uint8_t reserved45D[0x2A5C - 0x45D];
    float dpi;              // +0x2A5C BMP: header's y-pixels-per-meter * 0.0254f
    uint8_t reservedEnd[0x2A70 - 0x2A60];
};
TS3_X86_OFFSET(TextureObject, mode, 0x0C);
TS3_X86_OFFSET(TextureObject, width, 0x10);
TS3_X86_OFFSET(TextureObject, height, 0x14);
TS3_X86_OFFSET(TextureObject, bytesPerPixel, 0x18);
TS3_X86_OFFSET(TextureObject, depth, 0x1C);
TS3_X86_OFFSET(TextureObject, paletteCount, 0x24);
TS3_X86_OFFSET(TextureObject, palette, 0x28);
TS3_X86_OFFSET(TextureObject, formatKind, 0x428);
TS3_X86_OFFSET(TextureObject, compressed, 0x42C);
TS3_X86_OFFSET(TextureObject, dpiKind, 0x430);
TS3_X86_OFFSET(TextureObject, blockBytes, 0x434);
TS3_X86_OFFSET(TextureObject, alphaBits, 0x438);
TS3_X86_OFFSET(TextureObject, redBits, 0x43C);
TS3_X86_OFFSET(TextureObject, greenBits, 0x440);
TS3_X86_OFFSET(TextureObject, blueBits, 0x444);
TS3_X86_OFFSET(TextureObject, pitchX, 0x44C);
TS3_X86_OFFSET(TextureObject, pitchY, 0x454);
TS3_X86_OFFSET(TextureObject, mipmapFlag, 0x45C);
TS3_X86_OFFSET(TextureObject, dpi, 0x2A5C);
TS3_X86_SIZE(TextureObject, 0x2A70);

// The 24-byte object 0x00415370 builds next to the mip count it returns.
struct DdsProbeObject {
    uint32_t magic;       // +0x00 0x53335443 "CT3S"
    uint32_t totalSize;   // +0x04 0x18
    uint32_t mipCount;    // +0x08 the returned count
    uint32_t reserved0C;  // +0x0C
    void* stream;         // +0x10 the probed stream
    uint32_t reserved14;  // +0x14
};
TS3_X86_OFFSET(DdsProbeObject, totalSize, 0x04);
TS3_X86_OFFSET(DdsProbeObject, mipCount, 0x08);
TS3_X86_OFFSET(DdsProbeObject, stream, 0x10);
TS3_X86_SIZE(DdsProbeObject, 0x18);

// The 0x4C-byte GIF image object 0x00416410 allocates and 0x00416100 fills.
struct GifContext;

struct GifImage {
    int32_t width;            // +0x00 logical screen width (the GIF's int16, sign-extended)
    int32_t height;           // +0x04
    int32_t colorDepth;       // +0x08 ((packed >> 4) & 7) + 1
    int32_t bitsPerEntry;     // +0x0C (packed & 7) + 1
    int32_t backgroundIndex;  // +0x10 the packed byte 2
    uint8_t pad14[0x3C - 0x14];
    uint8_t* globalPalette;   // +0x3C 3 * 2^(bitsPerEntry) bytes, null when none
    int32_t flags40;          // +0x40 written 0 by 0x00416410
    int32_t flags44;          // +0x44 never written (alloc garbage)
    GifContext* context;      // +0x48 the 0x6170-byte decoder context
};
TS3_X86_OFFSET(GifImage, height, 0x04);
TS3_X86_OFFSET(GifImage, colorDepth, 0x08);
TS3_X86_OFFSET(GifImage, bitsPerEntry, 0x0C);
TS3_X86_OFFSET(GifImage, backgroundIndex, 0x10);
TS3_X86_OFFSET(GifImage, globalPalette, 0x3C);
TS3_X86_OFFSET(GifImage, flags40, 0x40);
TS3_X86_OFFSET(GifImage, context, 0x48);
TS3_X86_SIZE(GifImage, 0x4C);

// The 0x6170-byte decoder context 0x00416410 allocates next to the GifImage; this
// unit only writes the first 0x70 bytes, the rest is alloc garbage.
struct GifContext {
    int32_t field00;        // +0x00 written 0
    int32_t field04;        // +0x04 written 0
    uint8_t pad08[0x38 - 0x08];
    io::IStream* stream;    // +0x38 the GIF's stream
    int32_t field3C;        // +0x3C written 0
    int32_t bitMasks[12];   // +0x40..+0x6F 1,3,7,...,0xFFF (code-size bit masks)
    uint8_t tail[0x6170 - 0x70];
};
TS3_X86_OFFSET(GifContext, stream, 0x38);
TS3_X86_OFFSET(GifContext, field3C, 0x3C);
TS3_X86_OFFSET(GifContext, bitMasks, 0x40);
TS3_X86_SIZE(GifContext, 0x6170);

// ---------------------------------------------------------------------------
// Shared helper
// ---------------------------------------------------------------------------

// 0x004461C0 (ENTRY, leaf, 25 call sites; __stdcall the original instruction sequence): returns 0 when the
// count is negative, else IStream::Read (vtable slot 12) with (buffer, count).
int32_t __stdcall StreamRead(io::IStream* stream, void* buffer, int32_t count);

// ---------------------------------------------------------------------------
// Texture header readers
// ---------------------------------------------------------------------------

// 0x00414370 (fastcall, argument in ECX): shifts out the zero bits, then counts the
// consecutive 1-bits from the lowest set bit upwards; 0 for a zero argument.
int __fastcall CountLowBitRun(uint32_t value);

// 0x00414C00 (argument in EAX): maps a channel-mask bitmask to the channel layout
// index: 0x400 -> 0, 0x800 -> 1, 0x1000 -> 2, 0x2000 -> 3, 0x4000 -> 4, else
// 0x8000 -> 5, otherwise 0 (checked in that order, first match wins).
int MaskToFormat(uint32_t mask);

// 0x00414530 (ENTRY, vtable; __stdcall `ret 8`): reads a BMP header (12-, 40- or
// 108-byte BITMAPINFO header, plus the 12 mask bytes of a BI_BITFIELDS header) and
// builds the TextureObject. Reads no pixel data. Returns null when a header or
// palette read fails; frees the object when the last read failed after the
// allocation. EA quirks kept: header sizes > 128 overflow the original's 0x8A-byte
// stack buffer (untested, not exercised), bpp 0 is treated as 8, a palette colour
// count of 0 skips the palette entirely, and a 4-byte-entry palette with all alpha
// bytes zero gets every alpha forced to 0xFF.
TextureObject* __stdcall BmpReadTextureHeader(TextureSource* source, int32_t mip);

// 0x004154D0 (ENTRY, vtable; __stdcall `ret 8`): reads a 0x70-byte DDS-style header
// (magic "CT3S") and builds the TextureObject; `mip` shifts the dimensions when the
// flags ask for it. Returns null when a read fails or the allocation does. EA quirk
// kept: when the header's flags bit 0 is set but the +0x6C extra dword has neither
// the FOURCC-range bits nor bit 3, the computed pitch pair (+0x44C/+0x454) is left
// at 0 while the format fields still describe the texture.
TextureObject* __stdcall DdsReadTextureHeader(TextureSource* source, int32_t mip);

// 0x00415370 (ENTRY, vtable; __stdcall the original instruction sequence; the last two arguments are
// pushed by callers and never read): reads the same 0x70-byte header, allocates a
// DdsProbeObject, stores the stream and the derived mip count in it, writes it to
// *outObject and returns the count. EA quirk kept: when the flags bit 0 is set and
// the +0x18 dword is not a power of two, the count is the largest power of two at
// most that value, capped at 1024 - and when that path is taken (big-flag header,
// no 0x20000) a resulting 0 is NOT clamped to 1, unlike every other path.
int32_t __stdcall DdsProbeMipCount(DdsProbeObject** outObject, io::IStream* stream, uint32_t unusedA,
                                   uint32_t unusedB);

// ---------------------------------------------------------------------------
// GIF loader
// ---------------------------------------------------------------------------

// 0x00416100 (argument in ESI): reads the logical screen descriptor (width, height,
// packed byte, background index) and, when the packed byte's sign bit is set, the
// global colour table, into image. Returns 1 on success, 0 when a read fails (or
// when the colour-table allocation fails: the original then still returns 1 and the
// loop writes through the null pointer - the game never runs out of memory here).
int32_t GifReadScreenDescriptor(GifImage* image);

// 0x00416410 (ENTRY; stream in EAX, plain `ret`): allocates the GifImage (0x4C) and
// GifContext (0x6170) pair, validates the "GIF89a"/"GIF87a" magic (6 bytes read,
// compared as 7 with the terminator) and fills the screen descriptor through
// 0x00416100. Returns the GifImage, or null - freeing both allocations - when any
// step fails.
GifImage* OpenGifFromStream(io::IStream* stream);

#if defined(_M_IX86)
// The original takes its only argument in EAX (the single caller 0x00416A30 pushes
// nothing) and returns with a plain cdecl ret; the harness hook targets this shim.
GifImage* __cdecl GifOpenFromStreamEax(void);
#endif

}  // namespace ts3::ui
