// The audio codec unit around TS3W.exe 0x00F20B20 (subsystem ResourceSystem): three
// game classes sharing one object prefix, rewritten together. The game's base constructor
// 0x004E17E0, block fetch 0x00F20BA0 / 0x00F21340, block release 0x00403E00 and the
// IMA ADPCM decode 0x00F21A10 stay original and are reached through the bridge.
//
// Kept as the original does them (they affect observable outputs):
//   - AudioEncoder_Pcm::ReadRows spins forever when the cursor already sits at the
//     block's size (its guard is a je-only continue — sic) and divides unsigned, so a
//     cursor past the end wraps into a huge row count;
//   - the two cdecl constructors have no null-object guard around the virtual dispatch;
//   - CAELayer3Decoder::Construct leaves the +0x78 region and +0x14 as it found them;
//   - AudioDecoder_FlvImaAdpcm::Init stores the new state pointer twice (same value).
// See the project notes.
#include "ts3/resourcesystem/cls_f20b20_ResourceSystem.h"

#include <cstddef>
#include <cstring>

namespace ts3::resourcesystem {

namespace {
// The vtables the placement constructors install; only the x86 image has them, but the
// constants compile everywhere (x64 never runs this code: the bridges abort first).
constexpr uintptr_t kPcmVtable = 0x0112CBC8u;
constexpr uintptr_t kImaVtable = 0x0112CE30u;
constexpr uintptr_t kCaeVtable = 0x0113855Cu;
}  // namespace

// TS3W.exe 0x00F20B20 — AudioEncoder_Pcm virtual slot 1 (thiscall, ret 0x14, leaf).
// Arguments 2 and 5 are ignored.
void AudioEncoderPcm::Init(void* allocatorArg, void*, uint32_t widthArg, uint32_t depthArg, void*)
{
    allocator = allocatorArg;
    depth = depthArg;   // [ecx+0x54] <- arg 4
    width = widthArg;   // [ecx+0x50] <- arg 3
}

// TS3W.exe 0x00F20B40 — AudioEncoder_Pcm virtual slot 2 (thiscall, plain ret). Hands the
// fetched block back through the block source (game Free 0x00403E00, which dereferences
// its this immediately) and forgets it; without a block nothing happens (eax = 0).
void AudioEncoderPcm::ReleaseBlock()
{
    if (!block)
        return;
    CodecBlock* const b = block;
    CallGameMethod<void, 0x00403E00>(blockSource, b);
    block = nullptr;
}

// TS3W.exe 0x00F20D10 — AudioEncoder_Pcm virtual slot 4 (thiscall, ret 4). Converts the
// fetched block's rows into the caller's sample buffer: 8-bit unsigned samples widen to
// 16-bit as (v - 128) << 8, 16-bit samples are copied as words, any other depth skips
// the copy but still advances the cursor. Fetching and releasing stay game code.
// Returns *out; with a null argument the original returns out itself (eax = a2), which
// is the same nullptr here.
AudioSampleBuffer* AudioEncoderPcm::ReadRows(AudioSampleBuffer** out)
{
    if (out) {
        AudioSampleBuffer* buf = *out;
        uint32_t remaining = buf->count;  // [eax+0xC] read once, then tracked on the stack
        if (!block)                       // runs before the row-count check, ecx still this
            CallGameMethod<void, 0x00F20BA0>(this);
        while (remaining) {
            CodecBlock* blk = block;
            if (!blk)
                break;
            if (cursor == blk->size)
                continue;  // sic: the original's je-only guard spins here forever
            const uint32_t bpr = (width * depth) >> 3;              // rows to bytes
            const uint32_t avail = (blk->size - cursor) / bpr;     // unsigned: a cursor past
                                                                   // the end wraps (EA)
            const uint32_t rows = avail < remaining ? avail : remaining;
            if (depth == 8) {
                const auto* src = static_cast<const uint8_t*>(blk->data) + cursor;
                auto* dst = reinterpret_cast<uint16_t*>(buf->data) + width * (buf->count - remaining);
                const uint32_t elements = rows * width;
                for (uint32_t i = 0; i < elements; ++i) {
                    // (v - 128) << 8 in 16 bits, wraparound included: computed in uint32
                    // and truncated, which gives the same bits as the original's
                    // movzx/sub/shl cx.
                    dst[i] = static_cast<uint16_t>((static_cast<uint32_t>(src[i]) - 128u) << 8);
                }
            } else if (depth == 16) {
                const void* src = static_cast<const uint8_t*>(blk->data) + cursor;
                void* dst = static_cast<uint8_t*>(buf->data) + 2 * width * (buf->count - remaining);
                std::memcpy(dst, src, rows * width * 2);  // the original's word-copy loop
            }
            cursor += rows * ((width * depth) >> 3);  // width/depth re-read after the copy
            remaining -= rows;
            if (cursor == blk->size) {
                CallGameMethod<void, 0x00403E00>(blockSource, blk);
                CallGameMethod<void, 0x00F20BA0>(this);
            }
        }
        buf->count -= remaining;  // sic: only the rows it could NOT deliver
    }
    return out ? *out : nullptr;
}

// TS3W.exe 0x00F20E60 — AudioEncoder_Pcm's registration-table entry (__cdecl, object on
// the stack): installs the game's vtable, runs the base constructor on the +0x18
// subobject, clears the prefix fields (+4, +8, +0xC, +0x10, +0x48, +0x4C — not +0x14,
// and not +0x50/+0x54, which Init sets), then calls virtual slot 1 with the description's
// first four dwords and a 0. Returns 1; a null object crashes on the dispatch, as in the
// original.
char ConstructAudioEncoderPcm(void* memory, const AudioCodecDesc* desc)
{
    auto* self = static_cast<AudioEncoderPcm*>(memory);
    if (self) {
        self->vtable = reinterpret_cast<void*>(kPcmVtable);
        CallGameMethod<void, 0x004E17E0>(&self->baseObject, 0u, 1u);
        self->allocator = nullptr;
        self->blockSource = nullptr;
        self->sourceStateA = 0;
        self->sourceStateB = 0;
        self->block = nullptr;
        self->cursor = 0;
    }
    CallGameVirtual<void, 1>(self, desc->blockSource, desc->field04, desc->field08, desc->field0C, 0u);
    return 1;  // the original ends mov al,1 — only the low byte is meaningful
}

// TS3W.exe 0x00F21CC0 — AudioDecoder_FlvImaAdpcm's table entry (same shape): this class's
// vtable, +0x54 cleared as well, and all five description dwords forwarded to Init.
char ConstructAudioDecoderFlvImaAdpcm(void* memory, const AudioCodecDesc* desc)
{
    auto* self = static_cast<AudioDecoderFlvImaAdpcm*>(memory);
    if (self) {
        self->vtable = reinterpret_cast<void*>(kImaVtable);
        CallGameMethod<void, 0x004E17E0>(&self->baseObject, 0u, 1u);
        self->allocator = nullptr;
        self->blockSource = nullptr;
        self->sourceStateA = 0;
        self->sourceStateB = 0;
        self->block = nullptr;
        self->cursor = 0;
        self->state = nullptr;
    }
    CallGameVirtual<void, 1>(self, desc->blockSource, desc->field04, desc->field08, desc->field0C,
                             desc->field10);
    return 1;
}

// TS3W.exe 0x00F21D50 — AudioDecoder_FlvImaAdpcm virtual slot 1 (thiscall, ret 0x14).
// Arguments 2, 4 and 5 are ignored. Through the allocator (slot-2 Alloc, flags 1) it
// builds the 0x1004-byte decoder state and the 2034 * width sample buffer; the state's
// first eight dwords are fixed (dwords 6 = width), which is also what makes the game's
// decoder 0x00F21A10 no-op when that field is neither 1 nor 2. A null allocator leaves
// the rest of the object alone.
void AudioDecoderFlvImaAdpcm::Init(void* allocatorArg, void*, uint32_t widthArg, void*, void*)
{
    allocator = allocatorArg;
    width = widthArg;
    if (allocatorArg) {
        void* st = CallGameVirtual<void*, 2>(allocatorArg, static_cast<size_t>(0x1004u),
                                             "AudioDecoder_MsImaAdpcm internal codec.", 1u);
        if (st) {
            auto* p = static_cast<uint32_t*>(st);
            p[0] = 0;
            p[1] = 0;
            p[2] = 7;
            p[3] = 0;
            p[4] = 0;
            p[5] = 7;
            p[6] = width;  // [esi+0x50] re-read after the allocation
            p[7] = 0;
        }
        state = st;  // the original stores this twice (same value)
        rowsPerBlock = 1017;
        void* sample =
            CallGameVirtual<void*, 2>(allocator, static_cast<size_t>(2034u * width),
                                      "AudioDecoder_MsImaAdpcm sample buffer.", 1u);
        sampleBuffer = sample;
        pendingRows = 0;
        pendingPtr = sample;
    }
}

// TS3W.exe 0x00F21DE0 — AudioDecoder_FlvImaAdpcm virtual slot 4 (thiscall, ret 4). Drains
// the leftover decoded rows first, then decodes: blocks of more than 1017 rows go
// straight into the destination, a shorter tail decodes into the sample buffer and the
// rest stays pending there. The decoder 0x00F21A10, the fetch 0x00F21340 and the release
// 0x00403E00 stay game code. Returns *out; with a null argument the original leaves eax
// undefined, and this returns out (the same nullptr).
AudioSampleBuffer* AudioDecoderFlvImaAdpcm::ReadRows(AudioSampleBuffer** out)
{
    if (out) {
        uint8_t* dst = static_cast<uint8_t*>((*out)->data);
        uint32_t remaining = (*out)->count;
        if (pendingRows) {
            const uint32_t take = remaining < pendingRows ? remaining : pendingRows;
            const uint32_t bytes = 2 * take * width;
            std::memcpy(dst, pendingPtr, bytes);
            pendingRows -= take;
            pendingPtr = static_cast<uint8_t*>(pendingPtr) + bytes;
            dst += bytes;
            remaining -= take;
        }
        if (!block)
            CallGameMethod<void, 0x00F21340>(this);
        while (remaining) {
            CodecBlock* blk = block;
            if (!blk)
                break;
            const auto* src = static_cast<const uint8_t*>(blk->data) + cursor;
            if (remaining <= 1017u) {
                CallGameMethod<void, 0x00F21A10>(state, src, sampleBuffer);
                const uint32_t bytes = 2 * remaining * width;
                std::memcpy(dst, sampleBuffer, bytes);
                pendingPtr = static_cast<uint8_t*>(sampleBuffer) + bytes;
                pendingRows = 1017u - remaining;
                remaining = 0;
                // the state is re-read after the decode
                cursor += *reinterpret_cast<const uint32_t*>(static_cast<const uint8_t*>(state) + 0x18) << 9;
            } else {
                CallGameMethod<void, 0x00F21A10>(state, src, dst);
                dst += 2034u * width;
                remaining -= 1017u;
                cursor += *reinterpret_cast<const uint32_t*>(static_cast<const uint8_t*>(state) + 0x18) << 9;
            }
            if (cursor == blk->size) {
                CallGameMethod<void, 0x00403E00>(blockSource, blk);
                CallGameMethod<void, 0x00F21340>(this);
            }
        }
        (*out)->count -= remaining;  // sic: only the undelivered rows
    }
    return out ? *out : nullptr;
}

// TS3W.exe 0x00F2A4B0 — CAELayer3Decoder's constructor body (thiscall, ret 4; the game
// caller 0x00F2A920 pushes the description's first dword and calls it). Installs the
// vtable, runs the base constructor on the +0x18 subobject, stores the block source
// (loaded from the stack after that call in the original), zeroes the fields it owns and
// returns this. The +0x18..+0x47 region comes from the base constructor and the +0x78
// region is left untouched.
CAELayer3Decoder* CAELayer3Decoder::Construct(void* blockSourceArg)
{
    vtable = reinterpret_cast<void*>(kCaeVtable);
    CallGameMethod<void, 0x004E17E0>(&baseObject, 0u, 1u);
    blockSource = blockSourceArg;
    unused04 = nullptr;
    sourceStateA = 0;
    sourceStateB = 0;
    unused14 = 0;
    block = nullptr;
    cursor = 0;
    field50 = 0;
    state = nullptr;
    field58 = 0;
    sampleBuffer = nullptr;
    pendingRows = 0;
    pendingPtr = nullptr;
    field68 = 0;
    field6C = 0;
    field70 = 0;
    field74 = 0;
    field90 = 0;
    return this;
}

}  // namespace ts3::resourcesystem
