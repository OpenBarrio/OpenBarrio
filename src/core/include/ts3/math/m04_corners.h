#pragma once
#include "ts3/math/m04_box_union.h"
#include "ts3/math/float_values.h"

namespace ts3::math {
// TS3W.exe 0x0068C110: emit eight XYZ box corners with W=1, returning borrowed out; reload bounds after each store.
// bounds needs 32 live bytes, out 128, both aligned16; aligned overlaps retain sequential feedback.
Float4Value* BoxCorners(const UnionBox* bounds, Float4Value* out) noexcept;
}
