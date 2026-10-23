// Cluster unit clu_414370_UI (subsystem UI): texture header readers (BMP 0x00414530,
// DDS/CT3S 0x004154D0 + probe 0x00415370), the GIF loader pair (0x00416410 +
// 0x00416100) and the shared stream-read helper 0x004461C0, rewritten from
// TS3W.exe. Every function below is instruction-for-instruction faithful to its
// original, including the stack-buffer zeroing the originals do before each read
// (a short read leaves the rest of the buffer at 0, which the parse then sees).
// None of the readers touch pixel data - only headers and palettes.
//
// Dependencies kept as game calls (this unit does not own them):
//  - 0x00446180 / 0x004461A0 (units clu_4143a0_UI / clu_415e10_UI): the game heap's
//    allocation and free, stdcall; reached through the file-local CallGameStdcall below
//    (ts3/layout.h's bridge carries only cdecl/thiscall/vtable shapes).
//  - 0x00446220 (clu_4143a0_UI): seek to absolute position 0, stdcall ret 0xC; its
//    third argument is ignored and the position type is hard-wired to Begin.
//  - IStream::Read itself is vtable slot 12, wrapped by the rewritten 0x004461C0.
//
// EA quirks kept on purpose because they affect outputs:
//  - 0x00414530: a BITMAPINFO header size > 128 overflows the original's 0x8A-byte
//    stack buffer (the parse trusts the file; not exercised by the tests).
//  - 0x00414530: bpp 0 is treated as 8, a palette colour count of 0 skips the palette
//    entirely, and counts above 256 clamp to 256; a 4-byte-entry palette whose alpha
//    bytes are all zero gets every alpha forced to 0xFF (alphaBits stays 0 then).
//  - 0x00415370: the mip count derived from the +0x18 dword is the largest power of
//    two at most that value, capped at 1024; when the count comes from that path
//    (big-flag header, no 0x20000) a resulting 0 is NOT clamped to 1, unlike every
//    other path. Its read of the 4 extra bytes ignores the read's result, so a
//    truncated stream leaves those 4 bytes as whatever was on the stack (the tests
//    keep the 4 bytes present, so both sides see defined data).
//  - 0x004154D0: when flags bit 0 is set but the extra dword matches neither the
//    FOURCC range nor bit 0x200, the pitch fields (+0x44C/+0x454) stay 0 while the
//    rest of the object still describes a valid format.
//  - 0x00416100: a failed global-colour-table allocation still returns 1 and the
//    entry loop writes through the null pointer (the game never runs out of memory
//    for a 768-byte table).
#include "ts3/ui/clu_414370_UI.h"

#include <cstring>

