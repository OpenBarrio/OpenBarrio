// Unit clu_4de560_Debug (the project notes 1004): the buffered log-writer the game
// builds on an EA::IO::IStream (seen in the memory-category dump at 0x00932EB0, which
// constructs it as 0x004DE780(0, 0x10000, fileStream, 0)). The class name is ours; IDA
// has no names for these functions. All three TS3W.exe functions are covered; 0x004DE560
// is a private helper called only by 0x004DE6E0:
//
//   DebugLogWriter::Close        0x004DE6A0  thiscall, no stack args  [vtable slot 0]
//   DebugLogWriter::Write        0x004DE6E0  thiscall, ret 8 (Src, Size) [vtable slot 9]
//   DebugLogWriter::BufferWrite  0x004DE560  thiscall, ret 8 (Src, Size), private
//
// Write copies into the buffer when +0x2C is non-zero, else goes straight to the
// stream; the buffer flushes through the game's 0x004DE160 (unit clu_4d5170_x, called
// via ts3::CallGameMethod), which writes to stream slot 14 and resyncs through slot 9
// on failure. Close flushes (result ignored by the original), zeroes the counters and
// tail-calls stream slot 6. The object's own vtable slot 0 is Close.
#pragma once

#include <cstddef>
#include <cstdint>

#include "ts3/io/stream.h"
#include "ts3/layout.h"

namespace ts3::debug {

class DebugLogWriter {
public:
    // 0x004DE6E0: appends `size` bytes at the logical position. With a buffer the bytes
    // are copied (flushing 0x004DE160 whenever it fills); with +0x2C == 0 they go
    // straight to stream slot 14. Returns false without writing when the stream is null.
    bool Write(const void* src, size_t size);
    // 0x004DE6A0 [vtable slot 0]: flushes, zeroes position/committed/buffer state and
    // closes the stream; returns the stream Close's result (false with no stream).
    bool Close();
    // 0x004DE560: the buffer half of Write (called when +0x2C != 0); public only so the
    // differential test can hit the original directly.
    bool BufferWrite(const void* src, size_t size);

    void* const* vtable;  // 0x00 (slot 0 = 0x004DE6A0, slot 9 = 0x004DE6E0)
    ts3::io::IStream* stream;  // 0x04 (null = closed; every entry point early-outs)
    uint8_t gap08[4];          // 0x08 (used by 0x004DE780's construction, not by this unit)
    // 0x0C logical write position. EA keeps it one step "ahead": the buffered path adds
    // Size here even when the buffer write failed (kept, it is the stream's seek target).
    uint32_t position;
    uint32_t committed;  // 0x10 bytes handed to the stream so far (== position when unbuffered)
    uint8_t gap14[0x0C]; // 0x14..0x1F (unused by this unit)
    uint32_t stateFlag;  // 0x20 zeroed by Close and by the write-prep; owner unknown
    uint32_t seekPending;  // 0x24 non-zero = first seek to `position` before the next write
    char* buffer;           // 0x28
    uint32_t bufferCapacity;  // 0x2C 0 = unbuffered (writes go straight to the stream)
    uint32_t bufferStart;     // 0x30 file offset of buffer[0]
    uint32_t buffered;        // 0x34 bytes currently in the buffer

private:
    // 0x004DE160 (game, unit clu_4d5170_x): hands the buffered bytes to the stream and
    // clears the buffer; resyncs committed/bufferStart from GetPosition on write failure.
    bool FlushBuffered();
};
TS3_X86_OFFSET(DebugLogWriter, stream, 0x04);
TS3_X86_OFFSET(DebugLogWriter, position, 0x0C);
TS3_X86_OFFSET(DebugLogWriter, committed, 0x10);
TS3_X86_OFFSET(DebugLogWriter, stateFlag, 0x20);
TS3_X86_OFFSET(DebugLogWriter, seekPending, 0x24);
TS3_X86_OFFSET(DebugLogWriter, buffer, 0x28);
TS3_X86_OFFSET(DebugLogWriter, bufferCapacity, 0x2C);
TS3_X86_OFFSET(DebugLogWriter, bufferStart, 0x30);
TS3_X86_OFFSET(DebugLogWriter, buffered, 0x34);
TS3_X86_SIZE(DebugLogWriter, 0x38);

}  // namespace ts3::debug
