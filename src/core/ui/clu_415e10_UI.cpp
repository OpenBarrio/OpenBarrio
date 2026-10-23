// clu_415e10_UI: portable C++ for the fifteen GIF-decoder functions listed in
// the project notes. The weird register conventions of the originals (object in ESI
// for 0x00415E10/0x00416200, in EAX for 0x00415E50, in ESI for 0x00415EF0, in EDI for
// 0x004167B0) are internal to this unit; the exported entry points keep the exact stack
// ABIs. Functions of other units are reached through the bridges (0x00446180 allocator,
// 0x00446220 seek, 0x00446240 GetSize, and the clu_415e50_x LZW helpers through
// x86-only naked trampolines). Already rewritten dependencies (0x00416410 and
// 0x004461C0 from clu_414370_UI, 0x004E3F70 from clu_4e3f70_Scratch) are called
// directly.
//
// EA quirks kept because they show up in outputs:
//  - 0x00416A30 reads its block kind and last-extension locals before they are written
//    (uninitialized in the original); the rewrite starts them at 0.
//  - 0x00416EA0 copies the palette from +0x3C even when a local palette at +0x40
//    exists, and 0x00416E20 frees the parsed object before returning its frame count.
//  - 0x00417720's grey ramp steps by 255, not 256.
//  - 0x00418AB0's 15/16-bit palette path reads u16 values at odd byte offsets and uses
//    the original's magic divide-by-31 multiply.
//  - 0x00416200 leaks the freshly allocated local palette when a palette read fails.
#include "ts3/ui/clu_415e10_UI.h"

#include <cstring>

