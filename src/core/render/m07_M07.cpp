#include "ts3/render/m07_M07.h"
namespace ts3::render::m07::m07_init {
uintptr_t OriginalDispatch(uint32_t op,void* self,uintptr_t a,uintptr_t b,uintptr_t c) {
#if defined(_M_IX86)
    switch(op) {
    case 0xFFFFFF00: return *reinterpret_cast<uintptr_t*>(a);
    case 0xFFFFFF01: ts3::CallGameVirtual<void,1>(self); return 0;
    case 0xFFFFFF02: *reinterpret_cast<uint32_t*>(a)=uint32_t(b); return 0;
    case 0xFFFFFF03: ts3::CallGameVirtual<void,9>(self,reinterpret_cast<void*>(a),uint32_t(b),uint32_t(0),uint32_t(0)); return 0;
    case 0x58AAD0: return ts3::CallGameFunction<uint8_t,0x58AAD0>(*reinterpret_cast<void**>(0x114FAB0),reinterpret_cast<const wchar_t*>(a),int(b));
    case 0x58ABA0: return ts3::CallGameFunction<int32_t,0x58ABA0>(*reinterpret_cast<void**>(0x114FAB0),reinterpret_cast<const wchar_t*>(a),int(b));
    case 0x523860: return ts3::CallGameMethod<uint8_t,0x523860>(reinterpret_cast<void*>(0x11CDDE0),uint32_t(a));
    case 0x4E3FB0: return ts3::CallGameFunction<uintptr_t,0x4E3FB0>(uint32_t(a),4,0,reinterpret_cast<const char*>(b),0,0,0,0);
    case 0x4E3F90: return ts3::CallGameFunction<uintptr_t,0x4E3F90>(uint32_t(a),reinterpret_cast<const char*>(b),0,0,0,0);
    case 0x61A300: return ts3::CallGameMethod<uintptr_t,0x61A300>(self);
    case 0x6CFCA0: return ts3::CallGameMethod<uintptr_t,0x6CFCA0>(self);
    case 0x61A380: return ts3::CallGameMethod<uintptr_t,0x61A380>(self);
    case 0x6CBD00: return ts3::CallGameMethod<uintptr_t,0x6CBD00>(self);
    case 0x703510: return ts3::CallGameMethod<uintptr_t,0x703510>(self);
    case 0x6B1AA0: return ts3::CallGameMethod<uintptr_t,0x6B1AA0>(self);
    case 0x6DE9C0: return ts3::CallGameMethod<uintptr_t,0x6DE9C0>(self);
    case 0x61A010: return ts3::CallGameMethod<uint8_t,0x61A010>(self,reinterpret_cast<void*>(a),uint32_t(b));
    case 0x6CF470: return ts3::CallGameMethod<uint8_t,0x6CF470>(self,reinterpret_cast<void*>(a),uint32_t(b),uint32_t(c));
    case 0x61A1C0: return ts3::CallGameMethod<uint8_t,0x61A1C0>(self,uint32_t(a),uint32_t(b),reinterpret_cast<const char*>(c));
    case 0x79B840: return ts3::CallGameMethod<uintptr_t,0x79B840>(self,reinterpret_cast<const char*>(a),uint32_t(b),uint32_t(c));
    case 0x79A130: return ts3::CallGameMethod<uintptr_t,0x79A130>(self,uint32_t(a),uint32_t(b),uint32_t(c));
    case 0x79A160: return ts3::CallGameMethod<uintptr_t,0x79A160>(self,reinterpret_cast<const char*>(a),uint32_t(b),uint32_t(c));
    case 0x6E9510: return ts3::CallGameFunction<uint8_t,0x6E9510>();
    case 0x6CB850: ts3::CallGameFunction<void,0x6CB850>(reinterpret_cast<void*>(a)); return 0;
    case 0x6FDE20: ts3::CallGameFunction<void,0x6FDE20>(reinterpret_cast<void*>(a)); return 0;
    case 0x6B05B0: ts3::CallGameFunction<void,0x6B05B0>(reinterpret_cast<void*>(a)); return 0;
    case 0x6DE480: ts3::CallGameFunction<void,0x6DE480>(reinterpret_cast<void*>(a)); return 0;
    case 0x60E2C0: return ts3::CallGameFunction<uintptr_t,0x60E2C0>();
    case 0x610500: ts3::CallGameMethod<void,0x610500>(self,reinterpret_cast<void*>(a)); return 0;
    case 0x44C470: return ts3::CallGameFunction<uintptr_t,0x44C470>();
    case 0x6935A0: ts3::CallGameMethod<void,0x6935A0>(self); return 0;
    }
#else
    (void)op;(void)self;(void)a;(void)b;(void)c;
#endif
    throw std::runtime_error("m07_M07: unavailable original boundary");
}
uint8_t Initialize(System* s,Dispatch dispatch) {
    auto call=[&](uint32_t op,void* self=nullptr,uintptr_t a=0,uintptr_t b=0,uintptr_t c=0) {return dispatch(op,self,a,b,c);};
    s->shadows=uint8_t(call(0x58AAD0,nullptr,0xFF79E4,1));
    s->highRez=uint8_t(call(0x58AAD0,nullptr,0xFCBFF0,0));
    s->debug=int32_t(call(0x58ABA0,nullptr,0xFCC054,0))>0;
    s->minspec=uint8_t(call(0x58AAD0,nullptr,0xFF79B0,0));
    auto instancing=call(0x58ABA0,nullptr,0xFF7988,0);
    s->originalShadows=s->shadows;
    s->disableInstancing=int32_t(instancing)!=0;
    s->capability=uint8_t(call(0x523860,nullptr,0x8000));
    if(int32_t(call(0x58ABA0,nullptr,0xFF7968,0))) call(0xFFFFFF02,nullptr,0x115903C,81);
    void* params=reinterpret_cast<void*>(call(0xFFFFFF00,nullptr,0x11D7530));
    void* shaders=reinterpret_cast<void*>(call(0xFFFFFF00,nullptr,0x11D753C));
    auto create=[&](uint32_t size,uint32_t label,uint32_t ctor,bool aligned=true) -> void* {
        auto p=call(aligned?0x4E3FB0:0x4E3F90,nullptr,size,label);
        return p?reinterpret_cast<void*>(call(ctor,reinterpret_cast<void*>(p))):nullptr;
    };
    auto ref=[&](void* p){call(0xFFFFFF01,p);};
    auto vf=[&](void* p,uint32_t data,uint32_t count){return uint8_t(call(0x61A010,p,data,count));};
    auto& r=s->resources;
    r[1]=create(36,0xFF7944,0x61A300);
    if(!r[1]) return 0;
    ref(r[1]);
    if(!vf(r[1],0xFF76E8,1)) return 0;
    r[0]=create(80,0xFF791C,0x6CFCA0);
    if(!r[0] || !uint8_t(call(0x6CF470,r[0],reinterpret_cast<uintptr_t>(r[1]),0x10000,0x40000))) return 0;
    r[3]=create(36,0xFF78F4,0x61A300);
    if(!r[3]) return 0;
    ref(r[3]);
    if(!vf(r[3],0xFF76F0,2)) return 0;
    r[2]=create(80,0xFF78C8,0x6CFCA0);
    if(!r[2] || !uint8_t(call(0x6CF470,r[2],reinterpret_cast<uintptr_t>(r[3]),0x4000,0x10000))) return 0;
    r[4]=create(32,0xFF78A4,0x61A380);
    if(!r[4]) return 0;
    ref(r[4]);
    if(!uint8_t(call(0x61A1C0,r[4],0x10000,1,0xFF7894))) return 0;
    for(unsigned i=7;i<10;++i) {
        r[i]=create(36,0xFF7874,0x61A300);
        if(!r[i]) return 0;
        ref(r[i]);
    }
    if(!vf(r[7],0xFF76B0,2) || !vf(r[8],0xFF76C0,2) || !vf(r[9],0xFF76D0,3)) return 0;
    auto technique=[&](unsigned slot,unsigned kind){s->techniques[slot]=reinterpret_cast<void*>(call(0x79B840,shaders,0xFB8A04,kind,1));};
    technique(2,1); technique(0,2); technique(1,6); technique(3,3);
    auto hash=[&](unsigned word,uint32_t id,unsigned type){s->parameters[word-5]=reinterpret_cast<void*>(call(0x79A130,params,id,1,type));};
    auto name=[&](unsigned word,uint32_t id,unsigned count,unsigned type){s->parameters[word-5]=reinterpret_cast<void*>(call(0x79A160,params,id,count,type));};
    hash(5,1207485594u,6); hash(6,1991880542u,5);
    name(7,0xFF2F0C,1,2); name(49,0xFF785C,1,2);
    hash(23,492251788u,6); hash(24,uint32_t(-484786816),5);
    hash(25,1247237339u,6); hash(26,1342944353u,5);
    hash(47,uint32_t(-956193850),5); hash(48,uint32_t(-197035491),5);
    if(uint8_t(call(0x6E9510))) {
        hash(35,516715872u,6); hash(36,1793388708u,5);
        hash(37,917653439u,6); hash(39,1368046289u,5);
        hash(38,917653438u,6); hash(40,1368046288u,5);
        hash(41,uint32_t(-266401871),6); hash(42,uint32_t(-2129305506),6);
    } else {
        hash(27,uint32_t(-63579009),6); hash(29,1354439057u,5);
        hash(31,2129578378u,6); hash(33,852228326u,5);
    }
    hash(43,662485542u,2);
    hash(8,1738382669u,6); hash(9,1975351315u,5);
    hash(10,uint32_t(-1269698946),6); hash(11,uint32_t(-1140053774),5);
    hash(12,223655811u,6); hash(13,223652975u,6); hash(14,223569688u,5);
    if(s->capability) {
        hash(15,697572883u,6); hash(16,1584606201u,5);
        hash(17,1784620324u,6); hash(18,435537592u,5);
        hash(19,uint32_t(-211326148),6); hash(20,735463696u,5);
        hash(21,290389438u,6); hash(22,1860377906u,5);
    }
    hash(44,uint32_t(-177601888),5); hash(45,uint32_t(-816017598),6);
    hash(46,1950398374u,5); hash(50,1665037026u,5);
    name(51,0xFF7840,1,2); name(54,0xFF7828,4,2);
    name(52,0xFF7810,4,2); name(53,0xFF77F8,4,2);
    name(55,0xFF77E0,4,2); name(56,0xFF77C8,4,2);
    name(57,0xFF77B0,4,2); name(58,0xFF2278,4,2); name(59,0xFF2260,4,2);
    auto publish=[&](unsigned size,uint32_t label,uint32_t ctor,uint32_t setter) {
        call(setter,nullptr,reinterpret_cast<uintptr_t>(create(size,label,ctor,false)));
    };
    publish(80,0xFF778C,0x6CBD00,0x6CB850);
    publish(4520,0xFF7770,0x703510,0x6FDE20);
    publish(44,0xFF7754,0x6B1AA0,0x6B05B0);
    publish(4,0xFF7734,0x6DE9C0,0x6DE480);
    void* host=reinterpret_cast<void*>(call(0x60E2C0));
    call(0x610500,host,reinterpret_cast<uintptr_t>(s));
    s->initialized=1;
    call(0x610500,host,reinterpret_cast<uintptr_t>(r[2]));
    call(0x610500,host,reinterpret_cast<uintptr_t>(r[0]));
    ref(s);
    void* events=reinterpret_cast<void*>(call(0x44C470));
    call(0xFFFFFF03,events,reinterpret_cast<uintptr_t>(&s->secondary),0xFC6AA1FE);
    call(0x6935A0,s->helper);
    return 1;
}
}
uint8_t ts3::render::m07::M07::InitializeGame() {return m07_init::Initialize(this);}
