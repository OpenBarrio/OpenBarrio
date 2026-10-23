#include "ts3/anim/m05_B16.h"
#include <intrin.h>
#include <cstring>
namespace ts3::anim::b16 {
static float Quiet(float value) {
    // EA's FLD/FSTP round trip quiets signalling NaNs before forwarding.
    uint32_t bits; std::memcpy(&bits,&value,4);
    if ((bits&0x7f800000)==0x7f800000 && (bits&0x7fffff)) bits|=0x400000;
    std::memcpy(&value,&bits,4); return value;
}
void SetTime(State* self,float value) {
    if(self->delegate) CallGameVirtual<void,2>(self->delegate,Quiet(value));
}
void SetWeight(State* self,float value) {
    if(self->delegate) CallGameVirtual<void,4>(self->delegate,Quiet(value));
}
float Time(State* self) {
    return self->owner && self->delegate ? CallGameVirtual<float,5>(self->delegate) : 0.0f;
}
float Duration(State* self) {
    return self->owner && self->delegate ? CallGameVirtual<float,6>(self->delegate) : 0.0f;
}
static float DefaultValue() {
#if defined(_M_IX86)
    return *reinterpret_cast<const float*>(0x0114f594);
#else
    AbortNotRewritten("m05_B16 default controller value"); return 0.0f;
#endif
}
float Rate(State* self) {
    if(!self->owner) return 0.0f;
    return self->delegate ? CallGameVirtual<float,7>(self->delegate) : DefaultValue();
}
float Start(State* self) {
    if(!self->owner) return 0.0f;
    return self->delegate ? CallGameVirtual<float,8>(self->delegate) : DefaultValue();
}
float End(State* self) {
    if(!self->owner) return 0.0f;
    return self->delegate ? CallGameVirtual<float,9>(self->delegate) : DefaultValue();
}
float Weight(State* self) {
    if(!self->owner) return 0.0f;
    return self->delegate ? CallGameVirtual<float,10>(self->delegate) : DefaultValue();
}
bool Active(State* self) {
    return self->delegate ? CallGameVirtual<bool,11>(self->delegate) : false;
}
bool Finished(State* self) {
    return self->delegate ? CallGameVirtual<bool,12>(self->delegate) : false;
}
void Reset(State* self) {
    if(self->delegate) CallGameVirtual<void,1>(self->delegate);
}
uint32_t Type(void*) { return 0x073b9d8c; }
void* Query(void* self,uint32_t id) {
    return id==0x073b9d8c ? static_cast<unsigned char*>(self)-8 : nullptr;
}
static uintptr_t Assign(Ref*& field,Ref* value) {
    Ref* old=field;
    if(old==value) return reinterpret_cast<uintptr_t>(old);
    if(value) _InterlockedExchangeAdd(reinterpret_cast<volatile long*>(&value->refs),1);
    field=value;
    if(old && _InterlockedExchangeAdd(reinterpret_cast<volatile long*>(&old->refs),-1)==1) {
        _InterlockedExchange(reinterpret_cast<volatile long*>(&old->refs),1);
        return CallGameVirtual<uintptr_t,0>(old,uint32_t(1));
    }
    return reinterpret_cast<uintptr_t>(old);
}
uintptr_t SetOwner(State* self,Ref* value) { return Assign(self->owner,value); }
uintptr_t SetAuxiliary(State* self,Ref* value) { return Assign(self->auxiliary,value); }
}
