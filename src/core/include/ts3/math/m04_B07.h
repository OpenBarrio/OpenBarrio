#pragma once
namespace ts3::math::b07 {
// TS3W.exe 0x006E9740: Reflects four lanes using an XYZ normal with W cleared after the dot.
float* Reflect(float* out, const float* a, const float* normal);
// TS3W.exe 0x00709B00: Decodes reversed RGB bytes and transforms all four lanes; W is retained as input.
float* Decode(float* out, const unsigned char* packed);
// TS3W.exe 0x0071B020: Normalizes XYZ using x87 and an additive epsilon.
float* Normalize(float* out, const float* a);
// TS3W.exe 0x0076C2A0: Copies XYZ sequentially and preserves destination W.
float* Copy3(const float* a, float* out);
// TS3W.exe 0x0076C300: Copies four lanes sequentially with observable overlap.
float* Copy4(const float* a, float* out);
// TS3W.exe 0x007B8010: Returns one for any unequal or unordered XYZ comparison.
int Different(const float* a, const float* b);
// TS3W.exe 0x008C3FA0: Computes the x87 dot product in Z,Y,X order.
double Dot(const float* a, const float* b);
// TS3W.exe 0x0091CB70: Selects XYZ minima, preferring A for ties and unordered comparisons.
float* Min3(float* out, const float* a, const float* b);
// TS3W.exe 0x0091CBD0: Selects XYZ maxima, preferring A for ties and unordered comparisons.
float* Max3(float* out, const float* a, const float* b);
// TS3W.exe 0x0091CC30: Selects four minima, retaining selected source pointers before stores.
float* Min4(float* out, const float* a, const float* b);
// TS3W.exe 0x0091CCA0: Selects four maxima, retaining selected source pointers before stores.
float* Max4(float* out, const float* a, const float* b);
// TS3W.exe 0x0091EC30: Normalizes XYZ with epsilon and callee stack cleanup.
float* __stdcall NormalizeStd(float* out, const float* a);
// TS3W.exe 0x0091F720: Scales XYZ by reciprocal squared length, not reciprocal length.
float* __stdcall InverseSquare(float* out, const float* a);
// Register-input entry: EAX destination, ECX packed bytes.
#if defined(_M_IX86)
void DecodeEntry();
int __fastcall DifferentEntry(const float* a, void*, const float* b);
#endif
}
