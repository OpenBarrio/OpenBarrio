#pragma once
#include "ts3/math/float_values.h"
namespace ts3::math {
// TS3W.exe 0x00555110: rotates all four lanes with the original ordered SSE quaternion kernel; returns pose, not out.
const SrtValue* RotateVector(const Float4Value* v, const SrtValue* pose, Float4Value* out) noexcept;
// TS3W.exe 0x00555080: scales, rotates and translates all four lanes; stores intermediate before translation reload and returns pose.
const SrtValue* TransformPoint(const Float4Value* v, const SrtValue* pose, Float4Value* out) noexcept;
}
