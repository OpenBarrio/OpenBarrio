#pragma once
#include "ts3/math/float_values.h"
namespace ts3::math {
// TS3W.exe 0x00577260: decompose matrix into owned SRT lanes; preserves ordered alias reloads and returns matrix.
const Matrix44Value* DecomposeSrt(SrtValue* out, const Matrix44Value* matrix) noexcept;
}
