#include "ts3/routing/clu_6c8900_Routing.h"
#include <cmath>
#include <cstring>
#include <limits>

namespace ts3::routing {
namespace {
Vec4 Add(Vec4 a, Vec4 b) { return {a.x+b.x,a.y+b.y,a.z+b.z,a.w+b.w}; }
Vec4 Sub(Vec4 a, Vec4 b) { return {a.x-b.x,a.y-b.y,a.z-b.z,a.w-b.w}; }
Vec4 Mul(Vec4 a, float b) { return {a.x*b,a.y*b,a.z*b,a.w*b}; }
Vec4 Transform(const Matrix& m, Vec4 v) { return Add(Add(Add(Mul(m.row[0],v.x),Mul(m.row[1],v.y)),Mul(m.row[2],v.z)),Mul(m.row[3],v.w)); }
uint32_t Bits(float v) { uint32_t b; std::memcpy(&b,&v,4); return b; }
float Select(float difference, float negative, float positive) { return Bits(difference)&0x80000000u ? negative:positive; }
uint32_t Integer(float v) { if (!std::isfinite(v) || static_cast<double>(v)>=9223372036854775808.0 || static_cast<double>(v)<-9223372036854775808.0) return 0; return static_cast<uint32_t>(static_cast<int64_t>(v)); }
}
// 0x006C8900. Constants dumped from FF59F8 (3EC90FDB), 107A538 (3F800000).
DebugLineVertex* DrawCircle(DebugLineBuffer* self, const Matrix& matrix, float radius, uint32_t color, bool diameters)
{
    static const auto points = [] {
        struct Table { Vec4 p[16]; } table{};
        for (unsigned i=0;i<16;++i) {
            const float angle=static_cast<float>(static_cast<double>(i+1)*static_cast<double>(0x1.921fb6p-2f));
            table.p[i]={static_cast<float>(std::sin(static_cast<double>(angle))),static_cast<float>(std::cos(static_cast<double>(angle))),0,0};
        }
        return table;
    }();
    const Vec4* circlePoints=points.p;
#if defined(_M_IX86)
    auto& flags=*reinterpret_cast<uint32_t*>(0x11D1394);
    flags|=1;
    auto& initialized=*reinterpret_cast<uint8_t*>(0x11D1390);
    auto* gamePoints=reinterpret_cast<Vec4*>(0x11D1290);
    if (!initialized) { std::memcpy(gamePoints,points.p,sizeof points.p); initialized=1; }
    circlePoints=gamePoints;
#endif
    alignas(16) Vec4 previous=Transform(matrix,{0*radius,0*radius,radius,1});
    alignas(16) Vec4 cardinal[4];
    for (unsigned i=0;i<16;++i) {
        alignas(16) Vec4 next=Transform(matrix,{circlePoints[i].x*radius,0*radius,circlePoints[i].y*radius,1});
        self->AddSegment(&previous,&next,color,color);
        if (diameters && (i+1)%4==0) cardinal[i/4]=next;
        previous=next;
    }
    if (diameters) { self->AddSegment(cardinal,cardinal+2,color,color); return self->AddSegment(cardinal+1,cardinal+3,color,color); }
    return nullptr; // Original returns its expired stack-local cardinal end.
}
// 0x006C9270, thiscall ret 16.
DebugLineVertex* DrawCircleAt(DebugLineBuffer* self, const Vec4* point, float radius, uint32_t color, bool diameters)
{
    Matrix matrix{{{1,0,0,0},{0,1,0,0},{0,0,1,0},*point}};
    return DrawCircle(self,matrix,radius,color,diameters);
}
// 0x00860FE0; FBB10C=3E2AAAAB, F9A5AC=3F000000, F988F0=80000000.
DebugLineVertex* DrawArrow(DebugLineBuffer* self, const Vec4* point, const Vec4* direction, uint32_t color, float length, float width)
{
    if (!self) return nullptr; // Original stale EAX, no meaningful return contract.
    Vec4 d=*direction;
    Vec4 axis{d.y*0.0f-d.z*1.0f,d.z*0.0f-d.x*0.0f,d.x*1.0f-d.y*0.0f,d.x*0.0f-d.x*0.0f};
    Vec4 negative=Sub({0,0,0,0},d);
    alignas(16) Vec4 tail=Add(Mul(negative,length*0x1.555556p-3f),*point);
    Vec4 half=Mul(negative,length*0.5f);
    alignas(16) Vec4 tip=Sub(*point,half);
    Vec4 base=Add(half,*point);
    alignas(16) Vec4 left=Add(Mul(axis,-0.0f-width*0.5f),base);
    alignas(16) Vec4 right=Add(Mul(axis,width*0.5f),base);
    self->AddSegment(&tip,&left,color,color); self->AddSegment(&tip,&right,color,color);
    self->AddSegment(&tail,&left,color,color); return self->AddSegment(&tail,&right,color,color);
}
// 0x0074C030. Preserve bit-sign selects and maximum initialized to zero.
DebugLineVertex* DrawSamples(const Vec4&, DebugLineBuffer* self, const Sample* samples, uint32_t count, const Vec4& high, const Vec4& middle, const Vec4& low)
{
    auto* result=reinterpret_cast<DebugLineVertex*>(const_cast<Sample*>(samples));
    if (!samples || !count || !self) return result;
    float minimum=std::numeric_limits<float>::max(), maximum=0;
    for (uint32_t i=0;i<count;++i) { float v=samples[i].value.x; minimum=Select(minimum-v,minimum,v); maximum=Select(maximum-v,v,maximum); }
    float range=maximum-minimum;
    float inverse=range>0 ? 1.0f/range:0;
    for (uint32_t i=0;i<count;++i) {
        float t=1;
        if (range>0) { float v=(samples[i].value.x-minimum)*inverse; t=Select(-0.0f-v,v,0); t=Select(1.0f-t,1,t); }
        Vec4 c=!(t>=0.5f) && range>0 ? Add(Mul(Sub(middle,low),t*2),low):Add(Mul(Sub(high,middle),(t-0.5f)*2),middle);
        uint32_t color=((((Integer(c.w*255)<<8)|Integer(c.x*255))<<8)|Integer(c.y*255))<<8|Integer(c.z*255);
        Vec4 d=samples[i].direction;
        float squared=(d.z*d.z+d.y*d.y)+d.x*d.x;
        alignas(16) Vec4 p=samples[i].point;
#if defined(_M_IX86)
        // Runtime offset, not a constant: 11D40F0 is zero in the file.
        p=Add(p,*reinterpret_cast<const Vec4*>(0x11D40F0));
#endif
        if (!(squared>0)) result=DrawCircleAt(self,&p,0.05f,color,false);
        else result=DrawArrow(self,&p,&samples[i].direction,color,0.1f,0.1f);
    }
    return result;
}
// 0x0074C650: all three lazy colors initialized to {1,0,0,1}.
DebugLineVertex* DrawRouting(const Vec4& vector, DebugLineBuffer* self, const Sample* samples, uint32_t count)
{
    const Vec4 red{1,0,0,1};
#if defined(_M_IX86)
    auto& flags=*reinterpret_cast<uint32_t*>(0x11D41B0);
    auto& high=*reinterpret_cast<Vec4*>(0x11D41A0);
    auto& middle=*reinterpret_cast<Vec4*>(0x11D4190);
    auto& low=*reinterpret_cast<Vec4*>(0x11D4180);
    if (!(flags&1)) { flags|=1; high=red; }
    if (!(flags&2)) { flags|=2; middle=red; }
    if (!(flags&4)) { flags|=4; low=red; }
    return DrawSamples(vector,self,samples,count,high,middle,low);
#else
    return DrawSamples(vector,self,samples,count,red,red,red);
#endif
}
}
