#pragma once
#include "ts3/io/m01_streams.h"
namespace ts3::io::m01 {
struct Decimal { uint32_t words[4]; };
// TS3W.exe 0x004F3CA0: Return the lead byte's continuation count, or zero for invalid/ASCII bytes.
int ContinuationCount(const uint8_t* text);
// TS3W.exe 0x00774F30: Write eight integer bytes through the cached stream.
int WriteInt64(uint32_t handle, uint64_t value);
// TS3W.exe 0x00774FC0: Write four float bytes through the cached stream.
int WriteFloat(uint32_t handle, float value);
// TS3W.exe 0x00775040: Write eight double bytes through the cached stream.
int WriteDouble(uint32_t handle, double value);
// TS3W.exe 0x007750C0: Write a decimal in EA's reordered wire layout.
int WriteDecimal(uint32_t handle, Decimal value);
// TS3W.exe 0x00775160: Write a managed array slice; zero count is an error.
int WriteByteArray(uint32_t handle, uint32_t array, int32_t offset, int32_t count);
// TS3W.exe 0x00775230: Convert one UTF16 code unit using the original converter and write it.
int WriteCharUTF8(uint32_t handle, uint16_t value);
// TS3W.exe 0x007752C0: Read at most four bytes into a zeroed integer; negative sizes are not rejected.
int ReadInt(uint32_t handle, int32_t count, uint32_t* value);
// TS3W.exe 0x00775350: Read eight bytes into a zeroed integer, retaining partial data on failure.
int ReadInt64(uint32_t handle, uint64_t* value);
// TS3W.exe 0x007753E0: Read a float, publishing only on an exact read.
int ReadFloat(uint32_t handle, float* value);
// TS3W.exe 0x00775470: Read a double, publishing only on an exact read.
int ReadDouble(uint32_t handle, double* value);
// TS3W.exe 0x00775510: Read and reorder a decimal only on an exact read.
int ReadDecimal(uint32_t handle, Decimal* value);
// TS3W.exe 0x007755C0: Read UTF8; EA compares the continuation read to one, not its request.
int ReadCharUTF8(uint32_t handle, uint16_t* value);
// TS3W.exe 0x007756D0: Read a managed array slice and return the actual count (zero allowed).
int ReadByteArray(uint32_t handle, uint32_t array, int32_t offset, int32_t count);
// TS3W.exe 0x00775920: Return size with stream errors mapped to -3.
int GetSize(uint32_t handle);
// TS3W.exe 0x00775990: Return absolute position with stream errors mapped to -3.
int GetPosition(uint32_t handle);
// TS3W.exe 0x00775A00: Seek using an unsigned origin check and signed displacement.
int SetPosition(uint32_t handle, int32_t position, uint32_t origin);
}
