#include "ts3/math/m04_B17.h"
#include <cmath>
namespace ts3::math::b17 {
double Area(const float* a,const float* b,const float* c) {
 float v=((a[0]*b[1]-b[0]*a[1])+(c[1]*b[0]-c[0]*b[1]))+(c[0]*a[1]-c[1]*a[0]);
 v=v*0.5f; return v>=0.0f?v:-0.0f-v;
}
double LineDistance(const float* a,const float* b,const float* c) {
 float dx=c[0]-b[0],dy=c[1]-b[1],nx=-0.0f-dx;
 float k=b[1]*dx-b[0]*dy;
 return std::fabs((double(a[0])*dy+double(a[1])*nx+double(k))/std::sqrt(double(nx)*nx+double(dy)*dy));
}
bool Contains(const float* p,const float* v,int n) {
 const float* prev=v+2*n-2; bool result=false,above=prev[1]>=p[1];
 for(int i=0;i<n;++i,v+=2) {
  bool next=v[1]>=p[1];
  if(above!=next && (((v[1]-p[1])*(prev[0]-v[0]) >= (prev[1]-v[1])*(v[0]-p[0]))==next)) result=!result;
  prev=v; above=next;
 }
 return result;
}
char IntersectEpsilon(const float* a,const float* b,const float* c,const float* d,float* out,float eps) {
 float cy=c[1]-d[1],by=b[1],cx=c[0]-d[0],bx=b[0]-a[0];
 float det=cy*bx-(by-a[1])*cx;
 if(eps>std::fabs(det)) return 0;
 float inv=1.0f/det,px=c[0]-a[0],py=c[1]-a[1];
 float t=((c[1]-d[1])*px-py*cx)*inv;
 float u=(py*bx-(by-a[1])*px)*inv;
 // COMISS/JB also rejects unordered parameters (including a zero determinant).
 if(!(t>=0.0f)||!(1.0f>=t)||!(u>=0.0f)||!(1.0f>=u)) return 0;
 if(out) {out[0]=bx*t+a[0];out[1]=(b[1]-a[1])*t+a[1];} return 1;
}
char Intersect(const float* a,const float* b,const float* c,const float* d,float* out) {
 return IntersectEpsilon(a,b,c,d,out,0.01f);
}
char Bounds(const Points* p,float* out) {
 if(p->begin==p->end) return 0;
 const float* first=reinterpret_cast<const float*>(p->begin);
 out[0]=first[0];out[1]=first[1];out[2]=out[0];out[3]=out[1];
 for(auto i=p->begin+1;i!=p->end;++i) {
  const float* v=reinterpret_cast<const float*>(i);
  // The original uses JA, retaining the old value on ties AND unordered.
  // A C++ <= ternary is neither NaN-equivalent nor safe to lower to MINSS.
  auto choose=[](float oldValue,float value,bool maximum) {
   __m128 oldLane=_mm_set_ss(oldValue),newLane=_mm_set_ss(value);
   __m128 mask=maximum?_mm_cmpgt_ss(newLane,oldLane):_mm_cmpgt_ss(oldLane,newLane);
   return _mm_cvtss_f32(_mm_or_ps(_mm_and_ps(mask,newLane),_mm_andnot_ps(mask,oldLane)));
  };
  out[0]=choose(out[0],v[0],false);out[1]=choose(out[1],v[1],false);
  out[2]=choose(out[2],v[0],true);out[3]=choose(out[3],v[1],true);
 } return 1;
}
char NearestHit(const float* a,const float* b,const float* c,const float* d,char found,float* dist,float* out) {
 float hit[2]; if(!Intersect(a,b,c,d,hit)) return 0;
 double y=double(hit[1])-a[1],x=double(hit[0])-a[0];double len=std::sqrt(x*x+y*y);
 if(found && *dist<=len) return 1;
 *dist=float(len);out[0]=hit[0];out[1]=hit[1]; return 1;
}
bool RectangleHit(const float* a,const float* b,const float* lo,const float* hi,float* out) {
 auto inside=[&](const float* p){return p[0]>=lo[0]&&hi[0]>=p[0]&&p[1]>=lo[1]&&hi[1]>=p[1];};
 unsigned target=inside(a)!=inside(b)?1:2,hits=0;float dist=0;
 float c[2]={lo[0],lo[1]},d[2]={lo[0],hi[1]};
 if(NearestHit(a,b,c,d,0,&dist,out) && ++hits>=target) return true;
 d[0]=hi[0];d[1]=lo[1];
 if(NearestHit(a,b,c,d,hits!=0,&dist,out) && ++hits>=target) return true;
 c[0]=hi[0];c[1]=hi[1];
 if(NearestHit(a,b,c,d,hits!=0,&dist,out) && ++hits>=target) return true;
 d[0]=lo[0];d[1]=hi[1];
 if(NearestHit(a,b,c,d,hits!=0,&dist,out)) ++hits;
 return hits!=0;
}
char Project(float x,float y,float ax,float ay,float bx,float by,float* out) {
 float dx=bx-ax,dy=by-ay;
 if(dx<=1.1920929e-7f && dx>=-1.1920929e-7f && dy<=1.1920929e-7f && dy>=-1.1920929e-7f) return 0;
 float t=((y*dy+dx*x)-(dx*ax+dy*ay))/(dy*dy+dx*dx);
 out[0]=dx*t+ax;out[1]=dy*t+ay;return 1;
}
char SegmentPolygon(const float* a,const float* b,const float* v,unsigned n) {
 if(Contains(a,v,n)||Contains(b,v,n)) return 1;
 for(unsigned i=0;i<n;++i) if(Intersect(a,b,v+2*i,v+2*((i+1)%n),nullptr)) return 1;
 return 0;
}
char CyclicEqual(const Points* a,const Points* b) {
 auto n=a->end-a->begin,m=b->end-b->begin;
 if(n!=m) return 0;
#if defined(_M_IX86)
 // EA's lazy tolerance is writable process state, not a compile-time literal.
 auto& flags=*reinterpret_cast<unsigned*>(0x011D88D4);
 auto& tolerance=*reinterpret_cast<float*>(0x011D88D0);
 if(!(flags&1)) {flags|=1;tolerance=0.0001f;}
 const float epsilon=tolerance;
#else
 const float epsilon=0.0001f;
#endif
 if(!n) return 0;
 auto sq=[](const auto& x,const auto& y){float dx=x.x-y.x,dy=x.y-y.y;return dx*dx+dy*dy;};
 decltype(n) j=0;
 while(j<m && !(epsilon>sq(a->begin[0],b->begin[j]))) ++j;
 if(j==m) return 0;
 for(decltype(n) i=1;i<n;++i) if(sq(a->begin[i],b->begin[(j+i)%n])>=epsilon) return 0;
 return 1;
}
bool PolygonsOverlap(const Points* a,const float* ab,const Points* b,const float* bb) {
 if(ab[2]<=bb[0]||bb[2]<=ab[0]||ab[3]<=bb[1]||bb[3]<=ab[1]) return false;
 unsigned n=unsigned(a->end-a->begin),m=unsigned(b->end-b->begin);if(!n||!m) return false;
 float w1=ab[2]-ab[0],w2=bb[2]-bb[0];float eps=(w1<=w2?w1:w2)*0.001f;
 if(!(eps>=1.1920929e-7f)) eps=1.1920929e-7f;
 const float* av=reinterpret_cast<const float*>(a->begin);const float* bv=reinterpret_cast<const float*>(b->begin);
 for(unsigned i=0;i<n;++i) for(unsigned j=0;j<m;++j)
  if(IntersectEpsilon(av+2*i,av+2*((i+1)%n),bv+2*j,bv+2*((j+1)%m),nullptr,eps)) return true;
 return Contains(av,bv,m)||Contains(bv,av,n)||(n==m&&CyclicEqual(a,b));
}
float OrientedArea(const __m128* a,const __m128* b,const __m128* c,float scale) {
 float x=a->m128_f32[0],y=a->m128_f32[1];
 return ((x-c->m128_f32[0])*(y-b->m128_f32[1])-(y-c->m128_f32[1])*(x-b->m128_f32[0]))*scale;
}
}
