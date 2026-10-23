// DatabasePackedStream's raw read path (TS3W.exe 0x0072CF90, 0x0072D300, 0x0072D360,
// 0x0072D3E0). Reads are submitted to EA's async request API — constructed by 0x00401000,
// started by 0x00401400, waited on with 0x00401290 and destroyed by 0x00401040 — which
// stays original game code, called by address (ts3::CallGameMethod), as does the throttled
// handle opener 0x0072CE10. The MemoryDB read path is not rewritten yet.
#pragma once

#include <cstdint>

#include "ts3/resource/databases.h"

namespace ts3::resource {

// TS3W.exe 0x0072CF90: submits one asynchronous read and waits for it. `flag` picks the
// throttled handle (opening it through 0x0072CE10 when there is none yet) over the primary
// one. Returns 1 when the request reached the done state, else 0. Not a class method in the
// game either; ReadRaw is its only caller there.
int PackedStreamAsyncRead(DatabasePackedStream* self, uint32_t offset, void* dst, uint32_t size, uint8_t flag);

}  // namespace ts3::resource
