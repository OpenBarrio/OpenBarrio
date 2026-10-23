#include "ts3/anim/m05_B34.h"
#include <cstring>
namespace ts3::anim::b34 {
Pool* __fastcall ConstructPool(Pool* s,void*) {
    void* temporary=nullptr;
    CallGameMethod<void,0x004E07A0>(&temporary,static_cast<void*>(nullptr),512u,16u,4u,0u);
    s->sentinel.next=nullptr; s->sentinel.previous=nullptr; s->free=nullptr;
    CallGameMethod<void,0x004E07A0>(&s->free,temporary,512u,16u,4u,0u);
    s->sentinel.next=&s->sentinel; s->sentinel.previous=&s->sentinel;
    CallGameMethod<void,0x004E07A0>(&s->free,s->storage,512u,16u,4u,0u);
    return s;
}
const char* __fastcall TreeName(void*,void*) {
#if defined(_M_IX86)
    return reinterpret_cast<const char*>(0x01023218);
#else
    return "Tree Builder";
#endif
}
uint32_t* __fastcall CopyFirst(void*,void*,uint32_t* out,const FourValues* in) { *out=in->values[0]; return out; }
float __fastcall TreeValue(const Tree* s,void*) {
#if defined(_M_IX86)
    return s->value; // FLD supplies the original ST0 result.
#else
    uint32_t bits; std::memcpy(&bits,&s->value,4);
    if((bits&0x7f800000u)==0x7f800000u && (bits&0x007fffffu)) bits|=0x00400000u;
    float value; std::memcpy(&value,&bits,4); return value;
#endif
}
uint8_t __fastcall AnyChild(Tree* s,void*) {
    Node* end=&s->children;
    if(end->next==end) return 1;
    for(Node* n=end->next;n!=end;n=n->next) {
        if(static_cast<uint8_t>(CallGameVirtual<uint32_t,4>(n->child))!=0) return 1;
    }
    return 0;
}
uint32_t __fastcall Visit(Tree* s,void*,uint32_t depth) {
    uint32_t result=depth;
    for(Node* n=s->children.next;n!=&s->children;n=n->next)
        result=CallGameVirtual<uint32_t,7>(n->child,depth+1u);
    return result;
}
uint32_t* __fastcall CopySecond(void*,void*,uint32_t* out,const FourValues* in) { *out=in->values[1]; return out; }
const char* __fastcall TurnName(void*,void*) {
#if defined(_M_IX86)
    return reinterpret_cast<const char*>(0x010233F8);
#else
    return "TurnAngle";
#endif
}
uint32_t __fastcall Turn(void*,void*,const Parameters* in,uint32_t* out) { *out=in->turnAngle; return 0; }
uint32_t __fastcall Distance(void*,void*,const Parameters* in,uint32_t* out) { *out=in->distance; return 0; }
uint32_t* __fastcall CopyFourth(void*,void*,uint32_t* out,const FourValues* in) { *out=in->values[3]; return out; }
const char* __fastcall DistanceName(void*,void*) {
#if defined(_M_IX86)
    return reinterpret_cast<const char*>(0x01023380);
#else
    return "Distance";
#endif
}
const char* __fastcall MoveName(void*,void*) {
#if defined(_M_IX86)
    return reinterpret_cast<const char*>(0x010233BC);
#else
    return "MoveAngle";
#endif
}
uint32_t* __fastcall CopyThird(void*,void*,uint32_t* out,const FourValues* in) { *out=in->values[2]; return out; }
uint32_t __fastcall Move(void*,void*,const Parameters* in,uint32_t* out) {
    *out=in->moveAngle; // Copy precedes the distance read, including when out aliases distance.
    float distance; std::memcpy(&distance,&in->distance,4);
    return distance==0.0f ? 1u : 0u;
}
}
