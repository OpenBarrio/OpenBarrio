#include "ts3/ui/m08_B12.h"
#include <cmath>
#include <cstring>
namespace ts3::ui {
namespace {
B12Window* window(void* s) { return reinterpret_cast<B12Window*>(static_cast<unsigned char*>(s)-offsetof(B12Window,secondary)); }
void* iface(B12Window* w) { return &w->secondary; }
B12Window* child(void* n) { return n ? reinterpret_cast<B12Window*>(static_cast<unsigned char*>(n)-offsetof(B12Window,next)) : nullptr; }
struct Message { void* source; void* target; uint32_t kind,a,b; void* p; void* q; };
void send(void* manager,void* s,Message* m) { CallGameVirtual<void,4>(manager,s,s,m,0u); }
}
void B12SetWord(void* s,uint32_t v) {
    auto w=window(s); w->interfaceWord=v;
    if(w->manager) CallGameMethod<void,0x0050AEC0>(w->manager,w);
}
void B12SetResource(void* s,void* r) {
    auto w=window(s); auto old=w->resource;
    if(r==old) return;
    if(r) CallGameVirtual<void,0>(r);
    w->resource=r;
    if(old) CallGameVirtual<void,1>(old);
    CallGameVirtual<void,0x90/4>(s);
}
void B12Notify(void* s) {
    auto w=window(s); if(w->manager) CallGameMethod<void,0x0050B3C0>(w->manager,w);
}
void B12Restore(B12Window* w) {
    for(int i=0;i<4;++i) w->rect[i]=w->rect[i+4];
    CallGameVirtual<void,0x94/4>(iface(w));
    for(auto n=w->children.next;n!=&w->children;n=*static_cast<void**>(n)) B12Restore(child(n));
}
void B12NotifyAlternate(void* s) {
    auto w=window(s); if(w->manager) CallGameMethod<void,0x0050B470>(w->manager,w);
}
void B12Broadcast(B12Window* w,void* payload) {
    for(auto n=w->children.next;n!=&w->children;n=*static_cast<void**>(n)) {
        auto c=child(n); auto s=iface(c);
        if(CallGameVirtual<void*,5>(s)) {
            auto manager=CallGameVirtual<void*,5>(s);
            CallGameVirtual<void,4>(manager,iface(w),s,payload,0u);
            B12Broadcast(c,payload);
        }
    }
}
void B12Dirty(void* s) {
    auto w=window(s);
    if(w->dirty&0x20) return;
    w->dirty|=0x20;
    for(auto p=w->ancestor;p && !(p->dirty&0x40);p=p->ancestor) p->dirty|=0x40;
    if(w->manager) CallGameMethod<void,0x0050AF20>(w->manager);
}
void B12Dispatch(void* s) {
    Message m; m.source=s; m.target=s; m.kind=0x10003;
    CallGameVirtual<void,0x118/4>(s,&m);
}
void B12Propagate(B12Window* w) {
    bool state=(w->flags&0x400)||(w->clip&4);
    for(auto n=w->children.next;n!=&w->children;n=*static_cast<void**>(n)) {
        auto c=child(n);
        if(state!=bool(c->clip&4)) {
            c->clip=uint8_t((c->clip&~4)|(state?4:0));
            B12Propagate(c);
            CallGameVirtual<void,0x90/4>(iface(c));
        }
    }
}
void* B12GetValue(void* s) { return window(s)->value; }
void B12SetIndex(void* s,uint32_t v) {
    auto w=window(s); w->index=v;
    CallGameVirtual<void,0x90/4>(s);
    if(w->manager) { Message m; m.kind=14; m.a=4; send(w->manager,s,&m); }
}
bool B12Hit(void* s,float x,float y) {
    auto w=window(s); float left=0,top=0;
    float right=w->rect[2]-w->rect[0],bottom=w->rect[3]-w->rect[1];
    if(w->clip&4) {
        float l=w->rect[8],t=w->rect[9],r=w->rect[10],b=w->rect[11];
        if(0>=r || l>=right || 0>=b || t>=bottom) return false;
        if(l>0) left=l;
        if(t>0) top=t;
        if(right>r) right=r;
        if(bottom>b) bottom=b;
    }
    // JB after COMISS also branches for unordered operands.
    if(!(x>=left) || !(y>=top) || !(right>=x) || !(bottom>=y)) return false;
    uint8_t result=1;
    if(w->manager && ((w->flags&0x20)||(w->clip&3))) {
        Message m; m.kind=19;
        static_assert(sizeof(float)==sizeof(uint32_t));
        memcpy(&m.a,&x,4); memcpy(&m.b,&y,4); m.q=&result;
        send(w->manager,s,&m);
    }
    return result!=0;
}
float* B12Map(void* s,float* out,float x,float y) {
    float point[2];
    bool ok=CallGameVirtual<bool,0xcc/4>(s,x,y,point);
    out[0]=ok?point[0]:0.0f; out[1]=ok?point[1]:0.0f;
    return out;
}
}
