#include "ts3/ui/m08_B14.h"
#include "ts3/ui/m08_abi.h"
#include <xmmintrin.h>
#include <intrin.h>
#include <cstring>
namespace ts3::ui::m08::b14 {
namespace {
void Prepare(Window* w) {
    if(!w->manager) return;
    Window* last=nullptr;
    for(auto p=w;p;p=p->parent) if(p->state & 0x4u) last=p;
    if(last) CallGameMethod<void,0x0050DB00>(w->manager,last);
}
void Notify(Window* w) { if(w->manager) CallGameMethod<void,0x0050AF20>(w->manager); }
void Unlink(Link* n) { n->next->previous=n->previous;n->previous->next=n->next; }
void Insert(Link* n,Link* before) {
    Link* prev=before->previous;prev->next=n;before->previous=n;n->previous=prev;n->next=before;
}
Link* Partition(Window* w,Link* p) {
    while(p!=&w->children && (FromLink(p)->flags&0x40)) p=p->next;
    return p;
}
}
void SetResource(void* self,void* value) {
    auto w=Owner(self);if(value==w->service) return;
    auto old=w->service;
    if(value) CallGameVirtual<uint32_t,0>(value);
    w->service=value;
    if(old) CallGameVirtual<uint32_t,1>(old);
    void* result=nullptr;
    if(w->service) CallGameVirtual<void,7>(w->service,0x019C46FBu,&result);
    w->state=(w->state&~0x200u)|(result?0x200u:0);
    if(w->manager) {
        // Only type and subtype are defined in this stack message.
        uint32_t message[7];message[2]=14;message[3]=4;
        void* secondary=w?&w->secondary:nullptr;
        CallGameVirtual<void,4>(w->manager,secondary,secondary,message,0u);
    }
}
Vector2* ToScreen(void* self,Vector2* out,float x,float y) {
    auto w=Owner(self);Prepare(w);
    __m128 v=_mm_set_ps(x,0,y,x);
    for(auto p=w;p;p=p->parent) {
        const float cx=(p->rect[2]+p->rect[0])*0.5f;
        const float cy=(p->rect[3]+p->rect[1])*0.5f;
        const float width=p->rect[2]-p->rect[0],height=p->rect[3]-p->rect[1];
        __m128 q=_mm_sub_ps(v,_mm_mul_ps(_mm_set_ps(width,0,height,width),_mm_set1_ps(0.5f)));
        // Match 00511D60's exact order: x*row0 + translation + y*row1 + z*row2.
        __m128 r=_mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(q,q,0),_mm_loadu_ps(p->transform)),_mm_loadu_ps(p->transform+12));
        r=_mm_add_ps(r,_mm_mul_ps(_mm_shuffle_ps(q,q,0x55),_mm_loadu_ps(p->transform+4)));
        r=_mm_add_ps(r,_mm_mul_ps(_mm_shuffle_ps(q,q,0xaa),_mm_loadu_ps(p->transform+8)));
        v=_mm_add_ps(_mm_set_ps(cx,0,cy,cx),r);
    }
    float lanes[4];_mm_storeu_ps(lanes,v);out->x=lanes[0];out->y=lanes[1];return out;
}
uint8_t FromScreen(void* self,float x,float y,Vector2* out) {
    auto w=Owner(self);Prepare(w);if(w->inverseKind==4) return 0;
    auto m=w->inverse;
    __m128 v=_mm_add_ps(_mm_mul_ps(_mm_set1_ps(x),_mm_loadu_ps(m)),_mm_mul_ps(_mm_set1_ps(y),_mm_loadu_ps(m+4)));
    v=_mm_add_ps(v,_mm_loadu_ps(m+12));
    constexpr float eps=0x1p-23f;
    // COMISS unordered branches also take the division path.
    if(!(m[10]<=eps && m[10]>=-eps)) {
        __m128 z=_mm_div_ps(_mm_shuffle_ps(v,v,0xaa),_mm_set1_ps(m[10]));
        v=_mm_sub_ps(v,_mm_mul_ps(_mm_loadu_ps(m+8),z));
    }
    float lanes[4];_mm_storeu_ps(lanes,v);
    if(lanes[3]<=eps && lanes[3]>=-eps) { out->x=0;out->y=0;return 0; }
    __m128 reciprocal=_mm_div_ps(_mm_set1_ps(1.f),_mm_shuffle_ps(v,v,0xff));
    __m128 xx=_mm_mul_ps(_mm_shuffle_ps(v,v,0),reciprocal);
    __m128 yy=_mm_mul_ps(_mm_shuffle_ps(v,v,0x55),reciprocal);
    out->x=_mm_cvtss_f32(xx);out->y=_mm_cvtss_f32(yy);return 1;
}
Link** First(void* self,Link** out) { *out=Owner(self)->children.next;return out; }
Link** End(void* self,Link** out) { *out=&Owner(self)->children;return out; }
Link** FindChild(void* self,Link** out,void* child) {
    auto w=Owner(self);auto c=child?Owner(child):nullptr;
    if(c->parent==w) *out=&c->sibling;
    else { Link* result;auto p=CallGameVirtual<Link**,0xD4/4>(self,&result);*out=*p; }
    return out;
}
uint8_t Attach(void* self,void* child) {
    auto w=Owner(self);auto c=child?Owner(child):nullptr;
    CallGameVirtual<uint32_t,0>(c);c->parent=w;
    Insert(&c->sibling,Partition(w,w->children.next));
    if(w->manager) CallGameMethod<void,0x0050D9F0>(w->manager,c);
    if(!CallGameVirtual<uint8_t,6>(c)) { CallGameVirtual<void,0xE4/4>(self,&c->secondary);return 0; }
    CallGameMethod<void,0x00513DC0>(w);return 1;
}
void BringFront(void* self,void* child) {
    auto w=Owner(self);auto c=child?Owner(child):nullptr;auto n=&c->sibling;
    auto p=w->children.next;
    if(!(c->flags&0x40)) { while(p!=n && (FromLink(p)->flags&0x40)) p=p->next; }
    if(p!=n) { Unlink(n);Insert(n,p);Notify(w); }
}
void SendBack(void* self,void* child) {
    auto w=Owner(self);auto c=child?Owner(child):nullptr;auto n=&c->sibling;
    auto p=(c->flags&0x40)?Partition(w,n->next):&w->children;
    if(p!=n) { Unlink(n);Insert(n,p); }
    Notify(w);
}
void* NextValue(void* self,void* value) {
    auto w=Owner(self);bool found=value==nullptr;
    for(auto p=w->values.next;p!=&w->values;p=p->next) {
        auto n=reinterpret_cast<ValueNode*>(p);if(found) return n->value;if(n->value==value) found=true;
    }
    return nullptr;
}
uint8_t Message(Window* w,const GridMessage* m) {
    if(m->type==13) return CallGameVirtual<uint8_t,9>(w,m->payload_a);
    if(m->type==21) {
        if(w->service && !m->source) return CallGameVirtual<uint8_t,6>(w->service,m->payload_a,0u,0u);
        return 0;
    }
    if(m->type==19 && (w->state&0x200u)) {
        if(!w->service) return 0;
        float rect[4]={0,0,w->rect[2]-w->rect[0],w->rect[3]-w->rect[1]};
        float x,y;std::memcpy(&x,&m->source,4);std::memcpy(&y,&m->arg_10,4);
        int point[2]={_mm_cvtt_ss2si(_mm_set_ss(x)),_mm_cvtt_ss2si(_mm_set_ss(y))};
        uint32_t payload[4]={0,0,0,0xffffffffu};
        uint8_t r=CallGameVirtual<uint8_t,5>(w->service,rect,point,payload);
        *const_cast<uint8_t*>(static_cast<const uint8_t*>(m->payload_a))=r;return 1;
    }
    auto code=reinterpret_cast<uintptr_t>(m->source);
    if(m->type==14 && (code==0xffffffffu || (code==1 && ((m->arg_14^m->arg_10)&1)))) CallGameMethod<void,0x00514AB0>(w);
    return 0;
}
void* Lookup(void* self,uint32_t key) {
    auto& map=Owner(self)->map;
    for(auto n=map.buckets[key%map.count];n;n=n->next) if(n->key==key) return n==map.buckets[map.count]?nullptr:n->value;
    // EA compares the iterator node against buckets[count] (its end sentinel).
    return nullptr;
}
uint32_t Retain(void* self) {
    auto w=Owner(self);return static_cast<uint32_t>(_InterlockedIncrement(reinterpret_cast<volatile long*>(&w->refs)));
}
}
