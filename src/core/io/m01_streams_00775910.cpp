#include "ts3/io/m01_streams_00775910.h"
#include "ts3/casutils/clu_774d50_CASUtils.h"

namespace ts3::io::m01 {
// Exact jump-thunk twin of 004FDCD0/0073C0F0/00742CB0: only callee and signature differ.
// The target is already verified, so call its rewrite rather than the image bridge.
uint32_t Close(uint32_t handle)
{
    return ts3::casutils::ResolveCasObject774E30(handle);
}
}
