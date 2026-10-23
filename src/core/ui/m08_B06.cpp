#include "ts3/ui/m08_B06.h"
#include <cstring>
#include <xmmintrin.h>
namespace ts3::ui::b06 {
static void Repair(Link* a,Link* b) {
    if(a->next==b) a->next=a->previous=a;
    else { a->previous->next=a; a->next->previous=a; }
}
Link* __fastcall SwapHeads(Link* a,void*,Link* b) {
    auto n=a->next; auto p=a->previous;
    a->next=b->next; a->previous=b->previous; b->previous=p; b->next=n;
    Repair(a,b); Repair(b,a); return b;
}
void* __fastcall MainInterface(Manager* s,void*) { return s->main ? &s->main->secondary : nullptr; }
// Only the fields consumed by slot 6 are initialized by the original. The tail
// and (for object messages) the first two words are undefined stack bytes.
struct Message { uint32_t a,b,type; union { Point position; struct { uint32_t id; void* object; } object; }; uint32_t tail[2]; };
uintptr_t __fastcall PositionMessage(Manager* s,void*,float x,float y) {
    Message m; m.a=m.b=0; m.type=0x4ECD5687; m.position={x,y};
    return CallGameVirtual<uintptr_t,6>(s,0u,0u,&m);
}
uintptr_t __fastcall ObjectMessage(Manager* s,void*,uint32_t id,void* object) {
    Message m; m.type=0x74D2E9;
    if(object) CallGameVirtual<void,0>(object);
    m.object.id=id; m.object.object=object;
    return CallGameVirtual<uintptr_t,6>(s,0u,0u,&m);
}
void __fastcall SetSource(Manager* s,void*,void* r) {
    auto old=s->source;
    if(r!=old) {
        if(r) CallGameVirtual<void,0>(r);
        s->source=r;
        if(old) CallGameVirtual<void,1>(old);
    }
}
void __fastcall RefreshCursor(Manager* s,void*) {
    auto w=s->focus;
    if(!w) return;
    void* cursor=nullptr;
    do { cursor=w->cursor; if(cursor) break; w=w->parent; } while(w);
    if(s->cursor==cursor) return;
    s->cursor=cursor;
    if(s->cursorService) CallGameVirtual<void,3>(s->cursorService,cursor);
}
void __fastcall SetCursorService(Manager* s,void*,void* service) {
    auto old=s->cursorService;
    if(service!=old) {
        if(service) CallGameVirtual<void,0>(service);
        s->cursorService=service;
        if(old) CallGameVirtual<void,1>(old);
    }
    if(service && s->cursor) CallGameVirtual<void,3>(s->cursorService,s->cursor);
}
static float StoredFloat(float value) {
    // FLD/FSTP quiets signaling NaNs, unlike a bare SSE move.
    uint32_t bits; std::memcpy(&bits,&value,4);
    if((bits&0x7F800000u)==0x7F800000u && (bits&0x007FFFFFu)) bits|=0x00400000u;
    std::memcpy(&value,&bits,4); return value;
}
Point* __fastcall GetPosition(Manager* s,void*,Point* out) {
    out->x=StoredFloat(s->position.x); out->y=StoredFloat(s->position.y); return out;
}
static uint32_t Bias(float f) {
    float v=_mm_cvtss_f32(_mm_add_ss(_mm_set_ss(f),_mm_set_ss(12582912.0f)));
    uint32_t bits; std::memcpy(&bits,&v,4); return bits+0xB4C00000u;
}
void __fastcall SetPosition(Manager* s,void*,const Point* p) {
    if(s->cursorService) {
        auto y=Bias(p->y); auto x=Bias(p->x);
        CallGameVirtual<void,4>(s->cursorService,x,y);
    }
}
static void Append(Link* head,Link* n) {
    n->previous=head->previous; n->next=head; head->previous=n; n->previous->next=n;
}
void __fastcall UpdateActive(Manager* s,void*,Window* w) {
    auto n=&w->active;
    if(!(w->state&1) && !(w->flags&8)) {
        if(!n->previous) return;
        if(s->current==n) {
            s->changed=1; auto p=n->previous; auto next=n->next;
            p->next=next; next->previous=p; s->current=next;
        } else { n->previous->next=n->next; n->next->previous=n->previous; }
        n->previous=nullptr;
    } else if(!n->previous) Append(&s->active,n);
}
void __fastcall AncestorChanged(Manager* s,void*,Window* w) {
    for(auto a=s->focus;a;a=a->parent) if(a==w) { RefreshCursor(s,nullptr); return; }
}
Link* __fastcall QueuePending(Manager* s,void*,Window* w) {
    if(!w->pending.previous && !s->draining) { Append(&s->pending,&w->pending); return &w->pending; }
    return reinterpret_cast<Link*>(w);
}
void __fastcall DrainPending(Manager* s,void*) {
    if(s->draining) return;
    s->draining=1; auto head=&s->pending;
    while(head->previous!=head) {
        auto last=head->previous; auto prev=last->previous;
        prev->next=head; head->previous=last->previous; last->previous=nullptr;
    }
}
Window* __fastcall HitTest(Manager* s,void*,Window* w,const Point* p,uint32_t arg,Point* out) {
    if(!(w->flags&1)) return nullptr;
    Point local;
    auto candidate=w;
    auto secondary=&w->secondary;
    if(!CallGameMethod<uint8_t,0x00514DB0>(secondary,*p,&local)) {
        if(w->flags&0x400) return nullptr;
        candidate=nullptr;
    }
    if(!CallGameMethod<uint8_t,0x00513EB0>(secondary,local)) {
        if(w->flags&0x400) return nullptr;
        candidate=nullptr;
    }
    if(w->flags&0x10) candidate=nullptr;
    if(!(w->flags&0x1000)) {
        auto head=&w->children;
        for(auto n=head->next;n!=head;n=n->next) {
            auto child=n ? reinterpret_cast<Window*>(reinterpret_cast<char*>(n)-offsetof(Window,child)) : nullptr;
            if(auto hit=HitTest(s,nullptr,child,p,arg,out)) return hit;
        }
    }
    if(out) *out=local;
    return candidate;
}
uint32_t __fastcall CopyTransform(void*,void*,void* out,const void* source) {
    auto src=reinterpret_cast<const float*>(static_cast<const char*>(source)+0x150);
    auto dst=static_cast<float*>(out);
    for(int i=0;i<16;i+=4) _mm_store_ps(dst+i,_mm_load_ps(src+i));
    uint32_t last; std::memcpy(&last,src+16,4); std::memcpy(dst+16,&last,4); return last;
}
}
