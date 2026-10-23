#include "ts3/anim/m05_B40.h"
#include <cmath>
#include <algorithm>
namespace ts3::anim::b40 {
// Adapted from WorkQueue::IsPending, FileType, PackedFileRecordQueryInterfaceById.
uint8_t Enabled(const Range* s) { return s->enabled; }
uint32_t Type(const void*) { return 0x94d79ad1u; }
void* Query(void* s,uint32_t id) { return id==0x74d3f00cu || id==0x94d79ad1u ? s : nullptr; }
static bool Endpoint(float a,float b) {
    float d=b-a;
    float neg=-1.0f*d;
    float magnitude=neg>d ? neg:d; // MAXPS returns operand two for unordered inputs.
    return a>b || magnitude<=0x1p-16f;
}
uint32_t Endpoints(const Range* s) {
    return Endpoint(s->low1,s->high0) && Endpoint(s->high0,s->low0) &&
           Endpoint(s->low1,s->high1) && Endpoint(s->high1,s->low0);
}
bool Contains(uint32_t lo,uint32_t hi,const Pair* p,uint32_t n) {
    if(p) for(uint32_t i=0;i<n;++i) if(p[i].lo==lo && p[i].hi==hi) return true;
    return false;
}
void SetSpeed(Vehicle* s,float f) { s->radius=f; s->speed=f; }
bool Intersects(const Point* a,const Point* b,const Point* c,const Point* d,float margin) {
    float dx=b->x-a->x,dy=b->y-a->y;
    float ex=d->x-c->x,ey=d->y-c->y;
    float rx=c->x-d->x,ry=c->y-d->y;
    float det=ry*dx-rx*dy;
    float ma=static_cast<float>(double(margin)/std::sqrt(double(dx)*dx+double(dy)*dy));
    float mb=static_cast<float>(double(margin)/std::sqrt(double(ey)*ey+double(ex)*ex));
    if(0.01f>std::fabs(det)) return false;
    float inv=1.0f/det;
    float cx=c->x-a->x,cy=c->y-a->y;
    float t=(ry*cx-cy*rx)*inv;
    float u=(cy*dx-dy*cx)*inv;
    return t>=ma && (1.0f-ma)>=t && u>=mb && (1.0f-mb)>=u;
}
void* Region(const Float4Value* p,bool fallback) {
#if defined(_M_IX86)
    void* world=*reinterpret_cast<void**>(0x011ecbc4);
    if(!world) return nullptr;
    void* cell=CallGameMethod<void*,0x00c68990>(world,p->lane[0],p->lane[2]);
    if(!cell) return nullptr;
    void* found;
    if(CallGameFunction<bool,0x00b01390>(cell,p,&found,1)) return found;
    struct Search { float a,b; void* value; } search{-1.0f,-1.0f,nullptr};
    if(fallback) CallGameFunction<void,0x00add7a0>(cell,p,&search);
    else CallGameFunction<void,0x00add740>(cell,p,&search);
    return search.value;
#else
    (void)p;(void)fallback; AbortNotRewritten("B40 world region"); return nullptr;
#endif
}
Placement* Place(const Float4Value* p,Placement* out) {
    Float4Value saved=*p;
    void* region=Region(p,false);
    float height=0;
    void* terrain=CallGameFunction<void*,0x008292d0>();
    if(terrain) height=CallGameMethod<float,0x00829470>(terrain,saved.lane[0],saved.lane[2],region,1,1);
    saved.lane[1]=height;
    out->position=saved; out->zero={}; out->weight=1; out->region=region;
    out->flags[0]=out->flags[1]=out->flags[2]=0;
    return out;
}
bool Empty(const Vehicle* s) { return s->nodes.next==&s->nodes; }
static float Distance(const Float4Value& a,const Float4Value& b) {
    float x=a.lane[0]-b.lane[0],y=a.lane[1]-b.lane[1],z=a.lane[2]-b.lane[2];
    return (z*z+y*y)+x*x;
}
bool Broad(const Vehicle* s,const Float4Value* p,float extra,bool fixed) {
    float r=s->bound;
    if(!fixed) { r=(s->scale-1.0f)*s->radius; r=r*2.3f; r=r+s->radius*1.3f; r=r+s->bound; }
    r=r+extra;
    return r*r>Distance(s->position,*p);
}
bool Hit(const Vehicle* s,const Float4Value* p,float radius,bool all,bool fixed) {
    if(Empty(s) || !Broad(s,p,radius*2.3f,fixed)) return false;
    float scale=fixed ? 1.0f:s->scale*2.3f;
    float r=s->radius*scale+radius, rr=r*r;
    for(const Link* l=s->nodes.next;l!=&s->nodes;l=l->next) {
        auto* n=reinterpret_cast<const Node*>(l);
        if((all || n->active) && rr>=Distance(*p,n->position)) return true;
    }
    return false;
}
void Clear(Scratch* s) {
    Link* l=s->nodes.next;
    while(l!=&s->nodes) { Link* next=l->next; CallGameFunction<void,0x004e3f70>(l); l=next; }
    s->nodes.next=s->nodes.previous=&s->nodes; s->dirty=1;
}
}
