#include "ts3/math/m04_B16.h"
#include <cmath>
#include <cfloat>
#include <cstring>
#include <xmmintrin.h>
namespace ts3::math::b16 {
namespace {
size_t Count(const Polygon* p) { return p->begin==p->end ? 0 : p->end-p->begin; }
bool Segment(const Point* a,const Point* b,const Point* c,const Point* d) {
    return CallGameFunction<uint8_t,0x007CDAA0>(a,b,c,d,static_cast<Point*>(nullptr),0.01f)!=0;
}
bool SegmentEdges(const Polygons* list,const Point* a,const Point* b) {
    for(auto p=list->begin;p!=list->end;++p) {
        const auto n=Count(p);
        for(size_t i=1;i<n;++i) if(Segment(a,b,p->begin+i-1,p->begin+i)) return true;
        // The original also attempts the closing edge for a zero-length polygon.
        if(Segment(a,b,p->begin+n-1,p->begin)) return true;
    }
    return false;
}
bool Tiny(float v) {
    const auto x=_mm_set1_ps(v);
    return _mm_movemask_ps(_mm_cmple_ps(_mm_max_ps(_mm_mul_ps(_mm_set1_ps(-1.0f),x),x),_mm_set1_ps(0x1p-16f)))==15;
}
}
int __stdcall Contains(const Polygon* polygon,const Point* point) {
    const auto n=Count(polygon);
    int inside=0;
    size_t previous=n-1;
    for(size_t i=0;i<n;++i) {
        const Point& c=polygon->begin[i]; const Point& p=polygon->begin[previous];
        if((point->y>=c.y && p.y>point->y) || (point->y>=p.y && c.y>point->y)) {
            const float x=(point->y-c.y)*(p.x-c.x)/(p.y-c.y)+c.x;
            if(x>point->x) inside=!inside;
        }
        previous=i;
    }
    return inside;
}
uint8_t __stdcall ContainsAny(const Polygons* list,const Point* point) {
    for(auto p=list->begin;p!=list->end;++p) if(Contains(p,point)) return 1;
    return 0;
}
uint8_t __stdcall ContainsRadius(const Polygon* polygon,const Point* point,float radius) {
    if(Contains(polygon,point)) return 1;
    if(polygon->begin==polygon->end) return 0;
    const float rr=radius*radius, px=point->x, py=point->y;
    auto previous=polygon->end-1;
    for(auto current=polygon->begin;current!=polygon->end;++current) {
        float x=previous->x,y=previous->y;
        const float dx=current->x-x,dy=current->y-y;
        const float projection=(px-x)*dx+(py-y)*dy;
        if(!(projection<=0.0f)) {
            const float length=dx*dx+dy*dy;
            if(projection>length) { x=current->x; y=current->y; }
            else { const float t=projection/length; x=x+dx*t; y=previous->y+t*dy; }
        }
        const float ex=px-x,ey=py-y;
        if(rr>=ex*ex+ey*ey) return 1;
        previous=current;
    }
    return 0;
}
uint8_t __stdcall ConsistentTurns(const Polygon* polygon) {
    const auto n=Count(polygon); auto p=polygon->begin;
    if(n<=3) return 1;
    if(n==4) return Segment(p,p+2,p+1,p+3);
    Point previous=p[n-2],current=p[n-1];
    float sign=0.0f;
    for(size_t i=0;i<n;++i) {
        const float turn=(p[i].y-current.y)*(current.x-previous.x)-(current.y-previous.y)*(p[i].x-current.x);
        if(Tiny(turn)) continue; // EA: do not advance either saved vertex here.
        if(Tiny(sign)) sign=turn;
        if((sign<0.0f && turn>0.0f) || (sign>0.0f && turn<0.0f)) return 0;
        previous=current; current=p[i];
    }
    return 1;
}
uint8_t __stdcall ContainsRadiusAny(const Polygons* list,const Point* point,float radius) {
    for(auto p=list->begin;p!=list->end;++p) if(ContainsRadius(p,point,radius)) return 1;
    return 0;
}
uint8_t __stdcall IntersectsSegment(const Polygons* list,const Point* a,const Point* b) {
    if(ContainsAny(list,a)) return 1;
    if(a->x==b->x && a->y==b->y) return 0;
    if(ContainsAny(list,b)) return 1;
    return SegmentEdges(list,a,b);
}
uint8_t __fastcall ContainsBox(void*,void*,const Polygon* polygon,const Point* a,const Point* b) {
    const Point center{(b->x+a->x)*0.5f,(b->y+a->y)*0.5f};
    if(!Contains(polygon,&center)) return 0;
    const Point half{(b->x-a->x)*0.5f,(b->y-a->y)*0.5f};
    const auto n=Count(polygon);
    size_t previous=n-1;
    for(size_t i=0;i<n;++i) {
#if defined(_M_IX86)
        using Fn=uint8_t (__stdcall*)(const Point*,const Point*,const Point*,const Point*);
        if(reinterpret_cast<Fn>(0x006771C0)(polygon->begin+previous,polygon->begin+i,&center,&half)) return 0;
#else
        AbortNotRewritten("0x006771C0 segment/box dependency");
#endif
        previous=i;
    }
    return 1;
}
uint8_t __fastcall ContainsBoxAny(void* self,void* edx,const Polygons* list,const float* box) {
    const Point a{box[0],box[1]},b{box[2],box[3]};
    for(auto p=list->begin;p!=list->end;++p) if(ContainsBox(self,edx,p,&a,&b)) return 1;
    return 0;
}
float __stdcall NearestAny(const Polygons* list,const Point* point,Point* output) {
    float best=FLT_MAX;
    output->x=0.0f; output->y=0.0f;
    for(auto p=list->begin;p!=list->end;++p) {
        misc::clu6795::Points view{p->begin,p->end,p->capacity,{0,0,0}};
        Point nearest;
        const float distance=misc::clu6795::Nearest(point,&view,&nearest);
        if(best>distance) { *output=nearest; best=distance; }
    }
    return best;
}
uint8_t __fastcall IntersectsSwept(void* self,void*,const Polygons* list,const Point* a,const Point* b,float radius) {
    if(ContainsRadiusAny(list,a,radius)) return 1;
    if(a->x==b->x && a->y==b->y) return 0;
    if(ContainsRadiusAny(list,b,radius)) return 1;
    if(!(radius>0.001f)) return SegmentEdges(list,a,b);
    const float dx=b->x-a->x;
    const double dy=double(b->y)-double(a->y);
    const float inv=static_cast<float>(1.0/std::sqrt(dy*dy+double(dx)*double(dx)));
    const float ux=dx*inv,uy=(b->y-a->y)*inv;
    const float start=a->y*uy+a->x*ux,finish=b->y*uy+b->x*ux;
    const float delta=start-finish;
    uint32_t bits; std::memcpy(&bits,&delta,4);
    const float transverse=a->y*ux-a->x*uy;
    const Point low{bits>>31 ? start:finish,transverse-radius};
    const Point high{bits>>31 ? finish:start,transverse+radius};
    Polygon rotated{nullptr,nullptr,nullptr,
#if defined(_M_IX86)
        reinterpret_cast<const char*>(0x00FED140)
#else
        "Physics/CollisionHelpers"
#endif
    };
    uint8_t hit=0;
    for(auto p=list->begin;p!=list->end;++p) {
        const auto n=Count(p);
        CallGameMethod<void,0x0062A7A0>(&rotated,static_cast<unsigned>(n));
        for(size_t i=0;i<n;++i) {
            const float y=p->begin[i].y,x=p->begin[i].x;
            rotated.begin[i].x=x*ux+y*uy;
            rotated.begin[i].y=y*ux-x*uy;
        }
        if(CallGameMethod<uint8_t,0x0067C6C0>(self,&rotated,&low,&high)) { hit=1; break; }
    }
    if(rotated.begin) CallGameFunction<void,0x004E3F70>(rotated.begin);
    return hit;
}
uint8_t __cdecl InTriangle(const Point* p,const Point* a,const Point* b,const Point* c) {
    bool hit=false;
    const bool ay=a->y>=p->y,by=b->y>=p->y,cy=c->y>=p->y;
    if(cy!=ay && (((a->y-p->y)*(c->x-a->x)>=(c->y-a->y)*(a->x-p->x))==ay)) hit=true;
    if(ay!=by && (((a->x-b->x)*(b->y-p->y)>=(a->y-b->y)*(b->x-p->x))==by)) hit=!hit;
    if(by!=cy && (((b->x-c->x)*(c->y-p->y)>=(b->y-c->y)*(c->x-p->x))==cy)) hit=!hit;
    return hit;
}
double __cdecl Cross(const Point* a,const Point* b,const Point* c) {
    return (double(c->y)-b->y)*(double(a->x)-b->x)-(double(c->x)-b->x)*(double(a->y)-b->y);
}
int __cdecl InRectangle(const Point* p,const Point* low,const Point* high) {
    return p->x>=low->x && high->x>p->x && p->y>=low->y && high->y>p->y;
}
}
