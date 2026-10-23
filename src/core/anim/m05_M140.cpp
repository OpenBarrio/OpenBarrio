#include "ts3/anim/m05_M140.h"
#include "ts3/route/clu_848d20_Route.h"
#include <intrin.h>
#include <xmmintrin.h>
#include <cstring>
namespace ts3::anim::M140 {
namespace {
template<class R, unsigned A, class... Args> R M(void* p, Args... args) { return CallGameMethod<R,A>(p,args...); }
template<class R, unsigned A, class... Args> R C(Args... args) { return CallGameFunction<R,A>(args...); }
template<class R,int S,class... Args> R V(void* p,Args... args) { return CallGameVirtual<R,S>(p,args...); }
struct Ref { void** vtable; long count; };
void Retain(void* p) { _InterlockedIncrement(&static_cast<Ref*>(p)->count); }
void Release(void* p) {
    auto r=static_cast<Ref*>(p);
    if (!_InterlockedDecrement(&r->count)) {
        _InterlockedExchange(&r->count,1);
        V<void,0>(p,1);
    }
}
void* Resolve(Body* b) { return M<void*,0x00c93970>(b->actor); }
void Stop(Body* b,int arg) { M<void,0x0084b0c0>(b,arg); }
bool Ready(Body* b) { return M<uint8_t,0x00848590>(b)!=0; }
void ResetCounts(Body* b) {
    b->completed=b->target; b->scheduled=b->target;
    M<void,0x00851280>(b);
}
bool CanTransition(Body* b) { return M<uint8_t,0x0084c590>(b)!=0; }
bool Start(Body* b,bool a,int second) { return M<uint8_t,0x00851710>(b,int(a),second)!=0; }
bool Other(Body* b) { return M<uint8_t,0x00850b90>(b)!=0; }
bool ActorAvailable(Body* b) {
    if(!Resolve(b)) return false;
    return !V<uint8_t,79>(Resolve(b));
}
uint64_t Key(void* actor) { return V<uint64_t,4>(actor); }
float Time(void* controller,void* actor) { return static_cast<float>(V<double,41>(controller,Key(actor))); }
bool Progression(Body* b) { return M<uint8_t,0x0085a8f0>(b->progression)!=0; }
bool NearZero(__m128 v,int mask=15) {
    const __m128 negative=_mm_mul_ps(_mm_set1_ps(-1.0f),v);
    const int bits=_mm_movemask_ps(_mm_cmple_ps(_mm_max_ps(negative,v),_mm_set1_ps(0x1p-23f)));
    return (bits&mask)==mask;
}
void Tune(Body* b,int mode) { reinterpret_cast<ts3::route::RouteTuner*>(b)->Update84AF90(mode,1); }
}
void Body::Finish(int32_t mode) {
    auto sentinel=&controllers;
    for(auto n=sentinel->next;n!=sentinel;) {
        auto c=n->controller;
        if(c && !c->finished && V<uint8_t,4>(c->host,c)) { n=n->next; continue; }
        // Save next after notification, as the original does.
        if(c) {
            M<void,0x00566590>(c->delegates,&secondary);
            if(c->finished) {
                if(n->kind==3) ++completed;
                else if(n->kind==10) { --balance; flag250=0; }
                else if(n->kind==9) { ++balance; flag251=0; }
            }
        }
        auto next=n->next;
        // Original recovers the node from next->previous after notification.
        auto erased=next->previous;
        erased->previous->next=erased->next;
        erased->next->previous=erased->previous;
        c=erased->controller;
        if(c && c->refs--==1) { c->refs=1; V<void,0>(c,1); }
        C<void,0x004e3f70>(erased);
        n=next;
    }
    if(disabled) return;
    if(Progression(this) && hasPending) {
        if(!running) { Stop(this,0); return; }
        remaining=0;
        if(Ready(this)) { ResetCounts(this); return; }
        if(!flag251 && !flag250 && !stopping) { M<void,0x008504d0>(this,pending); return; }
        if(sentinel->next==sentinel) { Stop(this,0); return; }
    }
    void* c=Resolve(this);
    if(!c) return;
    Retain(c);
    // The first virtual returns the ECX argument to the capability query.
    uintptr_t capabilities=V<uintptr_t,3>(c,0);
    float offset=0.0f;
    if(M<uint8_t,0x0081c880>(reinterpret_cast<void*>(capabilities),0x2000,0) && !V<void*,31>(c))
        offset=C<float,0x008484d0>(routeKind,routeMask);
    float current=static_cast<float>(V<double,41>(c,Key(motionActor))+offset);
    if(current>=V<double,75>(c)) {
        if(mode==1 || mode==4) {
            if(!stopping) {
                if(!running || (!CanTransition(this) && !Start(this,ActorAvailable(this),0) && !Other(this) &&
                   (!C<uint8_t,0x0055da60>(routeMask) || sentinel->next==sentinel))) Stop(this,0);
            }
        } else if(mode==2 || mode==3) Stop(this,1);
    }
    Release(c);
}
void Body::Update(uint32_t unused) {
    (void)unused;
    void* c=Resolve(this);
    if(!c) return;
    Retain(c);
    if(active && controllers.next==&controllers) V<void,6>(this);
    int mode=M<int,0x0084af20>(this);
    if(!active && !Advance(mode)) Stop(this,0);
    Finish(mode);
    Release(c);
}
uint8_t Body::Advance(int32_t mode) {
    auto sentinel=&controllers;
    for(auto n=sentinel->next;n!=sentinel;) {
        if(!n->controller->finished) { n=n->next; continue; }
        // Notification precedes reading the node links and kind.
        auto c=n->controller;
        M<void,0x00566590>(c->delegates,&secondary);
        if(n->kind==3) ++completed;
        else if(n->kind==10) { --balance; flag250=0; }
        else if(n->kind==9) { ++balance; flag251=0; }
        auto next=n->next;
        auto erased=next->previous;
        erased->previous->next=erased->next;
        erased->next->previous=erased->previous;
        c=erased->controller;
        if(c && c->refs--==1) { c->refs=1; V<void,0>(c,1); }
        C<void,0x004e3f70>(erased);
        n=next;
    }
    if(running && hasPending) {
        remaining=0;
        if(Ready(this)) ResetCounts(this);
        else if(flag251 || flag250 || stopping) {
            if(sentinel->next==sentinel) { Stop(this,0); return 0; }
        } else if(!M<uint8_t,0x008504d0>(this,pending)) { Stop(this,0); return 0; }
        active=1; return 1;
    }
    if(!running && hasPending) { Stop(this,0); return 0; }
#if defined(_M_IX86)
    // These opaque actor interfaces have not yet been acquired by the native port.
    // All pointer-bearing storage in Body itself is native-width; external x86
    // interfaces below are accessed only while using the original-image bridge.
    auto P=[](void* p,size_t off)->void*& { return *reinterpret_cast<void**>(static_cast<char*>(p)+off); };
    auto I=[](void* p,size_t off)->int32_t& { return *reinterpret_cast<int32_t*>(static_cast<char*>(p)+off); };
    auto B=[](void* p,size_t off)->uint8_t& { return *reinterpret_cast<uint8_t*>(static_cast<char*>(p)+off); };
    void* c=Resolve(this);
    if(c) Retain(c);
    alignas(16) __m128 root,slave,delta;
    V<void,7>(actor,&root);
    bool changed=false;
    if(((mode==2 || mode==1) || (slaveCount>0 && useSlaves)) && c && !V<uint8_t,40>(c)) {
        if(!useSlaves) {
            float t=Time(c,actor);
            if(NearZero(_mm_set1_ps(t))) { V<void,64>(c,&root); changed=true; }
        } else {
            void* style=V<void*,1>(actor,0x14cfd2f8);
            if(style) {
                bool special=(M<uint32_t,0x00953660>(style,0)&0xcf00)==0x200 || (M<uint32_t,0x00953690>(style)&0xcf00)==0xe00;
                if(special) {
                    for(int i=0;i<slaveCount;++i) {
                        void* a=M<void*,0x00839ca0>(actor,i);
                        V<void,7>(a,&slave);
                        float t=Time(c,a);
                        if(NearZero(_mm_set1_ps(t))) { V<void,64>(c,&slave); changed=true; }
                    }
                } else {
                    float t=Time(c,actor);
                    if(NearZero(_mm_set1_ps(t))) { V<void,64>(c,&root); changed=true; }
                }
            }
        }
    }
    auto owner=V<void*,31>(c);
    bool specialOwner=false;
    if(owner && *V<uint32_t*,12>(V<void*,31>(c))==1144997326 && V<double,75>(c)<0.15000001f) {
        Retain(c);
        specialOwner=C<uint8_t,0x0081f550>(c)!=0;
    }
    if(specialOwner) {
        // The key remains on the stack across the zero-argument slot75 call,
        // and slot42 consumes it together with the newly pushed float.
        uint64_t key=Key(actor);
        float t=static_cast<float>(V<double,75>(c));
        V<void,42>(c,t,key);
    } else {
        float t=static_cast<float>(V<double,70>(c,&root));
        if(t>V<double,41>(c,Key(actor))) V<void,42>(c,t,Key(actor));
    }
    for(int i=0;i<slaveCount;++i) {
        void* a=M<void*,0x00839ca0>(actor,i);
        if(a) { V<void,7>(a,&slave); float t=static_cast<float>(V<double,70>(c,&slave)); V<void,42>(c,t,Key(a)); }
    }
    if(useSlaves) { Retain(c); motionActor=M<void*,0x0083a1b0>(actor,c); }
    else motionActor=actor;
    if(changed) {
        Retain(c);
        void* a=M<void*,0x0083a280>(actor,c);
        if(a) {
            void* style=V<void*,1>(a,0x14cfd2f8);
            void* route=P(style,0x10c);
            if(route) {
                float scale=1.0f;
                if(M<void*,0x00580420>(actor)) {
                    void* x=M<void*,0x008469b0>(M<void*,0x00580420>(actor));
                    if(x) std::memcpy(&scale,static_cast<char*>(x)+12,4);
                }
                if(P(route,0x14)==static_cast<char*>(route)+0x14) {
                    void* s=V<void*,1>(actor,0x14cfd2f8);
                    if(I(actor,12)>0 && B(actor,32) && ((M<uint32_t,0x00953660>(s,0)&0xcf00)==0x200 || (M<uint32_t,0x00953690>(s)&0xcf00)==0xe00)) {
                        void* first=M<void*,0x00839ca0>(actor,0);
                        if(first && first!=a) M<void,0x0085d760>(route,first);
                    }
                }
                M<void,0x0085d760>(route,static_cast<char*>(style)+8);
                C<void,0x0085d980>(static_cast<char*>(style)+8,c,route,scale);
            }
        }
    }
    float offset=0.0f;
    uintptr_t capabilities=V<uintptr_t,3>(c,0);
    if(M<uint8_t,0x0081c880>(reinterpret_cast<void*>(capabilities),0x2000,0) && !V<void*,31>(c)) {
        uint32_t mask=routeMask | (1u<<(routeKind&31));
        if(void* service=C<void*,0x005eb7c0>()) offset=M<float,0x005ecba0>(service,mask);
    }
    auto finish=[&](uint8_t result) { Release(c); return result; };
    auto threshold=[&]() { return M<double,0x00848890>(this,0); };
    auto ensureRunning=[&]() { if(!running) M<void,0x0084f890>(this); };
    if(mode==1 || mode==2) {
        Retain(c);
        float distance=C<float,0x00849810>(c,Key(motionActor),routeKind,routeMask,0);
        if(threshold()>distance) {
            ensureRunning();
            if(!CanTransition(this) && !Start(this,true,0) && !Other(this)) Stop(this,0);
            active=1; return finish(1);
        }
        if(offset>0x1p-23f || offset< -0x1p-23f) {
            distance=distance-offset;
            if(threshold()>distance) {
                ensureRunning();
                if(!CanTransition(this)) {
                    delta=_mm_setzero_ps(); Retain(c);
                    C<void,0x00849600>(c,&delta,motionActor);
                    if((allowDelta || NearZero(delta,7) || *V<uint32_t*,12>(c)==0xe15cafc5 || !M<uint8_t,0x0084bc70>(this,&delta,1,1)) && !Start(this,true,1) && !Other(this)) Stop(this,0);
                }
                active=1; return finish(1);
            }
        }
    }
    float current=static_cast<float>(V<double,41>(c,Key(motionActor))+offset);
    double limit=V<double,75>(c);
    if(current>=limit) {
        // FCOMI/JB sends unordered values down the below-limit path too.
        if(mode!=1 && mode!=4) { active=1; Stop(this,1); return finish(0); }
        ensureRunning();
        if(CanTransition(this) || Start(this,ActorAvailable(this),0) || Other(this)) { active=1; return finish(1); }
        active=1; Stop(this,0); return finish(0);
    }
    active=0;
    switch(mode) {
    case 1: case 2:
        if(flag252) { Tune(this,2); if(M<uint8_t,0x0084fb20>(this)) active=1; break; }
        {
            void* hold=Resolve(this); if(hold) Retain(hold);
            if(hold) Retain(hold);
            float distance=C<float,0x00849810>(hold,Key(motionActor),routeKind,routeMask,1);
            while(remaining && threshold()>distance) --remaining;
            balance=static_cast<int32_t>(remaining);
            Tune(this,3); running=1;
            M<void,0x00852520>(this); active=1;
            if(hold) Release(hold);
        }
        break;
    case 3:
        transitioning=1;
        if(scheduled<target || M<uint8_t,0x00848560>(this)) {
            if(M<uint8_t,0x008491e0>(this)) { if(M<uint8_t,0x00850120>(this)) active=1; }
            else if(M<uint8_t,0x00848560>(this) || flag250 || flag251) active=1;
            break;
        }
        if(flag253 || Progression(this)) {
            remaining=0;
            if(Ready(this)) ResetCounts(this);
            else if(!flag251 && !flag250 && !stopping) M<void,0x008504d0>(this,pending);
            active=1; break;
        }
        Stop(this,0); active=1; return finish(0);
    case 4:
        transitioning=1;
        if(M<uint8_t,0x008491e0>(this)) { if(M<uint8_t,0x00850120>(this)) active=1; break; }
        if(M<uint8_t,0x00848560>(this) || flag250 || flag251) { active=1; break; }
        if(scheduled<target || M<uint8_t,0x00848560>(this)) {
            if(!stopping || sentinel->next!=sentinel || Start(this,ActorAvailable(this),0) || Other(this)) active=1;
        } else if(flag253 || Progression(this)) {
            remaining=0;
            if(Ready(this)) ResetCounts(this);
            else if(!flag251 && !flag250 && !stopping) M<void,0x008504d0>(this,pending);
            active=1;
        } else { Stop(this,0); active=1; return finish(0); }
        break;
    default: break;
    }
    if(completed<scheduled && sentinel->next==sentinel) {
        int n=scheduled-completed; scheduled=completed;
        while(n-->0) M<void,0x00850120>(this);
    }
    uint8_t result=active; active=1; return finish(result);
#else
    NotRewritten("M140 external root-motion actor interfaces (x86 original bridge)");
#endif
}
}
