#pragma once
#include <cstdint>
#include <xmmintrin.h>
namespace ts3::math::b12 {
// TS3W.exe 0x008E47B0: Extracts heading and elevation. Keeps _CIasin EAX residue on the normal path, heading pointer on the axial path.
float* Angles(float* a1, float* a2, float* a3);
// TS3W.exe 0x008E4EC0: Projects a point in place and returns the matrix pointer.
float* Project(float* a1, float* a2);
// TS3W.exe 0x0091CD80: Selects cube face and UV coordinates, keeping tie precedence.
float* CubeUV(float* a1, int* a2, float* a3, float* a4);
// TS3W.exe 0x009258F0: Selects a signed dominant axis, keeping Z tie precedence.
int CubeFace(float a1, float a2, float a3);
// TS3W.exe 0x007D29D0: Builds a quaternion between two directions.
__m128* Between(__m128* a1, __m128* a2, __m128* a3);
// TS3W.exe 0x00829600: Rotates a vector by a quaternion, keeping alias reload order.
float* Rotate(float* a1, float* a2, float* a3);
// TS3W.exe 0x00858610: Builds a direction quaternion with a squared-length threshold of 0.001. Keeps two RSQRT refinements and unchanged output on rejection.
bool DirectionThreshold(__m128* a2, __m128* a3);
// TS3W.exe 0x00860460: Builds a direction quaternion with a zero squared-length threshold. Keeps two RSQRT refinements and unchanged output on rejection.
bool Direction(__m128* a1, __m128* a2);
// TS3W.exe 0x008CF3A0: Converts a quaternion to a packed scalar 3x3.
float* ToMatrix(float* a1, float* a2);
// TS3W.exe 0x008E4500: Converts a packed scalar 3x3 to a quaternion with a trace threshold.
float* FromMatrix(float* a1, float* a2, float a3);
// TS3W.exe 0x0092A690: Multiplies quaternions, keeping the early Z store and subsequent reloads.
float* Multiply(float* a1, float* a2, float* a3);
// TS3W.exe 0x00967E00: Stores three basis rows at owner offsets 0x620 through 0x640 and returns the second input.
__m128* OwnerBasis(__m128* self, __m128* a2, __m128* a3);
struct HookOwner {
    void* Basis(void* a,void* b) { return OwnerBasis(reinterpret_cast<__m128*>(this),static_cast<__m128*>(a),static_cast<__m128*>(b)); }
};
}
