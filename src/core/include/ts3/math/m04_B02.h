#pragma once
#include <cstdint>
namespace ts3::math::b02 {
struct CosineTable { float scale, step; float pairs[32]; };
// TS3W.exe 0x008E4450: Evaluate the runtime cosine table with magic-bias range reduction.
double ApproxCos(float x, const CosineTable& table);
#if defined(_M_IX86)
// TS3W.exe 0x008E4450: Bind the evaluator to the game-owned runtime table without adding an ST0 spill.
double GameApproxCos(float x);
#endif
// TS3W.exe 0x008E4700: Approximate atan2 with the original SSE polynomial and unordered branches.
double ApproxAtan(float y, float x);
// TS3W.exe 0x00919510: Clamp by the original x87 absolute-value expression.
double Clamp(float x, float lo, float hi);
// TS3W.exe 0x0092C650: Evaluate clamped smoothstep in x87 order, including zero-width division.
double Smooth(float x, float lo, float hi);
// TS3W.exe 0x00A53440: Compute an unsigned-integer ellipse perimeter approximation.
double Ellipse(uint32_t a, uint32_t b);
// TS3W.exe 0x00AF8390: Transform four integers into byte outputs using FISTP rounding and byte wrap.
uint8_t* ByteWeights(int32_t a,int32_t b,int32_t c,int32_t d,uint8_t* p,uint8_t* q,uint8_t* r,uint8_t* s);
// TS3W.exe 0x00AF8450: Transform four floats into four half-sum differences, returning the last output.
float* Weights(float a,float b,float c,float d,float* p,float* q,float* r,float* s);
// TS3W.exe 0x00AFD780: Quantize an angle with FISTP and signed remainder eight.
double Octant(float x);
// TS3W.exe 0x00B163C0: Test a strict open tolerance interval.
int Near(float a,float b);
// TS3W.exe 0x00B3C7E0: Round with FISTP, then optionally adjust toward floor or ceiling outside a dead zone.
double __stdcall Snap(char enabled,char up,float x);
// TS3W.exe 0x00B95D20: Test all three coordinates against inclusive symmetric limits.
int Within(const float* xyz,float margin);
// TS3W.exe 0x00C89320: Quantize an angle on a thousand-step turn using FISTP.
double TurnStep(float x);
// TS3W.exe 0x00CC9430: Compute wrapped angular distance with SSE truncation and x87 subtraction.
double AngleDistance(float a,float b);
// TS3W.exe 0x00CFA620: Write a heading from FPATAN, or zero below the L1 threshold; returns AL only.
char __stdcall Heading(int unused,float x,float y,float* out);
}
