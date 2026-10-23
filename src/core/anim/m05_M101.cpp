#include "ts3/anim/m05_M101.h"
#include "ts3/layout.h"
#include <cstring>
namespace ts3::anim::m101 {
uintptr_t Map(Clip* clip,Rig* rig,int32_t* output,void*) {
    auto* base=reinterpret_cast<unsigned char*>(clip);
    auto* records=clip->words[8]==0xffffffffu?nullptr:base+int32_t(clip->words[8]);
    uintptr_t result=reinterpret_cast<uintptr_t>(records);
    for(uint32_t i=0;i<clip->words[6];++i) {
        uint32_t hash; std::memcpy(&hash,records+i*20+4,4);
        result=hash;
        output[i]=hash==0x2af0a8aeu?-1:b21::FindIndex(&rig->skeleton,hash);
        if(hash!=0x2af0a8aeu) result=uint32_t(output[i]);
    }
    return (result&~uintptr_t(255))|1;
}
uintptr_t Bind(Clip* clip,Rig* rig,int32_t* output,void* extra) {
    if(clip->words[2]==1) return CallGameFunction<uintptr_t,0x005841c0>(clip,rig,output,extra);
    return Map(clip,rig,output,extra);
}
template<int Stride> static const unsigned char* Lower(const unsigned char* first,const unsigned char* last,const uint32_t* key) {
    auto count=(last-first)/Stride;
    while(count>0) {
        auto half=count/2;
        auto* middle=first+half*Stride;
        uint16_t value; std::memcpy(&value,middle,2);
        if(uint32_t(value)<*key) { first=middle+Stride; count-=half+1; }
        else count=half;
    }
    return first;
}
const unsigned char* Lower6(const unsigned char* a,const unsigned char* b,const uint32_t* k) { return Lower<6>(a,b,k); }
const unsigned char* Lower10(const unsigned char* a,const unsigned char* b,const uint32_t* k) { return Lower<10>(a,b,k); }
const unsigned char* Lower12(const unsigned char* a,const unsigned char* b,const uint32_t* k) { return Lower<12>(a,b,k); }
const unsigned char* Lower8(const unsigned char* a,const unsigned char* b,const uint32_t* k) { return Lower<8>(a,b,k); }
}
