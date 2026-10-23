#pragma once
#include "ts3/layout.h"
#include "ts3/math/float_values.h"
namespace ts3::anim::b40 {
using ts3::math::Float4Value;
struct Link { Link* next; Link* previous; };
struct Range { void* vtable; uint32_t opaque[2]; float low0,low1,high0,high1; uint32_t mode; uint8_t enabled,flag; };
struct alignas(16) Node { Link link; uint8_t opaque[8]; Float4Value position; uint8_t unknown[16]; uint8_t active; };
struct alignas(16) Vehicle { void* vtable; uint32_t unknown; Link nodes; uint8_t opaque[32]; Float4Value position; float radius,speed,bound; uint8_t tail[0xac-0x4c]; float scale; };
struct Pair { uint32_t lo,hi; };
struct Point { float x,y; };
struct alignas(16) Placement { Float4Value position,zero; float weight; void* region; uint32_t flags[3]; };
struct Scratch { uint8_t prefix[20]; Link nodes; uint8_t opaque[0xa5-0x1c]; uint8_t dirty; };
// TS3W.exe 0x0085A8F0: Read the byte flag (AL only).
uint8_t Enabled(const Range*);
// TS3W.exe 0x0085A940: Return the controller type identifier.
uint32_t Type(const void*);
// TS3W.exe 0x0085A950: Query either supported controller interface identifier.
void* Query(void*,uint32_t);
// TS3W.exe 0x0085A980: Test four endpoint comparisons with EA's tolerance.
uint32_t Endpoints(const Range*);
// TS3W.exe 0x0085AAA0: Search a nullable array of identifier pairs (AL result).
bool Contains(uint32_t,uint32_t,const Pair*,uint32_t);
// TS3W.exe 0x0085AAF0: Store the same speed in both scalar fields.
void SetSpeed(Vehicle*,float);
// TS3W.exe 0x0085AC20: Intersect two segments with length-relative endpoint margins.
bool Intersects(const Point*,const Point*,const Point*,const Point*,float);
// TS3W.exe 0x0085AD90: Resolve a world region using the requested fallback query.
void* Region(const Float4Value*,bool);
// TS3W.exe 0x0085AE60: Build a ground placement record; return its output pointer.
Placement* Place(const Float4Value*,Placement*);
// TS3W.exe 0x0085AF40: Test the intrusive list sentinel (AL result).
bool Empty(const Vehicle*);
// TS3W.exe 0x0085AF50: Strict bounding sphere broad-phase test, ignoring W.
bool Broad(const Vehicle*,const Float4Value*,float,bool);
// TS3W.exe 0x0085B0C0: Test active list nodes after the strict broad-phase gate.
bool Hit(const Vehicle*,const Float4Value*,float,bool,bool);
// TS3W.exe 0x0085B430: Free every scratch node and restore its sentinel and dirty flag.
void Clear(Scratch*);
TS3_X86_OFFSET(Range,enabled,0x20);
TS3_X86_OFFSET(Vehicle,position,0x30);
TS3_X86_OFFSET(Vehicle,scale,0xac);
TS3_X86_OFFSET(Node,active,0x30);
TS3_X86_OFFSET(Scratch,dirty,0xa5);
}
namespace ts3::anim::b40 {
// ABI-only adapters for mechanical thiscall hook generation.
struct Hooks {
    uint32_t Type() { return b40::Type(this); }
    void* Query(uint32_t id) { return b40::Query(this,id); }
    uint32_t Endpoints() { return b40::Endpoints(reinterpret_cast<Range*>(this)); }
    void SetSpeed(float f) { b40::SetSpeed(reinterpret_cast<Vehicle*>(this),f); }
    bool Empty() { return b40::Empty(reinterpret_cast<Vehicle*>(this)); }
    bool Broad(void* p,float f,bool b) { return b40::Broad(reinterpret_cast<Vehicle*>(this),static_cast<Float4Value*>(p),f,b); }
    bool Hit(void* p,float f,bool a,bool b) { return b40::Hit(reinterpret_cast<Vehicle*>(this),static_cast<Float4Value*>(p),f,a,b); }
    void Clear() { b40::Clear(reinterpret_cast<Scratch*>(this)); }
    static bool Contains(uint32_t a,uint32_t b,void* p,uint32_t n) { return b40::Contains(a,b,static_cast<Pair*>(p),n); }
    static bool Intersects(void* a,void* b,void* c,void* d,float f) { return b40::Intersects(static_cast<Point*>(a),static_cast<Point*>(b),static_cast<Point*>(c),static_cast<Point*>(d),f); }
    static void* Region(void* p,bool b) { return b40::Region(static_cast<Float4Value*>(p),b); }
    static void* Place(void* p,void* o) { return b40::Place(static_cast<Float4Value*>(p),static_cast<Placement*>(o)); }
};
}
