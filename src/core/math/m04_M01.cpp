// m04_M01.cpp - Stage 3 m04 batch M01.
// 0x0046D9B0 (matrix-symmetric): weighted covariance upper triangle.
// 0x00511DB0 (matrix-tagged): general 4x4 inverse via cofactors with the
// original |det| < 9.99999997e-7 rejection (NaN determinant proceeds).
// Both are exact scalar-SSE ports; every statement below marked with the
// transcribed instruction performs one original instruction in the original
// operand order, so NaN payloads and signed zeros match bit for bit.
#include "ts3/math/m04_M01.h"
#include <cmath>
#include <cstdint>
#include <cstring>
#include <xmmintrin.h>

namespace ts3::math::m04m01 {
namespace {

// Each helper is exactly one SSE instruction with the given operand order.
float Mul(float a, float b) { return _mm_cvtss_f32(_mm_mul_ss(_mm_set_ss(a), _mm_set_ss(b))); }
float Add(float a, float b) { return _mm_cvtss_f32(_mm_add_ss(_mm_set_ss(a), _mm_set_ss(b))); }
float Sub(float a, float b) { return _mm_cvtss_f32(_mm_sub_ss(_mm_set_ss(a), _mm_set_ss(b))); }
float Div(float a, float b) { return _mm_cvtss_f32(_mm_div_ss(_mm_set_ss(a), _mm_set_ss(b))); }

float FromBits(uint32_t bits) { float f; std::memcpy(&f, &bits, 4); return f; }
// Read-only image constants: 00F988F0, 0107A538, 00FBAA24.
const float kNegZero = FromBits(0x80000000u);
const float kOne = FromBits(0x3f800000u);
const float kDetEps = FromBits(0x358637bdu);

} // namespace

// TS3W.exe 0x0046D9B0. The original unrolls four points per iteration, but the
// accumulator association is strictly sequential in point order, so the two
// loops below reproduce the identical add/mul chains. The mean pass keeps the
// original operand orders: sum += w, sum += component*w, and the mixed
// inv*sumX / sumY*inv / sumZ*inv means; the covariance pass stores
// (component*w)*component per element in xx,xy,xz,yy,yz,zz order.
float* CovarianceSymmetric(float* out, int count, const float* points, const float* weights) {
    float sumW = 0.0f, sumX = 0.0f, sumY = 0.0f, sumZ = 0.0f;
    for (int i = 0; i < count; ++i) {
        const unsigned u = static_cast<unsigned>(i);
        const float w = weights[u];
        sumW = Add(w, sumW);
        sumX = Add(Mul(points[u * 3u + 0u], w), sumX);
        sumY = Add(Mul(points[u * 3u + 1u], w), sumY);
        sumZ = Add(Mul(points[u * 3u + 2u], w), sumZ);
    }
    // divss 1.0f/sum with no zero guard (EA): inf/NaN means when sum is 0.
    const float inv = Div(1.0f, sumW);
    const float meanX = Mul(inv, sumX);
    const float meanY = Mul(sumY, inv);
    const float meanZ = Mul(sumZ, inv);
    out[0] = 0.0f; out[1] = 0.0f; out[2] = 0.0f;
    out[3] = 0.0f; out[4] = 0.0f; out[5] = 0.0f;
    for (int i = 0; i < count; ++i) {
        const unsigned u = static_cast<unsigned>(i);
        const float w = weights[u];
        const float dx = Sub(points[u * 3u + 0u], meanX);
        const float dy = Sub(points[u * 3u + 1u], meanY);
        const float dz = Sub(points[u * 3u + 2u], meanZ);
        // Weight applied once: (component*w)*component, product added first.
        out[0] = Add(Mul(Mul(dx, w), dx), out[0]);
        out[1] = Add(Mul(Mul(dy, w), dx), out[1]);
        out[2] = Add(Mul(Mul(dz, w), dx), out[2]);
        out[3] = Add(Mul(Mul(dy, w), dy), out[3]);
        out[4] = Add(Mul(Mul(dz, w), dy), out[4]);
        out[5] = Add(Mul(Mul(dz, w), dz), out[5]);
    }
    return out;
}

// TS3W.exe 0x00511DB0. Direct transcription of the original scalar-SSE
// dataflow (stack slots sNNN mirror the original esp-relative spill slots;
// registers tNN mirror xmm registers). The det check reproduces
// fld/fabs/fld kDetEps/fcompi/fstp/jbe: jbe continues when kDetEps <= |det|
// or when the comparison is unordered, so NaN determinants fall through and
// produce a NaN inverse returning 1 (EA quirk). Output rows are stored with
// aligned 16-byte movaps, matching the original store instructions.
char MatrixInverse(const float* aIn, float* aOut) {

    // Original spill slots (esp-relative dword slots).
    float s170;
    float s174;
    float s178;
    float s17c;
    float s150;
    float s154;
    float s158;
    float s15c;
    float s160;
    float s164;
    float s168;
    float s16c;
    float s0d0;
    float s0d4;
    float s0d8;
    float s0dc;
    float s070;
    float s074;
    float s078;
    float s07c;
    float s0c0;
    float s0c4;
    float s0c8;
    float s0cc;
    float s080;
    float s084;
    float s088;
    float s08c;
    float s140;
    float s144;
    float s148;
    float s14c;
    float s0e0;
    float s0e4;
    float s0e8;
    float s0ec;
    float s100;
    float s104;
    float s108;
    float s10c;
    float s0f0;
    float s0f4;
    float s0f8;
    float s0fc;
    float s020;
    float s024;
    float s028;
    float s02c;
    float s130;
    float s134;
    float s138;
    float s13c;
    float s0b0;
    float s0b4;
    float s0b8;
    float s0bc;
    float s110;
    float s114;
    float s118;
    float s11c;
    float s120;
    float s124;
    float s128;
    float s12c;
    float s03c;
    float s00c;
    float s014;
    float s010;
    float s008;
    float s040;
    float s01c;
    float s018;
    float s0a0;
    float s0a8;
    float s0ac;
    float s0a4;
    float s060;
    float s048;
    float s05c;
    float s054;
    float s044;
    float s058;
    float s064;
    float s04c;
    float s06c;
    float s050;
    float s068;
    float s09c;


    const __m128 t1 = _mm_load_ps(aIn + 0);
    { alignas(16) float sp1[4]; _mm_store_ps(sp1, t1);
      s170 = sp1[0];
      s174 = sp1[1];
      s178 = sp1[2];
      s17c = sp1[3];
    }
    { alignas(16) float sp2[4]; _mm_store_ps(sp2, t1);
      s150 = sp2[0];
      s154 = sp2[1];
      s158 = sp2[2];
      s15c = sp2[3];
    }
    { alignas(16) float sp3[4]; _mm_store_ps(sp3, t1);
      s160 = sp3[0];
      s164 = sp3[1];
      s168 = sp3[2];
      s16c = sp3[3];
    }
    { alignas(16) float sp4[4]; _mm_store_ps(sp4, t1);
      s0d0 = sp4[0];
      s0d4 = sp4[1];
      s0d8 = sp4[2];
      s0dc = sp4[3];
    }
    const __m128 t2 = _mm_load_ps(aIn + 4);
    { alignas(16) float sp5[4]; _mm_store_ps(sp5, t2);
      s070 = sp5[0];
      s074 = sp5[1];
      s078 = sp5[2];
      s07c = sp5[3];
    }
    { alignas(16) float sp6[4]; _mm_store_ps(sp6, t2);
      s0c0 = sp6[0];
      s0c4 = sp6[1];
      s0c8 = sp6[2];
      s0cc = sp6[3];
    }
    { alignas(16) float sp7[4]; _mm_store_ps(sp7, t2);
      s080 = sp7[0];
      s084 = sp7[1];
      s088 = sp7[2];
      s08c = sp7[3];
    }
    { alignas(16) float sp8[4]; _mm_store_ps(sp8, t2);
      s140 = sp8[0];
      s144 = sp8[1];
      s148 = sp8[2];
      s14c = sp8[3];
    }
    const __m128 t3 = _mm_load_ps(aIn + 8);
    { alignas(16) float sp9[4]; _mm_store_ps(sp9, t3);
      s0e0 = sp9[0];
      s0e4 = sp9[1];
      s0e8 = sp9[2];
      s0ec = sp9[3];
    }
    { alignas(16) float sp10[4]; _mm_store_ps(sp10, t3);
      s100 = sp10[0];
      s104 = sp10[1];
      s108 = sp10[2];
      s10c = sp10[3];
    }
    { alignas(16) float sp11[4]; _mm_store_ps(sp11, t3);
      s0f0 = sp11[0];
      s0f4 = sp11[1];
      s0f8 = sp11[2];
      s0fc = sp11[3];
    }
    const __m128 t4 = _mm_load_ss(&s0f4);
    { alignas(16) float sp12[4]; _mm_store_ps(sp12, t3);
      s020 = sp12[0];
      s024 = sp12[1];
      s028 = sp12[2];
      s02c = sp12[3];
    }
    const __m128 t5 = _mm_load_ps(aIn + 12);
    const __m128 t6 = _mm_load_ss(&s02c);
    { alignas(16) float sp13[4]; _mm_store_ps(sp13, t5);
      s130 = sp13[0];
      s134 = sp13[1];
      s138 = sp13[2];
      s13c = sp13[3];
    }
    const __m128 t7 = _mm_load_ss(&s134);
    { alignas(16) float sp14[4]; _mm_store_ps(sp14, t5);
      s0b0 = sp14[0];
      s0b4 = sp14[1];
      s0b8 = sp14[2];
      s0bc = sp14[3];
    }
    const __m128 t8 = _mm_load_ss(&s0b8);
    const __m128 t9 = _mm_mul_ss(t8, t6);
    { alignas(16) float sp15[4]; _mm_store_ps(sp15, t5);
      s110 = sp15[0];
      s114 = sp15[1];
      s118 = sp15[2];
      s11c = sp15[3];
    }
    const __m128 t10 = _mm_load_ss(&s11c);
    { alignas(16) float sp16[4]; _mm_store_ps(sp16, t5);
      s120 = sp16[0];
      s124 = sp16[1];
      s128 = sp16[2];
      s12c = sp16[3];
    }
    const __m128 t11 = _mm_mul_ss(t7, t6);
    _mm_store_ss(&s03c, t11);
    const __m128 t12 = _mm_load_ss(&s108);
    const __m128 t13 = _mm_mul_ss(t10, t12);
    const __m128 t14 = _mm_sub_ss(t13, t9);
    const __m128 t15 = _mm_load_ss(&s0b8);
    const __m128 t16 = _mm_mul_ss(t10, t4);
    const __m128 t17 = _mm_mul_ss(t15, t4);
    const __m128 t18 = _mm_mul_ss(t7, t12);
    const __m128 t19 = _mm_sub_ss(t11, t16);
    const __m128 t20 = _mm_sub_ss(t16, _mm_load_ss(&s03c));
    const __m128 t21 = _mm_mul_ss(t14, _mm_load_ss(&s084));
    _mm_store_ss(&s00c, t19);
    const __m128 t22 = _mm_mul_ss(t19, _mm_load_ss(&s078));
    const __m128 t23 = _mm_sub_ss(t17, t18);
    const __m128 t24 = _mm_load_ss(&s14c);
    const __m128 t25 = _mm_mul_ss(t23, t24);
    const __m128 t26 = _mm_add_ss(t25, t21);
    const __m128 t27 = _mm_load_ss(&s0e0);
    const __m128 t28 = _mm_add_ss(t26, t22);
    _mm_store_ss(&s014, t28);
    const __m128 t29 = _mm_load_ss(&s120);
    const __m128 t30 = _mm_mul_ss(t29, _mm_load_ss(&s108));
    const __m128 t31 = _mm_mul_ss(t29, t6);
    const __m128 t32 = _mm_load_ss(&s11c);
    const __m128 t33 = _mm_mul_ss(t32, t27);
    const __m128 t34 = _mm_sub_ss(t31, t33);
    const __m128 t35 = _mm_load_ss(&s0b8);
    const __m128 t36 = _mm_mul_ss(t35, t27);
    _mm_store_ss(&s010, t14);
    _mm_store_ss(&s008, t36);
    const __m128 t37 = _mm_sub_ss(t36, t30);
    _mm_store_ss(&s040, t34);
    const __m128 t38 = _mm_mul_ss(t34, _mm_load_ss(&s078));
    _mm_store_ss(&s01c, t37);
    const __m128 t39 = _mm_load_ss(&s0c0);
    const __m128 t40 = _mm_mul_ss(t14, t39);
    const __m128 t41 = _mm_mul_ss(t37, t24);
    const __m128 t42 = _mm_add_ss(t41, t38);
    const __m128 t43 = _mm_load_ss(&s120);
    const __m128 t44 = _mm_add_ss(t42, t40);
    const __m128 t45 = _mm_set_ss(kNegZero);
    const __m128 t46 = _mm_sub_ss(t45, t44);
    _mm_store_ss(&s018, t46);
    const __m128 t47 = _mm_load_ss(&s134);
    const __m128 t48 = _mm_mul_ss(t47, _mm_load_ss(&s0e0));
    _mm_store_ss(&s0a0, t20);
    const __m128 t49 = _mm_mul_ss(t43, _mm_load_ss(&s0f4));
    const __m128 t50 = _mm_sub_ss(t30, _mm_load_ss(&s008));
    const __m128 t51 = _mm_sub_ss(t48, t49);
    const __m128 t52 = _mm_load_ss(&s040);
    const __m128 t53 = _mm_mul_ss(t52, _mm_load_ss(&s084));
    const __m128 t54 = _mm_mul_ss(t24, t51);
    const __m128 t55 = _mm_add_ss(t53, t54);
    const __m128 t56 = _mm_mul_ss(t39, t20);
    const __m128 t57 = _mm_load_ss(&s084);
    const __m128 t58 = _mm_mul_ss(t57, t50);
    const __m128 t59 = _mm_add_ss(t55, t56);
    const __m128 t60 = _mm_mul_ss(t23, t39);
    const __m128 t61 = _mm_add_ss(t60, t58);
    const __m128 t62 = _mm_load_ss(&s078);
    const __m128 t63 = _mm_mul_ss(t62, t51);
    const __m128 t64 = _mm_add_ss(t61, t63);
    const __m128 t65 = _mm_set_ss(kNegZero);
    const __m128 t66 = _mm_sub_ss(t65, t64);
    const __m128 t67 = _mm_load_ss(&s0dc);
    _mm_store_ss(&s0a8, t66);
    _mm_store_ss(&s03c, t51);
    const __m128 t68 = _mm_load_ss(&s168);
    const __m128 t69 = _mm_mul_ss(t67, t66);
    const __m128 t70 = _mm_mul_ss(t68, t59);
    const __m128 t71 = _mm_add_ss(t69, t70);
    _mm_store_ss(&s0ac, t50);
    const __m128 t72 = _mm_load_ss(&s154);
    const __m128 t73 = _mm_mul_ss(t72, _mm_load_ss(&s018));
    const __m128 t74 = _mm_add_ss(t71, t73);
    _mm_store_ss(&s0a4, t59);
    const __m128 t75 = _mm_load_ss(&s170);
    const __m128 t76 = _mm_mul_ss(t75, _mm_load_ss(&s014));
    const __m128 t77 = _mm_add_ss(t74, t76);
    _mm_store_ss(&s008, t77);
    const float det = s008;                    // fld [esp+8]
    const float detAbs = std::fabs(det);        // fabs
    // fld kDetEps; fcompi st(1); fstp st(0); jbe continue
    // jbe taken iff kDetEps <= |det| or unordered (NaN proceeds: EA quirk)
    if (!(kDetEps <= detAbs) && detAbs == detAbs) {
        return 0;
    }
    const __m128 t78 = _mm_set_ss(kOne);
    const __m128 t79 = _mm_div_ss(t78, t77);
    const __m128 t80 = _mm_load_ss(&s02c);
    const __m128 t81 = _mm_mul_ss(t80, _mm_load_ss(&s078));
    const __m128 t82 = _mm_mul_ss(t67, t79);
    const __m128 t83 = _mm_mul_ss(t75, t79);
    _mm_store_ss(&s060, t83);
    const __m128 t84 = _mm_load_ss(&s108);
    const __m128 t85 = _mm_mul_ss(t84, t24);
    const __m128 t86 = _mm_sub_ss(t81, t85);
    const __m128 t87 = _mm_load_ss(&s084);
    _mm_store_ss(&s048, t86);
    const __m128 t88 = _mm_load_ss(&s0f4);
    const __m128 t89 = _mm_mul_ss(t88, t24);
    _mm_store_ss(&s05c, t89);
    const __m128 t90 = _mm_load_ss(&s02c);
    const __m128 t91 = _mm_mul_ss(t90, t87);
    _mm_store_ss(&s054, t91);
    const __m128 t92 = _mm_load_ss(&s108);
    const __m128 t93 = _mm_mul_ss(t92, t87);
    _mm_store_ss(&s008, t93);
    const __m128 t94 = _mm_load_ss(&s0f4);
    const __m128 t95 = _mm_mul_ss(t94, _mm_load_ss(&s078));
    const __m128 t96 = _mm_load_ss(&s008);
    const __m128 t97 = _mm_sub_ss(t96, t95);
    const __m128 t98 = _mm_load_ss(&s134);
    const __m128 t99 = _mm_mul_ss(t98, t24);
    _mm_store_ss(&s044, t99);
    const __m128 t100 = _mm_load_ss(&s11c);
    const __m128 t101 = _mm_mul_ss(t100, _mm_load_ss(&s084));
    _mm_store_ss(&s058, t97);
    const __m128 t102 = _mm_load_ss(&s0b8);
    const __m128 t103 = _mm_mul_ss(t102, t24);
    const __m128 t104 = _mm_load_ss(&s0b8);
    const __m128 t105 = _mm_mul_ss(t104, _mm_load_ss(&s084));
    _mm_store_ss(&s064, t101);
    const __m128 t106 = _mm_load_ss(&s11c);
    const __m128 t107 = _mm_mul_ss(t106, _mm_load_ss(&s078));
    const __m128 t108 = _mm_sub_ss(t107, t103);
    _mm_store_ss(&s04c, t108);
    const __m128 t109 = _mm_load_ss(&s134);
    const __m128 t110 = _mm_mul_ss(t109, _mm_load_ss(&s078));
    const __m128 t111 = _mm_sub_ss(t105, t110);
    _mm_store_ss(&s008, t111);
    const __m128 t112 = _mm_load_ss(&s05c);
    const __m128 t113 = _mm_sub_ss(t112, _mm_load_ss(&s054));
    const __m128 t114 = _mm_mul_ss(t68, t79);
    const __m128 t115 = _mm_mul_ss(t113, t114);
    const __m128 t116 = _mm_mul_ss(t72, t79);
    const __m128 t117 = _mm_mul_ss(t82, _mm_load_ss(&s058));
    const __m128 t118 = _mm_add_ss(t115, t117);
    const __m128 t119 = _mm_mul_ss(t116, _mm_load_ss(&s048));
    const __m128 t120 = _mm_add_ss(t118, t119);
    const __m128 t121 = _mm_set_ss(kNegZero);
    const __m128 t122 = _mm_sub_ss(t121, t120);
    const __m128 t123 = _mm_load_ss(&s044);
    const __m128 t124 = _mm_sub_ss(t123, _mm_load_ss(&s064));
    _mm_store_ss(&s0dc, t122);
    const __m128 t125 = _mm_load_ss(&s008);
    const __m128 t126 = _mm_mul_ss(t124, t114);
    const __m128 t127 = _mm_mul_ss(t125, t82);
    const __m128 t128 = _mm_add_ss(t126, t127);
    const __m128 t129 = _mm_load_ss(&s04c);
    const __m128 t130 = _mm_mul_ss(t129, t116);
    const __m128 t131 = _mm_add_ss(t128, t130);
    _mm_store_ss(&s0d8, t131);
    const __m128 t132 = _mm_load_ss(&s010);
    const __m128 t133 = _mm_mul_ss(t132, t116);
    const __m128 t134 = _mm_mul_ss(t23, t82);
    const __m128 t135 = _mm_add_ss(t134, t133);
    const __m128 t136 = _mm_load_ss(&s00c);
    const __m128 t137 = _mm_mul_ss(t136, t114);
    const __m128 t138 = _mm_add_ss(t135, t137);
    const __m128 t139 = _mm_sub_ss(t121, t138);
    _mm_store_ss(&s0d4, t139);
    const __m128 t140 = _mm_mul_ss(t79, _mm_load_ss(&s014));
    const __m128 t141 = _mm_load_ss(&s0c0);
    const __m128 t142 = _mm_load_ss(&s02c);
    const __m128 t143 = _mm_mul_ss(t142, t141);
    _mm_store_ss(&s0d0, t140);
    const __m128 t144 = _mm_set_ps(s0dc, s0d8, s0d4, s0d0);
    _mm_store_ps(aOut + 0, t144);  // movaps, 16-byte aligned
    const __m128 t145 = _mm_load_ss(&s0e0);
    const __m128 t146 = _mm_mul_ss(t145, _mm_load_ss(&s14c));
    const __m128 t147 = _mm_sub_ss(t146, t143);
    const __m128 t148 = _mm_load_ss(&s108);
    const __m128 t149 = _mm_mul_ss(t148, t141);
    _mm_store_ss(&s06c, t149);
    const __m128 t150 = _mm_load_ss(&s0e0);
    const __m128 t151 = _mm_mul_ss(t150, _mm_load_ss(&s078));
    _mm_store_ss(&s050, t151);
    const __m128 t152 = _mm_load_ss(&s120);
    const __m128 t153 = _mm_mul_ss(t152, _mm_load_ss(&s14c));
    _mm_store_ss(&s00c, t153);
    const __m128 t154 = _mm_load_ss(&s11c);
    const __m128 t155 = _mm_mul_ss(t154, t141);
    const __m128 t156 = _mm_load_ss(&s00c);
    const __m128 t157 = _mm_sub_ss(t156, t155);
    const __m128 t158 = _mm_load_ss(&s0b8);
    const __m128 t159 = _mm_mul_ss(t158, _mm_load_ss(&s0c0));
    _mm_store_ss(&s00c, t159);
    const __m128 t160 = _mm_load_ss(&s120);
    const __m128 t161 = _mm_mul_ss(t160, _mm_load_ss(&s078));
    _mm_store_ss(&s068, t161);
    const __m128 t162 = _mm_load_ss(&s06c);
    const __m128 t163 = _mm_sub_ss(t162, _mm_load_ss(&s050));
    const __m128 t164 = _mm_mul_ss(t163, t82);
    _mm_store_ss(&s014, t157);
    const __m128 t165 = _mm_load_ss(&s00c);
    const __m128 t166 = _mm_sub_ss(t165, _mm_load_ss(&s068));
    _mm_store_ss(&s09c, t147);
    const __m128 t167 = _mm_mul_ss(t147, t114);
    const __m128 t168 = _mm_add_ss(t164, t167);
    const __m128 t169 = _mm_load_ss(&s060);
    const __m128 t170 = _mm_mul_ss(t169, _mm_load_ss(&s048));
    const __m128 t171 = _mm_add_ss(t168, t170);
    const __m128 t172 = _mm_load_ss(&s014);
    _mm_store_ss(&s02c, t171);
    const __m128 t173 = _mm_load_ss(&s04c);
    const __m128 t174 = _mm_mul_ss(t166, t82);
    const __m128 t175 = _mm_mul_ss(t172, t114);
    const __m128 t176 = _mm_add_ss(t174, t175);
    const __m128 t177 = _mm_load_ss(&s060);
    const __m128 t178 = _mm_mul_ss(t173, t177);
    const __m128 t179 = _mm_add_ss(t176, t178);
    const __m128 t180 = _mm_set_ss(kNegZero);
    const __m128 t181 = _mm_sub_ss(t180, t179);
    const __m128 t182 = _mm_load_ss(&s040);
    _mm_store_ss(&s028, t181);
    const __m128 t183 = _mm_load_ss(&s01c);
    const __m128 t184 = _mm_mul_ss(t183, t82);
    const __m128 t185 = _mm_mul_ss(t182, t114);
    const __m128 t186 = _mm_add_ss(t184, t185);
    const __m128 t187 = _mm_load_ss(&s010);
    const __m128 t188 = _mm_mul_ss(t187, t177);
    const __m128 t189 = _mm_add_ss(t186, t188);
    _mm_store_ss(&s024, t189);
    const __m128 t190 = _mm_mul_ss(t79, _mm_load_ss(&s018));
    _mm_store_ss(&s020, t190);
    const __m128 t191 = _mm_set_ps(s02c, s028, s024, s020);
    _mm_store_ps(aOut + 4, t191);  // movaps, 16-byte aligned
    const __m128 t192 = _mm_load_ss(&s0f4);
    const __m128 t193 = _mm_mul_ss(t192, _mm_load_ss(&s0c0));
    _mm_store_ss(&s01c, t193);
    const __m128 t194 = _mm_load_ss(&s0e0);
    const __m128 t195 = _mm_mul_ss(t194, _mm_load_ss(&s084));
    const __m128 t196 = _mm_load_ss(&s01c);
    const __m128 t197 = _mm_sub_ss(t196, t195);
    const __m128 t198 = _mm_load_ss(&s134);
    const __m128 t199 = _mm_mul_ss(t198, _mm_load_ss(&s0c0));
    _mm_store_ss(&s01c, t199);
    const __m128 t200 = _mm_load_ss(&s120);
    const __m128 t201 = _mm_mul_ss(t200, _mm_load_ss(&s084));
    _mm_store_ss(&s018, t197);
    const __m128 t202 = _mm_load_ss(&s01c);
    const __m128 t203 = _mm_sub_ss(t202, t201);
    const __m128 t204 = _mm_load_ss(&s054);
    const __m128 t205 = _mm_sub_ss(t204, _mm_load_ss(&s05c));
    const __m128 t206 = _mm_mul_ss(t205, t177);
    _mm_store_ss(&s010, t203);
    const __m128 t207 = _mm_load_ss(&s018);
    const __m128 t208 = _mm_mul_ss(t207, t82);
    const __m128 t209 = _mm_add_ss(t206, t208);
    const __m128 t210 = _mm_load_ss(&s09c);
    const __m128 t211 = _mm_mul_ss(t210, t116);
    const __m128 t212 = _mm_add_ss(t209, t211);
    const __m128 t213 = _mm_set_ss(kNegZero);
    const __m128 t214 = _mm_sub_ss(t213, t212);
    const __m128 t215 = _mm_load_ss(&s064);
    const __m128 t216 = _mm_sub_ss(t215, _mm_load_ss(&s044));
    const __m128 t217 = _mm_mul_ss(t216, t177);
    _mm_store_ss(&s02c, t214);
    const __m128 t218 = _mm_load_ss(&s010);
    const __m128 t219 = _mm_mul_ss(t218, t82);
    const __m128 t220 = _mm_mul_ss(t82, _mm_load_ss(&s03c));
    const __m128 t221 = _mm_add_ss(t217, t219);
    const __m128 t222 = _mm_load_ss(&s014);
    const __m128 t223 = _mm_mul_ss(t222, t116);
    const __m128 t224 = _mm_add_ss(t221, t223);
    const __m128 t225 = _mm_set_ss(kNegZero);
    _mm_store_ss(&s028, t224);
    const __m128 t226 = _mm_load_ss(&s040);
    const __m128 t227 = _mm_mul_ss(t226, t116);
    const __m128 t228 = _mm_add_ss(t227, t220);
    const __m128 t229 = _mm_mul_ss(t177, _mm_load_ss(&s0a0));
    const __m128 t230 = _mm_add_ss(t228, t229);
    const __m128 t231 = _mm_sub_ss(t225, t230);
    const __m128 t232 = _mm_load_ss(&s018);
    _mm_store_ss(&s024, t231);
    const __m128 t233 = _mm_mul_ss(t79, _mm_load_ss(&s0a4));
    const __m128 t234 = _mm_mul_ss(t79, _mm_load_ss(&s0a8));
    _mm_store_ss(&s020, t233);
    const __m128 t235 = _mm_set_ps(s02c, s028, s024, s020);
    _mm_store_ps(aOut + 8, t235);  // movaps, 16-byte aligned
    const __m128 t236 = _mm_load_ss(&s050);
    const __m128 t237 = _mm_sub_ss(t236, _mm_load_ss(&s06c));
    const __m128 t238 = _mm_mul_ss(t237, t116);
    const __m128 t239 = _mm_mul_ss(t232, t114);
    const __m128 t240 = _mm_add_ss(t238, t239);
    const __m128 t241 = _mm_mul_ss(t177, _mm_load_ss(&s058));
    const __m128 t242 = _mm_add_ss(t240, t241);
    const __m128 t243 = _mm_load_ss(&s010);
    _mm_store_ss(&s02c, t242);
    const __m128 t244 = _mm_load_ss(&s068);
    const __m128 t245 = _mm_sub_ss(t244, _mm_load_ss(&s00c));
    const __m128 t246 = _mm_mul_ss(t243, t114);
    const __m128 t247 = _mm_mul_ss(t114, _mm_load_ss(&s03c));
    const __m128 t248 = _mm_mul_ss(t245, t116);
    const __m128 t249 = _mm_mul_ss(t116, _mm_load_ss(&s0ac));
    const __m128 t250 = _mm_add_ss(t248, t246);
    const __m128 t251 = _mm_load_ss(&s008);
    const __m128 t252 = _mm_mul_ss(t23, t177);
    const __m128 t253 = _mm_mul_ss(t251, t177);
    const __m128 t254 = _mm_add_ss(t250, t253);
    const __m128 t255 = _mm_add_ss(t252, t249);
    const __m128 t256 = _mm_sub_ss(t225, t254);
    const __m128 t257 = _mm_add_ss(t255, t247);
    _mm_store_ss(&s028, t256);
    _mm_store_ss(&s024, t257);
    _mm_store_ss(&s020, t234);
    const __m128 t258 = _mm_set_ps(s02c, s028, s024, s020);
    _mm_store_ps(aOut + 12, t258);  // movaps, 16-byte aligned
    return 1;
}
} // namespace ts3::math::m04m01

