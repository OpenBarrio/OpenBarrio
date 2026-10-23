#pragma once
#include "ts3/scratch/clu_689d10_Scratch.h"
namespace ts3::math {
// TS3W.exe 0x0068C360: pack six planes in the owned 704-byte mathematical used-region; retain SSE NaN, padding and overflow behavior.
void PreparePlaneCoefficients(scratch::ScratchClipTest689D10* value) noexcept;
}
