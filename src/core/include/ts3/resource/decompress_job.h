// The async loader's decompress job (TS3W.exe 0x00727950, __thiscall, queued through the
// entry shim 0x00728070). When a resource read through DatabasePackedStream's async I/O
// comes back compressed, the loader (0x007297E0) queues this job; it decodes the record
// outside the caller's thread and stores the result back on the request.
//
// The layouts are partial: unknown fields are padding sized to the game's x86 offsets,
// which the harness hands over unchanged.
#pragma once

#include <cstdint>

#include "ts3/layout.h"
#include "ts3/resource/compression_codec.h"
#include "ts3/thread/mutex.h"

namespace ts3::resource {

// The object that owns queued jobs: the loader counts them and keeps a mutex over the
// requests' record pointers. The read job (0x0072A150) and the completion handler
// (0x0072A020) share these fields.
struct DecompressJobHost {
    uint8_t unknown000[0x34];
    uint32_t maxPendingBytes;  // 0x034 high-water mark, updated by 0x0072A020
    uint8_t unknown038[0x08];
    thread::Mutex mutex;       // 0x040 guards the requests' record pointers
    uint8_t unknown070[0x04];
    volatile long pendingJobs;   // 0x074 queued but not yet run
    volatile long pendingBytes;  // 0x078 raw (still compressed) bytes queued but not decoded
};
TS3_X86_OFFSET(DecompressJobHost, maxPendingBytes, 0x034);
TS3_X86_OFFSET(DecompressJobHost, mutex, 0x040);
TS3_X86_OFFSET(DecompressJobHost, pendingJobs, 0x074);
TS3_X86_OFFSET(DecompressJobHost, pendingBytes, 0x078);

// One queued resource read. The game's object is larger; everything past 0x0A0 belongs to
// the read job and the completion handler (0x0072A020), which are called as game code.
struct DecompressRequest {
    uint8_t unknown000[0x10];
    uint8_t key[0x10];        // 0x010 the resource key (copied into the record; opaque here)
    uint32_t unknown020;
    void* sourceRecord;       // 0x024 the record the read produced, dropped by the job
    void* resultRecord;       // 0x028 the finished record, assigned under the host mutex
    void* container;          // 0x02C what the read went through (its slot 3 is the kind id)
    uint32_t unknown030[2];
    uint32_t storedBytes;     // 0x038 raw bytes read (subtracted from pendingBytes on run)
    uint32_t memSize;         // 0x03C expected decompressed size; -1 means the read failed
    uint32_t compression;     // 0x040 compression id (0 none, 0xFFFF RefPack)
    uint32_t unknown044;
    uint8_t asyncIo[0x4C];    // 0x048 the async I/O request: cancel 0x00401260, wait 0x00401290
    uint8_t flag0x94;         // 0x094 the completion handler (0x0072A020) has work
    uint8_t flag0x95;         // 0x095 cleared on run
    uint8_t flag0x96;         // 0x096 set on run
    uint8_t unknown0x97[3];
    uint8_t flag0x9A;
    uint8_t unknown0x9B[2];
    uint8_t cancelled;        // 0x09D the read was cancelled: slot 21 of the container runs
    uint8_t failed;           // 0x09E the job failed (bad size, no codec, decode mismatch)
    uint8_t unknown0x9F;
    uint8_t ioInFlight;       // 0x0A0 an async I/O request is outstanding: cancel and wait
};
TS3_X86_OFFSET(DecompressRequest, key, 0x010);
TS3_X86_OFFSET(DecompressRequest, sourceRecord, 0x024);
TS3_X86_OFFSET(DecompressRequest, resultRecord, 0x028);
TS3_X86_OFFSET(DecompressRequest, container, 0x02C);
TS3_X86_OFFSET(DecompressRequest, storedBytes, 0x038);
TS3_X86_OFFSET(DecompressRequest, memSize, 0x03C);
TS3_X86_OFFSET(DecompressRequest, compression, 0x040);
TS3_X86_OFFSET(DecompressRequest, asyncIo, 0x048);
TS3_X86_OFFSET(DecompressRequest, cancelled, 0x09D);
TS3_X86_OFFSET(DecompressRequest, failed, 0x09E);
TS3_X86_OFFSET(DecompressRequest, ioInFlight, 0x0A0);

// TS3W.exe 0x00727950. Runs the job: state 4 executes it, any other state destroys the
// queued read (cancels outstanding I/O, runs the container's slot 21 if the read was
// cancelled, drops the source record). On execute the counters on the host drop by one
// job / by the request's stored bytes, the record is decoded through the RefPack codec or
// the resource manager, and the result is assigned under the host mutex. The completion
// handler 0x0072A020 (still game code) has the last word on the return value.
bool DecompressJob(DecompressJobHost* host, const void* taskContext, uint32_t state);

}  // namespace ts3::resource
