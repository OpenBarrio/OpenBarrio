#pragma once
#include "ts3/scratch/clu_689d10_Scratch.h"

namespace ts3::math {
using UnionBox = scratch::Float4Pair689D10;
// TS3W.exe 0x0068BA00: union all four bounds lanes, returning A (not dst), min store before max loads.
// Each view needs 32 live bytes aligned to 16; aligned overlaps are allowed.
const UnionBox* UnionBounds(UnionBox* dst, const UnionBox* a, const UnionBox* b) noexcept;
}
