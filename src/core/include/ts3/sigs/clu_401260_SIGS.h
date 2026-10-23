// Unit clu_401260_SIGS (SIGS subsystem): the async loader's decompress-job entry helpers.
// The cluster's third function, the job itself (TS3W.exe 0x00727950), is already rewritten
// and verified as ts3::resource::DecompressJob in src/core/resource/decompress_job.cpp
// (task 5); this unit re-exports it instead of duplicating 756 bytes of verified code.
#pragma once

#include "ts3/resource/decompress_job.h"

#include <cstdint>

namespace ts3::sigs {

// TS3W.exe 0x004A3870: stores `mutex` in the guard object's first word and locks the mutex
// with the game's global forever timeout (0x00FB217C = 0xFFFFFFFF); returns the guard.
// __thiscall with one stack argument (ret 4). EA keeps no null checks here, so a null
// mutex/guard is dereferenced exactly like the original.
void* MutexGuardCtor(void* self, void* mutex);

// TS3W.exe 0x00401260: cancels/reaps the host's async read. Returns the already-created
// read pointer at self+4 untouched when it is non-null; otherwise derives the read object
// from self+0x40 (through the loader singleton 0x00402160 when the holder's byte +6 is set)
// and hands it to the read's cancel routine (0x00401810, one stack argument: this). Keeps
// EA's quirk of dereferencing self+0x40 without a null check.
void* AsyncIoCancel(void* self);

// TS3W.exe 0x00727950: the loader's decompress job (state 4 executes, any other state
// destroys). Forwards to the verified ts3::resource::DecompressJob implementation.
bool DecompressJob(resource::DecompressJobHost* host, const void* taskContext, uint32_t state);

}  // namespace ts3::sigs