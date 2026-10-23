#include "ts3/animation/clu_60ffb0_Animation.h"

namespace ts3::animation {

// Original 0x0060FFB0 (thiscall, ret 4, returns EAX = the out pointer):
//   P   = *(this + 0xE0)
//   out[0] = P[1]
//   out[1] = P[2]
//   out[2] = P[1] + P[3]
//   out[3] = P[2] + P[4]
// The two adds are the original's integer the original instruction sequence / the original instruction sequence (dword
// adds on the raw bits); the caller later interprets the results as floats.
Box* BoundsOwner::GetBounds(Box* out)
{
    const Rect* r = rect;
    out->x0 = r->x;
    out->y0 = r->y;
    out->x1 = r->x + r->w;
    out->y1 = r->y + r->h;
    return out;
}

}  // namespace ts3::animation
