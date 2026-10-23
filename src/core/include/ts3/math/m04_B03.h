#pragma once
#include <cstdint>
namespace ts3::math::b03 {
// TS3W.exe 0x00D408A0: compute heading from XY, zeroing output below the L1 threshold.
bool Heading(void*, float x, float y, float* out);
// TS3W.exe 0x00D432F0: heading variant with an unused leading argument.
bool Heading4(void*, uint32_t unused, float x, float y, float* out);
// TS3W.exe 0x00925590: advance the three-component incremental radical-inverse sequence.
uint32_t Sequence3(void* state);
// TS3W.exe 0x009257A0: advance the two-component incremental radical-inverse sequence.
uint32_t Sequence2(void* state);
// TS3W.exe 0x0068B990: copy two aligned vectors in order, returning the first input.
const void* Set(void* dst, const void* lo, const void* hi);
// TS3W.exe 0x0068B9B0: write the four-lane midpoint, returning the output.
void* Center(const void* box, void* out);
// TS3W.exe 0x0068B9E0: sort each bounds lane with SSE second-operand NaN/zero semantics.
void Order(void* box);
// TS3W.exe 0x0068BA30: extend bounds by a box, returning the source box.
const void* Include(void* box, const void* other);
// TS3W.exe 0x0068BA60: intersect XYZ bounds, leaving output unchanged on strict separation.
bool Intersect(void* out, const void* a, const void* b);
// TS3W.exe 0x0068BAC0: include a point, reloading it after the minimum store.
const void* IncludePoint(void* box, const void* point);
// TS3W.exe 0x0068BB50: initialize empty bounds with finite +/-16777216 in all four lanes.
void Empty(void* box);
// TS3W.exe 0x0068BBB0: write center then radius, reloading bounds after the center store.
void* Sphere(const void* box, void* center, float* radius);
// TS3W.exe 0x0068BC20: convert to center/radius and half extents, preserving source reload aliasing.
const void* CenterExtents(void* out, const void* box);
// TS3W.exe 0x0068BF80: affine transform bounds using max(-axis,axis), returning the matrix.
const void* Transform(void* out, const void* box, const void* matrix);
// Registration-only member adapters: express the original ECX ABI without
// embedding any x86 calling convention in the portable value kernels above.
struct HookOwner {
    bool Heading(float x,float y,void* out) { return b03::Heading(this,x,y,static_cast<float*>(out)); }
    bool Heading4(uint32_t u,float x,float y,void* out) { return b03::Heading4(this,u,x,y,static_cast<float*>(out)); }
    uint32_t Sequence2() { return b03::Sequence2(this); }
    uint32_t Sequence3() { return b03::Sequence3(this); }
    void* Set(const void* a,const void* b) { return const_cast<void*>(b03::Set(this,a,b)); }
    void* Center(void* out) { return b03::Center(this,out); }
    void Order() { b03::Order(this); }
    void* Include(const void* b) { return const_cast<void*>(b03::Include(this,b)); }
    bool Intersect(const void* a,const void* b) { return b03::Intersect(this,a,b); }
    void* IncludePoint(const void* p) { return const_cast<void*>(b03::IncludePoint(this,p)); }
    void Empty() { b03::Empty(this); }
    void* Sphere(void* c,void* r) { return b03::Sphere(this,c,static_cast<float*>(r)); }
    void* CenterExtents(const void* b) { return const_cast<void*>(b03::CenterExtents(this,b)); }
    void* Transform(const void* b,const void* m) { return const_cast<void*>(b03::Transform(this,b,m)); }
};
}
