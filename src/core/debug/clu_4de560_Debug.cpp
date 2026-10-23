// Unit clu_4de560_Debug: the buffered log-writer over an EA::IO::IStream (see the
// header for the map). Transcribed from TS3W.exe 1.67.2; every field access carries
// the original address. Kept EA quirks (also in the project notes where they affect
// outputs): Write adds Size to position +0x0C even when the buffer write failed;
// Close ignores the 0x004DE160 flush result; BufferWrite's fast-path sum is a 32-bit
// add that wraps on overflow.
#include <cstdint>
#include <cstring>

#include "ts3/debug/clu_4de560_Debug.h"
#include "ts3/layout.h"

namespace ts3::debug {

bool DebugLogWriter::FlushBuffered()
{
    // 0x004DE160 (game, unit clu_4d5170_x): not ours to rewrite yet.
    return ts3::CallGameMethod<bool, 0x004DE160>(this);
}

bool DebugLogWriter::BufferWrite(const void* src, size_t size)
{
    // 0x004DE560: copies into the buffer, flushing through 0x004DE160 whenever it
    // fills; returns whether every flush so far succeeded (true for a zero size).
    uint32_t remaining = static_cast<uint32_t>(size);  // 004DE569 ebp
    if (!remaining)
        return true;  // 004DE56E

    const uint32_t buffered = this->buffered;  // 004DE578 (+0x34)
    if (!buffered)
        this->bufferStart = this->committed;  // 004DE584: empty buffer starts at the committed offset
    // 004DE581: 32-bit add, wraps on overflow - the fast path can take over for huge
    // sizes. Unreachable from Write with sane buffers, kept bit-identical.
    if (buffered + remaining <= this->bufferCapacity) {  // 004DE58D (+0x2C)
        std::memcpy(this->buffer + buffered, src, remaining);  // 004DE593
        this->buffered = buffered + remaining;                 // 004DE59E
        return true;                                           // 004DE5A4
    }

    auto* p = static_cast<const uint8_t*>(src);  // 004DE5AE ebx
    bool ok = true;                              // v10, byte at [esp+F]
    for (;;) {
        if (!ok)  // 004DE5B9: break when a flush failed and bytes are left
            return false;                        // 004DE5FC..601 result = v10
        const uint32_t bufferedNow = this->buffered;
        uint32_t chunk = this->bufferCapacity - bufferedNow;  // 004DE5C1..C8
        if (chunk >= remaining)
            chunk = remaining;  // 004DE5CA..CF
        if (chunk) {
            std::memcpy(this->buffer + bufferedNow, p, chunk);  // 004DE5D6
            this->buffered = bufferedNow + chunk;               // 004DE5DE
            p += chunk;                                         // 004DE5E5
            remaining -= chunk;                                 // 004DE5E8
        }
        if (this->buffered == this->bufferCapacity)  // 004DE5EB..F0: full
            ok = FlushBuffered();                    // 0x004DE5F5
        if (!remaining)
            return ok;  // 004DE5F8
    }
}

bool DebugLogWriter::Write(const void* src, size_t size)
{
    // 0x004DE6E0 [vtable slot 9], ret 8.
    ts3::io::IStream* out = this->stream;  // 004DE6E3 (+0x04)
    if (!out)
        return false;  // 004DE6EA
    if (this->seekPending) {  // 004DE6F0 (+0x24)
        const uint32_t pos = this->position;  // 004DE6F6 (+0x0C)
        const bool needsSeek = pos != this->committed;  // 004DE6F9
        this->stateFlag = 0;      // 004DE6FC (+0x20)
        this->seekPending = 0;    // 004DE703 (+0x24)
        if (needsSeek)
            out->SetPosition(static_cast<ts3::io::off_type>(pos), ts3::io::kPositionTypeBegin);  // 004DE70C..14
    }
    const auto size32 = static_cast<uint32_t>(size);
    if (this->bufferCapacity) {  // 004DE716 (+0x2C)
        const bool ok = BufferWrite(src, size);  // 0x004DE729
        this->position += size32;  // 004DE72E EA quirk: added even when BufferWrite failed
        return ok;                  // 0x004DE733
    }
    if (this->stream->Write(src, size)) {  // 004DE745 (vtable slot 14, +0x38)
        this->committed += size32;      // 004DE74D (+0x10)
        this->position = this->committed;  // 004DE750..53
        return true;                    // 004DE756 returns bl = 1
    }
    // 004DE75E..70: resync both counters from the stream's real position.
    this->committed = static_cast<uint32_t>(this->stream->GetPosition(ts3::io::kPositionTypeBegin));  // 004DE768 (+0x24 slot)
    this->position = this->committed;  // 004DE76A..6D
    return false;                      // 004DE770 bl = 0
}

bool DebugLogWriter::Close()
{
    // 0x004DE6A0 [vtable slot 0], no stack args.
    if (!this->stream)  // 004DE6A6 (+0x04)
        return false;   // 004DE6CF
    FlushBuffered();  // 0x004DE6AB
    this->stateFlag = 0;        // 004DE6B3 (+0x20)
    this->seekPending = 0;      // 004DE6B6 (+0x24)
    this->bufferStart = 0;      // 004DE6B9 (+0x30)
    this->buffered = 0;         // 004DE6BC (+0x34)
    this->position = 0;         // 004DE6BF (+0x0C)
    this->committed = 0;        // 004DE6C2 (+0x10)
    // 004DE6CA..CC: tail-jump to stream slot 6 (+0x18) with the stream in ecx.
    return this->stream->Close();
}

}  // namespace ts3::debug
