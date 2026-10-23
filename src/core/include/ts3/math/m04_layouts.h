#pragma once
#include "ts3/math/float_values.h"
#include "ts3/math/m04_box_union.h"
#include "ts3/math/m04_plane_classifier.h"
#include "ts3/misc/clu_6795d0_x.h"
#include "ts3/ui/clu_50b5f0_UI.h"

namespace ts3::math::m04 {
// Scalar XYZ is intentionally not SIMD padded. No universal Quaternion alias:
// quaternion lanes in the proven SrtValue use Float4Value storage.
struct Vector3 { float x, y, z; };
struct Matrix33 { float element[3][3]; };
struct Plane { float x, y, z, d; };
// Minimum used region, NOT a claim about the complete original frustum class.
struct alignas(16) ExtendedFrustumStorage {
    unsigned char coefficients[0x380];
    Float4Value corners[8];
    Float4Value cachedMin, cachedMax;
};
static_assert(sizeof(Vector3)==12 && sizeof(Matrix33)==36 && sizeof(Plane)==16);
static_assert(sizeof(ExtendedFrustumStorage)==0x420 && alignof(ExtendedFrustumStorage)==16);
TS3_X86_SIZE(Vector3,12);
TS3_X86_OFFSET(Vector3,y,4);
TS3_X86_OFFSET(Vector3,z,8);
TS3_X86_SIZE(Matrix33,36);
TS3_X86_OFFSET(Matrix33,element,0);
TS3_X86_SIZE(Plane,16);
TS3_X86_OFFSET(Plane,d,12);
TS3_X86_SIZE(ExtendedFrustumStorage,0x420);
TS3_X86_OFFSET(ExtendedFrustumStorage,corners,0x380);
TS3_X86_OFFSET(ExtendedFrustumStorage,cachedMin,0x400);
TS3_X86_OFFSET(ExtendedFrustumStorage,cachedMax,0x410);
static_assert(sizeof(Float4Value)==16 && sizeof(SrtValue)==48 && sizeof(Matrix44Value)==64);
static_assert(sizeof(UnionBox)==32 && sizeof(scratch::ScratchClipTest689D10)==704);
static_assert(sizeof(misc::clu6795::Point)==8 && sizeof(misc::clu6795::Record)==16);
TS3_X86_OFFSET(Float4Value,lane,0);
TS3_X86_OFFSET(Matrix44Value,vector,0);
TS3_X86_OFFSET(UnionBox,second,16);
// Tagged UI and polygon container remain their canonical owner types; neither
// is redefined here. Native pointers in Points stay pointer-sized on x64.
}
