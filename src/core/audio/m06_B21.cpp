#include "ts3/audio/m06_B21.h"
#include <cmath>
#include <cstring>
namespace ts3::audio {
B21Services b21Services{};
namespace {
b21* Init(b21* s,int kind) {
#if defined(_M_IX86)
    const uintptr_t tables[]={0x01029CC0,0x01029DB0,0x01029DF0};
    s->vtable=reinterpret_cast<void**>(tables[kind]);
    s->secondary=reinterpret_cast<void*>(0x01029EE0);
    s->reference=reinterpret_cast<void*>(0x01029888);
#else
    s->vtable=b21Services.tables[kind]; s->secondary=b21Services.secondary; s->reference=b21Services.reference;
#endif
    s->state=0; s->enabled=1; s->value=0; s->link=nullptr; s->owner=nullptr;
    s->nameHash=0; s->tick=0; s->period=1; s->radius=kind == 0 ? 1.0f : 2.0f;
    if (kind == 1) s->distanceSquared=10000.0f;
    return s;
}
uint32_t Name(b21* s,const char* name) {
    if (s->nameHash == 0) {
#if defined(_M_IX86)
        s->nameHash=CallGameFunction<uint32_t,0x004FA590>(name,0x811c9dc5u,1);
#else
        s->nameHash=b21Services.hash(name,0x811c9dc5u,1);
#endif
    }
    return s->nameHash;
}
void* System() {
#if defined(_M_IX86)
    return CallGameFunction<void*,0x008B0A70>();
#else
    return b21Services.system();
#endif
}
void* World() {
#if defined(_M_IX86)
    return *reinterpret_cast<void**>(0x011ECBC4);
#else
    return b21Services.world;
#endif
}
uintptr_t Publish(b21* s,float f) {
#if defined(_M_IX86)
    return CallGameVirtual<uintptr_t,14>(s,f);
#else
    return b21Services.publish(s,f);
#endif
}
uintptr_t Read(void* r,unsigned slot,uint32_t key,void* out) {
#if defined(_M_IX86)
    if (slot == 10) return CallGameVirtual<uintptr_t,10>(r,key,out);
    return CallGameVirtual<uintptr_t,12>(r,key,out);
#else
    return b21Services.read(r,slot,key,out);
#endif
}
uintptr_t Configure(b21* s,bool outside) {
    void* r=nullptr; auto sys=System();
    uint32_t lo=outside?0xCD2F3090u:0xF32330B5u;
    uint32_t hi=outside?0xA5E415DFu:0x95302B69u;
#if defined(_M_IX86)
    auto result=CallGameVirtual<uintptr_t,7>(sys,lo,hi,&r,0x1407ECu);
#else
    auto result=b21Services.resolve(sys,lo,hi,&r,0x1407ECu);
#endif
    uint8_t low=0;
    if (static_cast<uint8_t>(result)) {
        Read(r,10,0xCB3F671D,&s->period);
        Read(r,12,0x2E65B70F,&s->radius);
        if (outside) {
            float distance;
            if (static_cast<uint8_t>(Read(r,12,0x03593710,&distance)))
                s->distanceSquared=distance*distance;
        }
#if defined(_M_IX86)
        result=CallGameMethod<uintptr_t,0x008A6D90>(s);
#else
        result=b21Services.configure(s);
#endif
        low=static_cast<uint8_t>(result);
    }
    if (r) {
#if defined(_M_IX86)
        result=CallGameVirtual<uintptr_t,1>(r);
#else
        result=b21Services.release(r);
#endif
    }
    return (result & ~uintptr_t(255)) | low;
}
B21Vector Position(void* sys,int index) {
#if defined(_M_IX86)
    auto p=CallGameVirtual<const float*,6>(sys,index);
#else
    auto p=b21Services.position(sys,index);
#endif
    return {p[0],p[1],p[2],p[0]};
}
B21Lots Lots(void* w) {
#if defined(_M_IX86)
    return *CallGameMethod<B21Lots*,0x00C68960>(w,0);
#else
    return b21Services.lots(w);
#endif
}
void* Rooms(void* lot) {
#if defined(_M_IX86)
    return *reinterpret_cast<void**>(static_cast<char*>(lot)+0x298);
#else
    return b21Services.rooms(lot);
#endif
}
void Bounds(void* lot,B21Vector& a,B21Vector& b) {
#if defined(_M_IX86)
    a=*reinterpret_cast<B21Vector*>(static_cast<char*>(lot)+0x230);
    b=*reinterpret_cast<B21Vector*>(static_cast<char*>(lot)+0x240);
#else
    b21Services.bounds(lot,&a,&b);
#endif
}
B21Vector Local(void* lot,const B21Vector& p) {
    B21Vector out;
#if defined(_M_IX86)
    CallGameMethod<void,0x00AB88A0>(lot,&p,&out);
#else
    b21Services.local(lot,&p,&out);
#endif
    return out;
}
bool InBounds(void* lot,const B21Vector& p) {
    int32_t x,z;
#if defined(_M_IX86)
    x=*reinterpret_cast<int32_t*>(static_cast<char*>(lot)+0x174);
    z=*reinterpret_cast<int32_t*>(static_cast<char*>(lot)+0x178);
#else
    b21Services.dimensions(lot,&x,&z);
#endif
    return p.x>=0 && static_cast<float>(x)>=p.x && p.z>=0 && static_cast<float>(z)>=p.z;
}
uint16_t Room(void* rooms,B21Cell& cell,int a,int b) {
#if defined(_M_IX86)
    return CallGameMethod<uint16_t,0x00B499D0>(rooms,&cell,a,b);
#else
    return b21Services.room(rooms,&cell,a,b);
#endif
}
float Coverage(uint32_t count,uint32_t total) {
#if defined(_M_IX86)
    float out;
    // Preserve x87 division/subtraction at the caller's precision, one final store.
    const double n=static_cast<double>(count),d=static_cast<double>(total);
    __asm {
        fld qword ptr [n]
        fld qword ptr [d]
        fdivp st(1),st(0)
        fld1
        fsubrp st(1),st(0)
        fstp dword ptr [out]
    }
    return out;
#else
    return static_cast<float>(1.0-static_cast<double>(count)/total);
#endif
}
float UnsignedFloat(uint32_t bits) {
#if defined(_M_IX86)
    float result;
    const float correction=4294967296.0f;
    __asm {
        fild dword ptr [bits]
        cmp dword ptr [bits],0
        jge converted
        fadd dword ptr [correction]
    converted:
        fstp dword ptr [result]
    }
    return result;
#else
    return static_cast<float>(static_cast<double>(bits));
#endif
}
float Angle(int i,bool stagger) {
    const float step=1.0471976f, phase=0.52359879f;
    float angle;
#if defined(_M_IX86)
    __asm {
        fild dword ptr [i]
        fmul dword ptr [step]
    }
    if (stagger) { __asm { fadd dword ptr [phase] } }
    __asm { fstp dword ptr [angle] }
#else
    angle=static_cast<float>(static_cast<double>(i)*step+(stagger?phase:0.0));
#endif
    return angle;
}
void SinCos(float a,float& s,float& c) {
#if defined(_M_IX86)
    float sineValue,co;
    __asm {
        fld dword ptr [a]
        fsin
        fstp dword ptr [sineValue]
        fld dword ptr [a]
        fcos
        fstp dword ptr [co]
    }
    s=sineValue; c=co;
#else
    s=std::sin(a); c=std::cos(a);
#endif
}
void Append(B21Points* p,const B21Vector* value) {
    if (p->end < p->capacity) {
        auto dest=p->end++;
        if (dest && value) *dest=*value;
    } else {
#if defined(_M_IX86)
        B21Vector unused;
        CallGameMethod<void,0x0086EE00>(p,p->end,value?value:&unused);
#else
        b21Services.grow(p,value);
#endif
    }
}
}
b21* b21::InitOcean() { return Init(this,0); }
b21* b21::InitOutside() { return Init(this,1); }
b21* b21::InitOutsideLot() { return Init(this,2); }
uint32_t b21::NameOcean() { return Name(this,"ocean"); }
uint32_t b21::NameOutside() { return Name(this,"outside"); }
uint32_t b21::NameOutsideLot() { return Name(this,"outsidelot"); }
uintptr_t b21::ConfigureOutside() { return Configure(this,true); }
uintptr_t b21::ConfigureOutsideLot() { return Configure(this,false); }
bool B21Intersects(const B21Vector* a,const B21Vector* b) {
    auto d=_mm_sub_ps(_mm_load_ps(&a->x),_mm_load_ps(&b->x));
    auto v=_mm_mul_ps(_mm_mul_ps(_mm_set_ps(-1,1,1,1),d),d);
    auto sum=_mm_add_ps(_mm_shuffle_ps(v,v,14),v);
    sum=_mm_add_ps(_mm_shuffle_ps(sum,sum,1),sum);
    return _mm_cvtss_f32(sum)<0.0f;
}
B21Vector* B21Sample(const B21Vector* center,float radius,B21Points* points) {
    Append(points,center);
    for (int ring=0;ring<2;++ring) {
        for (int i=0;i<6;++i) {
            Append(points,nullptr);
            auto dest=points->end-1;
            float s,c; SinCos(Angle(i,ring == 0),s,c);
            auto v=_mm_mul_ps(_mm_set_ps(s,c,0,s),_mm_set1_ps(radius));
            _mm_store_ps(&dest->x,v);
            _mm_store_ps(&dest->x,_mm_add_ps(v,_mm_load_ps(&center->x)));
        }
    }
    return points->end-1;
}
uintptr_t b21::EvaluateOutside(uint32_t,uint32_t) {
    ++tick;
    int32_t t; std::memcpy(&t,&tick,4);
    int32_t quotient=t/period;
    auto world=World();
    if (t%period || !world) return static_cast<uint32_t>(quotient);
#if defined(_M_IX86)
    auto key=CallGameMethod<uint64_t,0x00C5FB00>(world,0);
    auto lot=CallGameMethod<char*,0x00C62D40>(world,uint32_t(key),uint32_t(key>>32));
    bool below=lot && *reinterpret_cast<int32_t*>(lot+0x39C)<0;
#else
    bool below=b21Services.underground(world);
#endif
    if (below) return Publish(this,0);
    auto sys=System(); auto center=Position(sys,2),camera=Position(sys,1);
    auto diff=_mm_sub_ps(_mm_load_ps(&center.x),_mm_load_ps(&camera.x));
    auto sq=_mm_mul_ps(diff,diff);
    auto sum=_mm_add_ps(_mm_add_ps(_mm_shuffle_ps(sq,sq,2),_mm_shuffle_ps(sq,sq,1)),sq);
    // COMISS/JB takes the sampling branch for unordered as well as less-than.
    if (_mm_cvtss_f32(sum)>=distanceSquared) return Publish(this,1);
    alignas(16) B21Vector storage[13]; B21Points points{storage,storage,storage+13};
    B21Sample(&center,radius,&points);
    // EA overwrites the sphere-test W with the sampling radius.
    auto sphereCenter=center; sphereCenter.w=radius;
    uint32_t covered=0; auto lots=Lots(world);
    for (auto it=lots.begin;it!=lots.end;++it) {
        auto rooms=Rooms(*it); if (!rooms) continue;
        B21Vector sphere;
#if defined(_M_IX86)
        CallGameMethod<void,0x0069D460>(&sphere,static_cast<char*>(*it)+0x230);
#else
        b21Services.sphere(*it,&sphere);
#endif
        if (!B21Intersects(&sphereCenter,&sphere)) continue;
        for (auto p=points.begin;p!=points.end;++p) {
            auto local=Local(*it,*p); if (!InBounds(*it,local)) continue;
#if defined(_M_IX86)
            auto level=CallGameMethod<int32_t,0x00AB78B0>(*it,&local,0);
#else
            auto level=b21Services.level(*it,&local);
#endif
            B21Cell cell{local.x,local.z,level};
            if (level<0 || Room(rooms,cell,0,0)) ++covered;
        }
    }
    return Publish(this,Coverage(covered,static_cast<uint32_t>(points.end-points.begin)));
}
uintptr_t b21::EvaluateOutsideLot(uint32_t,uint32_t) {
    ++tick;
    int32_t t; std::memcpy(&t,&tick,4);
    int32_t quotient=t/period;
    auto world=World();
    if (t%period || !world) return static_cast<uint32_t>(quotient);
    auto center=Position(System(),2);
    alignas(16) B21Vector storage[13]; B21Points points{storage,storage,storage+13};
    B21Sample(&center,radius,&points);
    center.w=radius;
    uint32_t covered=0; auto lots=Lots(world);
    for (auto it=lots.begin;it!=lots.end;++it) {
        B21Vector lo,hi; Bounds(*it,lo,hi);
        auto a=_mm_load_ps(&lo.x),b=_mm_load_ps(&hi.x);
        auto d=_mm_sub_ps(b,a),sq=_mm_mul_ps(d,d);
        auto sum=_mm_add_ps(_mm_add_ps(_mm_shuffle_ps(sq,sq,2),_mm_shuffle_ps(sq,sq,1)),sq);
        auto radius4=_mm_sqrt_ps(_mm_shuffle_ps(sum,sum,0));
        auto middle=_mm_add_ps(a,b);
        auto temp=_mm_shuffle_ps(middle,radius4,250);
        auto ball=_mm_mul_ps(_mm_shuffle_ps(middle,temp,132),_mm_set1_ps(0.5f));
        B21Vector sphere; _mm_store_ps(&sphere.x,ball);
        if (!B21Intersects(&center,&sphere)) continue;
        for (auto p=points.begin;p!=points.end;++p) {
            auto local=Local(*it,*p);
            if (InBounds(*it,local)) ++covered;
        }
    }
    return Publish(this,Coverage(covered,static_cast<uint32_t>(points.end-points.begin)));
}
#if defined(_M_IX86)
static uintptr_t __fastcall B21RoomLive(b21* self,void*,uint32_t,uint32_t) {
#else
static uintptr_t B21RoomLive(b21* self,void*,uint32_t,uint32_t) {
#endif
    auto world=World(); auto center=Position(System(),2);
#if defined(_M_IX86)
    auto lot=CallGameMethod<void*,0x00C68990>(world,center.x,center.z);
#else
    auto lot=b21Services.lotAt(world,center.x,center.z);
#endif
    uint32_t value=0;
    if (lot) {
        B21Cell cell{-1,-1,0};
#if defined(_M_IX86)
        CallGameFunction<void,0x00ADD740>(lot,&center,&cell);
#else
        auto local=Local(lot,center); cell={local.x,local.z,b21Services.level(lot,&local)};
#endif
        auto rooms=Rooms(lot); auto room=Room(rooms,cell,1,0);
        if (room) {
#if defined(_M_IX86)
            auto map=CallGameMethod<void*,0x00B485B0>(rooms,1);
            value=CallGameMethod<uint32_t,0x00C060B0>(map,static_cast<uint32_t>(room));
#else
            value=b21Services.roomValue(b21Services.roomMap(rooms,1),room);
#endif
        }
    }
    return Publish(self,UnsignedFloat(value));
}
#if defined(_M_IX86)
__declspec(naked) uintptr_t b21::EvaluateRoom(uint32_t,uint32_t) {
    __asm {
        cmp dword ptr ds:[0x011ECBC4],0
        jne live
        ret 8
    live:
        jmp B21RoomLive
    }
}
#else
uintptr_t b21::EvaluateRoom(uint32_t a,uint32_t b) {
    if (!World()) return 0; // Native ABI has no meaningful incoming EAX.
    return B21RoomLive(this,nullptr,a,b);
}
#endif
}
