// Cluster unit clu_71b550_Shader (subsystem Shader): rewrites of the three TS3W.exe
// leaves declared in ts3/shader/clu_71b550_Shader.h (original addresses in the header).
//
// The two copies at 0x0071B550 / 0x0071C240 are the same compiler-generated element copy
// (they move the identical set of fields; only the float-store order differs, which does
// not change the resulting memory). They copy every member but skip the padding holes
// (0x06, 0x1C, 0x45, 0x5E, 0x64, 0x75..0x77), which stay with the destination.
//
// EA behaviour kept on purpose (affects outputs):
//   - The twelve float fields are stored through the x87 (fld/fstp), which quiets a
//     signaling NaN: it sets the quiet bit 0x400000 while keeping sign and payload
//     (7F800001 -> 7FC00001, probed in the original binary; same behaviour documented for
//     clu_cc90c0_x's StoreX87). Integers/bytes/words are bit-exact (mov), and +/-inf and
//     quiet NaNs are unchanged. The rewrite reproduces this so the copied floats match
//     the original bit-for-bit on every input.

#include "ts3/shader/clu_71b550_Shader.h"

#include <cstdint>
#include <cstring>

namespace ts3::shader {
namespace {

// fld/fstp store: quiet a signaling NaN the way the x87 does.
void StoreX87(float* dst, float v)
{
    uint32_t bits;
    std::memcpy(&bits, &v, sizeof bits);
    if ((bits & 0x7F800000u) == 0x7F800000u && (bits & 0x007FFFFFu) != 0 &&
        (bits & 0x00400000u) == 0)
        bits |= 0x00400000u;
    std::memcpy(dst, &bits, sizeof bits);
}

// The shared body of 0x0071B550 / 0x0071C240. Copies the used fields only; the padding
// gaps (0x06, 0x1C, 0x45, 0x5E, 0x64, 0x75..0x77) are left untouched, exactly like the
// original's field-by-field copy.
ShaderParamRecord* AssignImpl(ShaderParamRecord* dst, const ShaderParamRecord& src)
{
    dst->d00 = src.d00;
    dst->b04 = src.b04;
    dst->b05 = src.b05;
    dst->d08 = src.d08;
    dst->d0C = src.d0C;
    dst->index = src.index;
    dst->d14 = src.d14;
    dst->d18 = src.d18;
    // 0x1C gap untouched
    dst->d20 = src.d20;
    dst->d24 = src.d24;
    dst->d28 = src.d28;
    StoreX87(&dst->f2C, src.f2C);
    StoreX87(&dst->f30, src.f30);
    StoreX87(&dst->f34, src.f34);
    StoreX87(&dst->f38, src.f38);
    StoreX87(&dst->f3C, src.f3C);
    StoreX87(&dst->f40, src.f40);
    dst->b44 = src.b44;
    // 0x45 gap untouched
    StoreX87(&dst->f48, src.f48);
    StoreX87(&dst->f4C, src.f4C);
    StoreX87(&dst->f50, src.f50);
    StoreX87(&dst->f54, src.f54);
    StoreX87(&dst->f58, src.f58);
    dst->w5C = src.w5C;
    // 0x5E gap untouched
    StoreX87(&dst->f60, src.f60);
    // 0x64 gap untouched
    dst->d68 = src.d68;
    dst->d6C = src.d6C;
    dst->b70 = src.b70;
    dst->b71 = src.b71;
    dst->b72 = src.b72;
    dst->b73 = src.b73;
    dst->b74 = src.b74;
    // 0x75..0x77 gap untouched
    return dst;
}

}  // namespace

ShaderParamRecord* ShaderParamRecord::Assign71B550(const ShaderParamRecord& src)
{
    return AssignImpl(this, src);
}

ShaderParamRecord* ShaderParamRecord::Assign71C240(const ShaderParamRecord& src)
{
    return AssignImpl(this, src);
}

uint32_t ShaderParamRecord::GetIndexIfAligned() const
{
    return (index & 3u) ? 0u : index;
}

}  // namespace ts3::shader
