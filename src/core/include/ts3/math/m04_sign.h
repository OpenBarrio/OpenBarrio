#pragma once
namespace ts3::math {
// TS3W.exe 0x00555030: classify float as +0, +1 or -1; unordered returns -1, DAZ affects subnormals.
float FloatSign(float value) noexcept;
}
