#include "ts3/anim/m05_M117.h"
#include <cwchar>
#include <intrin.h>
namespace ts3::anim::m117 {
namespace {
const wchar_t* Get(void* p,const wchar_t* key) { return CallGameMethod<const wchar_t*,0x00471070>(p,key); }
bool Equal(const wchar_t* a,const wchar_t* b) { return std::wcscmp(a,b)==0; }
void Assign(void*& slot,void* value) {
    void* old=slot;
    if(value==old) return;
    if(value) _InterlockedExchangeAdd(reinterpret_cast<volatile long*>(static_cast<unsigned char*>(value)+sizeof(void*)),1);
    slot=value;
    if(old) {
        auto* refs=reinterpret_cast<volatile long*>(static_cast<unsigned char*>(old)+sizeof(void*));
        if(_InterlockedDecrement(refs)==0) {
            _InterlockedExchange(refs,1);
            CallGameVirtual<void,0>(old,1);
        }
    }
}
}
bool __fastcall Build(void* self,void*,void* builder,uint32_t tag,void* properties,Definition* out,uint32_t productFlags) {
    // All initial lookups precede any state writes (including product-flag conversion).
    const wchar_t* keys[]={L"age",L"gender",L"species",L"pre_start",L"start",L"cycle",L"stop",L"emergency_stop",L"post_stop",L"step",L"turn",L"unmirror",L"wait",L"speedinto",L"slowoutof",L"loopsound",L"stepLeft",L"stepRight",L"stopLeft",L"stopRight",L"turnLeft",L"turnRight",L"direction",L"oneStep",L"twoSteps",L"mirrorStop",L"productflags"};
    const wchar_t* v[27];
    for(unsigned i=0;i<27;++i) v[i]=Get(properties,keys[i]);
    if(v[26]) productFlags=CallGameFunction<uint32_t,0x004fed70>(v[26],static_cast<void*>(nullptr),10);
    out->tag=tag;
    out->direction=0xffffffff;
    const wchar_t* unmirror=L"a_trans_mirror2neutral_x";
    const wchar_t* ages[]={L"baby",L"toddler",L"child",L"teen",L"youngadult",L"adult",L"elder"};
    const wchar_t* defaults[]={L"b_trans_mirror2neutral_x",L"p_trans_mirror2neutral_x",L"c_trans_mirror2neutral_x",L"t_trans_mirror2neutral_x",L"h_trans_mirror2neutral_x",L"a_trans_mirror2neutral_x",L"e_trans_mirror2neutral_x"};
    if(!v[0]) out->age=5;
    else for(unsigned i=0;i<7;++i) if(Equal(v[0],ages[i])) { out->age=i; unmirror=defaults[i]; break; }
    out->gender=v[1] && Equal(v[1],L"female")?13:12;
    const wchar_t* species[]={L"human",L"horse",L"cat",L"littledog",L"dog",L"deer",L"raccoon",L"largebird",L"bird",L"shark",L"paddleboat",L"waterscooter",L"speedboat",L"rowboat",L"sailboat",L"windsurfboard",L"houseboatsmall",L"houseboatmedium",L"houseboatlarge"};
    const uint32_t codes[]={256,512,768,1280,1024,1536,1792,2048,2048,17408,3072,3328,3584,3840,17664,17920,16640,16896,17152};
    if(!v[2]) out->species=256;
    else for(unsigned i=0;i<19;++i) if(Equal(v[2],species[i])) { out->species=codes[i]; break; }
    if(v[22]) {
        if(Equal(v[22],L"up") || Equal(v[22],L"front")) out->direction=1;
        else if(Equal(v[22],L"downExitLeft")) out->direction=2;
        else out->direction=Equal(v[22],L"downExitRight")?3:0;
    }
    if(v[11]) unmirror=v[11];
    if(v[15]) CallGameMethod<void*,0x00408e20>(&out->loopSound,v[15],v[15]+std::wcslen(v[15]));
    out->disablePose=Get(properties,L"disablePoseModification")!=nullptr;
    out->stopOverride=Get(properties,L"stopOverrideSlaveRoutes")!=nullptr;
    out->disableTurn=Get(properties,L"disableTurnAnd")!=nullptr;
    out->skipOne=Get(properties,L"skipOneStairStep")!=nullptr;
    out->skipTwo=Get(properties,L"skipTwoStairSteps")!=nullptr;
    *reinterpret_cast<uint32_t*>(static_cast<unsigned char*>(builder)+0x30)=4;
    out->mirrorStop=v[25]!=nullptr;
    const unsigned indices[]={3,4,5,6,7,8,9,10,12,13,14,16,17,18,19,20,21,23,24};
    const unsigned slots[]={0,1,2,3,4,5,6,7,9,10,11,12,13,14,15,16,17,18,19};
    for(unsigned i=0;i<19;++i) {
        void* controller=CallGameMethod<void*,0x00840080>(self,builder,v[indices[i]],productFlags,i==4?1:0);
        Assign(out->controllers[slots[i]],controller);
    }
    Assign(out->controllers[8],CallGameMethod<void*,0x00840080>(self,builder,unmirror,productFlags,0));
    return true;
}
}