namespace ts3::ui {
namespace {

// ---------------------------------------------------------------------------
// Game-side helpers (other units, not rewritten here)
// ---------------------------------------------------------------------------

// Calls a (__stdcall) function of the game at its TS3W.exe address, for the heap
// alloc/free and the seek that this unit borrows from clu_4143a0_UI / clu_415e10_UI.
// The shared bridge in ts3/layout.h only carries cdecl (CallGameFunction), thiscall
// (CallGameMethod) and vtable (CallGameVirtual) shapes - no stdcall - and that header is
// shared/off-limits to a single unit, so this unit keeps the same guarded, fail-closed
// shape locally. In x64 builds there is no game code to call.
template <class R, unsigned kAddress, class... Args>
R CallGameStdcall(Args... args)
{
#if defined(_M_IX86)
    using Function = R(__stdcall*)(Args...);
    return reinterpret_cast<Function>(static_cast<uintptr_t>(kAddress))(args...);
#else
    ((void)args, ...);
    AbortNotRewritten("a game stdcall function");
    return R{};
#endif
}

// 0x00446180: tag the allocation and hand it to the game heap (stdcall ret 4).
void* GameAlloc(uint32_t size)
{
    return CallGameStdcall<void*, 0x00446180>(size);
}

// 0x004461A0: free through the game heap (stdcall ret 4; the return byte is ignored
// by every caller).
void GameFree(void* block)
{
    (void)CallGameStdcall<unsigned char, 0x004461A0>(block);
}

// 0x00446220: seek to absolute 0 (stdcall ret 0xC; the third argument is ignored and
// the type is hard-wired to Begin inside the original).
void SeekToStart(io::IStream* stream)
{
    (void)CallGameStdcall<bool, 0x00446220>(stream, 0, 0);
}

// Little-endian field loads out of the header stack buffers (x86 order, as the
// original's movzx/mov instructions do).
uint16_t Load16(const uint8_t* p)
{
    uint16_t v;
    std::memcpy(&v, p, 2);
    return v;
}

uint32_t Load32(const uint8_t* p)
{
    uint32_t v;
    std::memcpy(&v, p, 4);
    return v;
}

// 0x00415370's power-of-two rounding: unchanged when x already is a power of two
// (0 and 1 included), otherwise the largest power of two <= x/2, capped at 1024.
int32_t Power2Round(int32_t x)
{
    if (((x - 1) & x) == 0)
        return x;
    const int32_t half = x >> 1;
    int32_t i = 2;
    int32_t last = 2;
    for (;;) {
        last = i;
        if (half < i)
            break;
        i += i;
        if (i > 1024)
            break;
    }
    return last;
}

}  // namespace

// ---------------------------------------------------------------------------
// 0x004461C0
// ---------------------------------------------------------------------------

int32_t __stdcall StreamRead(io::IStream* stream, void* buffer, int32_t count)
{
    if (count < 0)
        return 0;
    return static_cast<int32_t>(CallGameVirtual<io::size_type, 12>(stream, buffer, static_cast<io::size_type>(count)));
}

// ---------------------------------------------------------------------------
// Leaves
// ---------------------------------------------------------------------------

// 0x00414370: skip the leading zero bits, then count the run of 1-bits from the
// lowest set bit upwards (the shift is logical, so the count is unsigned).
int __fastcall CountLowBitRun(uint32_t value)
{
    if (value == 0)
        return 0;
    while ((value & 1) == 0)
        value >>= 1;
    int count = 0;
    do {
        value >>= 1;
        ++count;
    } while (value & 1);
    return count;
}

// 0x00414C00: first matching bit wins.
int MaskToFormat(uint32_t mask)
{
    if (mask & 0x400)
        return 0;
    if (mask & 0x800)
        return 1;
    if (mask & 0x1000)
        return 2;
    if (mask & 0x2000)
        return 3;
    if (mask & 0x4000)
        return 4;
    return (mask & 0x8000) ? 5 : 0;
}

// ---------------------------------------------------------------------------
// 0x00414530: BMP header reader
// ---------------------------------------------------------------------------

TextureObject* __stdcall BmpReadTextureHeader(TextureSource* source, int32_t /*mip*/)
{
    io::IStream* stream = source->stream;
    SeekToStart(stream);

    uint8_t header[0x8A];  // the original's stack buffer: 14-byte file header + up to
                           // a 124-byte info header
    std::memset(header, 0, sizeof header);
    if (!StreamRead(stream, header, 0x12))
        return nullptr;

    const uint32_t headerSize = Load32(header + 0x0E);
    int32_t lastRead = StreamRead(stream, header + 0x12, static_cast<int32_t>(headerSize - 4));

    int32_t kind = 0;      // v2: 0 plain, 1 core (12-byte), 2 V4 (108-byte), 3 bitfields
    int32_t height = 0;    // v4
    int32_t width = 0;     // v25
    int32_t bpp = 0;       // v21
    uint32_t paletteSize = 0;  // v5: the colour table's entry count
    uint32_t compression = 0;  // v23
    int32_t yPixelsPerMeter = 0;  // v27
    uint32_t maskR = 0, maskG = 0, maskB = 0, maskA = 0;  // v24/v26/v28/v30

    if (headerSize == 12) {
        width = Load16(header + 18);
        height = Load16(header + 20);
        bpp = Load16(header + 24);
        kind = 1;
        paletteSize = 1u << (static_cast<uint32_t>(bpp) & 31);
        compression = 0;
    } else {
        width = static_cast<int32_t>(Load32(header + 18));
        height = static_cast<int32_t>(Load32(header + 22));
        bpp = Load16(header + 28);
        compression = Load32(header + 30);
        paletteSize = Load32(header + 46);
        yPixelsPerMeter = static_cast<int32_t>(Load32(header + 38));
        if (compression == 3 && headerSize == 40) {
            kind = 3;
            lastRead = StreamRead(stream, header + 54, 12);  // the three mask dwords
        } else if (headerSize == 108) {
            kind = 2;
        }
        // Loaded either way; for the paths without a mask source the bytes are the
        // buffer's zeros, exactly as the original reads them.
        maskR = Load32(header + 54);
        maskG = Load32(header + 58);
        maskB = Load32(header + 62);
        maskA = Load32(header + 66);
    }

    if (height < 0)
        height = -height;
    if (!lastRead)
        return nullptr;

    TextureObject* obj = static_cast<TextureObject*>(GameAlloc(0x2A70));
    if (!obj)
        return nullptr;
    std::memset(obj, 0, 0x2A70);

    obj->magic = 0x2E424D50;  // "PMB."
    obj->totalSize = 0x2A70;
    obj->version = 345;
    obj->mode = 0;
    obj->width = width;
    obj->height = height;
    obj->formatKind = kind;
    obj->compressed = (compression == 1 || compression == 2) ? 1 : 0;
    obj->dpi = static_cast<float>(yPixelsPerMeter) * 0.0254f;
    obj->dpiKind = 100;
    obj->alphaBits = 0;
    obj->redBits = 8;
    obj->greenBits = 8;
    obj->blueBits = 8;
    obj->blockBytes = 0;
    obj->pitchX = 0;
    obj->pitchY = 0;
    obj->reserved448 = 0;
    obj->reserved450 = 0;
    obj->reserved458 = 0;
    obj->paletteCount = 0;
    obj->mipmapFlag = 0;

    if (bpp > 8) {
        obj->bytesPerPixel = 32;
        obj->depth = bpp;
        obj->paletteCount = 0;
        if (kind >= 2) {
            obj->alphaBits = (bpp == 32 && kind == 2) ? CountLowBitRun(maskA) : 0;
            obj->redBits = CountLowBitRun(maskR);
            obj->greenBits = CountLowBitRun(maskG);
            obj->blueBits = CountLowBitRun(maskB);
        } else if (bpp == 16) {
            obj->depth = 15;
            obj->redBits = 5;
            obj->greenBits = 5;
            obj->blueBits = 5;
        }
    } else {
        if (!bpp) {
            bpp = 8;  // EA: a zero bit count means eight bits per pixel
        }
        if (!paletteSize)
            paletteSize = 1u << (static_cast<uint32_t>(bpp) & 31);
        if (paletteSize > 256)
            paletteSize = 256;
        if (!paletteSize) {
            obj->bytesPerPixel = 8;
            obj->depth = bpp;
            obj->paletteCount = 0;
        } else if (kind == 1) {
            // Core-header palette: 3 BGR bytes per entry, alpha forced to 0xFF.
            uint8_t entries[3 * 256];
            std::memset(entries, 0, sizeof entries);
            lastRead = StreamRead(stream, entries, 3 * static_cast<int32_t>(paletteSize));
            for (uint32_t i = 0; i < paletteSize; ++i) {
                obj->palette[4 * i + 0] = entries[3 * i + 0];
                obj->palette[4 * i + 1] = entries[3 * i + 1];
                obj->palette[4 * i + 2] = entries[3 * i + 2];
                obj->palette[4 * i + 3] = 0xFF;
            }
            obj->bytesPerPixel = 8;
            obj->depth = bpp;
            obj->paletteCount = static_cast<int32_t>(paletteSize);
        } else {
            // Info-header palette: 4 BGRA bytes per entry, kept verbatim; the alpha
            // scan decides +0x438, and an all-zero alpha column gets filled with 0xFF.
            uint8_t entries[4 * 256];
            std::memset(entries, 0, sizeof entries);
            lastRead = StreamRead(stream, entries, 4 * static_cast<int32_t>(paletteSize));
            bool anyAlphaNonZero = false;
            bool allAlphaFF = true;
            for (uint32_t i = 0; i < paletteSize; ++i) {
                std::memcpy(obj->palette + 4 * i, entries + 4 * i, 4);
                const uint8_t a = entries[4 * i + 3];
                if (a)
                    anyAlphaNonZero = true;
                if (a != 0xFF)
                    allAlphaFF = false;
            }
            if (anyAlphaNonZero)
                obj->alphaBits = allAlphaFF ? 0 : 8;
            else {
                for (uint32_t i = 0; i < paletteSize; ++i)
                    obj->palette[4 * i + 3] = 0xFF;
                obj->alphaBits = 0;
            }
            obj->bytesPerPixel = 8;
            obj->depth = bpp;
            obj->paletteCount = static_cast<int32_t>(paletteSize);
        }
    }

    if (!lastRead) {
        GameFree(obj);
        return nullptr;
    }
    return obj;
}

// ---------------------------------------------------------------------------
// 0x004154D0: DDS/CT3S header reader
// ---------------------------------------------------------------------------

TextureObject* __stdcall DdsReadTextureHeader(TextureSource* source, int32_t mip)
{
    io::IStream* stream = source->stream;
    SeekToStart(stream);

    uint8_t header[0x70];
    std::memset(header, 0, sizeof header);
    int32_t readResult = StreamRead(stream, header, 0x70);

    const uint32_t flags = Load32(header + 8);
    const bool hasExtra = (flags & 1) != 0;
    const bool bigHeader = (flags & 0x800000) != 0;
    uint32_t extra = 0;  // v25: the dword after the 0x70-byte header
    if (hasExtra)
        readResult = StreamRead(stream, &extra, 4);
    if (!readResult)
        return nullptr;

    int32_t width, height;  // v19 / v16
    if (flags & 0x20000) {
        // The original shifts the raw dword (x86 shr masks the count to 5 bits) and
        // clamps only when the *unsigned* shifted value is 0 (`jb`/`cmp 1`), so a
        // -1 dimension shifted by mip 0 stays -1 rather than clamping to 1.
        const uint32_t shift = static_cast<uint32_t>(mip) & 31;
        uint32_t w = Load32(header + 0x10) >> shift;
        if (w < 1)
            w = 1;
        uint32_t h = Load32(header + 0x0C) >> shift;
        if (h < 1)
            h = 1;
        width = static_cast<int32_t>(w);
        height = static_cast<int32_t>(h);
    } else {
        height = static_cast<int32_t>(Load32(header + 0x0C));
        width = static_cast<int32_t>(Load32(header + 0x10));
    }

    int32_t alphaBits = 0, redBits = 0, greenBits = 0, blueBits = 0;  // v14 / v3 / v2 / v15
    int32_t depth = 0;                                                // v17
    int32_t fourCC = 0;                                               // v7
    if (!(flags & 0x1000)) {
        // Not a FOURCC/format-flag header: the default 565-with-alpha description.
        fourCC = 1;
        depth = 16;
        alphaBits = 1;
        redBits = 5;
        greenBits = 6;
        blueBits = 5;
    } else {
        fourCC = (static_cast<int32_t>(Load32(header + 0x50)) >> 2) & 1;
        depth = static_cast<int32_t>(Load32(header + 0x58));
        if (fourCC) {
            // The original assembles the fourCC bytes big-endian.
            const uint32_t code = (static_cast<uint32_t>(header[0x54]) << 24) |
                                  (static_cast<uint32_t>(header[0x55]) << 16) |
                                  (static_cast<uint32_t>(header[0x56]) << 8) |
                                  static_cast<uint32_t>(header[0x57]);
            depth = 16;
            redBits = 5;
            greenBits = 6;
            blueBits = 5;
            switch (code) {
                case 0x44585431:  // "DXT1"
                    alphaBits = 1;
                    break;
                case 0x44585432:  // "DXT2"
                case 0x44585433:  // "DXT3"
                    alphaBits = 4;
                    break;
                case 0x44585434:  // "DXT4"
                case 0x44585435:  // "DXT5"
                    alphaBits = 3;
                    break;
                default:
                    alphaBits = 0;  // unknown fourCC: everything stays at 0
                    depth = 16;
                    redBits = 5;
                    greenBits = 6;
                    blueBits = 5;
                    break;
            }
        } else if (depth == 16) {
            const uint32_t maskR = Load32(header + 0x5C);
            const uint32_t maskG = Load32(header + 0x60);
            const uint32_t maskB = Load32(header + 0x64);
            if (maskR == 0xF000) {
                if (maskG == 0xF0 && maskB == 0xF) {
                    redBits = 4;
                    greenBits = 4;
                    blueBits = 4;
                    alphaBits = 4;
                }
            } else if (maskR == 0x7C00) {
                if (maskG == 0x3E0 && maskB == 0x1F) {
                    redBits = 5;
                    greenBits = 5;
                    blueBits = 5;
                    alphaBits = 1;
                }
            } else if (maskR == 0xF800) {
                if (maskG == 0x7E0 && maskB == 0x1F) {
                    // 565 without alpha: same widths as the default, alpha 0.
                    redBits = 5;
                    greenBits = 6;
                    blueBits = 5;
                    alphaBits = 0;
                }
            }
        } else if (depth == 24) {
            redBits = 8;
            greenBits = 8;
            blueBits = 8;
            alphaBits = 0;
        } else if (depth == 32) {
            redBits = 8;
            greenBits = 8;
            blueBits = 8;
            alphaBits = 8;
        }
    }

    int32_t pitchX = 0, pitchY = 0;  // v21 / v22
    int32_t extraFormat = 0;         // v18
    uint32_t extraBit3 = 0;          // v20: +0x6C & 8
    if (hasExtra) {
        extraFormat = ((extra & 0xFC00) != 0 && (extra & 0x200) != 0) ? 1 : 0;
        extraBit3 = Load32(header + 0x6C) & 8;
        if (extraFormat) {
            const uint32_t mask = Load32(header + 0x6C);
            const int32_t mult = extraBit3 ? mip : MaskToFormat(mask);
            pitchY = mult * height;  // v22
            // v21 = pitchY - trunc(height * -0.5f)
            pitchX = pitchY - static_cast<int32_t>(static_cast<float>(height) * -0.5f);
        }
    }

    TextureObject* obj = static_cast<TextureObject*>(GameAlloc(0x2A70));
    if (!obj)
        return nullptr;
    std::memset(obj, 0, 0x2A70);

    obj->magic = 0x53335443;  // "CT3S"
    obj->totalSize = 0x2A70;
    obj->version = 345;
    obj->mode = mip;
    obj->width = width;
    obj->height = height;
    obj->bytesPerPixel = 32;
    obj->depth = depth;
    obj->paletteCount = 0;
    if (extraFormat)
        obj->formatKind = extraBit3 ? 0x40000 : 42;
    else
        obj->formatKind = bigHeader ? 0x100 : 0;
    obj->compressed = fourCC ? 3 : 0;
    obj->dpiKind = 90;
    obj->blockBytes = 8 * ((width + 3) / 4) * ((height + 3) / 4);
    obj->alphaBits = alphaBits;
    obj->redBits = redBits;
    obj->greenBits = greenBits;
    obj->blueBits = blueBits;
    obj->reserved448 = 0;
    obj->pitchX = pitchX;
    obj->reserved450 = 0;
    obj->pitchY = pitchY;
    obj->reserved458 = 0;
    obj->mipmapFlag = 0;
    return obj;
}

// ---------------------------------------------------------------------------
// 0x00415370: DDS mip-count probe
// ---------------------------------------------------------------------------

int32_t __stdcall DdsProbeMipCount(DdsProbeObject** outObject, io::IStream* stream, uint32_t /*unusedA*/,
                                   uint32_t /*unusedB*/)
{
    SeekToStart(stream);

    uint8_t header[0x70];
    std::memset(header, 0, sizeof header);
    int32_t result = StreamRead(stream, header, 0x70);
    if (!result)
        return 0;

    DdsProbeObject* obj = static_cast<DdsProbeObject*>(GameAlloc(0x18));
    if (!obj)
        return 0;
    obj->magic = 0x53335443;
    obj->totalSize = 0x18;
    obj->mipCount = 0;
    obj->reserved0C = 0;
    obj->stream = nullptr;
    obj->reserved14 = 0;

    const uint32_t flags = Load32(header + 8);
    const bool bigHeader = (flags & 0x800000) != 0;
    const uint32_t mipBase = Load32(header + 0x18);  // v14[6]
    const int32_t depth = static_cast<int32_t>(Load32(header + 0x1C));  // v14[7]

    if (flags & 1) {
        const uint8_t low = header[0x6C];  // v8: only the low byte is kept
        uint32_t extra = 0;
        (void)StreamRead(stream, &extra, 4);  // EA: the result is ignored
        if ((extra & 0xFC00) != 0 && (extra & 0x200) != 0 && (low & 8) != 0) {
            result = 6;
        } else if (!(flags & 0x20000)) {
            if (bigHeader) {
                // EA: this path skips the "0 becomes 1" clamp below.
                result = Power2Round(static_cast<int32_t>(mipBase));
                goto store;
            }
            result = 1;
        } else {
            result = depth;
            if (!result)
                result = 1;
        }
    } else if (bigHeader) {
        result = Power2Round(static_cast<int32_t>(mipBase));
        if (!result)
            result = 1;
    } else {
        result = depth;
        if (!result)
            result = 1;
    }

store:
    obj->stream = stream;
    obj->mipCount = static_cast<uint32_t>(result);
    *outObject = obj;
    return result;
}

// ---------------------------------------------------------------------------
// 0x00416100 / 0x00416410: GIF loader
// ---------------------------------------------------------------------------

int32_t GifReadScreenDescriptor(GifImage* image)
{
    GifContext* ctx = image->context;
    io::IStream* stream = ctx->stream;

    int16_t word = 0;
    if (!StreamRead(stream, &word, 2))
        return 0;
    image->width = word;  // the original sign-extends the 16-bit width
    if (!StreamRead(stream, &word, 2))
        return 0;
    image->height = word;

    uint8_t packed[3] = {};
    if (!StreamRead(stream, packed, 3))
        return 0;
    image->colorDepth = ((packed[0] >> 4) & 7) + 1;
    const int32_t bitsPerEntry = (packed[0] & 7) + 1;
    image->bitsPerEntry = bitsPerEntry;
    image->backgroundIndex = packed[1];
    if ((packed[0] & 0x80) == 0)
        return 1;  // sign bit clear: no global colour table

    const int32_t count = 1 << (static_cast<uint32_t>(bitsPerEntry) & 31);
    image->globalPalette = static_cast<uint8_t*>(GameAlloc(static_cast<uint32_t>(3 * count)));
    if (count <= 0)
        return 1;
    uint8_t entry[3] = {};
    for (int32_t i = 0; i < count; ++i) {
        if (!StreamRead(stream, entry, 3))
            return 0;
        image->globalPalette[3 * i + 0] = entry[0];
        image->globalPalette[3 * i + 1] = entry[1];
        image->globalPalette[3 * i + 2] = entry[2];
    }
    return 1;
}

GifImage* OpenGifFromStream(io::IStream* stream)
{
    GifImage* image = static_cast<GifImage*>(GameAlloc(0x4C));
    if (!image)
        return nullptr;
    GifContext* ctx = static_cast<GifContext*>(GameAlloc(0x6170));
    if (!ctx) {
        GameFree(image);
        return nullptr;
    }

    image->context = ctx;
    image->flags40 = 0;
    image->globalPalette = nullptr;
    ctx->field04 = 0;
    ctx->stream = stream;
    ctx->field00 = 0;
    ctx->field3C = 0;
    static const int32_t kMasks[12] = {1, 3, 7, 0xF, 0x1F, 0x3F, 0x7F, 0xFF, 0x1FF, 0x3FF, 0x7FF, 0xFFF};
    for (int i = 0; i < 12; ++i)
        ctx->bitMasks[i] = kMasks[i];

    char magic[8] = {};
    const bool magicOk = StreamRead(stream, magic, 6) &&
                         (std::memcmp(magic, "GIF89a", 7) == 0 || std::memcmp(magic, "GIF87a", 7) == 0);
    if (!magicOk || !GifReadScreenDescriptor(image)) {
        GameFree(ctx);
        GameFree(image);
        return nullptr;
    }
    return image;
}

#if defined(_M_IX86)
// The original takes its only argument in EAX (the single caller 0x00416A30 pushes
// nothing) and returns with a plain cdecl ret. MSVC asm can only call through a
// pointer, hence kGifOpen.
static GifImage* (__cdecl* const kGifOpen)(io::IStream*) = &OpenGifFromStream;

__declspec(naked) GifImage* __cdecl GifOpenFromStreamEax(void)
{
    __asm {
        push eax                 // the stream, exactly the original's argument
        call dword ptr [kGifOpen]
        add esp, 4
        ret
    }
}
#endif

}  // namespace ts3::ui