namespace ts3::ui {
namespace {

// ---------------------------------------------------------------------------
// Bridges into game code of other units (see the header comment).
// ---------------------------------------------------------------------------

// The shared layout.h bridge carries only cdecl/thiscall/vtable; the game heap and the
// stream helpers are __stdcall (ret 4 / ret 0xC), so this unit keeps a guarded local
// helper of the same shape clu_414370_UI uses.
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

// 0x00446180 (clu_4143a0_UI): the game heap's allocation.
void* GameAlloc(uint32_t size)
{
    return CallGameStdcall<void*, 0x00446180>(size);
}

// 0x00446220 (clu_4143a0_UI): seek; returns the low byte of the underlying result.
uint8_t SeekStream446220(io::IStream* stream, int32_t position)
{
    return static_cast<uint8_t>(
        CallGameStdcall<int32_t, 0x00446220>(stream, position, position >> 31));
}

// 0x00446240 (cls_446240_x): GetSize, low 32 bits (the original also returns zero in
// edx, which no caller of this unit reads).
uint32_t StreamSize446240(io::IStream* stream)
{
    return CallGameStdcall<uint32_t, 0x00446240>(stream);
}

#if defined(_M_IX86)
// The clu_415e50_x LZW helpers take `this` in a register (EAX/ESI/EDI), so they need
// naked trampolines; the calls below reproduce the original register setup exactly.
uint8_t* (__cdecl* const kFn415DB0)(void*, void**) =
    reinterpret_cast<uint8_t* (__cdecl*)(void*, void**)>(0x00415DB0u);
int32_t (__cdecl* const kFn415E50)(void*) =
    reinterpret_cast<int32_t (__cdecl*)(void*)>(0x00415E50u);
int32_t (__cdecl* const kFn415EF0)(void*) =
    reinterpret_cast<int32_t (__cdecl*)(void*)>(0x00415EF0u);
int32_t (__cdecl* const kFn4167B0)(void*, int32_t, void*) =
    reinterpret_cast<int32_t (__cdecl*)(void*, int32_t, void*)>(0x004167B0u);

__declspec(naked) uint8_t* __cdecl Call415DB0(void* self, void** out)
{
    __asm {
        push edi
        mov eax, dword ptr [esp + 8]
        mov edi, dword ptr [esp + 0xC]
        call dword ptr [kFn415DB0]
        pop edi
        ret
    }
}

__declspec(naked) int32_t __cdecl Call415E50(void* self)
{
    __asm {
        mov eax, dword ptr [esp + 4]
        call dword ptr [kFn415E50]
        ret
    }
}

__declspec(naked) int32_t __cdecl Call415EF0(void* stream)
{
    __asm {
        push esi
        mov esi, dword ptr [esp + 8]
        call dword ptr [kFn415EF0]
        pop esi
        ret
    }
}

__declspec(naked) int32_t __cdecl Call4167B0(void* self, int32_t width, void* row)
{
    __asm {
        push edi
        mov edi, dword ptr [esp + 8]
        mov eax, dword ptr [esp + 0xC]
        push dword ptr [esp + 0x10]
        call dword ptr [kFn4167B0]
        add esp, 4
        pop edi
        ret
    }
}
#else
uint8_t* Call415DB0(void* self, void** out)
{
    (void)self;
    (void)out;
    AbortNotRewritten("0x00415DB0");
    return nullptr;
}
int32_t Call415E50(void* self)
{
    (void)self;
    AbortNotRewritten("0x00415E50");
    return 0;
}
int32_t Call415EF0(void* stream)
{
    (void)stream;
    AbortNotRewritten("0x00415EF0");
    return 0;
}
int32_t Call4167B0(void* self, int32_t width, void* row)
{
    (void)self;
    (void)width;
    (void)row;
    AbortNotRewritten("0x004167B0");
    return 0;
}
#endif

// The game's cleanup list and its registration flag.
#if defined(_M_IX86)
constexpr uintptr_t kCleanupListHead = 0x011B02AC;
constexpr uintptr_t kCleanupRegistered = 0x011B02A2;
constexpr uintptr_t kCleanupNode = 0x01146140;
#endif

// Little-endian 16-bit load out of a byte buffer (the originals' `movzx` pair).
uint16_t Load16(const uint8_t* p)
{
    uint16_t value;
    std::memcpy(&value, p, 2);
    return value;
}

// The original's `(unsigned __int64)x * 138547333 >> 32` divide-by-31 helper used by the
// 15/16-bit palette conversion.
uint32_t MagicDiv31(uint32_t x)
{
    return static_cast<uint32_t>((static_cast<uint64_t>(x) * 138547333ull) >> 32);
}

}  // namespace

// ---------------------------------------------------------------------------
// 0x00415E10
// ---------------------------------------------------------------------------

int32_t FreeGifFile415E10(GifFile* file)
{
    if (file == nullptr)
        return 0;
    uint8_t* const localPalette = file->localPalette;
    GifContext* const context = file->context;
    if (localPalette != nullptr)
        Free4461A0(localPalette);
    if (file->globalPalette != nullptr)
        Free4461A0(file->globalPalette);
    if (context != nullptr)
        Free4461A0(context);
    Free4461A0(file);
    return 1;
}

// ---------------------------------------------------------------------------
// 0x00416200
// ---------------------------------------------------------------------------

int32_t ReadImageDescriptor416200(GifFile* file)
{
    GifContextFile* const context = reinterpret_cast<GifContextFile*>(file->context);
    io::IStream* const stream = context->stream;
    int16_t word = 0;
    if (!StreamRead(stream, &word, 2))
        return 0;
    file->frameLeft = word;
    if (!StreamRead(stream, &word, 2))
        return 0;
    file->frameTop = word;
    if (!StreamRead(stream, &word, 2))
        return 0;
    file->frameWidth = word;
    if (!StreamRead(stream, &word, 2))
        return 0;
    file->frameHeight = word;

    uint8_t packed = 0;
    if (!StreamRead(stream, &packed, 1))
        return 0;
    const int32_t bits = (packed & 7) + 1;
    file->localBits = bits;
    file->interlaceFlag = packed & 0x40;
    if ((packed & 0x80) != 0) {
        const int32_t count = 1 << (static_cast<uint32_t>(bits) & 31);
        if (file->localPalette != nullptr)
            Free4461A0(file->localPalette);
        file->localPalette = static_cast<uint8_t*>(GameAlloc(static_cast<uint32_t>(3 * count)));
        for (int32_t i = 0; i < count; ++i) {
            uint8_t entry[3] = {};
            if (!StreamRead(stream, entry, 3))
                return 0;
            file->localPalette[3 * i + 0] = entry[0];
            file->localPalette[3 * i + 1] = entry[1];
            file->localPalette[3 * i + 2] = entry[2];
        }
    }
    context->decodedSize = file->frameWidth * file->frameHeight;
    (void)Call415E50(file);
    return 1;
}

// ---------------------------------------------------------------------------
// 0x00416840
// ---------------------------------------------------------------------------

int32_t __cdecl BlitGifFrame416840(GifFile* file, uint8_t* dest, int32_t stride)
{
    int32_t left = file->frameLeft;
    int32_t top = file->frameTop;
    int32_t row = top;
    int32_t col = left;
    if (left < 0) {
        col = 0;
        left = 0;
    }
    if (top < 0) {
        row = 0;
        top = 0;
    }
    const int32_t frameHeight = file->frameHeight;
    if (frameHeight + top > file->height) {
        row = file->height - frameHeight;
        top = file->height - frameHeight;
    }
    const int32_t frameWidth = file->frameWidth;
    if (frameWidth + left > file->width) {
        col = file->width - frameWidth;
        left = file->width - frameWidth;
    }
    if (top < 0 || left < 0)
        return 0;

    uint8_t* palette = file->globalPalette;
    if (file->localPalette != nullptr)
        palette = file->localPalette;
    uint8_t* const buffer = static_cast<uint8_t*>(GameAlloc(static_cast<uint32_t>(frameWidth)));
    if (frameHeight <= 0) {
        Free4461A0(buffer);
        return 1;
    }

    int32_t step = 8;
    int32_t pass = 0;
    int32_t rowIndex = 0;
    for (;;) {
        std::memset(buffer, static_cast<int>(static_cast<int8_t>(file->transparentIndex)),
                    static_cast<size_t>(frameWidth));
        if (!Call4167B0(file, frameWidth, buffer)) {
            Free4461A0(buffer);
            return 0;
        }
        if (frameWidth > 0) {
            int32_t offset = 4 * col;
            for (int32_t i = 0; i < frameWidth; ++i) {
                if (static_cast<int32_t>(buffer[i]) != file->transparentIndex) {
                    uint8_t* const pixel = dest + offset + stride * row;
                    pixel[3] = 0xFF;
                    pixel[2] = palette[3 * buffer[i] + 0];
                    pixel[1] = palette[3 * buffer[i] + 1];
                    pixel[0] = palette[3 * buffer[i] + 2];
                }
                offset += 4;
            }
        }
        const int32_t height = file->frameHeight;
        if (rowIndex < height - 1) {
            if (file->interlaceFlag != 0) {
                const int32_t top0 = file->frameTop;
                row += step;
                if (row >= height + top0) {
                    if (pass != 0) {
                        if (pass == 2) {
                            pass = 1;
                            step = 2;
                            row = top0 + 1;
                        } else {
                            if (pass != 4)
                                break;
                            step = 4;
                            pass = 2;
                            row = top0 + 2;
                        }
                    } else {
                        pass = 4;
                        step = 8;
                        row = top0 + 4;
                    }
                }
            } else {
                ++row;
            }
        }
        if (++rowIndex >= height) {
            Free4461A0(buffer);
            return 1;
        }
    }
    Free4461A0(buffer);
    return 0;
}

// ---------------------------------------------------------------------------
// 0x00416A30
// ---------------------------------------------------------------------------

GifFile* __cdecl DecodeGif416A30(io::IStream* stream, int32_t maxFrame, uint8_t* dest, int32_t stride)
{
    int32_t ok = 1;
    int32_t frame = -1;
    // The original reads these out of uninitialized stack slots before the first
    // successful stream read; deterministic zero here (see the file comment).
    int32_t block = 0;
    uint8_t* subBlock = nullptr;
    int32_t label = 0;

    (void)SeekStream446220(stream, 0);
    GifFile* const file = reinterpret_cast<GifFile*>(OpenGifFromStream(stream));
    if (file == nullptr)
        return nullptr;
    file->transparentIndex = -1;
    file->disposal = 0;

    for (;;) {
        GifContextFile* const context = reinterpret_cast<GifContextFile*>(file->context);
        uint8_t byte = 0;
        if (StreamRead(context->stream, &byte, 1)) {
            if (byte == '!') {
                block = 3;
            } else if (byte == ',') {
                block = 2;
            } else {
                block = byte != ';' ? 0 : 4;
            }
        } else {
            ok = 0;
        }

        if (block != 2) {
            if (block == 3) {
                uint8_t value = 0;
                if (!StreamRead(context->stream, &value, 1)) {
                    ok = 0;
                } else {
                    label = value;
                    uint8_t* out = nullptr;
                    if (Call415DB0(file, reinterpret_cast<void**>(&out)) == nullptr)
                        ok = 0;
                    subBlock = out;
                }
                if (label == 0xF9 && frame < maxFrame) {
                    file->disposal = subBlock[1] & 0x1C;
                    file->delay = Load16(subBlock + 2);
                    if ((subBlock[1] & 1) != 0)
                        file->transparentIndex = subBlock[4];
                }
                if (subBlock != nullptr) {
                    // The original reuses the length byte's stack slot across
                    // iterations; keep the value so a short read behaves the same.
                    uint8_t length = 0;
                    for (;;) {
                        if (!StreamRead(context->stream, &length, 1))
                            ok = 0;
                        if (length == 0) {
                            subBlock = nullptr;
                            break;
                        }
                        uint8_t* const buffer = reinterpret_cast<uint8_t*>(context) + 0x70;
                        buffer[0] = length;
                        subBlock = buffer;
                        if (!StreamRead(context->stream, buffer + 1, length))
                            ok = 0;
                        if (subBlock == nullptr)
                            break;
                    }
                }
            }
        } else {
            // Image descriptor.
            if (!ReadImageDescriptor416200(file))
                ok = 0;
            uint8_t* activePalette = file->globalPalette;
            if (file->localPalette != nullptr)
                activePalette = file->localPalette;
            ++frame;
            bool skipOnly = false;
            if (dest == nullptr) {
                skipOnly = true;
            } else {
                int32_t background = file->transparentIndex;
                if (background < 0)
                    background = file->backgroundIndex;

                if (frame == 0) {
                    // The frame-0 fill prefers the global table (the blit prefers the
                    // local one); only a missing global table falls back to the local.
                    uint8_t* fillPalette = file->globalPalette;
                    if (fillPalette == nullptr)
                        fillPalette = file->localPalette;
                    if (fillPalette == nullptr) {
                        ok = 0;
                    } else if (file->height > 0) {
                        const uint8_t* const entry = fillPalette + 3 * background;
                        const uint8_t alpha =
                            background == file->transparentIndex ? 0 : 0xFF;
                        uint8_t* rowPtr = dest;
                        for (int32_t y = 0; y < file->height; ++y) {
                            uint8_t* pixel = rowPtr;
                            for (int32_t x = 0; x < file->width; ++x) {
                                pixel[0] = entry[2];
                                pixel[1] = entry[1];
                                pixel[2] = entry[0];
                                pixel[3] = alpha;
                                pixel += 4;
                            }
                            rowPtr += stride;
                        }
                    }
                }

                if (frame >= maxFrame) {
                    if (!BlitGifFrame416840(file, dest, stride))
                        goto cleanup;
                } else {
                    const int32_t disposal = file->disposal;
                    if (disposal == 0 || disposal == 4) {
                        if (!BlitGifFrame416840(file, dest, stride))
                            goto cleanup;
                    } else {
                        if (disposal == 8) {
                            if (activePalette == nullptr) {
                                ok = 0;
                                skipOnly = true;
                            } else {
                                const uint8_t* const entry = activePalette + 3 * background;
                                const uint8_t alpha =
                                    background == file->transparentIndex ? 0 : 0xFF;
                                const int32_t endY = file->frameTop + file->frameHeight;
                                if (file->frameTop < endY) {
                                    int32_t y = file->frameTop;
                                    do {
                                        const int32_t offset = stride * y;
                                        if (y >= 0 && y < file->height) {
                                            int32_t x = file->frameLeft;
                                            int32_t width = file->frameWidth;
                                            if (x < 0) {
                                                width += x;
                                                x = 0;
                                            }
                                            if (x + width > file->width)
                                                width = file->width - x;
                                            if (width > 0) {
                                                uint8_t* pixel = dest + offset + 4 * x;
                                                do {
                                                    pixel[0] = entry[2];
                                                    pixel[1] = entry[1];
                                                    pixel[2] = entry[0];
                                                    pixel[3] = alpha;
                                                    pixel += 4;
                                                } while (--width != 0);
                                            }
                                        }
                                        ++y;
                                    } while (y < file->frameTop + file->frameHeight);
                                }
                                skipOnly = true;
                            }
                        } else {
                            skipOnly = true;
                        }
                    }
                }
            }
            if (skipOnly) {
                if (!Call415EF0(stream))
                    goto cleanup;
            }
        }

        if (ok == 0)
            goto cleanup;
        if (block == 4)
            break;
        if (dest == nullptr)
            continue;
        if (frame < maxFrame)
            continue;
        break;
    }

    if (dest == nullptr)
        file->frameCount = frame + 1;
    return file;

cleanup:
    {
        GifContext* const context = file->context;
        if (file->localPalette != nullptr)
            Free4461A0(file->localPalette);
        if (file->globalPalette != nullptr)
            Free4461A0(file->globalPalette);
        if (context != nullptr)
            Free4461A0(context);
        Free4461A0(file);
    }
    return nullptr;
}

// ---------------------------------------------------------------------------
// 0x00416E20
// ---------------------------------------------------------------------------

int32_t __stdcall CreateGifWrapper416E20(GifFile** out, io::IStream* stream, uint32_t unusedA,
                                         uint32_t unusedB)
{
    (void)unusedA;
    (void)unusedB;
    uint8_t* const wrapper = static_cast<uint8_t*>(GameAlloc(0x18));
    if (wrapper == nullptr)
        return 0;
    *reinterpret_cast<uint32_t*>(wrapper + 0x14) = 0;
    *reinterpret_cast<uint32_t*>(wrapper + 0x08) = 0;
    *reinterpret_cast<uint32_t*>(wrapper + 0x0C) = 0xFFFFFFFFu;
    *reinterpret_cast<uint32_t*>(wrapper + 0x00) = 0x2E474946u;  // "FIG."
    *reinterpret_cast<uint32_t*>(wrapper + 0x04) = 0x18;
    *reinterpret_cast<io::IStream**>(wrapper + 0x10) = stream;
    *out = reinterpret_cast<GifFile*>(wrapper);

    GifFile* const file = DecodeGif416A30(stream, 0, nullptr, 0);
    if (file == nullptr)
        return 0;
    const int32_t frameCount = file->frameCount;
    *reinterpret_cast<uint32_t*>(wrapper + 0x08) = static_cast<uint32_t>(frameCount);
    // The original keeps the count in EBX and returns it after freeing the object.
    if (FreeGifFile415E10(file) == 0)
        return 0;
    return frameCount;
}

// ---------------------------------------------------------------------------
// 0x00416EA0
// ---------------------------------------------------------------------------

TextureObject* __stdcall ExtractGifFrame416EA0(TextureSource* source, uint32_t frameIndex)
{
    uint8_t* result = nullptr;
    GifFile* const file =
        DecodeGif416A30(source->stream, static_cast<int32_t>(frameIndex), nullptr, 0);
    if (file != nullptr) {
        const int32_t width = file->width;
        const int32_t height = file->height;
        const int32_t delay = file->delay;
        int32_t bits = file->bitsPerEntry;
        float scale = 15.0f;
        if (delay != 0)
            scale = 100.0f / static_cast<float>(delay);
        if (bits == 0)
            bits = 8;
        int32_t paletteCount = 1 << (static_cast<uint32_t>(bits) & 31);
        if (paletteCount > 256)
            paletteCount = 256;
        if (static_cast<int32_t>(frameIndex) < *reinterpret_cast<int32_t*>(
                reinterpret_cast<uint8_t*>(source) + 8)) {
            TextureObject* const texture = static_cast<TextureObject*>(GameAlloc(10864));
            result = reinterpret_cast<uint8_t*>(texture);
            if (texture != nullptr) {
                std::memset(texture, 0, 0x2A70);
                int32_t* const t = reinterpret_cast<int32_t*>(texture);
                t[3] = static_cast<int32_t>(frameIndex);
                t[5] = height;
                int32_t grey = 0;
                t[4] = width;
                std::strcpy(reinterpret_cast<char*>(texture), "FIG.p*");
                reinterpret_cast<uint8_t*>(texture)[7] = 0;
                t[2] = 345;
                const int32_t interlace = file->interlaceFlag != 0 ? 1 : 0;
                t[267] = 1;
                t[268] = 100;
                t[270] = 1;
                t[274] = 0;
                t[275] = 0;
                t[276] = 0;
                t[266] = interlace;
                t[271] = 8;
                t[272] = 8;
                t[273] = 8;
                t[277] = 0;
                t[278] = 0;
                reinterpret_cast<uint8_t*>(texture)[1116] = 0;
                t[6] = 32;
                t[7] = bits;
                t[9] = paletteCount;
                *reinterpret_cast<float*>(reinterpret_cast<uint8_t*>(texture) + 0x2A60) = scale;
                if (file->globalPalette != nullptr) {
                    if (paletteCount > 0) {
                        uint8_t* dst = reinterpret_cast<uint8_t*>(texture) + 42;
                        int32_t src = 0;
                        do {
                            dst[1] = 0xFF;
                            dst[0] = file->globalPalette[src + 0];
                            dst[-1] = file->globalPalette[src + 1];
                            dst[-2] = file->globalPalette[src + 2];
                            dst += 4;
                            src += 3;
                            --paletteCount;
                        } while (paletteCount != 0);
                    }
                } else {
                    uint8_t* dst = reinterpret_cast<uint8_t*>(texture) + 42;
                    do {
                        dst[0] = static_cast<uint8_t>(grey);
                        dst[-1] = static_cast<uint8_t>(grey);
                        dst[-2] = static_cast<uint8_t>(grey);
                        dst[1] = 0xFF;
                        ++grey;
                        dst += 4;
                    } while (grey < 256);
                }
            }
        }
        GifContext* const context = file->context;
        if (file->localPalette != nullptr)
            Free4461A0(file->localPalette);
        if (file->globalPalette != nullptr)
            Free4461A0(file->globalPalette);
        if (context != nullptr)
            Free4461A0(context);
        Free4461A0(file);
    }
    return reinterpret_cast<TextureObject*>(result);
}

// ---------------------------------------------------------------------------
// 0x004170A0
// ---------------------------------------------------------------------------

int32_t __stdcall DecodeGifToBuffer4170A0(TextureSource* source, const void* frameInfo,
                                          uint8_t* dest, int32_t stride)
{
    int32_t result = 0;
    const int32_t maxFrame =
        *reinterpret_cast<const int32_t*>(reinterpret_cast<const uint8_t*>(frameInfo) + 0x0C);
    GifFile* const file = DecodeGif416A30(source->stream, maxFrame, dest, stride);
    if (file != nullptr) {
        GifContext* const context = file->context;
        if (file->localPalette != nullptr)
            Free4461A0(file->localPalette);
        if (file->globalPalette != nullptr)
            Free4461A0(file->globalPalette);
        if (context != nullptr)
            Free4461A0(context);
        Free4461A0(file);
        result = 1;
    }
    return result;
}

// ---------------------------------------------------------------------------
// 0x00417170
// ---------------------------------------------------------------------------

void __cdecl RegisterGifCleanup417170()
{
#if defined(_M_IX86)
    if (*reinterpret_cast<uint8_t*>(kCleanupRegistered) == 0) {
        *reinterpret_cast<uint8_t*>(kCleanupRegistered) = 1;
        (void)PushCleanup446270(reinterpret_cast<void*>(kCleanupNode));
    }
#else
    AbortNotRewritten("the GIF cleanup list");
#endif
}

// ---------------------------------------------------------------------------
// 0x00417720
// ---------------------------------------------------------------------------

TextureObject* __stdcall BuildGnpTexture417720(void* source, uint32_t unused)
{
    (void)unused;
    void** const record = *reinterpret_cast<void***>(reinterpret_cast<uint8_t*>(source) + 0x14);
    uint8_t* const descriptor = static_cast<uint8_t*>(record[1]);
    const int32_t depth = descriptor[0x18];
    int32_t v4 = 32;
    int32_t a1 = 32;
    int32_t v21 = depth;
    int32_t v22 = depth;
    int32_t v23 = depth;

    uint8_t* const result = static_cast<uint8_t*>(GameAlloc(10864));
    if (result == nullptr)
        return nullptr;
    std::memset(result, 0, 0x2A70);
    switch (descriptor[0x19]) {
    case 0:
        a1 = depth;
        v23 = 0;
        v22 = 0;
        v21 = 0;
        v4 = 8;
        break;
    case 2:
        a1 = 3 * depth;
        v21 = 0;
        break;
    case 3:
        a1 = depth;
        v4 = 8;
        break;
    case 4:
        a1 = 2 * depth;
        v23 = 0;
        v22 = 0;
        break;
    case 6:
        a1 = 4 * depth;
        break;
    default:
        break;
    }
    int32_t* const t = reinterpret_cast<int32_t*>(result);
    t[3] = 0;
    t[0] = 0x00504E47;  // "GNP"
    t[1] = 10864;
    t[2] = 345;
    t[4] = *reinterpret_cast<int32_t*>(descriptor + 0);
    t[5] = *reinterpret_cast<int32_t*>(descriptor + 4);
    t[270] = v21;
    t[269] = 0;
    t[266] = 0;
    t[267] = 0;
    t[274] = 0;
    t[275] = 0;
    t[276] = 0;
    t[277] = 0;
    t[278] = 0;
    t[9] = 0;
    t[272] = v22;
    t[273] = v23;
    t[268] = 100;
    t[271] = depth;
    result[1116] = 0;
    t[6] = v4;
    t[7] = a1;

    int32_t infoOut = 0;
    (void)TextureInfo434290(record[0], record[1], reinterpret_cast<int32_t*>(result + 0x65C),
                            &infoOut);
    infoOut = TextureHeight434260(record[0], record[1]);
    *reinterpret_cast<float*>(result + 0x2A5C) = static_cast<float>(
        static_cast<double>(static_cast<uint32_t>(infoOut)) * 0.03937007859349251);
    if (v4 > 8)
        return reinterpret_cast<TextureObject*>(result);

    if (descriptor[0x19] == 0) {
        int32_t count = 1 << (static_cast<uint32_t>(v4) & 31);
        t[9] = count;
        int32_t i = 0;
        if (count > 0) {
            int32_t v12 = 0;
            uint8_t* dst = result + 41;
            do {
                const int32_t v14 = v12 / (t[9] - 1);
                dst[2] = 0xFF;
                ++i;
                v12 += 255;
                dst += 4;
                dst[-3] = static_cast<uint8_t>(v14);
                dst[-4] = static_cast<uint8_t>(v14);
                dst[-5] = static_cast<uint8_t>(v14);
            } while (i < t[9]);
        }
        return reinterpret_cast<TextureObject*>(result);
    }

    t[9] = Load16(descriptor + 0x14);
    int32_t v15 = 0;
    if (Load16(descriptor + 0x14) == 0) {
        Free4461A0(result);
        return nullptr;
    }
    int32_t v23out = 0;
    int32_t v22out = 0;
    a1 = 0;
    int32_t v16 = 0;
    if (TextureFlags4342D0(record[0], record[1], &a1, &v23out, &v22out)) {
        v16 = a1;
        t[270] = 8;
    } else {
        a1 = 0;
    }
    if (t[9] <= 0)
        return reinterpret_cast<TextureObject*>(result);
    uint8_t* palette = static_cast<uint8_t*>(*reinterpret_cast<void**>(descriptor + 0x10));
    int32_t src = 0;
    uint8_t* dst = result + 41;
    do {
        dst[1] = palette[src + 0];
        dst[0] = palette[src + 1];
        dst[-1] = palette[src + 2];
        if (v16 != 0 && v15 < v23out)
            dst[2] = *reinterpret_cast<uint8_t*>(v16 + v15);
        else
            dst[2] = 0xFF;
        ++v15;
        src += 3;
        dst += 4;
    } while (v15 < t[9]);
    return reinterpret_cast<TextureObject*>(result);
}

// ---------------------------------------------------------------------------
// 0x00418AB0
// ---------------------------------------------------------------------------

TextureObject* __stdcall BuildAgtTexture418AB0(TextureSource* source, uint32_t unused)
{
    (void)unused;
    (void)SeekStream446220(source->stream, 0);
    const uint32_t streamSize = StreamSize446240(source->stream);

    uint8_t header[0x12] = {};
    if (!StreamRead(source->stream, header, 0x12))
        return nullptr;

    const uint8_t v34 = header[0x00];
    const uint8_t v35 = header[0x01];
    const uint8_t v36 = header[0x02];
    const int32_t v37 = Load16(header + 0x03);
    const int32_t v38 = Load16(header + 0x05);
    const uint8_t v39 = header[0x07];
    const int32_t v40 = static_cast<int16_t>(Load16(header + 0x08));
    const int32_t v41 = static_cast<int16_t>(Load16(header + 0x0A));
    const int32_t v42 = Load16(header + 0x0C);
    const int32_t v43 = Load16(header + 0x0E);
    const uint8_t v44 = header[0x10];
    const uint8_t v45 = header[0x11];

    const int32_t v6 = (v36 >> 3) & 1;
    int32_t v32 = v40;
    int32_t v33 = v41;
    if ((v45 & 0x20) == 0)
        v33 = -v41;
    if ((v45 & 0x10) != 0)
        v32 = -v40;

    int32_t v7 = v44 - 24;
    int32_t v28 = v7;
    if (v44 == 15 || v44 == 16 ||
        (v44 == 8 && v35 != 0 && (v39 == 15 || v39 == 16))) {
        v28 = (v45 & 0xF) == 1 ? 1 : 0;
        v7 = v28;
    }
    if (v7 > 0) {
        if (v7 > 8)
            v28 = 8;
    } else {
        v28 = 0;
    }

    // The original's v30 is a 32-bit int: the seek contributes its low byte, but the
    // palette reads contribute their full (possibly >255) byte count to the final
    // "did anything fail" test.
    int32_t v30 = SeekStream446220(source->stream, static_cast<int32_t>(v34) + 18);
    const int32_t v9 = v36 & 7;
    int32_t v10 = 0;
    if (v9 == 1)
        v10 = v38;
    else if (v9 == 3)
        v10 = 256;

    uint8_t* const result = static_cast<uint8_t*>(GameAlloc(10864));
    if (result == nullptr)
        return nullptr;
    std::memset(result, 0, 0x2A70);
    int32_t* const t = reinterpret_cast<int32_t*>(result);
    t[5] = v43;
    t[4] = v42;
    t[270] = v28;
    t[269] = static_cast<int32_t>(streamSize);
    t[276] = v32;
    std::strcpy(reinterpret_cast<char*>(result), "AGT.p*");
    result[7] = 0;
    t[2] = 345;
    t[3] = 0;
    t[266] = 0;
    t[267] = v6;
    t[268] = 100;
    t[271] = 8;
    t[272] = 8;
    t[273] = 8;
    t[274] = 0;
    t[275] = 0;
    t[277] = v33;
    t[278] = 0;
    result[1116] = 0;

    uint8_t buffer[1024] = {};
    if (v44 < 15) {
        int32_t bits = v44;
        if (bits == 0)
            bits = 8;
        if (v10 == 0)
            v10 = 1 << (static_cast<uint32_t>(bits) & 31);
        if (v10 > 256)
            v10 = 256;
        const int32_t v15 = v37;
        t[6] = 8;
        t[7] = bits;
        t[9] = v10;
        t[8] = v15;
        if (v35 != 0) {
            switch (v39) {
            case 15:
            case 16: {
                t[273] = 5;
                t[272] = 5;
                t[271] = 5;
                t[270] = 0;
                v30 = StreamRead(source->stream, buffer, 2 * v10);
                if (v38 > 0) {
                    uint8_t* dst = &result[4 * v15 + 43];
                    int32_t i = 0;
                    do {
                        const uint32_t v24 = Load16(&buffer[2 * i]);
                        if (v28 != 0)
                            dst[0] = static_cast<uint8_t>(
                                -(static_cast<int32_t>(v24 & 0x8000) != 0));
                        else
                            dst[0] = 0xFF;
                        const uint32_t x = 255u * ((v24 >> 10) & 0x1F);
                        const uint32_t v25 = MagicDiv31(x);
                        dst[-1] = static_cast<uint8_t>((v25 + ((x - v25) >> 1)) >> 4);
                        dst[-2] = static_cast<uint8_t>(255u * ((v24 >> 5) & 0x1F) / 0x1Fu);
                        dst[-3] = static_cast<uint8_t>(255u * (v24 & 0x1F) / 0x1Fu);
                        ++i;
                        dst += 4;
                    } while (i < v38);
                }
                break;
            }
            case 24: {
                t[270] = 0;
                v30 = StreamRead(source->stream, buffer, 3 * v10);
                if (v38 > 0) {
                    const uint8_t* src = &buffer[1];
                    uint8_t* dst = &result[4 * v15 + 42];
                    int32_t remaining = v38;
                    do {
                        dst[1] = 0xFF;
                        dst[0] = src[1];
                        dst[-1] = src[0];
                        dst[-2] = src[-1];
                        dst += 4;
                        src += 3;
                        --remaining;
                    } while (remaining != 0);
                }
                break;
            }
            case 32: {
                t[270] = 8;
                v30 = StreamRead(source->stream, buffer, 4 * v10);
                if (v38 > 0) {
                    uint8_t* dst = &result[4 * v15 + 42];
                    int32_t i = 0;
                    do {
                        dst[1] = buffer[4 * i + 3];
                        dst[0] = buffer[4 * i + 2];
                        dst[-1] = buffer[4 * i + 1];
                        dst[-2] = buffer[4 * i + 0];
                        ++i;
                        dst += 4;
                    } while (i < v38);
                }
                break;
            }
            default:
                break;
            }
        } else {
            t[270] = 0;
            int32_t grey = 0;
            uint8_t* dst = result + 42;
            do {
                dst[0] = static_cast<uint8_t>(grey);
                dst[-1] = static_cast<uint8_t>(grey);
                dst[-2] = static_cast<uint8_t>(grey);
                dst[1] = 0xFF;
                ++grey;
                dst += 4;
            } while (grey < 256);
        }
    } else {
        int32_t bits = v44;
        if (bits == 16)
            bits = 15;
        t[6] = 32;
        t[7] = bits;
        t[9] = 0;
        if (bits <= 16) {
            t[273] = 5;
            t[272] = 5;
            t[271] = 5;
        }
        if (bits < 32)
            t[270] = 0;
    }

    if (v30 != 0)
        return reinterpret_cast<TextureObject*>(result);
    Free4461A0(result);
    return nullptr;
}

// ---------------------------------------------------------------------------
// 0x00434260 / 0x00434290 / 0x004342D0 / 0x004461A0 / 0x00446270
// ---------------------------------------------------------------------------

int32_t __cdecl TextureHeight434260(void* a1, void* a2)
{
    if (a1 != nullptr && a2 != nullptr &&
        (static_cast<int8_t>(*reinterpret_cast<uint8_t*>(static_cast<uint8_t*>(a2) + 8)) < 0) &&
        *reinterpret_cast<uint8_t*>(static_cast<uint8_t*>(a2) + 0x78) == 1)
        return *reinterpret_cast<int32_t*>(static_cast<uint8_t*>(a2) + 0x70);
    return 0;
}

int32_t __cdecl TextureInfo434290(void* a1, void* a2, int32_t* out1, int32_t* out2)
{
    if (a1 == nullptr || a2 == nullptr)
        return 0;
    uint8_t* const b = static_cast<uint8_t*>(a2);
    if (*reinterpret_cast<int32_t*>(b + 0x30) <= 0)
        return 0;
    if (out1 != nullptr)
        *out1 = *reinterpret_cast<int32_t*>(b + 0x38);
    if (out2 != nullptr)
        *out2 = *reinterpret_cast<int32_t*>(b + 0x30);
    return *reinterpret_cast<int32_t*>(b + 0x30);
}

int32_t __cdecl TextureFlags4342D0(void* a1, void* a2, int32_t* out1, int32_t* out2,
                                   int32_t* out3)
{
    int32_t result = 0;
    if (a1 == nullptr || a2 == nullptr)
        return 0;
    uint8_t* const b = static_cast<uint8_t*>(a2);
    if ((b[8] & 0x10) == 0)
        return 0;
    if (b[0x19] == 3) {
        if (out1 != nullptr) {
            *out1 = *reinterpret_cast<int32_t*>(b + 0x4C);
            result = 16;
        }
        if (out3 != nullptr)
            *out3 = static_cast<int32_t>(reinterpret_cast<uintptr_t>(b + 0x50));
    } else {
        if (out3 != nullptr) {
            *out3 = static_cast<int32_t>(reinterpret_cast<uintptr_t>(b + 0x50));
            result = 16;
        }
        if (out1 != nullptr)
            *out1 = 0;
    }
    if (out2 != nullptr) {
        *out2 = Load16(b + 0x16);
        return result | 0x10;
    }
    return result;
}

uint8_t __stdcall Free4461A0(void* block)
{
    // The free goes through the game's operator delete address (0x004E3F70) so the
    // harness and the differential tests can intercept it, exactly like every other
    // rewritten unit does.
    CallGameFunction<void, 0x004E3F70>(block);
    return 1;
}

void* __cdecl PushCleanup446270(void* node)
{
#if defined(_M_IX86)
    *reinterpret_cast<void**>(node) = *reinterpret_cast<void**>(kCleanupListHead);
    *reinterpret_cast<void**>(kCleanupListHead) = node;
    return node;
#else
    (void)node;
    AbortNotRewritten("the GIF cleanup list");
    return nullptr;
#endif
}

}  // namespace ts3::ui