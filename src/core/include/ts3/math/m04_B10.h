#pragma once
#include <cstdint>
namespace ts3::math::b10 {
// TS3W.exe 0x0071ADE0: RotateX value kernel; original grouping and mutation order retained.
void RotateX(float* self, float a2);
// TS3W.exe 0x0071AEE0: Export44 value kernel; original grouping and mutation order retained.
float* Export44(float* self, float* a2);
// TS3W.exe 0x008C4440: Inverse point; retains incoming EAX except AH replaced by scale-compare LAHF.
uintptr_t InversePoint(float* self, float* a2, uintptr_t incomingEax=0);
// TS3W.exe 0x008C4540: MatrixDifferent value kernel; original grouping and mutation order retained.
int MatrixDifferent(float* a1, float* a2);
// TS3W.exe 0x008C4CD0: TransformPoint value kernel; original grouping and mutation order retained.
float* TransformPoint(float* self, float* a2);
// TS3W.exe 0x008C7F90: RotateY value kernel; original grouping and mutation order retained.
void RotateY(float* self, float a2);
// TS3W.exe 0x008C8090: RotateZ value kernel; original grouping and mutation order retained.
void RotateZ(float* self, float a2);
// TS3W.exe 0x008C8470: PostCompose value kernel; original grouping and mutation order retained.
uintptr_t PostCompose(float* self, float* a2);
// TS3W.exe 0x008CF5A0: PreCompose value kernel; original grouping and mutation order retained.
uintptr_t PreCompose(float* self, float* a2);
// TS3W.exe 0x008D9C40: InverseVector value kernel; original grouping and mutation order retained.
float* InverseVector(float* self, float* a2);
// TS3W.exe 0x008DA360: TransformVector value kernel; original grouping and mutation order retained.
float* TransformVector(float* self, float* a2);
// TS3W.exe 0x008E4FB0: Transpose vector; EAX is incoming when rotation is disabled, otherwise the vector pointer.
uintptr_t TransposeVector(float* self, float* a2, uintptr_t incomingEax=0);
// TS3W.exe 0x008F6630: AppendMatrix value kernel; original grouping and mutation order retained.
float* AppendMatrix(float* self, float* a2);
// TS3W.exe 0x0091C4B0: MovePivot value kernel; original grouping and mutation order retained.
float* MovePivot(float* self, float* a2);
// TS3W.exe 0x0092C430: BasisFromAxis value kernel; original grouping and mutation order retained.
void BasisFromAxis(float* a1, float* a2);
#if defined(_M_IX86)
// TS3W.exe 0x0071ADE0: ABI adapter for RotateX.
void __fastcall RotateXEntry(float* self, void*, float a2);
// TS3W.exe 0x0071AEE0: ABI adapter for Export44.
float* __fastcall Export44Entry(float* self, void*, float* a2);
// TS3W.exe 0x008C4440: ABI adapter for InversePoint.
uintptr_t __fastcall InversePointEntry(float* self, void*, float* a2);
// TS3W.exe 0x008C4CD0: ABI adapter for TransformPoint.
float* __fastcall TransformPointEntry(float* self, void*, float* a2);
// TS3W.exe 0x008C7F90: ABI adapter for RotateY.
void __fastcall RotateYEntry(float* self, void*, float a2);
// TS3W.exe 0x008C8090: ABI adapter for RotateZ.
void __fastcall RotateZEntry(float* self, void*, float a2);
// TS3W.exe 0x008C8470: ABI adapter for PostCompose.
uintptr_t __fastcall PostComposeEntry(float* self, void*, float* a2);
// TS3W.exe 0x008CF5A0: ABI adapter for PreCompose.
uintptr_t __fastcall PreComposeEntry(float* self, void*, float* a2);
// TS3W.exe 0x008D9C40: ABI adapter for InverseVector.
float* __fastcall InverseVectorEntry(float* self, void*, float* a2);
// TS3W.exe 0x008DA360: ABI adapter for TransformVector.
float* __fastcall TransformVectorEntry(float* self, void*, float* a2);
// TS3W.exe 0x008E4FB0: ABI adapter for TransposeVector.
uintptr_t __fastcall TransposeVectorEntry(float* self, void*, float* a2);
// TS3W.exe 0x008F6630: ABI adapter for AppendMatrix.
float* __fastcall AppendMatrixEntry(float* self, void*, float* a2);
// TS3W.exe 0x0091C4B0: ABI adapter for MovePivot.
float* __fastcall MovePivotEntry(float* self, void*, float* a2);
#endif
}
