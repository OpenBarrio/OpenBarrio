#include "ts3/ui/m08_B18.h"
#include <cstring>
namespace ts3::ui::b18 {
static Button* Owner(void* p) {
    return reinterpret_cast<Button*>(static_cast<uint8_t*>(p)-offsetof(Button,button));
}
void* __fastcall Query(Button* s,void*,uint32_t id) {
    if(id==0x8ED27E7Au) return s?&s->button:nullptr;
    return CallGameMethod<void*,0x00513600>(s,id);
}
uint32_t __fastcall GetState(void* p,void*) { return Owner(p)->state; }
uintptr_t __fastcall ModifyState(void* p,void*,uint32_t mask,uint32_t set) {
    uint32_t old=Owner(p)->state;
    return CallGameVirtual<uintptr_t,9>(p,(set&0xFF)?(old|mask):(old&~mask));
}
uintptr_t __fastcall ModifyOptions(void* p,void*,uint32_t mask,uint32_t set) {
    uint32_t old=Owner(p)->options;
    return CallGameVirtual<uintptr_t,12>(p,(set&0xFF)?(old|mask):(old&~mask));
}
uintptr_t __fastcall Set264(void* p,void*,uint32_t value) {
    auto s=Owner(p); s->value264=value;
    return CallGameVirtual<uintptr_t,36>(&s->secondary);
}
uintptr_t __fastcall Set26C(Button* s,void*,uint32_t value) {
    s->value26C=value;
    return CallGameVirtual<uintptr_t,36>(&s->secondary);
}
uintptr_t __fastcall SetImage(void* p,void*,uint32_t index,uintptr_t value) {
    if(index>=8) return index;
    auto s=Owner(p); s->images[index]=value;
    return CallGameVirtual<uintptr_t,43>(s);
}
uintptr_t __fastcall GetImage(void* p,void*,uint32_t index) {
    return index<8?Owner(p)->images[index]:0;
}
uintptr_t __fastcall SelectImage(Button* s,void*) {
    const bool enabled=(CallGameVirtual<uint32_t,10>(&s->secondary)&2)!=0;
    const uint32_t state=s->state;
    uintptr_t selected=0;
    if(!enabled && s->images[1]) selected=s->images[1];
    else {
        if(state&4) {
            if((state&2) && s->images[6]) selected=s->images[6];
            else if((state&8) && s->images[5]) selected=s->images[5];
            else if(s->images[4]) selected=s->images[4];
        }
        if(!selected && (state&2)) selected=s->images[2];
        if(!selected && (state&8)) selected=s->images[3];
        if(!selected && (state&16)) selected=s->images[7];
        if(!selected && enabled) selected=s->images[0];
    }
    // No candidate means keep the previous selection, not clear it.
    if(selected) s->selected=selected;
    return CallGameVirtual<uintptr_t,36>(&s->secondary);
}
static float Stored(float f) {
    // Original FLD/FSTP quiets signaling NaNs even when the destination aliases.
    uint32_t bits; std::memcpy(&bits,&f,4);
    if((bits&0x7F800000u)==0x7F800000u && (bits&0x007FFFFFu)) bits|=0x00400000u;
    std::memcpy(&f,&bits,4); return f;
}
float* __fastcall GetRectangle(void* p,void*,float* a,float* b,float* c,float* d) {
    auto s=Owner(p);
    *a=Stored(s->rectangle[0]); *b=Stored(s->rectangle[1]);
    *c=Stored(s->rectangle[2]); *d=Stored(s->rectangle[3]); return c;
}
float* __fastcall GetPair(void* p,void*,float* a,float* b) {
    auto s=Owner(p); *a=Stored(s->pair[0]); *b=Stored(s->pair[1]); return a;
}
uint8_t __fastcall Press(Button* s,void*,int32_t kind,uint32_t key,uint32_t) {
    if(kind==0 || (kind>=2 && key!=0x7D8)) return 0;
    if(!(s->state&2)) {
        CallGameVirtual<uintptr_t,10>(&s->button,2u,1u);
        if(s->options&1) CallGameVirtual<uintptr_t,41>(s);
    }
    return 1;
}
uint8_t __fastcall Release(Button* s,void*,int32_t kind,uint32_t key,uint32_t) {
    if(kind==0 || (kind>=2 && key!=0x7D8)) return 0;
    if(s->state&2) {
        CallGameVirtual<uintptr_t,10>(&s->button,2u,0u);
        if(!(s->options&1)) CallGameVirtual<uintptr_t,41>(s);
    }
    return 1;
}
uint8_t __fastcall CapturePress(Button* s,void*,uint32_t,uint32_t,uint32_t,uint32_t) {
    if(CallGameVirtual<void*,24>(s->manager,1u)!=&s->secondary)
        CallGameVirtual<uintptr_t,25>(s->manager,1u,static_cast<void*>(&s->secondary),0u);
    if(!(s->state&2) && (s->state&8)) {
        CallGameVirtual<uintptr_t,10>(&s->button,2u,1u);
        if(s->options&1) CallGameVirtual<uintptr_t,41>(s);
    }
    return 1;
}
uint8_t __fastcall CaptureRelease(Button* s,void*,uint32_t,uint32_t,uint32_t,uint32_t) {
    if(CallGameVirtual<void*,24>(s->manager,1u)==&s->secondary)
        CallGameVirtual<uintptr_t,26>(s->manager,1u,static_cast<void*>(&s->secondary),0u);
    if(s->state&2) {
        CallGameVirtual<uintptr_t,10>(&s->button,2u,0u);
        if(!(s->options&1)) CallGameVirtual<uintptr_t,41>(s);
    }
    return 1;
}
}
